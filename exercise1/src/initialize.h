#ifndef INITIALIZE_H
#define INITIALIZE_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h> 

//void initialize(char *fname, int maxval, int xsize, int ysize);
void initialize_parallel(int k, char *fname, int rank, int size, int maxval);

#endif 


