/*
 * DataAcquisition.c
 *
 *  Created on: 9 Sep 2015
 *      Author: feynman
 */

#include <stdio.h>
#include <stdlib.h>

int getNextData(FILE *filename){

	int value;

	fscanf(filename, "%i", &value);

	return value;
}
