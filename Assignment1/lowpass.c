/*
 * lowpass.c
 *
 *  Created on: 9 Sep 2015
 *      Author: feynman
 */

#include <stdio.h>

int performLowPass(int x[], int y[], int x_pointer, int y_pointer, int x_size, int y_size){

	int x6 = (x_pointer - 6 < 0) ? x_pointer-6 + x_size : x_pointer-6;
	int x12 = (x_pointer - 12 < 0) ? x_pointer-12 + x_size : x_pointer-12;
	int y1 = (y_pointer - 1 < 0) ? y_pointer-1 + y_size : y_pointer-1;
	int y2 = (y_pointer - 2 < 0) ? y_pointer-2 + y_size : y_pointer-2;

	return 2*y[y1]-y[y2]+(x[x_pointer]-2*x[x6]+x[x12])/32;
}
