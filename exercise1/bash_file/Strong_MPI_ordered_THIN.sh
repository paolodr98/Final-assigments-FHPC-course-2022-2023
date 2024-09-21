#!/bin/bash 
#SBATCH --partition=THIN 
#SBATCH --job-name=MPI_Strong
#SBATCH --nodes=4
#SBATCH --ntasks-per-node=2 
#SBATCH --cpus-per-task=12
#SBATCH --mem=200gb 
#SBATCH --time=01:00:00 
#SBATCH --exclusive
#SBATCH --output=MPI_Strong_THIN_ordered.out

module load openMPI/4.1.6/gnu/14.2.1

cd ../

#make_dir="/u/dssc/pdarol00/Final-assigments-FHPC-course-2022-2023/exercise1"
output_dir="/u/dssc/pdarol00/Final-assigments-FHPC-course-2022-2023/exercise1/data/strong_MPI/ordered"

#srun $make_dir make clean
srun make  # Now I have all the needed executables

# Define MPI binding and OMP affinity
MAPBY=node
BINDTO=socket

export OMP_PLACES=cores
export OMP_PROC_BIND=close
export OMP_NUM_THREADS=12

e=0 #ordered
s=0
nsteps=100

filename="$output_dir/THIN_ordered"_"$mpi_task".csv
if [ ! -e $filename ]; then
    echo "ksize,mpi_task,time,thread" > $filename
fi  

for mpi_task in 5
do  
    for ksize in 10000
    do          
        mpirun -n $mpi_task --map-by $MAPBY --bind-to $BINDTO ./main.x -i -k $ksize # initialize the matrix
        for j in {1..5..1}
        do
            mpirun -n $mpi_task --map-by $MAPBY --bind-to $BINDTO ./main.x -r -k $ksize -e $e -n $nsteps -s $s  > output_THIN_ordered_MPI_strong.txt
            time_value=$(grep -o 'Time: [0-9.]*' output_THIN_ordered_MPI_strong.txt | cut -d' ' -f2)
            thread_value=$(grep -o 'Threads: [0-9.]*' output_THIN_ordered_MPI_strong.txt | cut -d' ' -f2)
            echo "$ksize,$mpi_task,$time_value,$thread_value" >> $filename
        done
    done
done

for mpi_task in {6..8..1}
do  
    for ksize in 100 500 1000 5000 10000
    do          
        mpirun -n $mpi_task --map-by $MAPBY --bind-to $BINDTO ./main.x -i -k $ksize # initialize the matrix
        for j in {1..5..1}
        do
            mpirun -n $mpi_task --map-by $MAPBY --bind-to $BINDTO ./main.x -r -k $ksize -e $e -n $nsteps -s $s  > output_THIN_ordered_MPI_strong.txt
            time_value=$(grep -o 'Time: [0-9.]*' output_THIN_ordered_MPI_strong.txt | cut -d' ' -f2)
            thread_value=$(grep -o 'Threads: [0-9.]*' output_THIN_ordered_MPI_strong.txt | cut -d' ' -f2)
            echo "$ksize,$mpi_task,$time_value,$thread_value" >> $filename
        done
    done
done


rm output_THIN_ordered_MPI_strong.txt # Remove useless temporary file
