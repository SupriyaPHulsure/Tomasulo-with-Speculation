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

//Number of reservation stations of each unit
int numberRSint = 4;
int numberRSmult = 2;
int numberRSload = 2;
int numberRSstore = 2;
int numberRSfpAdd = 3;
int numberRSfpMult = 4;
int numberRSfpDiv = 2;
int numberRSbu = 2;

//Number of renaming registers
int numberRenameReg = 8;


int main(int argc, char** argv) {
	//Validate command line argument

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

	initializeCPU (NI, NR, NB); //initialize CPU data structure

    int test_cycle;

    for (test_cycle = 0; test_cycle < 10; test_cycle ++){
        runClockCycle(NF, NI, NW, NB);

        //printFetchBuffer();

        printInstructionQueue();
        printReservationStations();
        printRenamingRegisters();
        printIntegerRegistersStatus();
        printFPRegistersStatus();
        printROB();

    }
	printROB();
	
	//while (runClockCycle()); //loop emulating simulator clock cycles

    /*
	printDataCache (); //print data cache


	printIntegerRegisters (); //print integer registers
	printFPRegisters (); //print floating point registers
	*/

	return 0;
}
