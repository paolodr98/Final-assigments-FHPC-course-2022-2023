#ifndef STATIC_EVOLUTION_H
#define STATIC_EVOLUTION_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h> 

void update_cell();
void static_ev(char *filename, int rank, int size, int k, int rows_read);

#endif 