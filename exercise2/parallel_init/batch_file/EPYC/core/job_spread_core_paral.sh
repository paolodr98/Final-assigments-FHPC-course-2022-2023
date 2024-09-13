#!/bin/bash 
#SBATCH --partition=EPYC 
#SBATCH --job-name=gemm_first_attempt
#SBATCH --nodes=1 
#SBATCH --ntasks-per-node=1 
#SBATCH --cpus-per-task=128
#SBATCH --mem=200gb 
#SBATCH --time=00:40:00 
#SBATCH --exclusive
#SBATCH --output=cores_spread.out

module load openBLAS/0.3.26-omp

export LD_LIBRARY_PATH=/u/dssc/pdarol00/myblis/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/u/dssc/pdarol00/intel/oneapi/mkl/2024.2/lib:$LD_LIBRARY_PATH

export OMP_PLACES=cores
export OMP_PROC_BIND=spread

make_dir="/u/dssc/pdarol00/Final-assigments-FHPC-course-2022-2023/exercise2/parallel_init"
output_dir="/u/dssc/pdarol00/Final-assigments-FHPC-course-2022-2023/exercise2/parallel_init/batch_file/EPYC/core/output_spread_paral"

srun -n1 make -C $make_dir cpu # Now I have all the needed executables.

m_size=10000

# Loop over implementations and data types
for implem in 'blis' # 'oblas' 'mkl' 
do
    for type in 'double' # 'float' 
    do
        filename="$output_dir/$implem"_"$type".csv
        # Check if the file exists, if not create it and write the header
        if [ ! -e $filename ]; then
            echo "n_threads,Time,GFLOPS" > $filename
        fi

        for n_threads in 1 {2..128..2}
        do
            export OMP_NUM_THREADS=$n_threads
            export BLIS_NUM_THREADS=$n_threads
            #export OPENBLAS_NUM_THREADS=$n_threads
            #export MKL_NUM_THREADS=$n_threads

            for j in 1 2 3 4 5 # Take multiple measurements
            do
                srun -n1 --cpus-per-task=$n_threads $make_dir/gemm_"$implem"_"$type".x $m_size $m_size $m_size > output_spread.txt # Just a temporary file
                
                # Extract information using grep and regular expressions
                times=$(grep -o 'Time: [0-9.]*' output_spread.txt | cut -d' ' -f2)
                gflops=$(grep -o 'GFLOPS: [0-9.]*' output_spread.txt | cut -d' ' -f2)
                
                # Append the thread information to the existing CSV file
                echo "$n_threads,$times,$gflops" >> $filename
            done
        done
    done
done
rm output_spread.txt # Delete the temporary file