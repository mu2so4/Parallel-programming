#!/bin/bash

#PBS -l walltime=00:03:00
#PBS -l select=1:ncpus=1:mpiprocs=1:mem=24000m,place=scatter
#PBS -m n


cd $PBS_O_WORKDIR

MPI_NP=$(wc -l $PBS_NODEFILE | awk '{ print $1 }')
echo "Number of MPI processes: $MPI_NP"


echo 'File $PBS_NODEFILE:'
cat  $PBS_NODEFILE
HEIGHT=480
WIDTH=470
REPORT_FILE=report-test.txt
CHECK_FILE=check-test.txt
FORMAT="$HEIGHT\t$WIDTH\t$MPI_NP\t"

echo -ne $FORMAT >>$REPORT_FILE
echo -ne $FORMAT >>$CHECK_FILE
mpirun -machinefile $PBS_NODEFILE -np $MPI_NP ./life $HEIGHT $WIDTH >>$CHECK_FILE 2>>$REPORT_FILE
