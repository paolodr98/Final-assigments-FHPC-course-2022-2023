#include "read_write.h"
#include "initialize.h"
// #include <mpi.h>
#include <omp.h>
#include <mpi.h> 



void update_cell(){

}


void static_ev(char *filename, int rank, int size, int k, int rows_read){

    // read the image from master processor
    if(rank==0){
        unsigned char *completeMatrix;
        int maxval = 255;

		//Initialize the matrix by reading the pgm file where it's stored
		read_pgm_image((void**)&completeMatrix, &maxval, &k, &k, filename);

        for (int i = 0; i< k*k; i++){
        printf("%d\n", completeMatrix[i]);
        }
  	}


    

    // Gathering all sizes and displacements in master processor
    int* list_rows_proc = NULL;  // To store the number of elements received from each processor
    
	if (rank == 0) {
        list_rows_proc = (int*) malloc(size * sizeof(int));
    }

	// Gather the sizes of each local array to the master processor
	MPI_Gather(&rows_read, 1, MPI_INT, list_rows_proc, 1, MPI_INT, 0, MPI_COMM_WORLD);

    /* check the number of rows for each processor
    if (rank == 0){
        for(int i = 0; i< size; i++){
            printf("%d\n", list_rows_proc[i]);
        }
    }
    */
    
    // allocate memory on each processor
    unsigned char* local_array = (unsigned char*)malloc((rows_read+2) * k * sizeof(unsigned char));

    /*
    // spread the pieces of array into the processors
    if (rank == 0){

        for (int i = 0; i < k; i++){
            local_array[i] = completeMatrix[(k-1)*k+i];
        }
        for (int i = 0; i < (rows_read+1)*k; i++){
            local_array[k+i] = completeMatrix[i];
        }

        for(int i = 0; i< (rows_read+2)*k; i++){
            printf("%d\n", local_array[i]);
        }


    }
    */

    // start the evolution for t steps

    // if needed gather the data in the master processor to write a .pgm

    // write the last step in a .pgm file

    // free the memory
    


}