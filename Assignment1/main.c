/*
 * main.c
 *
 *  Created on: 9 Sep 2015
 *      Author: feynman
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int getNextData(FILE *filename);
int performLowPass(int a[], int b[], int, int);
int performHighPass(int a[], int b[], int, int);
int performDerivative(int a[], int);
int performSquaring(int);
int performMWI(int a[]);

int main(){

	FILE *filename = fopen("ECG.txt", "r");

	int ECG_size = 13, lowPass_size = 33, highPass_size = 5, squaring_size = 30;
	int ECG_out[13] = {0}, lowPass_out[33] = {0}, highPass_out[5] = {0}, derivative_out = 0, squaring_out[30] = {0}, MWI_out = 0;
	int ECG_pointer = 0, lowPass_pointer = 0, highPass_pointer = 0, squaring_pointer = 0;

	int peak[10000] = {0}, rPeak[10000] = {0}, peak_pointer = 0, rPeak_pointer = 0, rr_interval_pointer = 0, rr_interval_ok_pointer = 0, interval_size = 8;
	int peak_temp_new = 0, peak_temp_old = 0;
	float timeCount = 0, timeDetected = 0, rr = 0, rr_interval[8] = {0}, rr_interval_ok[8] = {0};

	float spkf = 4600, npkf = 3700, threshold1 = 4300, threshold2 = 0.5*threshold1, rr_average = 0, rr_average_ok = 0, rr_low = 120*0.004, rr_high = 180*0.004, rr_miss = 40*0.004;

	clock_t time = clock();

	for (int i = 0; i < 10000; i++){
		ECG_out[ECG_pointer] = getNextData(filename);
		lowPass_out[lowPass_pointer] = performLowPass(ECG_out, lowPass_out, ECG_pointer, lowPass_pointer);
		highPass_out[highPass_pointer] = performHighPass(lowPass_out, highPass_out, lowPass_pointer, highPass_pointer);
		derivative_out = performDerivative(highPass_out, highPass_pointer);
		squaring_out[squaring_pointer] = performSquaring(derivative_out);
		MWI_out = performMWI(squaring_out);

		timeCount += 0.004;

		if (peak_temp_new > MWI_out && peak_temp_old < peak_temp_new) {

			peak[peak_pointer] = peak_temp_new;

//			printf("Time = %f   Peak no. %i   Peak = %i\n", timeCount, peak_pointer, peak[peak_pointer]);
			// RANDOM .git TEST

			// Check if current peak is over threshold1
			if (peak[peak_pointer] > threshold1 && (timeCount - timeDetected) >  0.05) {
				rr = timeCount - timeDetected;
				timeDetected = timeCount;
			//1	printf("%f", rr);


				//printf("RR = %f   Low = %f   High = %f   Miss = %f\n", rr, rr_low, rr_high, rr_miss);
				if (rr_low < rr && rr < rr_high) {
					rPeak[rPeak_pointer] = peak[peak_pointer];
					rPeak_pointer++;

					printf("Time = %f            R-Peak no. %i            R-Peak = %i            SPKF = %f            NPKF = %f            Threshold 1 = %f            Threshold 2 = %f            Low = %f            High = %f            Miss = %f\n", timeDetected, rPeak_pointer-1, peak[peak_pointer], spkf, npkf, threshold1, threshold2, rr_low, rr_high, rr_miss);

					spkf = 0.125*peak[peak_pointer]+0.875*spkf;

					rr_interval[rr_interval_pointer] = rr;
					rr_interval_ok[rr_interval_ok_pointer] = rr;

					float sum_ok = 0;
					for (int j = 0; j < interval_size; j++)
						sum_ok += rr_interval_ok[j];

					rr_average_ok = sum_ok / interval_size;
					rr_low = 0.92*rr_average_ok;
					rr_high = 1.16*rr_average_ok;
					rr_miss = 1.66*rr_average_ok;

					threshold1 = npkf +0.25*(spkf - npkf);
					threshold2 = 0.5*threshold1;

//					printf("SPKF = %f   NPKF = %f   Threshold 1 = %f   Threshold 2 = %f\n" , spkf, npkf, threshold1, threshold2);

					rr_interval_ok_pointer++;
					if (rr_interval_ok_pointer >= interval_size)
						rr_interval_ok_pointer = 0;

					rr_interval_pointer++;
					if (rr_interval_pointer >= interval_size)
						rr_interval_pointer = 0;

				} else {

					if (rr > rr_miss) {

						for (int searchback_pointer = peak_pointer; searchback_pointer >= 0; searchback_pointer--) {
							if (peak[searchback_pointer] > threshold2) {
								rPeak[rPeak_pointer] = peak[searchback_pointer];
								rPeak_pointer++;
								spkf = 0.25*peak[searchback_pointer]+0.75*spkf;

								rr_interval[rr_interval_pointer] = rr;

								float sum = 0;
								for (int j = 0; j < interval_size; j++){
									sum += rr_interval[j];
								}

								printf("Time = %f            R-Peak no. %i            Searchback R-Peak = %i              SPKF = %f            NPKF = %f            Threshold 1 = %f            Threshold 2 = %f            Low = %f            High = %f            Miss = %f\n", timeDetected, rPeak_pointer-1, peak[searchback_pointer], spkf, npkf, threshold1, threshold2, rr_low, rr_high, rr_miss);
								rr_average = sum / interval_size;
								rr_low = 0.92*rr_average;
								rr_high = 1.16*rr_average;
								rr_miss = 1.66*rr_average;

								threshold1 = npkf +0.25*(spkf - npkf);
								threshold2 = 0.5*threshold1;
								searchback_pointer = 0;

								rr_interval_pointer++;
								if (rr_interval_pointer >= interval_size)
									rr_interval_pointer = 0;

							}
						}
					} else {
						printf("Time = %f   Ignored %i   Miss = %f\n" , timeDetected,  peak[peak_pointer], rr_miss);
					}
				}

			} else {
				npkf = 0.125*peak[peak_pointer]  + 0.875*npkf;
				threshold1 = npkf +0.25*(spkf - npkf);
				threshold2 = 0.5*threshold1;



//				printf("SPKF = %f   NPKF = %f   Threshold 1 = %f   Threshold 2 = %f\n" , spkf, npkf, threshold1, threshold2);

			}

			peak_pointer++;

		}

		peak_temp_old = peak_temp_new;
		peak_temp_new = MWI_out;


		// printf("ECG: %i\t\t\t\t low-pass: %i\t\t\t\t high-pass: %i\t\t\t\t derivative: %i\t\t\t\t squaring: %i\t\t\t\t MWI: %i", ECG_out[ECG_pointer], lowPass_out[lowPass_pointer], highPass_out[highPass_pointer], derivative_out, squaring_out[squaring_pointer], MWI_out);


		ECG_pointer++;
		if (ECG_pointer >= ECG_size)
			ECG_pointer = 0;

		lowPass_pointer++;
		if (lowPass_pointer >= lowPass_size)
			lowPass_pointer = 0;

		highPass_pointer++;
		if (highPass_pointer >= highPass_size)
			highPass_pointer = 0;

		squaring_pointer++;
		if (squaring_pointer >= squaring_size)
			squaring_pointer = 0;

	}


	//for (int i = 0; i < 10000; i++){
	//	if (peak[i] != 0) printf("Peak no. %i          Peak : %i          R-peak : %i\n", i+1, peak[i], rPeak[i]);
	//}


	printf("Execution time: %lu", (long int) (clock()-time));

	return 0;

}
