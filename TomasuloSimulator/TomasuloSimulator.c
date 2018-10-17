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
int numberBufferLoad = 2;
int numberBufferStore = 2;
int numberRSfpAdd = 3;
int numberRSfpMult = 4;
int numberRSfpDiv = 2;
int numberRSbu = 2;

//Number of renaming registers
int numberRenameReg = 8;


int main(int argc, char** argv) {
	//Validate command line argument

	//TODO: read these variables from config file
	int NF = 4;
	int NI = 8;
	int NW = 4;
	int NR = 16;
	int NB = 4;

	if (argc == 2) {
		printf ("Simulator of Part 1.\n");

        fillInstructionAndDataCache (argv[1]); //call loader to fill caches

        printInstructionCache (); //print instruction cache

        printDataCache (); //print data cache

        printCodeLabels ();

        initializeCPU (NI, NR, NB); //initialize CPU data structure

        int test_cycle;



        //for (test_cycle = 0; test_cycle < 35; test_cycle ++){

        while(runClockCycle(NF, NI, NW, NB, NR)){//loop emulating simulator clock cycles
            //runClockCycle(NF, NI, NW, NB, NR);
            printInstructionQueue();
            printReservationStations();
            printRenamingRegisters();
            printIntegerRegistersStatus();
            printFPRegistersStatus();
            printROB();


        }

        printDataCache (); //print data cache


        printIntegerRegisters (); //print integer registers
        printFPRegisters (); //print floating point registers
	}

	if (argc == 3) {
	    printf ("Simulator of Part 2.\n");

	    fillInstructionAndDataCache (argv[1]); //call loader to fill caches
	    fillInstructionAndDataCache2 (argv[2]); //call loader to fill caches
	    printInstructionCache (); //print instruction cache
        printInstructionCache2 (); //print instruction cache
        printDataCache ();//print data cache
        printDataCache2 ();//print data cache
        printCodeLabels ();
        printCodeLabels2 ();

        initializeCPU (NI, NR, NB); //initialize CPU data structure

	    //TODO
	}
	

	return 0;

}
