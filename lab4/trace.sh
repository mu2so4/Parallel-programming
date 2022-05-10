#!/bin/bash

#PBS -l walltime=00:02:00
#PBS -l select=2:ncpus=8:mpiprocs=8:mem=24000m,place=scatter
#PBS -m n


cd $PBS_O_WORKDIR

MPI_NP=$(wc -l $PBS_NODEFILE | awk '{ print $1 }')
echo "Number of MPI processes: $MPI_NP"


echo 'File $PBS_NODEFILE:'
cat  $PBS_NODEFILE
HEIGHT=480
WIDTH=470
REPORT_FILE=report-trace.txt
CHECK_FILE=check-trace.txt

mpirun -trace -machinefile $PBS_NODEFILE -np $MPI_NP ./life $HEIGHT $WIDTH >>$CHECK_FILE 2>>$REPORT_FILE
