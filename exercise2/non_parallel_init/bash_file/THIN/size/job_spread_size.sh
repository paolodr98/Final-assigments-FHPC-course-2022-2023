#!/bin/bash 
#SBATCH --partition=THIN 
#SBATCH --job-name=gemm
#SBATCH --nodes=1 
#SBATCH --ntasks-per-node=1 
#SBATCH --cpus-per-task=24
#SBATCH --mem=200gb 
#SBATCH --time=02:00:00 
#SBATCH --exclusive 
#SBATCH --output=size_spread.out


module load openBLAS/0.3.26-omp


export LD_LIBRARY_PATH=/u/dssc/pdarol00/myblis/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/u/dssc/pdarol00/intel/oneapi/mkl/2024.2/lib:$LD_LIBRARY_PATH

export OMP_NUM_THREADS=12
export BLIS_NUM_THREADS=12

export OMP_PLACES=cores
export OMP_PROC_BIND=spread

make_dir="/u/dssc/pdarol00/Final-assigments-FHPC-course-2022-2023/exercise2/non_parallel_init"
output_dir="/u/dssc/pdarol00/Final-assigments-FHPC-course-2022-2023/exercise2/non_parallel_init/batch_file/THIN/size/output_spread_size"

srun $make_dir make clean
srun -n1 make -C $make_dir cpu # Now I have all the needed executables.


for implem in 'oblas' 'mkl' 'blis'
do
    for type in 'double' 'float'
    do
        filename="$output_dir/$implem"_"$type".csv
        # Check if the file exists, if not create it and write the header
        if [ ! -e $filename ]; then
            echo "n_threads,Time,GFLOPS" > $filename
        fi


        for m_size in {2000..20000..1000}
        do
            # Run everything with openBLAS double
            for j in 1 2 3 4 5 # Take multiple measurements
            do
                srun -n1 --cpus-per-task=12 $make_dir/gemm_"$implem"_"$type".x $m_size $m_size $m_size > output_spread.txt #just a temporary file
                # Extract information using grep and regular expressions
                size=$m_size
                times=$(grep -o 'Time: [0-9.]*' output_spread.txt| cut -d' ' -f2)
                gflops=$(grep -o 'GFLOPS: [0-9.]*' output_spread.txt| cut -d' ' -f2)

                echo "$size,$times,$gflops" >> $filename
            done
        done
    done
done
rm output_spread.txt # Delete the temporary file
