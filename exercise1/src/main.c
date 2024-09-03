#include <getopt.h>
//#include <mpi.h>
#include <omp.h>
#include <time.h>
#include "read_write.h"
#include "initialize.h"



int main(int argc, char **argv){

    char fname[100] = "try01.txt";
    int maxsize, xsize, ysize;
    maxsize = 1;
    xsize = 100;
    ysize = 100;

    initialize(fname, maxsize, xsize, ysize);


    return 0;
}

