rm -rv bin > /dev/null 2> /dev/null
mkdir bin
mpicc -o bin/sequ seq.c
mpicc -o bin/static static.c
mpicc -o bin/dynamic dynamic.c
