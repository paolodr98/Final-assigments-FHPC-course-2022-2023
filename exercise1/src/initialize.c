#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <omp.h>
#include <mpi.h>
#include "read_write.h"

void initialize_parallel(int k, char *fname, int rank, int size, int maxval){

	//_____________define variables_____________ 
	unsigned char *local_grid = NULL;
	unsigned char* gathered_grid = NULL;

	int rows_read = k / size; 
	int extra_rows = k%size;
    rows_read = (rank < k % size) ? rows_read+1 : rows_read;

	//_____________allocate memories on each processor depending on the number of rows_____________
	local_grid= (unsigned char*)malloc(rows_read*k *sizeof(unsigned char) );

	srand(time(NULL)+rank);

	int  half=  maxval/2;
	unsigned char minval= (unsigned char)0;
	unsigned char _maxval= (unsigned char)maxval;

	//_____________generate each sub-array on each processor_____________
	#pragma omp parallel for
	for(long i=0; i<rows_read*k; i++){
        	int random_number = (rand() % (maxval+1));
        	local_grid[i]= (random_number > half)?_maxval:minval;
			//printf("%d\n", (random_number > half)?_maxval:minval );
        }

	// DEBUG
	/*
	printf("\n");
	printf("%d rows for processor, %d, of n %d\n", rows_read, rank, size);
	*/

	//_____________GATHER THE DATA IN THE ROOT PROCESSOR_____________
	
	int* list_rows_proc = NULL;
	if (rank == 0) {
        	gathered_grid = (unsigned char*)malloc(k*k *sizeof(unsigned char));
			list_rows_proc = (int*) malloc(size * sizeof(int));

	}
	MPI_Gather(&rows_read, 1, MPI_INT, list_rows_proc, 1, MPI_INT, 0, MPI_COMM_WORLD); // Gather the sizes of each local array to the master processor
	
	
		// Calculate sendcounts and displacements
		int sendcounts[size];
		int displacements[size];
		int total_rows = 0;

	if (rank == 0){
		for (long i = 0; i < size; i++) {
			sendcounts[i] = list_rows_proc[i] * k;
			displacements[i] = total_rows * k;
			total_rows += list_rows_proc[i];
			//printf("Rank: %d, sendcounts: %d, displacement: %d \n",i, sendcounts[i], displacements[i]);
		}
	}

	MPI_Bcast(sendcounts, size, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(displacements, size, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);

	
	MPI_Gatherv(local_grid, rows_read * k, MPI_UNSIGNED_CHAR, gathered_grid, sendcounts, displacements, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
   	MPI_Barrier(MPI_COMM_WORLD);
	
	free(local_grid);

	if (rank == 0) {

		//DEBUG
		/*
		for (int i = 0; i< k*k; i++){
        	printf("%d element: %d\n", i, gathered_grid[i]);
        }
		printf("\n");
		*/
		char file_path[45] = "images/";
		strcat(file_path, fname);
		write_pgm_image((void *) gathered_grid, maxval, k, k, file_path);
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
	char file_path[45] = "images/";
	strcat(file_path, fname);
	
	write_pgm_image((void *) gathered_grid, maxval, k, k, file_path);
	free(gathered_grid);

}


