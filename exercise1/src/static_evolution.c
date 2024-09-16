#include "read_write.h"
#include "initialize.h"
#include <omp.h>
#include <mpi.h> 



void update_cell(unsigned char *local_array, unsigned char *new_local_array, int k, int rows_read){
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

        new_local_array[i] = (count == 765 || count == 510) ? 255 : 0; 

        //DEBUG
        printf("Element: %d, up: %d, middle: %d, down: %d, value array: %d\n", i, up, middle, down, new_local_array[i]);
    }
    printf("\n");
}

void write_array(unsigned char *local_array,int i, int k, int maxval, int rank, int size, int rows_read, int* list_rows_proc){
    int *recvcounts = NULL;
    int *displs = NULL;

    if (rank == 0) {
        recvcounts = (int *)malloc(size * sizeof(int));
        displs = (int *)malloc(size * sizeof(int));

        // Prepare recvcounts and displs based on list_rows_proc
        int offset = 0;
        for (int p = 0; p < size; p++) {
            recvcounts[p] = list_rows_proc[p] * k; // Number of elements to receive from each process
            displs[p] = offset;
            offset += recvcounts[p];
        }
    }
    // Send counts are the same for all processes
    int sendcount = rows_read * k; // Number of elements to send

    // Prepare the send buffer (excluding ghost rows)
    unsigned char *sendbuf = local_array + k; // Starting from the second row

    // The root process prepares the receive buffer
    unsigned char *recvbuf = NULL;
    if (rank == 0) {
        recvbuf = (unsigned char *)malloc(k * k * sizeof(unsigned char)); // Full image size
    }

    // Gather the data to the root process
    MPI_Gatherv(sendbuf, sendcount, MPI_UNSIGNED_CHAR,
                recvbuf, recvcounts, displs, MPI_UNSIGNED_CHAR,
                0, MPI_COMM_WORLD);

    // The root process writes the image to a PGM file
    if (rank == 0) {
        // Generate the output filename, e.g., "output_step_<i>.pgm"
        char output_filename[256];
        sprintf(output_filename, "static_parallel_output_step_%d.pgm", i);

        write_pgm_image((void *)recvbuf, maxval, k, k, output_filename);

        // Free the receive buffer after writing
        free(recvbuf);
        free(recvcounts);
        free(displs);
    }
}

void static_ev_parallel(char *filename, int rank, int size, int k, int maxval, int s, int t){

    int rows_read = k / size; 
    rows_read = (rank < k % size) ? rows_read+1 : rows_read;
    unsigned char *local_array = NULL;
    unsigned char *new_local_array = NULL;
    unsigned char *completeMatrix = NULL;
    double start_time, end_time;

    int rank_above= (rank==0)? size-1 : rank-1;
  	int rank_below= (rank==size-1)? 0: rank+1;


    // _____________read the image in root processor_____________
    if(rank==0){
        start_time = MPI_Wtime();
        completeMatrix = (unsigned char*)malloc(k*k *sizeof(unsigned char));
		read_pgm_image((void**)&completeMatrix, &maxval, &k, &k, filename); //Initialize the matrix by reading the pgm file where it's stored
        
        //DEBUG
        /*
        for (int i = 0; i< k*k; i++){
        printf("%d element: %d\n",i, completeMatrix[i]);
        }
        printf("\n");
        printf("----------------------\n");
        printf("\n");
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

    /*
    printf("PROCESSOR INIT %d\n", rank);
    for (int i = 0; i<(rows_read+2) * k; i++){
        printf("element %d of local array is %d\n", i, local_array[i]);
    }
    printf("\n");
    */
   
    

    // _____________start the evolution for t steps_____________
    new_local_array = (unsigned char*)malloc((rows_read+2) * k * sizeof(unsigned char));
    for(int i=1; i <=t; i++){ 

        printf("----------ITERATION %d-----------------\n",i);

        update_cell(local_array, new_local_array, k, rows_read);

        //DEBUG
        
        /*
        printf("PROCESSOR NEW %d, ITERATION %d\n", rank, i);
        for (int u = k; u<(rows_read+1) * k; u++){
        printf("element %d of NEW array is %d\n", u, new_local_array[u]);
        }
        printf("\n");
        */
        
        // change auxiliary rows 
        // Swap pointers before sending/receiving auxiliary rows
        MPI_Request request[4]; // Array of MPI_Request, necessary to ensure that the non blocking receive is complete

        if(rank == 0){ // TAG IS ALWAYS THE RANK OF THE SENDING PROCESS + n_step
            // Send message to last process
            MPI_Isend(new_local_array+k, k, MPI_UNSIGNED_CHAR, size-1, rank + i + 1, MPI_COMM_WORLD, &request[0]);
            // Send message to the local_array process
            MPI_Isend(new_local_array + rows_read*k, k, MPI_UNSIGNED_CHAR, rank+1, rank + i, MPI_COMM_WORLD, &request[1]);
            
            // Blocking receive message
            // Lower row receive
            MPI_Irecv(new_local_array + k + rows_read*k, k, MPI_UNSIGNED_CHAR, rank+1, rank+1 + i, MPI_COMM_WORLD, &request[2]);
            // Upper row receive from final process
            MPI_Irecv(new_local_array, k, MPI_UNSIGNED_CHAR, size-1, size-1 + i + 1, MPI_COMM_WORLD, &request[3]);

        }else if(rank == size-1){
            // Send message to process before
            MPI_Isend(new_local_array+k, k, MPI_UNSIGNED_CHAR, rank-1, rank + i, MPI_COMM_WORLD, &request[0]);
            // Send message to process 0
            MPI_Isend(new_local_array + rows_read*k, k, MPI_UNSIGNED_CHAR, 0, rank + i +1 , MPI_COMM_WORLD, &request[1]);
            
            // Lower row receive
            MPI_Irecv(new_local_array + k + rows_read*k, k, MPI_UNSIGNED_CHAR, 0, 0+i+1, MPI_COMM_WORLD, &request[2]);
            // Upper row receive
            MPI_Irecv(new_local_array, k, MPI_UNSIGNED_CHAR, rank-1, rank-1 + i, MPI_COMM_WORLD, &request[3]);
        }else{
            // Send message to process before
            MPI_Isend(new_local_array+k, k, MPI_UNSIGNED_CHAR, rank-1, rank + i, MPI_COMM_WORLD, &request[0]);
            // Send message to process after
            MPI_Isend(new_local_array + rows_read*k, k, MPI_UNSIGNED_CHAR, rank+1, rank + i, MPI_COMM_WORLD, &request[1]);
            
            // Upper row receive
            MPI_Irecv(new_local_array, k, MPI_UNSIGNED_CHAR, rank-1, rank-1 + i, MPI_COMM_WORLD, &request[2]);
            // Lower row receive
            MPI_Irecv(new_local_array + k + rows_read*k, k, MPI_UNSIGNED_CHAR, rank+1, rank+1+i, MPI_COMM_WORLD, &request[3]);
        } 
        MPI_Waitall(4, request, MPI_STATUS_IGNORE); // To ensure they all do not alter the buffer by moving to next iteration.

        unsigned char *temp = new_local_array;
        new_local_array = local_array;
        local_array = temp;

        /*
        printf("PROCESSOR NEXT STEP %d, ITERATION %d\n", rank, i);
        for (int u = 0; u<(rows_read+2) * k; u++){
        printf("element %d of FOR NEXT STEP array is %d\n", u, local_array[u]);
        }
        printf("\n");
        printf("\n");
        */
          
        if((s!=0)&&(i%s==0)){
            write_array(local_array, i, k, maxval, rank, size, rows_read, list_rows_proc);
        }
    }
    // write the last step in a .pgm file
    if (s==0){
        write_array(local_array, t, k, maxval, rank, size, rows_read, list_rows_proc);
    }
    // free the memory
    free(new_local_array);
    free(local_array);
    // print the result
    if (rank ==0){
        free(completeMatrix);
        end_time = MPI_Wtime();
        double elapsed_time = end_time - start_time;
        printf("STATIC_PARALLEL: Dimension: %ld,\t Size: %d,\t Threads: %d,\t Time: %f\n",k,size,omp_get_max_threads(),elapsed_time);


    }
}

void static_ev_serial(char *filename, int k, int maxval, int s, int t){

    unsigned char *completeMatrix = NULL;
    unsigned char *local_array = NULL;
    unsigned char *new_local_array = NULL;
    int rows_read = k;
    double start_time, end_time;


    start_time = MPI_Wtime();
    completeMatrix = (unsigned char*)malloc(k*k *sizeof(unsigned char));
	read_pgm_image((void**)&completeMatrix, &maxval, &k, &k, filename); //Initialize the matrix by reading the pgm file where it's stored

    local_array = (unsigned char*)malloc((rows_read+2) * k * sizeof(unsigned char)); // allocate memory on each processor
    
    for(int i = 0; i < k; i++){
        local_array[i] = completeMatrix[(k - 1) * k + i];
    }
    for(int i = 0; i < k*k;i++){
        local_array[k+i] = completeMatrix[i];
    }
    for(int i = 0; i<k; i++){
        local_array[k*(k+1)+i]=completeMatrix[i];
    }
    

    // _____________start the evolution for t steps_____________
    new_local_array = (unsigned char*)malloc((rows_read+2) * k * sizeof(unsigned char));
    
    for(int i=1; i <=t; i++){ 

        printf("----------ITERATION %d-----------------\n",i);
        printf("\n");

        update_cell(local_array, new_local_array, k, rows_read);

        for(int i = 0; i < k; i++){
            new_local_array[i] = new_local_array[k*k+i]; //update auxiliary rows
            new_local_array[k*(k+1)+i] = new_local_array[k+i];
        }

        unsigned char *temp = new_local_array;
        new_local_array = local_array;
        local_array = temp;

        if((s!=0)&&(i%s==0)){
            char output_filename[256];
            sprintf(output_filename, "static_serial_output_step_%d.pgm", i);
            for (int i = 0; i<k*k; i++){
                completeMatrix[i]=local_array[k+i];
            }

            write_pgm_image((void *)completeMatrix, maxval, k, k, output_filename); 
        }

    }
    // write the last step in a .pgm file
    if (s==0){
        char output_filename[256];
        sprintf(output_filename, "static_serial_output_step_%d.pgm", t);
        for (int i = 0; i<k*k; i++){
            completeMatrix[i]=local_array[k+i];
        }

        write_pgm_image((void *)completeMatrix, maxval, k, k, output_filename);
    }
    
    // free the memory
    free(new_local_array);
    free(local_array);
    // print the result
    free(completeMatrix);
    end_time = MPI_Wtime();
    double elapsed_time = end_time - start_time;
    printf("STATIC_SERIAL: Dimension: %ld,\t Size: %d,\t Threads: %d,\t Time: %f\n",k,1,omp_get_max_threads(),elapsed_time);

    
}