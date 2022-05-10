#!/bin/bash

#PBS -l walltime=00:50:00
#PBS -l select=2:ncpus=12:mpiprocs=12:mem=24000m,place=scatter
#PBS -m n


cd $PBS_O_WORKDIR

MPI_NP_GOT=$(wc -l $PBS_NODEFILE | awk '{ print $1 }')
echo "Number of MPI processes: $MPI_NP_GOT"


echo 'File $PBS_NODEFILE:'
cat  $PBS_NODEFILE
HEIGHT=480
WIDTH=470
REPORT_FILE=report.txt
CHECK_FILE=check.txt

for(( MPI_NP = 1; $MPI_NP <= $MPI_NP_GOT; MPI_NP++ )); do
	for(( ITER = 0; $ITER < 4; ITER++ )); do
		FORMAT="$HEIGHT\t$WIDTH\t$MPI_NP\t"
		echo -ne "$FORMAT" >>$REPORT_FILE
		echo -ne "$FORMAT" >>$CHECK_FILE
		mpirun -machinefile $PBS_NODEFILE -np $MPI_NP ./life $HEIGHT $WIDTH >>$CHECK_FILE 2>>$REPORT_FILE
	done
done
