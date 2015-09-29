/*
 * main.c
 *
 *  Created on: 9 Sep 2015
 *      Author: feynman
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>




int ECG_size = 13, lowPass_size = 33, highPass_size = 5, squaring_size = 30;
int ECG_out[13] = {0}, lowPass_out[33] = {0}, highPass_out[5] = {0}, derivative_out = 0, squaring_out[30] = {0}, MWI_out = 0;
int ECG_pointer = 0, lowPass_pointer = 0, highPass_pointer = 0, squaring_pointer = 0;

int peak[10000] = {0}, rPeak[10000] = {0}, peak_pointer = 0, rPeak_pointer = 0, rr_interval_pointer = 0, rr_interval_ok_pointer = 0, interval_size = 8;
int peak_temp_new = 0, peak_temp_old = 0;
int timeCount = 0, timeDetected = 0, rr = 0, rr_interval[8] = {600,600,600,600,600,600,600,600}, rr_interval_ok[8] = {600,600,600,600,600,600,600,600};

int spkf = 4600, npkf = 3700, threshold1 = 4300, threshold2 = 2150, rr_average = 0, rr_average_ok = 0, rr_low = 480, rr_high = 720, rr_miss = 160;
int skipped_beats = 0, vital_heartbeat_error = 0;

FILE *filterFile;

int getNextData(FILE *filename);
int performLowPass(int a[], int b[], int, int);
int performHighPass(int a[], int b[], int, int);
int performDerivative(int a[], int);
int performSquaring(int);
int performMWI(int a[]);



int applyFilters(){


	lowPass_out[lowPass_pointer] = performLowPass(ECG_out, lowPass_out, ECG_pointer, lowPass_pointer);
	highPass_out[highPass_pointer] = performHighPass(lowPass_out, highPass_out, lowPass_pointer, highPass_pointer);
	derivative_out = performDerivative(highPass_out, highPass_pointer);
	squaring_out[squaring_pointer] = performSquaring(derivative_out);
	MWI_out = performMWI(squaring_out);

	/*
	 if (MWI_out > 0 && MWI_out < 20000){
		fprintf(filterFile, "%d\n", lowPass_out[lowPass_pointer]);
	}
	*/

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

	return 0;
}

int isLocalMaximum(){
	return (peak_temp_new > MWI_out && peak_temp_old < peak_temp_new);
}



int main(){
	FILE *filename = fopen("ECG.txt", "r");
	//filterFile = fopen("checkFile.txt", "w");

	clock_t time = clock();

	for (int i = 0; i < 10000; i++){
		ECG_out[ECG_pointer] = getNextData(filename);
		applyFilters();



		timeCount += 4;

		if (isLocalMaximum()) {

			peak[peak_pointer] = peak_temp_new;
			//printf("%i \n", peak[peak_pointer]);


//			printf("Time = %i   Peak no. %i   Peak = %i\n", timeCount, peak_pointer, peak[peak_pointer]);

			if (peak[peak_pointer] > threshold1) {
				rr = timeCount - timeDetected;
				vital_heartbeat_error = 0;
			//	printf("%i", rr);


				//printf("RR = %i   Low = %i   High = %i   Miss = %i\n", rr, rr_low, rr_high, rr_miss);
				if (rr_low < rr && rr < rr_high) {
					skipped_beats = 0;
					rPeak[rPeak_pointer] = peak[peak_pointer];

					if (rPeak[rPeak_pointer] < 2000)
						printf("WARNING: Weak heartbeat!\n");

					rPeak_pointer++;

					//printf("Time = %i          RR = %i            R-Peak no. %i            R-Peak = %i            SPKF = %i            NPKF = %i            Threshold 1 = %i            Threshold 2 = %i            Low = %i            High = %i            Miss = %i\n", timeCount, rr, rPeak_pointer-1, peak[peak_pointer], spkf, npkf, threshold1, threshold2, rr_low, rr_high, rr_miss);

					spkf = peak[peak_pointer]*0.125+spkf*0.875;

					rr_interval[rr_interval_pointer] = rr;
					rr_interval_ok[rr_interval_ok_pointer] = rr;
					timeDetected = timeCount;

					int sum_ok = 0, sum = 0;
					for (int j = 0; j < interval_size; j++){
						sum_ok += rr_interval_ok[j];
						sum += rr_interval[j];
					}

					rr_average = sum / interval_size;
					rr_average_ok = sum_ok / interval_size;
					rr_low = rr_average_ok*0.92;
					rr_high = rr_average_ok*1.16;
					rr_miss = rr_average_ok*1.66;

					threshold1 = npkf + (spkf - npkf)*0.25;
					threshold2 = threshold1*0.5;

					//printf("Time = %i   i-value = %i  SPKF = %i   NPKF = %i   Threshold 1 = %i   Threshold 2 = %i   rr_average = %i   rr_average_ok = %i\n" ,timeCount, i, spkf, npkf, threshold1, threshold2, rr_average, rr_average_ok);

					//int pulse = 60000 / rr_average;
					//printf("pulse = %i   rr_average = %i\n", pulse, rr_average);

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

								if (rPeak[rPeak_pointer] < 2000)
									printf("Warning: Weak heartbeat!\n");

								skipped_beats++;
								if (skipped_beats == 5) {
									printf("WARNING: You have missed 5 regular R peaks!\n");
									skipped_beats = 0;
								}

								rPeak_pointer++;
								spkf = peak[searchback_pointer]*0.25+spkf*0.75;
								rr_interval[rr_interval_pointer] = rr;
								timeDetected = timeCount;

								int sum = 0;
								for (int j = 0; j < interval_size; j++){
									sum += rr_interval[j];
								}

								//printf("sum = %i \n", sum);
								rr_average = sum / interval_size;
								//printf("rr_average = %i\n",rr_average);

								rr_low = rr_average*0.92;
								rr_high = rr_average*1.16;
								rr_miss = rr_average*1.66;
								threshold1 = npkf + (spkf - npkf)*0.25;
								threshold2 = threshold1*0.5;

								//printf("Time = %i         RR = %i            R-Peak no. %i            Searchback R-Peak = %i              SPKF = %i            NPKF = %i            Threshold 1 = %i            Threshold 2 = %i            Low = %i            High = %i            Miss = %i\n", timeCount, rr, rPeak_pointer-1, peak[searchback_pointer], spkf, npkf, threshold1, threshold2, rr_low, rr_high, rr_miss);

								int pulse = 60000 / rr_average;
								//printf("pulse = %i   rr_average = %i\n", pulse, rr_average);

								searchback_pointer = 0;	// Found a new R-peak - BREAK

								rr_interval_pointer++;
								if (rr_interval_pointer >= interval_size)
									rr_interval_pointer = 0;

							}
						}
					} else {
						//printf("Time = %i   Ignored %i   rr = %i   Miss = %i\n" , timeCount,  peak[peak_pointer], rr, rr_miss);
					}
				}

			} else {
				vital_heartbeat_error++;

				npkf = peak[peak_pointer]*0.125  + npkf*0.875;
				threshold1 = npkf + (spkf - npkf)*0.25;
				threshold2 = threshold1*0.5;

				if (vital_heartbeat_error == 10)
					printf("WARNING: Heart failure!\n");


//				printf("SPKF = %i   NPKF = %i   Threshold 1 = %i   Threshold 2 = %i\n" , spkf, npkf, threshold1, threshold2);

			}

			peak_pointer++;

		}

		peak_temp_old = peak_temp_new;
		peak_temp_new = MWI_out;


		// printf("ECG: %i\t\t\t\t low-pass: %i\t\t\t\t high-pass: %i\t\t\t\t derivative: %i\t\t\t\t squaring: %i\t\t\t\t MWI: %i", ECG_out[ECG_pointer], lowPass_out[lowPass_pointer], highPass_out[highPass_pointer], derivative_out, squaring_out[squaring_pointer], MWI_out);


	}


	//for (int i = 0; i < 10000; i++){
	//	if (peak[i] != 0) printf("Peak no. %i          Peak : %i          R-peak : %i\n", i+1, peak[i], rPeak[i]);
	//}


	printf("Execution time: %g\n", ((double) (clock()-time)) / CLOCKS_PER_SEC);

	return 0;

}
