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

    FILE *file = fopen ( ENV_CONFIG_FILE, "r" );
    char config[64];
    int parameters[5];
    int i,temp;
    for (i = 0; i < 5; i++) {
        fgets(config, 64, file);
        strtok(config, "=");
        temp = atoi(strtok(NULL, ";"));
        parameters[i] = temp;
    }
    fclose(file);
    int NF = parameters[0];
    int NI = parameters[1];
    int NW = parameters[2];
    int NR = parameters[3];
    int NB = parameters[4];

	if (argc == 2) {
		printf ("Simulator of Part 1.\n");

        fillInstructionAndDataCache (argv[1]); //call loader to fill caches

        printInstructionCache (); //print instruction cache

        printDataCache (); //print data cache

        printCodeLabels ();

        initializeCPU (NI, NR); //initialize CPU data structure

        int test_cycle;



        for (test_cycle = 0; test_cycle < 33; test_cycle ++){

       // while(runClockCycle(NF, NW, NB, NR)){//loop emulating simulator clock cycles
            runClockCycle(NF, NI, NW, NB);
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

        initializeCPU (NI, NR); //initialize CPU data structure

        int test_cycle;
        for (test_cycle = 0; test_cycle < 8; test_cycle ++){
            runClockCycle2 (NF, NW, NB, NR);
            printInstructionQueue();
            printInstructionQueue2();
            printReservationStations();
            printRenamingRegisters();
            printRenamingRegisters2();
            printIntegerRegistersStatus();
            printFPRegistersStatus();
            printIntegerRegistersStatus2();
            printFPRegistersStatus2();
            printROB();
            printROB2();
        }
/*
        printInstructionQueue();
        printInstructionQueue2();
        printReservationStations();
        printRenamingRegisters();
        printRenamingRegisters2();
        printIntegerRegistersStatus();
        printFPRegistersStatus();
        printIntegerRegistersStatus2();
        printFPRegistersStatus2();
        printROB();
        printROB2();
*/

	    //TODO
	}
	

	return 0;

}
