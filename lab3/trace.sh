#!/bin/bash

#PBS -l walltime=00:02:00
#PBS -l select=1:ncpus=8:mpiprocs=8:mem=2000m,place=scatter
#PBS -m n


cd $PBS_O_WORKDIR

MPI_NP=$(wc -l $PBS_NODEFILE | awk '{ print $1 }')
echo "Number of MPI processes: $MPI_NP"

N=3000
K=3600
M=4200

echo 'File $PBS_NODEFILE:'
cat  $PBS_NODEFILE
ROWS=4
COLUMNS=2
echo "Count of rows: $ROWS"
echo "Count of columns: $COLUMNS"

mpirun -trace -machinefile $PBS_NODEFILE -np $MPI_NP ./parallel \
		$N $K $M $ROWS $COLUMNS >/dev/null
