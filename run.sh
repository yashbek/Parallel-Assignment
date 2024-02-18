#!/bin/bash

if [ -z "$(ls -A bin)" ]; then
    echo "Run the build.sh script to compile the files"
fi

p=10

if [ $# -eq 1 ]; then
    p=$1
fi

echo "Using $p processes:"

./bin/sequ
mpirun --oversubscribe -np "$p" --allow-run-as-root bin/static
mpirun --oversubscribe -np "$p" --allow-run-as-root bin/dynamic
