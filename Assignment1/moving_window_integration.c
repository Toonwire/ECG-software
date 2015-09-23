/*
 * moving_window_integration.c
 *
 *  Created on: 9 Sep 2015
 *      Author: feynman
 */

int performMWI(int x[]){

	int sum = 0;

	for (int i = 0; i < 30; i++){
		sum += x[i];
	}

	return sum/30;
}

