#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <omp.h>
#include <mpi.h>
#include "read_write.h"

void initialize_parallel(int k, char *fname, int rank, int size, int rows_read){

	/* define variables */
	unsigned char *local_grid;
	unsigned char random;

	/* allocate memories on each processor depending on the number of rows*/
	local_grid= (unsigned char*)malloc(rows_read*k*sizeof(unsigned char) );

	/* generate each sub-array on each processor */
	#pragma omp parallel
	{
		// set seed
		int myid = omp_get_thread_num();
		unsigned int my_seed = myid*myid + 2*myid + 3;

		#pragma omp for schedule(static, rows_read)
		for (int i=0; i<k*rows_read; i++)
		{
			random = (unsigned char)(rand_r(&my_seed)%2); //generate random number, considers it modulo 2, cast it in char; 
			local_grid[i] = random;
		}
	}
	printf("%d rows for processor, %d, of n %d\n", rows_read, rank, size);

	/* gather the data in the master processor */

	// Now the root processor needs to gather all sizes and displacements

    int* recv_counts = NULL;  // To store the number of elements received from each processor
    int* displs = NULL;       // To store the displacements where each segment will go in the final array

	if (rank == 0) {
        recv_counts = (int*) malloc(size * sizeof(int));
        displs = (int*) malloc(size * sizeof(int));
    }

	// Gather the sizes of each local array to the root processor
	MPI_Gather(&rows_read, 1, MPI_INT, recv_counts, 1, MPI_INT, 0, MPI_COMM_WORLD);

	char* gathered_grid = NULL;
    if (rank == 0) {
        int total_size = 0;
        displs[0] = 0;
        total_size += recv_counts[0];

        for (int i = 1; i < size; i++) {
            displs[i] = displs[i - 1] + recv_counts[i - 1];  // Calculate the displacement
            total_size += recv_counts[i];                    // Calculate total size
        }

        gathered_grid = (char*) malloc(k*k * sizeof(char));  // Allocate memory for gathered array
    }

    // Use MPI_Gatherv to gather the arrays of variable sizes
    MPI_Gatherv(local_grid, rows_read, MPI_CHAR,
                gathered_grid, recv_counts, displs, MPI_CHAR,
                0, MPI_COMM_WORLD);



	// the master writes the array in a .pgm file 
	if (rank == 0) {
		int maxval = 255;

        // Call the function to write the data to a .pgm file
        write_pgm_image((void *) gathered_grid, maxval, k, fname);

        // Free allocated memory
        free(recv_counts);
        free(displs);
        free(gathered_grid);
    }

	// free memory
	free(local_grid);
	return;
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
