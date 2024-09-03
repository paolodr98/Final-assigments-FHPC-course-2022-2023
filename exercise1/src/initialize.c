#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <omp.h>
#include "read_write.h"


void initialize(char *fname, int maxval, int xsize, int ysize){

    unsigned char *grid; 
    unsigned char random; 


    grid= (unsigned char*)malloc( xsize*ysize*sizeof(unsigned char) );

    // fill dynamic array in shared parallel
	#pragma omp parallel
	{
		// set seed
		int myid = omp_get_thread_num();
		unsigned int my_seed = myid*myid + 2*myid + 3;

		#pragma omp for schedule(static, xsize*ysize)
		for (int i=0; i<xsize*ysize; i++)
		{
			random = (unsigned char)(rand_r(&my_seed)%2); //generate random number, considers it modulo 2, cast it in char; 
			grid[i] = random;
		}
	}

    // write to pgm image
	printf("writing image to %s\n", fname);
    write_pgm_image((void *) grid, maxval, xsize, ysize, fname);
	
	free(grid);
    return;
}
