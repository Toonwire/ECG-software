/*
 * derivative.c
 *
 *  Created on: 9 Sep 2015
 *      Author: feynman
 */

#include <stdio.h>

int performDerivative(int x[], int x_pointer, int x_size){

	int x1 = (x_pointer - 1 < 0) ? x_pointer-1 + x_size : x_pointer-1;
	int x3 = (x_pointer - 3 < 0) ? x_pointer-3 + x_size : x_pointer-3;
	int x4 = (x_pointer - 4 < 0) ? x_pointer-4 + x_size : x_pointer-4;

	return (2*x[x_pointer]+x[x1]-x[x3]-2*x[x4])/8;
}
