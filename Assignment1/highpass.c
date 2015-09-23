/*
 * highpass.c
 *
 *  Created on: 9 Sep 2015
 *      Author: feynman
 */


int performHighPass(int x[], int y[], int x_pointer, int y_pointer){

	int x16 = (x_pointer - 16 < 0) ? x_pointer-16 + 33 : x_pointer-16;
	int x17 = (x_pointer - 17 < 0) ? x_pointer-17 + 33 : x_pointer-17;
	int x32 = (x_pointer - 32 < 0) ? x_pointer-32 + 33 : x_pointer-32;
	int y1 = (y_pointer - 1 < 0) ? y_pointer-1 + 5 : y_pointer-1;

	return y[y1]-x[x_pointer]/32+x[x16]-x[x17]+x[x32]/32;
}
