#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <omp.h>
#include <mpi.h>
#include "read_write.h"

void initialize_parallel(int k, char *fname, int rank, int size, int maxval){

	//_____________define variables_____________ 
	unsigned char *local_grid;
	unsigned char* gathered_grid = NULL;

	int rows_read = k / size; 
    rows_read = (rank < k % size) ? rows_read+1 : rows_read;

	//_____________allocate memories on each processor depending on the number of rows_____________
	local_grid= (unsigned char*)malloc(rows_read*k*sizeof(unsigned char) );

	srand(time(NULL)+rank);

	int  half=  maxval/2;
	unsigned char minval= (unsigned char)0;
	unsigned char _maxval= (unsigned char)maxval;

	//_____________generate each sub-array on each processor_____________
	#pragma omp parallel for
	for(long i=0; i<rows_read*k; i++){
        	int random_number = (rand() % (maxval+1));
        	local_grid[i]= (random_number > half)?_maxval:minval;
			printf("%d\n", (random_number > half)?_maxval:minval );
        }
	printf("\n");
	printf("%d rows for processor, %d, of n %d\n", rows_read, rank, size);

	//_____________GATHER THE DATA IN THE ROOT PROCESSOR_____________
	if (rank == 0) {
        	gathered_grid = (unsigned char*)malloc(k * k*sizeof(unsigned char));
	}

	// Calculate sendcounts and displacements
	int sendcounts[size];
	int displacements[size];
	int total_rows = 0;

	#pragma omp parallel for
    	for (long i = 0; i < size; i++) {
			sendcounts[i] = rows_read * k;
        	displacements[i] = total_rows * k;
        	total_rows += rows_read;
    	}

        
	MPI_Gatherv(local_grid, rows_read * k, MPI_UNSIGNED_CHAR, gathered_grid, sendcounts, displacements, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
   	MPI_Barrier(MPI_COMM_WORLD);
	free(local_grid);

	//DEBUG
	if (rank == 0) {
		for (int i = 0; i< k*k; i++){
        	printf("%d element: %d\n", i, gathered_grid[i]);
        }
		printf("\n");

		write_pgm_image((void *) gathered_grid, maxval, k, k, fname);
	    free(gathered_grid);
    }

	return;
}


void initialize_serial(int k, char *fname, int maxval){

	unsigned char* gathered_grid = NULL;
	gathered_grid = (unsigned char*)malloc(k * k*sizeof(unsigned char));

	srand(time(NULL));

	int  half=  maxval/2;
	unsigned char minval= (unsigned char)0;
	unsigned char _maxval= (unsigned char)maxval;

	#pragma omp parallel for
	for(long i=0; i<k*k; i++){
        	int random_number = (rand() % (maxval+1));
        	gathered_grid[i]= (random_number > half)?_maxval:minval;
			
        }
	write_pgm_image((void *) gathered_grid, maxval, k, k, fname);
	free(gathered_grid);

}



/*
void initialize(char *fname, int maxval, int k){

    unsigned char *grid; 
    unsigned char random; 


    grid= (unsigned char*)malloc(k*k*sizeof(unsigned char) );

    // fill dynamic array in shared parallel
	#pragma omp parallel
	{
		// set seed
		int myid = omp_get_thread_num();
		unsigned int my_seed = myid*myid + 2*myid + 3;

		#pragma omp for schedule(static, xsize)
		for (int i=0; i<k*k; i++)
		{
			random = (unsigned char)(rand_r(&my_seed)%2); //generate random number, considers it modulo 2, cast it in char; 
			grid[i] = random;
		}
	}
	for (int i=0; i<k*k; i++){
		printf("%d\n", grid[i]);
	}

    // write to pgm image
	printf("writing image to %s\n", fname);
    write_pgm_image((void *) grid, maxval, k, fname);
	
	free(grid);
    return;
}
*/
