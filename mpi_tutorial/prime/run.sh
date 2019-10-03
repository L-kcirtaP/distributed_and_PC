#!/bin/bash
if [ -f benchmark.txt ]; then
    rm benchmark.txt
fi

for ((n=1;n<=8;n++))
do
    echo Running with $n processes
    mpirun -f /home/mpi_config -n $n ./mpi_prime >> benchmark.txt
done    

grep -E "Using|Wall" benchmark.txt
