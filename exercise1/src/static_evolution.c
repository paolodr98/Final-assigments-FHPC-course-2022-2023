#include "read_write.h"
#include "initialize.h"
// #include <mpi.h>
#include <omp.h>
#include <mpi.h> 



void update_cell(){

}


void static_ev(char *filename, int rank, int size, int k, int maxval){

    int rows_read = k / size; 
    rows_read = (rank < k % size) ? rows_read+1 : rows_read;
    unsigned char *local_array = NULL;
    unsigned char *completeMatrix = NULL;

    // _____________read the image in root processor_____________
    if(rank==0){
        completeMatrix = (unsigned char*)malloc(k*k *sizeof(unsigned char));
		read_pgm_image((void**)&completeMatrix, &maxval, &k, &k, filename); //Initialize the matrix by reading the pgm file where it's stored
        /* DEBUG
        for (int i = 0; i< k*k; i++){
        printf("%d element: %d\n",i, completeMatrix[i]);
        }
        */  
  	}

    // _____________Gathering all sizes and displacements in master processor_____________
    int* list_rows_proc = NULL;  // To store the number of rows received from each processor
	if (rank == 0) {
        list_rows_proc = (int*) malloc(size * sizeof(int));
    }
	MPI_Gather(&rows_read, 1, MPI_INT, list_rows_proc, 1, MPI_INT, 0, MPI_COMM_WORLD); // Gather the sizes of each local array to the master processor

    /* DEBUG: check the number of rows for each processor
    if (rank == 0){
        for(int i = 0; i< size; i++){
            printf("%d\n", list_rows_proc[i]);
        }
    }
    */
    
    // _____________spread the pieces of array into the processors_____________
    local_array = (unsigned char*)malloc((rows_read+2) * k * sizeof(unsigned char)); // allocate memory on each processor
    int displ = 0; // Displacement for scattering
    if (rank == 0) {
        int count_row = 0;
        // Processor 0 receives the last row + its rows_read rows + 1 row

        for (int i = 0; i < k; i++) {
            local_array[i] = completeMatrix[(k - 1) * k + i]; // Last row
        }
        for (int i = 0; i < (rows_read + 1) * k; i++) {
            local_array[k + i] = completeMatrix[i]; // rows_read + 1 rows
        }
        count_row = rows_read;

        // Send to other processors
        for (int p = 1; p < size - 1; p++) {
            displ = (count_row - 1) * k; // Start from 1 row back
            MPI_Send(completeMatrix + displ, (list_rows_proc[p] + 2) * k, MPI_UNSIGNED_CHAR, p, 0, MPI_COMM_WORLD);
            count_row += list_rows_proc[p];
        }

        // Last processor
        displ = (count_row-1) * k; // 2 rows back for the last processor
        MPI_Send(completeMatrix + displ, (list_rows_proc[size-1]+1) * k, MPI_UNSIGNED_CHAR, size - 1, 0, MPI_COMM_WORLD);
        MPI_Send(completeMatrix, k, MPI_UNSIGNED_CHAR, size - 1, 1, MPI_COMM_WORLD); // Send first row

    } else if (rank == size - 1) {
        // Last processor receives (rows_read + 1) + first row
        MPI_Recv(local_array, (rows_read+1) * k, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(local_array + (rows_read+1) * k, k, MPI_UNSIGNED_CHAR, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    } else {
        // Intermediate processors receive rows_read + 2 rows
        MPI_Recv(local_array, (rows_read + 2) * k, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    /* DEBUG
    printf("PROCESSOR %d\n", rank);
    for (int i = 0; i<(rows_read+2) * k; i++){
        printf("element %d of local array is %d\n", i, local_array[i]);
    }
    */

    // _____________start the evolution for t steps_____________
    for(int i=1; i <=t; i++){





    }

    // if needed gather the data in the master processor to write a .pgm

    // write the last step in a .pgm file

    // free the memory
    


}