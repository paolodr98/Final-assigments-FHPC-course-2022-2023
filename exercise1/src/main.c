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
    int rank, size;
    int maxval = 255;
    MPI_Init( NULL, NULL );
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int t = 5;
    int s = 2;

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

    int rows_read = k / size; 
    rows_read = (rank < k % size) ? rows_read+1 : rows_read;

    /*
    initialize_parallel(k, fname, rank, size, maxval);
    if (rank == 0){
        printf("INIT DONE\n");
    }
    */

    static_ev(fname, rank, size, k, maxval, s,t);

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

