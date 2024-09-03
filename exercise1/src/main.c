#include <getopt.h>
//#include <mpi.h>
#include <omp.h>
#include <time.h>
#include "read_write.h"
#include "initialize.h"



int main(int argc, char **argv){

    char fname[100] = "try01.pgm";
    int maxsize, xsize, ysize;
    maxsize = 255;
    xsize = 10;
    ysize = 10;

    initialize(fname, maxsize, xsize, ysize);


    return 0;
}

