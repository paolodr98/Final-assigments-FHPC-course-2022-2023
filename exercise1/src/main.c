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

#define MAXVAL 255

#define INIT 1
#define RUN  2

#define K_DFLT 100

#define ORDERED 0
#define STATIC  1

char fname_deflt[] = "game_of_life.pgm";

int   action = 0;
int   k      = K_DFLT;
int   e      = ORDERED;
int   n      = 100;
int   s      = 0;
char *fname  = NULL;


int main ( int argc, char **argv){

    int maxval = MAXVAL;

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

    /*  START  */

    if (action == INIT){
        if(fname==NULL){
            printf("No name was passed, default name will be used\n");
            fname=(char*)malloc(sizeof(fname_deflt)+1);
            sprintf(fname, "%s", fname_deflt);
        }
        
        if (size == 1){
            initialize_serial(k,fname,maxval);
        }else{
            initialize_parallel(k, fname, rank, size, maxval);
        }
        

    }

    /*
    if (action == RUN){
        if (fname == NULL) {
                    printf("No name was passed, the program will try to read from %s\n\n", fname_deflt);
                fname = (char*) malloc(sizeof(fname_deflt));
                sprintf(fname, "%s", fname_deflt);
        }
        if(e==ORDERED){
            if(size==1){
                ordered_ev_serial(fname, k, maxval, s, n);
            } else {
                ordered_ev_parallel(fname, rank, size, k, maxval, s, n);
            }
        //run the static evolution
        }else if(e==STATIC){
            if(size==1){
                static_ev_serial(fname, k, maxval, s, n);
            } else {
                static_ev_parallel(fname, rank, size, k, maxval, s, n);
            }
        } else {
            printf("Invalid input: e can only take value in {0,1}\n"); 
            MPI_Finalize(); 
            exit( 1 );
        }     
    }
    */

    /*  END  */


    if (fname != NULL)
        free (fname);
    MPI_Finalize();

    return 0;
}


