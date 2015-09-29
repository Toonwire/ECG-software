/*
 * highpass.c
 *
 *  Created on: 9 Sep 2015
 *      Author: feynman
 */

#include <stdio.h>

int performHighPass(int x[], int y[], int x_pointer, int y_pointer, int x_size, int y_size){

	int x16 = (x_pointer - 16 < 0) ? x_pointer-16 + x_size : x_pointer-16;
	int x17 = (x_pointer - 17 < 0) ? x_pointer-17 + x_size : x_pointer-17;
	int x32 = (x_pointer - 32 < 0) ? x_pointer-32 + x_size : x_pointer-32;
	int y1 = (y_pointer - 1 < 0) ? y_pointer-1 + y_size : y_pointer-1;

	return y[y1]-x[x_pointer]/32+x[x16]-x[x17]+x[x32]/32;
}
