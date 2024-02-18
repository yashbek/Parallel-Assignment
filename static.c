#include <mpi.h>
#include <stdio.h>
#include <time.h>

#define WIDTH 640
#define HEIGHT 480
#define MAX_ITER 255

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
    double AVG = 0;
    int N = 1;
    int res[HEIGHT][WIDTH] = {0};
    double total_time;
    struct complex c;
    clock_t start_time;

    MPI_Init(NULL, NULL);

    int p;
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int PHeight = HEIGHT/p;

    if(rank == 0){
      start_time = clock();
    }
      for (int i = 0; i < PHeight; i++) {
        for (int j = 0; j < WIDTH; j++) {
          c.real = (j - (WIDTH / 2.0)) * 4.0 / WIDTH;
          c.imag = ((rank * PHeight) + i - (HEIGHT / 2.0)) * 4.0 / HEIGHT;
          image[rank * PHeight + i][j] = cal_pixel(c);
        }
      }
    MPI_Reduce(image, res, HEIGHT * WIDTH, MPI_INT, MPI_BOR, 0, MPI_COMM_WORLD);
    if(rank == 0){
      clock_t end_time = clock(); // End measuring time
      total_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
      save_pgm("../stat_mandelbrot.pgm", res);
      printf("The execution time of the static trial is: %f ms\n", total_time/N*1000);
    }
    MPI_Finalize();
    return 0;
}
