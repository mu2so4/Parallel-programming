#!/bin/bash

#PBS -l walltime=00:18:00
#PBS -l select=2:ncpus=12:mpiprocs=12:mem=2000m,place=scatter
#PBS -m n


cd $PBS_O_WORKDIR

MPI_NP=$(wc -l $PBS_NODEFILE | awk '{ print $1 }')
echo "Number of MPI processes: $MPI_NP"

N=3000
K=3600
M=4200

echo 'File $PBS_NODEFILE:'
cat  $PBS_NODEFILE
echo "Count of rows: $ROWS"
echo "Count of columns: $COLUMNS"
echo
echo

for (( i = 0; i < 3; i++ )); do
	ROWS=2
	COLUMNS=12
	echo -n -e "$ROWS\t$COLUMNS\t" >> report.txt
	mpirun -machinefile $PBS_NODEFILE -np $MPI_NP ./parallel \
			$N $K $M $ROWS $COLUMNS >/dev/null 2>>report.txt
	ROWS=3
	COLUMNS=8
	echo -n -e "$ROWS\t$COLUMNS\t" >> report.txt
	mpirun -machinefile $PBS_NODEFILE -np $MPI_NP ./parallel \
			$N $K $M $ROWS $COLUMNS >/dev/null 2>>report.txt
	ROWS=4
	COLUMNS=6
	echo -n -e "$ROWS\t$COLUMNS\t" >> report.txt
	mpirun -machinefile $PBS_NODEFILE -np $MPI_NP ./parallel \
			$N $K $M $ROWS $COLUMNS >/dev/null 2>>report.txt
	ROWS=6
	COLUMNS=4
	echo -n -e "$ROWS\t$COLUMNS\t" >> report.txt
	mpirun -machinefile $PBS_NODEFILE -np $MPI_NP ./parallel \
			$N $K $M $ROWS $COLUMNS >/dev/null 2>>report.txt
	ROWS=8
	COLUMNS=3
	echo -n -e "$ROWS\t$COLUMNS\t" >> report.txt
	mpirun -machinefile $PBS_NODEFILE -np $MPI_NP ./parallel \
			$N $K $M $ROWS $COLUMNS >/dev/null 2>>report.txt
	ROWS=12
	COLUMNS=2
	echo -n -e "$ROWS\t$COLUMNS\t" >> report.txt
	mpirun -machinefile $PBS_NODEFILE -np $MPI_NP ./parallel \
			$N $K $M $ROWS $COLUMNS >/dev/null 2>>report.txt
done
