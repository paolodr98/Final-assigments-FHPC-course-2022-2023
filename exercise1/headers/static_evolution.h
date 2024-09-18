#ifndef STATIC_EVOLUTION_H
#define STATIC_EVOLUTION_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h> 

void static_ev_parallel(char *filename, int rank, int size, int k, int maxval, int s, int t);
void static_ev_serial(char *filename, int k, int maxval, int s, int t);

#endif 