#!/bin/bash 
#SBATCH --partition=THIN 
#SBATCH --job-name=OMP_Init
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=2 
#SBATCH --cpus-per-task=12
#SBATCH --mem=200gb 
#SBATCH --time=02:00:00 
#SBATCH --exclusive
#SBATCH --output=OMP_Init_THIN_ordered.out

module load openMPI/4.1.6/gnu/14.2.1

cd ../

#make_dir="/u/dssc/pdarol00/Final-assigments-FHPC-course-2022-2023/exercise1"
output_dir="/u/dssc/pdarol00/Final-assigments-FHPC-course-2022-2023/exercise1/data/open_mp/ordered"

#srun $make_dir make clean
srun make  # Now I have all the needed executables

# Define MPI binding and OMP affinity
MAPBY=node
BINDTO=socket

export OMP_PLACES=cores
export OMP_PROC_BIND=close

e=0 #ordered
s=0
nsteps=100


for mpi_task in 2
do  
    filename="$output_dir/THIN_ordered"_"$mpi_task".csv
        if [ ! -e $filename ]; then
            echo "ksize,threads,time" > $filename
        fi  
    for ksize in 5000
    do        
        for n_threads in  {4..12..2} 
        do
            export OMP_NUM_THREADS=$n_threads
            mpirun -n $mpi_task --map-by $MAPBY --bind-to $BINDTO ./main.x -i -k $ksize # initialize the matrix
            for j in {1..5..1}
            do
                mpirun -n $mpi_task --map-by $MAPBY --bind-to $BINDTO ./main.x -r -k $ksize -e $e -n $nsteps -s $s  > output_THIN_ordered_openMP.txt
                time_value=$(grep -o 'Time: [0-9.]*' output_THIN_ordered_openMP.txt | cut -d' ' -f2)
                thread_value=$(grep -o 'Threads: [0-9.]*' output_THIN_ordered_openMP.txt | cut -d' ' -f2)
                echo "$ksize,$n_threads,$time_value" >> $filename
            done
        done
    done
done



for mpi_task in 2 
do  
    filename="$output_dir/THIN_ordered"_"$mpi_task".csv
        if [ ! -e $filename ]; then
            echo "ksize,threads,time" > $filename
        fi  
    for ksize in 10000
    do        
        for n_threads in 1 {2..12..2} 
        do
            export OMP_NUM_THREADS=$n_threads
            mpirun -n $mpi_task --map-by $MAPBY --bind-to $BINDTO ./main.x -i -k $ksize # initialize the matrix
            for j in {1..5..1}
            do
                mpirun -n $mpi_task --map-by $MAPBY --bind-to $BINDTO ./main.x -r -k $ksize -e $e -n $nsteps -s $s  > output_THIN_ordered_openMP.txt
                time_value=$(grep -o 'Time: [0-9.]*' output_THIN_ordered_openMP.txt | cut -d' ' -f2)
                thread_value=$(grep -o 'Threads: [0-9.]*' output_THIN_ordered_openMP.txt | cut -d' ' -f2)
                echo "$ksize,$n_threads,$time_value" >> $filename
            done
        done
    done
done


rm output_THIN_ordered_openMP.txt # Remove useless temporary file
