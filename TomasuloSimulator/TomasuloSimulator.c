/*
 * TomasuloSimulator.c
 *
 *  Created on: Oct 1, 2015
 *      Author: DebashisGanguly
 */

#include "./Global/TomasuloSimulator.h"

int numberOfIntRegisters = 32; //NIR
int numberOfFPRegisters = 32; //NFPR

int instructionCacheBaseAddress = 1000; //ICBA

int cacheLineSize = 4; //CLS


int main(int argc, char** argv) {
	//Validate command line argument
	printf("Number of args: %d\n", argc);
	
	if (argc != 2) {
		printf ("USAGE: TomasuloSimulator <benchmark_file>\n");
		exit (EXIT_FAILURE);
	}

	//TODO: read these variables from config file
	int NF = 4;
	int NI = 8;
	int NW = 4;
	int NR = 16;
	int NB = 4;

	fillInstructionAndDataCache (argv[1]); //call loader to fill caches

	printInstructionCache (); //print instruction cache

	printDataCache ();

	printCodeLabels (); //print data cache

	initializeCPU (); //initialize CPU data structure

	while (runClockCycle()); //loop emulating simulator clock cycles

	printDataCache (); //print data cache


	printIntegerRegisters (); //print integer registers
	printFPRegisters (); //print floating point registers

	return 0;
}
