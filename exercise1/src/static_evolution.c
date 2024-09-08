#include "read_write.h"
#include "initialize.h"
// #include <mpi.h>
#include <omp.h>
#include <mpi.h> 



void update_cell(unsigned char *local_array, unsigned char *new_local_array, int k, int rows_read, int rank){
    unsigned char max=255; //alive
	unsigned char min=0; //dead

    #pragma omp parallel for
    for (int i = k; i<(rows_read+1)*k; i++){
        int count = 0;
        int row, column;

        row = i/k;
        column = i%k;

        // Periodic boundary conditions for rows
        int row_above = row - 1;
        int row_below = row + 1;

        // Periodic boundary conditions for columns
        int col_left = (column == 0) ? k-1 : column - 1;
		int col_right = (column == (k-1)) ? 0 : column + 1;

        // Sum of neighbors (up, middle, down)
        int up = local_array[row_above * k + col_left] +
                 local_array[row_above * k + column] +
                 local_array[row_above * k + col_right];
                 
        int middle = local_array[row * k + col_left] +
                     local_array[row * k + col_right];

        int down = local_array[row_below * k + col_left] +
                   local_array[row_below * k + column] +
                   local_array[row_below * k + col_right];

        count= up + down + middle;

        new_local_array[i-k] = (count == 765 || count == 510) ? 255 : 0; 

        //DEBUG
        /*printf("Processor %d, element: %d, up: %d, middle: %d, down: %d, value array: %d\n",rank, i, up, middle, down, n_local_array[i-k]);*/
    }
}




void static_ev(char *filename, int rank, int size, int k, int maxval, int s, int t){

    int rows_read = k / size; 
    rows_read = (rank < k % size) ? rows_read+1 : rows_read;
    unsigned char *local_array = NULL;
    unsigned char *new_local_array = NULL;
    unsigned char *completeMatrix = NULL;

    int rank_above= (rank==0)? size-1 : rank-1;
  	int rank_below= (rank==size-1)? 0: rank+1;


    // _____________read the image in root processor_____________
    if(rank==0){
        completeMatrix = (unsigned char*)malloc(k*k *sizeof(unsigned char));
		read_pgm_image((void**)&completeMatrix, &maxval, &k, &k, filename); //Initialize the matrix by reading the pgm file where it's stored
        
        //DEBUG
        /*
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

  
    printf("PROCESSOR INIT %d\n", rank);
    for (int i = 0; i<(rows_read+2) * k; i++){
        printf("element %d of local array is %d\n", i, local_array[i]);
    }
    printf("\n");
   
    

    // _____________start the evolution for t steps_____________
    new_local_array = (unsigned char*)malloc((rows_read) * k * sizeof(unsigned char));
    for(int i=1; i <=t; i++){ 

        update_cell(local_array, new_local_array, k, rows_read, rank);

        //DEBUG
        
        
        printf("PROCESSOR NEW %d\n", rank);
        for (int i = 0; i<(rows_read) * k; i++){
        printf("element %d of NEW array is %d\n", i, new_local_array[i]);
        }
        printf("\n");

        unsigned char *temp = new_local_array;
        new_local_array = local_array;
        local_array = temp-k;
        

        // change auxiliary rows 
        // Swap pointers before sending/receiving auxiliary rows
        MPI_Request request[4]; // Array of MPI_Request, necessary to ensure that the non blocking receive is complete

        if(rank == 0){ // TAG IS ALWAYS THE RANK OF THE SENDING PROCESS + n_step
            // Send message to last process
            MPI_Isend(local_array+k, k, MPI_UNSIGNED_CHAR, size-1, rank + i + 1, MPI_COMM_WORLD, &request[0]);
            // Send message to the local_array process
            MPI_Isend(local_array + rows_read*k, k, MPI_UNSIGNED_CHAR, rank+1, rank + i, MPI_COMM_WORLD, &request[1]);
            
            // Blocking receive message
            // Lower row receive
            MPI_Irecv(local_array + k + rows_read*k, k, MPI_UNSIGNED_CHAR, rank+1, rank+1 + i, MPI_COMM_WORLD, &request[2]);
            // Upper row receive from final process
            MPI_Irecv(local_array, k, MPI_UNSIGNED_CHAR, size-1, size-1 + i + 1, MPI_COMM_WORLD, &request[3]);

        }else if(rank == size-1){
            // Send message to process before
            MPI_Isend(local_array+k, k, MPI_UNSIGNED_CHAR, rank-1, rank + i, MPI_COMM_WORLD, &request[0]);
            // Send message to process 0
            MPI_Isend(local_array + rows_read*k, k, MPI_UNSIGNED_CHAR, 0, rank + i +1 , MPI_COMM_WORLD, &request[1]);
            
            // Lower row receive
            MPI_Irecv(local_array + k + rows_read*k, k, MPI_UNSIGNED_CHAR, 0, 0+i+1, MPI_COMM_WORLD, &request[2]);
            // Upper row receive
            MPI_Irecv(local_array, k, MPI_UNSIGNED_CHAR, rank-1, rank-1 + i, MPI_COMM_WORLD, &request[3]);
        }else{
            // Send message to process before
            MPI_Isend(local_array+k, k, MPI_UNSIGNED_CHAR, rank-1, rank + i, MPI_COMM_WORLD, &request[0]);
            // Send message to process after
            MPI_Isend(local_array + rows_read*k, k, MPI_UNSIGNED_CHAR, rank+1, rank + i, MPI_COMM_WORLD, &request[1]);
            
            // Upper row receive
            MPI_Irecv(local_array, k, MPI_UNSIGNED_CHAR, rank-1, rank-1 + i, MPI_COMM_WORLD, &request[2]);
            // Lower row receive
            MPI_Irecv(local_array + k + rows_read*k, k, MPI_UNSIGNED_CHAR, rank+1, rank+1+i, MPI_COMM_WORLD, &request[3]);
        } 
        MPI_Waitall(4, request, MPI_STATUS_IGNORE); // To ensure they all do not alter the buffer by moving to next iteration.
        
        printf("PROCESSOR NEXT STEP %d\n", rank);
        for (int i = 0; i<(rows_read+2) * k; i++){
        printf("element %d of FOR NEXT STEP array is %d\n", i, local_array[i]);
        }
        printf("\n");
        printf("\n");
        

        
        if((s!=0)&&(i%s==0)){

        }
        


    }


    // write the last step in a .pgm file

    // free the memory
    


}