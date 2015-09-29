/*
 * main.c
 *
 *  Created on: 9 Sep 2015
 *      Author: feynman
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "sensor.h"

int ECG_size = 13, lowPass_size = 33, highPass_size = 5, squaring_size = 30;
int ECG_out[13] = {0}, lowPass_out[33] = {0}, highPass_out[5] = {0}, derivative_out = 0, squaring_out[30] = {0}, MWI_out = 0;
int ECG_pointer = 0, lowPass_pointer = 0, highPass_pointer = 0, squaring_pointer = 0;

int peak[10000] = {0}, rPeak[10000] = {0}, peak_pointer = 0, rPeak_pointer = 0, rr_interval_pointer = 0, rr_interval_ok_pointer = 0, interval_size = 8;
int peak_temp_new = 0, peak_temp_old = 0;
int timeCount = 0, timeDetected = 0, rr = 0, rr_interval[8] = {600,600,600,600,600,600,600,600}, rr_interval_ok[8] = {600,600,600,600,600,600,600,600};

int spkf = 4600, npkf = 3700, threshold1 = 4300, threshold2 = 2150, rr_average = 0, rr_average_ok = 0, rr_low = 480, rr_high = 720, rr_miss = 160;
int skipped_beats = 0, vital_heartbeat_error = 0;
int pulse = 60000 / 600;

int getNextData(FILE *filename);
int performLowPass(int a[], int b[], int a_pointer, int b_pointer, int a_size, int b_size);
int performHighPass(int a[], int b[], int a_pointer, int b_pointer, int a_size, int b_size);
int performDerivative(int a[], int a_pointer, int a_size);
int performSquaring(int);
int performMWI(int a[]);

int applyFilters(){

	lowPass_out[lowPass_pointer] = performLowPass(ECG_out, lowPass_out, ECG_pointer, lowPass_pointer, ECG_size, lowPass_size);
	highPass_out[highPass_pointer] = performHighPass(lowPass_out, highPass_out, lowPass_pointer, highPass_pointer, lowPass_size, highPass_size);
	derivative_out = performDerivative(highPass_out, highPass_pointer, highPass_size);
	squaring_out[squaring_pointer] = performSquaring(derivative_out);
	MWI_out = performMWI(squaring_out);

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

int average(int array[], int length) {

	int sum = 0;

	for (int i = 0; i < length; i++)
		sum += array[i];

	return sum/length;
}

int calculatePulse() {
	return 60000 / rr_average;
}

int printData() {
	printf("%i - Amplitude : %i   Pulse : %i\n", timeCount, rPeak[rPeak_pointer], pulse);
	return 0;
}

int setRegularRPeak() {

	// reset a warning variable, since a Rpeak has been detected
	// if 5 peaks has not been an Rpeak in a row, a warning will be executed
	skipped_beats = 0;

	// add the peak to the Rpeak array
	rPeak[rPeak_pointer] = peak[peak_pointer];

	// check if a heartbeat has a low amplitude and warn the patient
	if (rPeak[rPeak_pointer] < 2000)
		printf("WARNING: Weak heartbeat\n");

	// update rr_intervals with the new rr value
	rr_interval[rr_interval_pointer] = rr;
	rr_interval_ok[rr_interval_ok_pointer] = rr;

	// set timeDetected to the last time an Rpeak was detected, which is now
	timeDetected = timeCount;

	// find the average of rr interval
	rr_average = average(rr_interval, interval_size);
	rr_average_ok = average(rr_interval_ok, interval_size);

	// update dynamic values
	rr_low = rr_average_ok*0.92;
	rr_high = rr_average_ok*1.16;
	rr_miss = rr_average_ok*1.66;

	spkf = peak[peak_pointer]*0.125+spkf*0.875;
	threshold1 = npkf + (spkf - npkf)*0.25;
	threshold2 = threshold1*0.5;

	// calculate the pulse
	pulse = calculatePulse();

	// set rr pointers
	rr_interval_ok_pointer++;
	if (rr_interval_ok_pointer >= interval_size)
		rr_interval_ok_pointer = 0;

	rr_interval_pointer++;
	if (rr_interval_pointer >= interval_size)
		rr_interval_pointer = 0;

	// print required data
	printData();

	rPeak_pointer++;

	return 0;
}

int doSearchBack() {

	// search back in the normal peak array for a peak that is higher than threshold2 and register it as an R peak
	for (int searchback_pointer = peak_pointer; searchback_pointer >= 0; searchback_pointer--) {
		if (peak[searchback_pointer] > threshold2) {

			// store the new R peak
			rPeak[rPeak_pointer] = peak[searchback_pointer];

			// warn the patient if the heart has a low amplitude
			if (rPeak[rPeak_pointer] < 2000)
				printf("Warning: Weak heartbeat\n");

			// check if 5 regulat R peaks has been skipped and warn the patient
			// will be reset if a regular R peak happens
			skipped_beats++;
			if (skipped_beats == 5) {
				printf("WARNING: You have missed 5 regular R peaks\n");
				skipped_beats = 0;
			}

			// update rr interval with the new rr value
			rr_interval[rr_interval_pointer] = rr;

			// set timeDetected to the last time an Rpeak was detected, which is now
			timeDetected = timeCount;

			// find the average of rr interval
			rr_average = average(rr_interval, interval_size);

			// update dynamic variables
			rr_low = rr_average*0.92;
			rr_high = rr_average*1.16;
			rr_miss = rr_average*1.66;

			spkf = peak[searchback_pointer]*0.25+spkf*0.75;
			threshold1 = npkf + (spkf - npkf)*0.25;
			threshold2 = threshold1*0.5;

			// calculate the pulse
			pulse = calculatePulse();

			// update the rr pointers
			rr_interval_pointer++;
			if (rr_interval_pointer >= interval_size)
				rr_interval_pointer = 0;

			// print the required data
			printData();

			rPeak_pointer++;

			break;
		}
	}

	return 0;
}

int noRPeakDetected() {

	// warn the patient if 10 peak in a row is not an R peak - very weak heart or abnormal behavior
	vital_heartbeat_error++;
	if (vital_heartbeat_error == 10)
		printf("WARNING: You're dying!\n");

	// update dynamic values
	npkf = peak[peak_pointer]*0.125  + npkf*0.875;
	threshold1 = npkf + (spkf - npkf)*0.25;
	threshold2 = threshold1*0.5;

	return 0;
}

int checkForPeaks() {

	if (isLocalMaximum()) {

		// add the new peak to the peak array
		peak[peak_pointer] = peak_temp_new;

		// check if the peak is above threshold1
		if (peak[peak_pointer] > threshold1) {

			// set rr to the time between the new peak and the last found Rpeak
			rr = timeCount - timeDetected;

			// reset a warning variable, if each reach 10, a warning will be executed
			vital_heartbeat_error = 0;

			// check if the rr value is between rr_low and rr_high
			// if true, an Rpeak has been detected
			if (rr_low < rr && rr < rr_high) {

				setRegularRPeak();

			} else {
				if (rr > rr_miss) {

					doSearchBack();

				}
			}

		} else {

			// if a peak is below threshold1
			noRPeakDetected();

		}

		peak_pointer++;

	}

	peak_temp_old = peak_temp_new;
	peak_temp_new = MWI_out;

	return 0;
}

int main(){

	FILE *filename = fopen("ECG.txt", "r");

	clock_t time = clock();

	for (int i = 0; i < 10000; i++){

		// add 4 milliseconds to the simulated time
		timeCount += 4;

		// retrieve data from file
		ECG_out[ECG_pointer] = getNextData(filename);

		// apply all the filters
		applyFilters();

		// check for peaks, more detailed within the function
		checkForPeaks();

	}

	printf("Execution time: %g\n", ((double) (clock()-time)) / CLOCKS_PER_SEC);

	return 0;

}
