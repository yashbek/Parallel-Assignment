#include <mpi.h>
#include <stdio.h>
#include <time.h>

#define WIDTH 640
#define HEIGHT 480
#define MAX_ITER 255

#define MPI_PERFORM_TAG 1
#define MPI_DONE_TAG 0
#define MPI_TERMINATE_TAG 2

struct complex{
  double real;
  double imag;
};


int cal_pixel(struct complex c) {
            double z_real = 0;
            double z_imag = 0;

            double z_real2, z_imag2, lengthsq;

            int iter = 0;
            do {
                z_real2 = z_real * z_real;
                z_imag2 = z_imag * z_imag;

                z_imag = 2 * z_real * z_imag + c.imag;
                z_real = z_real2 - z_imag2 + c.real;
                lengthsq =  z_real2 + z_imag2;
                iter++;
            }
            while ((iter < MAX_ITER) && (lengthsq < 4.0));
            return iter;
}

void save_pgm(const char *filename, int image[HEIGHT][WIDTH]) {
    FILE* pgmimg;
    int temp;
    pgmimg = fopen(filename, "wb");
    fprintf(pgmimg, "P2\n"); // Writing Magic Number to the File
    fprintf(pgmimg, "%d %d\n", WIDTH, HEIGHT);  // Writing Width and Height
    fprintf(pgmimg, "255\n");  // Writing the maximum gray value
    int count = 0;

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            temp = image[i][j];
            fprintf(pgmimg, "%d ", temp); // Writing the gray values in the 2D array to the file
        }
        fprintf(pgmimg, "\n");
    }
    fclose(pgmimg);
}

int main() {
    int image[HEIGHT][WIDTH] = {0};
    double total_time;
    struct complex c;
    clock_t start_time;

    MPI_Init(NULL, NULL);

    int p;
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(rank == 0){
      start_time = clock();
      int i;
      int master_row = 0;
      int worker_message[WIDTH];

      for (i = 0; i < p; i++){
        MPI_Send(&master_row, 1, MPI_INT, i, MPI_PERFORM_TAG, MPI_COMM_WORLD);
        master_row++;
      }

      do{
        MPI_Status status;
        MPI_Recv(worker_message, WIDTH + 1, MPI_INT, MPI_ANY_SOURCE, MPI_DONE_TAG, MPI_COMM_WORLD, &status);
        i--;
        for (int k = 0; k < WIDTH; k++){
          image[worker_message[WIDTH]][k] = worker_message[k];
        }

        if(master_row < HEIGHT){
          MPI_Send(&master_row, 1, MPI_INT, status.MPI_SOURCE, MPI_PERFORM_TAG, MPI_COMM_WORLD);
          master_row++;
          i++;
        }
        else
        {
          MPI_Send(&master_row, 1, MPI_INT, status.MPI_SOURCE, MPI_TERMINATE_TAG, MPI_COMM_WORLD);
        }
      }while(i > 1);

      clock_t end_time = clock(); // End measuring time
      total_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
      save_pgm("../dyn_mandelbrot.pgm", image);
      printf("The execution time of the dynamic trial is: %f ms\n", total_time*1000);
    }
    else
    {
      int my_slice;
      MPI_Status my_status;
      int my_row[WIDTH+1];

      while(1){
        MPI_Recv(&my_slice, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &my_status);
        if (my_status.MPI_TAG == MPI_PERFORM_TAG){
          my_row[WIDTH] = my_slice;
          for (int j = 0; j < WIDTH; j++) {
            c.real = (j - (WIDTH / 2.0)) * 4.0 / WIDTH;
            c.imag = (my_slice - (HEIGHT / 2.0)) * 4.0 / HEIGHT;
            my_row[j] = cal_pixel(c);
          }
        }
        else
        {
          break;
        }
        MPI_Send(my_row, WIDTH + 1, MPI_INT, 0, MPI_DONE_TAG, MPI_COMM_WORLD);
      }
    }

    MPI_Finalize();
    return 0;
}
