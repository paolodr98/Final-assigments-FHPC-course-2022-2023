#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <mpi.h>
#include <omp.h>
#include <time.h>
#include "read_write.h"
#include "initialize.h"
#include "static_evolution.h"
#include "ordered_evolution.h"

/*

#define INIT 1
#define RUN  2

#define K_DFLT 100

#define ORDERED 0
#define STATIC  1



char fname_deflt[] = "game_of_life.pgm";

int   action = 0;
int   k      = K_DFLT;
int   e      = ORDERED;
int   n      = 10000;
int   s      = 1;
char *fname  = NULL;

*/

int main ( int argc, char **argv){

    char fname[100] = "try01.pgm";
    int k = 5;
    //int rank, size;
    int maxval = 255;
    int t = 2;
    int s = 1;

    MPI_Status status;
	MPI_Request req;

    	//Initialize MPI environment
	int mpi_provided_thread_level;
	MPI_Init_thread( &argc, &argv, MPI_THREAD_FUNNELED, &mpi_provided_thread_level);
	if ( mpi_provided_thread_level < MPI_THREAD_FUNNELED ) { 
		printf("a problem arise when asking for MPI_THREAD_FUNNELED level\n"); 
		MPI_Finalize(); 
		exit( 1 );
	}


    int rank, size;

    // Get the rank of the current process
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Get the total number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    printf("N size: %d\n", size);


    /*
    int action = 0;
    char *optstring = "irk:e:f:n:s:";

    int c;
    while ((c = getopt(argc, argv, optstring)) != -1) {
        switch(c) {
            
        case 'i':
            action = INIT; break;
            
        case 'r':
            action = RUN; break;
            
        case 'k':
            k = atoi(optarg); break;

        case 'e':
            e = atoi(optarg); break;

        case 'f':
            fname = (char*)malloc( sizeof(optarg)+1 );
            sprintf(fname, "%s", optarg );
            break;

        case 'n':
            n = atoi(optarg); break;

        case 's':
            s = atoi(optarg); break;

        default :
            printf("argument -%c not known\n", c ); break;
        }
    }
    */

    /*  START my code   */


    /*
    if (size == 1){
        initialize_serial(k,fname,maxval);
    }else{
        initialize_parallel(k, fname, rank, size, maxval);
    }
    if (rank == 0){
        printf("INIT DONE\n");
    }
    */

   /*
    if (size == 1){
        static_ev_serial(fname, k, maxval, s, t);
    }else{
        static_ev_parallel(fname, rank, size, k, maxval, s,t);
    }
    if (rank == 0){
        printf("EVOLUTION DONE\n");
    }
    */
    if (size == 1){
        ordered_ev_serial(fname, k, maxval, s, t);
    }else{
        ordered_ev_parallel(fname, rank, size, k, maxval, s,t);
    }
    if (rank == 0){
        printf("EVOLUTION DONE\n");
    }


    /*
    if(action == INIT){
    // create initial conditions
    initialize_parallel(k, fname, rank, size, rows_read);
    }
    */

    /*
    char fname[100] = "try01.pgm";
    int maxsize, xsize, ysize;
    maxsize = 255;
    xsize = 5;
    ysize = 5;

    initialize(fname, maxsize, xsize, ysize);

    printf("\n");

    static_ev(fname);
    */



    /*  END my code   */


    // if ( fname != NULL )
    //    free ( fname );
    MPI_Finalize();

    return 0;
}

