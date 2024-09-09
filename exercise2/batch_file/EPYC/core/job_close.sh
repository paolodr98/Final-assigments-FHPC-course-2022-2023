#!/bin/bash 
#SBATCH --partition=EPYC 
#SBATCH --job-name=gemm_first_attempt
#SBATCH --nodes=1 
#SBATCH --ntasks-per-node=1 
#SBATCH --cpus-per-task=128
#SBATCH --mem=200gb 
#SBATCH --time=02:00:00 
#SBATCH --exclusive
#SBATCH --output=cores_close.out


module load openBLAS/0.3.26

export LD_LIBRARY_PATH=/u/dssc/pdarol00/myblis/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/u/dssc/pdarol00/intel/oneapi/mkl/2024.2/lib:$LD_LIBRARY_PATH

export OMP_PLACES=cores
export OMP_PROC_BIND=close

make_dir="/u/dssc/pdarol00/Final-assigments-FHPC-course-2022-2023/exercise2"
output_dir="/u/dssc/pdarol00/Final-assigments-FHPC-course-2022-2023/exercise2/batch_file/EPYC/core/output_close"

srun -n1 make -C $make_dir cpu # Now I have all the needed executables.

m_size=10000


for implem in 'oblas' 'mkl' 'blis'
do
    for type in 'double' 'float'
    do
        for n_threads in 1 {2..128..2}
        do
            export OMP_NUM_THREADS=$n_threads
            export BLIS_NUM_THREADS=$n_threads 

            # Run everything with openBLAS double
            for j in 1 2 3 4 5 # Take multiple measurements
            do
                srun -n1 --cpus-per-task=$n_threads ./gemm_"$implem"_"$type".x $m_size $m_size $m_size > output_close.txt #just a temporary file
                
                # Extract information using grep and regular expressions
                times=$(grep -o 'Time: [0-9.]*' output_close.txt| cut -d' ' -f2)
                gflops=$(grep -o 'GFLOPS: [0-9.]*' output_close.txt| cut -d' ' -f2)
                # Store the extracted information in a CSV file
                filename="$output_dir/$implem"_"$type"_$n_threads.csv

                if [ ! -e $filename ]; then
                echo "n_threads,Time,GFLOPS" > $filename
                fi
                echo "$n_threads,$times,$gflops" >> $filename
            done
        done
    done
done
rm output_close.txt # Delete the temporary file