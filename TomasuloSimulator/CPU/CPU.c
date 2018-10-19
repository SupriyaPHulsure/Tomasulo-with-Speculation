/*
 * CPU.c
 *
 *  Created on: Oct 3, 2015
 *      Author: DebashisGanguly
 */

# include "../Global/TomasuloSimulator.h"

int getHashCodeFromPCHash (void *PCHash);
int compareInstructions (void *instruction1, void *instruction2);
int getHashCodeFromInstructionAddress(void *InstructionAddress);
int compareDecodedInstructions(void *decodedInstruction1, void *decodedInstruction2);
int getHashCodeFromBranchAddress(void *branchAddress);
int compareTargetAddress(void *targetAddress1, void *targetAddress2);
int getHashCodeFromROBNumber (void *ROBNumber);
int compareROBNumber (void *ROBNumber1, void *ROBNumber2);
int getHashCodeFromRegNumber (void *RegNumber);
int compareRegNumber (void *RegNumber1, void *RegNumber2);

DictionaryValue *checkReservationStation(DictionaryEntry *dictEntry, int isFloat);
void printPipeline(void *instruction, char *pipeline, int entering);
void branchHelper(CompletedInstruction *instructionAndResult);

int fetchMultiInstructionUnit(int NF);
int fetchMultiInstructionUnit2(int NF);
Instruction * decodeInstruction(char *instruction_str, int instructionAddress);
int decodeInstructionsUnit();
int decodeInstructionsUnit2();
void updateFetchBuffer();
void updateFetchBuffer2();
void updateInstructionQueue();
void updateInstructionQueue2();
void updateReservationStations();
int renameRegIsFull(Dictionary *renameReg, int d);
int addInstruction2RSint(Dictionary *renameRegInt, Dictionary *resSta, Dictionary *resStaResult,
                      char* rsType, int maxLenRS, Instruction *instruction, RegStatus **IntRegStatus);
int addInstruction2RSfloat(Dictionary *renameRegFP, Dictionary *resSta, Dictionary *resStaResult,
                      char* rsType, int maxLenRS, Instruction *instruction, RegStatus **FPRegStatus);
int addInstruction2RSbranch(Dictionary *renameRegInt, Dictionary *resSta, Dictionary *resStaResult,
                            char* rsType, int maxLenRS, Instruction *instruction, RegStatus **IntRegStatus);
int issueInstruction(Instruction *instruction);
int issueUnit(int NW);

int addInstruction2RSint2(Dictionary *renameRegInt, Dictionary *resSta, Dictionary *resStaResult,
                      char* rsType, int maxLenRS, Instruction *instruction, RegStatus **IntRegStatus);
int addInstruction2RSfloat2(Dictionary *renameRegFP, Dictionary *resSta, Dictionary *resStaResult,
                      char* rsType, int maxLenRS, Instruction *instruction, RegStatus **FPRegStatus);
int addInstruction2RSbranch2(Dictionary *renameRegInt, Dictionary *resSta, Dictionary *resStaResult,
                            char* rsType, int maxLenRS, Instruction *instruction, RegStatus **IntRegStatus);
int issueInstruction2(Instruction *instruction);
int issueUnit2(int NW);

ROB * InitializeROBEntry(Instruction * instruction);
ROB * InitializeROBEntry2(Instruction * instruction);
CompletedInstruction **execute(int NB);

void insertintoWriteBackBuffer(int NB);
int writeBackUnit(int NB, int returncount);
void updateOutputRES(CompletedInstruction *instruction);
int commitInstuctionCount();
int Commit(int NC, int NR, int returncount);
int CommitUnit(int NB, int NR);
int checkEnd();
int checkEnd2();


/**
 * This method initializes CPU data structures and all its data members
 */
void initializeCPU (int NI, int NR) {
	int i;

	cpu = (CPU *) malloc (sizeof(CPU));

	cpu -> cycle = 0;
	cpu -> PC = instructionCacheBaseAddress;
	cpu -> PC2 = instructionCacheBaseAddress;

	 //initialize integer registers
	cpu -> integerRegisters = (INTReg **) malloc (sizeof(INTReg *) * numberOfIntRegisters);
	cpu -> integerRegisters2 = (INTReg **) malloc (sizeof(INTReg *) * numberOfIntRegisters);

	for (i = 0; i < numberOfIntRegisters; i++) {
		cpu -> integerRegisters [i] = (INTReg *) malloc (sizeof(INTReg));
		cpu -> integerRegisters [i] -> data = 0;
		cpu -> integerRegisters [i] -> intResult = 0;
		cpu -> integerRegisters2 [i] = (INTReg *) malloc (sizeof(INTReg));
		cpu -> integerRegisters2 [i] -> data = 0;
		cpu -> integerRegisters2 [i] -> intResult = 0;
 	}

	 //initialize floating point registers
	cpu -> floatingPointRegisters = (FPReg **) malloc (sizeof(FPReg *) * numberOfFPRegisters);
	cpu -> floatingPointRegisters2 = (FPReg **) malloc (sizeof(FPReg *) * numberOfFPRegisters);

	for (i = 0; i < numberOfFPRegisters; i++) {
		cpu -> floatingPointRegisters [i] = (FPReg *) malloc (sizeof(FPReg));
		cpu -> floatingPointRegisters [i] -> data = 0.0;
		cpu -> floatingPointRegisters [i] -> fpResult = 0;
		cpu -> floatingPointRegisters2 [i] = (FPReg *) malloc (sizeof(FPReg));
		cpu -> floatingPointRegisters2 [i] -> data = 0.0;
		cpu -> floatingPointRegisters2 [i] -> fpResult = 0;
 	}


	//initialize integer registers status
	cpu -> IntRegStatus = (RegStatus **) malloc (sizeof(RegStatus *) * numberOfIntRegisters);
    cpu -> IntRegStatus2 = (RegStatus **) malloc (sizeof(RegStatus *) * numberOfIntRegisters);
	for (i = 0; i < numberOfIntRegisters; i++) {
		cpu -> IntRegStatus [i] = (RegStatus *) malloc (sizeof(RegStatus));
		cpu -> IntRegStatus [i] -> reorderNum = -1;
		cpu -> IntRegStatus [i] -> busy = 0;
		cpu -> IntRegStatus2 [i] = (RegStatus *) malloc (sizeof(RegStatus));
		cpu -> IntRegStatus2 [i] -> reorderNum = -1;
		cpu -> IntRegStatus2 [i] -> busy = 0;
 	}

 	//initialize floating point registers status
	cpu -> FPRegStatus = (RegStatus **) malloc (sizeof(RegStatus *) * numberOfFPRegisters);
    cpu -> FPRegStatus2 = (RegStatus **) malloc (sizeof(RegStatus *) * numberOfFPRegisters);
	for (i = 0; i < numberOfFPRegisters; i++) {
		cpu -> FPRegStatus [i] = (RegStatus *) malloc (sizeof(RegStatus));
		cpu -> FPRegStatus [i] -> reorderNum = -1;
		cpu -> FPRegStatus [i] -> busy = 0;
		cpu -> FPRegStatus2 [i] = (RegStatus *) malloc (sizeof(RegStatus));
		cpu -> FPRegStatus2 [i] -> reorderNum = -1;
		cpu -> FPRegStatus2 [i] -> busy = 0;
 	}

    //initialize Pipelines with bubbles and appropriate size
    cpu -> INTPipeline = createCircularQueue(1);
    cpu -> MULTPipeline = createCircularQueue(4);
    cpu -> LoadStorePipeline = createCircularQueue(1);
    cpu -> FPaddPipeline = createCircularQueue(3);
    cpu -> FPmultPipeline = createCircularQueue(4);
    cpu -> FPdivPipeline = createCircularQueue(8);
    cpu -> FPdivPipelineBusy = 0;
    cpu -> BUPipeline = createCircularQueue(1);
    for (i = 0; i < 7; i++) {
        if (i < 2) {
            enqueueCircular (cpu -> FPaddPipeline, NULL);
        }
        if (i < 3) {
            enqueueCircular (cpu -> MULTPipeline, NULL);
            enqueueCircular (cpu -> FPmultPipeline, NULL);
	    }
	    enqueueCircular (cpu -> FPdivPipeline, NULL);
    }
    //Initialize fetch buffer and instruction queue
	cpu -> fetchBuffer = createDictionary (getHashCodeFromPCHash, compareInstructions);
	cpu -> fetchBufferResult = createDictionary (getHashCodeFromPCHash, compareInstructions);
	cpu -> instructionQueue = createCircularQueue(NI);
	cpu -> instructionQueueResult = createCircularQueue(NI);
	cpu -> fetchBuffer2 = createDictionary (getHashCodeFromPCHash, compareInstructions);
	cpu -> fetchBufferResult2 = createDictionary (getHashCodeFromPCHash, compareInstructions);
	cpu -> instructionQueue2 = createCircularQueue(NI);
	cpu -> instructionQueueResult2 = createCircularQueue(NI);
	//Initialize BTB
	cpu -> branchTargetBuffer = createDictionary(getHashCodeFromBranchAddress, compareTargetAddress);
	cpu -> branchTargetBuffer2 = createDictionary(getHashCodeFromBranchAddress, compareTargetAddress);

    //Flag for next fetch unit
	cpu -> stallNextFetch = 0;
	cpu -> stallNextFetch2 = 0;
    //Initialize renaming register
	cpu -> renameRegInt = createDictionary(getHashCodeFromRegNumber, compareRegNumber);
	cpu -> renameRegFP = createDictionary(getHashCodeFromRegNumber, compareRegNumber);
	cpu -> renameRegInt2 = createDictionary(getHashCodeFromRegNumber, compareRegNumber);
	cpu -> renameRegFP2 = createDictionary(getHashCodeFromRegNumber, compareRegNumber);

    //Initialize reservation stations and load/store buffers
    cpu -> resStaInt = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaMult = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> loadBuffer = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> storeBuffer = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaFPadd = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaFPmult = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaFPdiv = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaBU = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaIntResult = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaMultResult = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> loadBufferResult = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> storeBufferResult = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaFPaddResult = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaFPmultResult = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaFPdivResult = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaBUResult = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
 
    //Initialize Stall counters
    cpu -> stallFullROB = 0;
    cpu -> stallFullROB2 = 0;
    cpu -> stallFullRS = 0;

	// Initialize WB and ROB
	cpu -> reorderBuffer = createCircularQueue(NR);
	cpu -> WriteBackBuffer = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
	cpu -> reorderBuffer2 = createCircularQueue(NR);
	

    //Initialize Flag of instructions after branch
    cpu -> isAfterBranch = 0;
    cpu -> isAfterBranch2 = 0;
    //Initialize flag of program that last next cycle fetched and next cycle decodes
    cpu->lastCycleFetchProgram = 0;
    cpu->nextCycleDecodeProgram = 0;

}

//Fetch Instructions Unit
int fetchMultiInstructionUnit(int NF){

    int i;

    if (cpu -> stallNextFetch == 0){
        for(i=0; i<NF; i++){
            if (cpu -> PC >= (instructionCacheBaseAddress + (cacheLineSize * numberOfInstruction))) { //check whether PC exceeds last instruction in cache
                printf ("All instructions are fetched...\n");
                return 0;
            }

            //*((int*)addrPtr) = cpu -> PC;
            int *addrPtr = (int*) malloc(sizeof(int));
            *addrPtr = cpu -> PC;

            DictionaryEntry *currentInstruction = getValueChainByDictionaryKey (instructionCache, addrPtr);

            char *instruction_str = (char *) malloc (sizeof(char) * MAX_LINE);
            strcpy (instruction_str, ((char*)currentInstruction -> value -> value));

            printf ("Fetched %d:%s\n", *addrPtr, instruction_str);

            addDictionaryEntry (cpu -> fetchBufferResult, addrPtr, instruction_str);

            //check if in BTB
            if (cpu -> branchTargetBuffer != NULL){
                DictionaryEntry *BTBEntry = getValueChainByDictionaryKey(cpu -> branchTargetBuffer, addrPtr);

                if (BTBEntry != NULL){

                    if (*((int*)BTBEntry -> key) == *addrPtr){
                        printf("Instruction %d is a branch in the BranchTargetBuffer with ", *addrPtr);
                        int targetAddress = *((int*)BTBEntry -> value -> value);

                        printf("target address %d.\n", targetAddress);
                        cpu -> PC = targetAddress;
                        return 1;
                    }
                }
            }
            cpu -> PC = cpu -> PC + 4;
        }
    }
    else{
        cpu -> stallNextFetch = 0;
        printf("Fetching stall in this cycle...\n");
    }
    return 1;
}

//Fetch Instructions Unit for part 2
int fetchMultiInstructionUnit2(int NF){

    int i;

    if (cpu -> stallNextFetch2 == 0){
        for(i=0; i<NF; i++){
            if (cpu -> PC2 >= (instructionCacheBaseAddress + (cacheLineSize * numberOfInstruction2))) { //check whether PC exceeds last instruction in cache
                printf ("All instructions are fetched...\n");
                return 0;
            }

            //*((int*)addrPtr) = cpu -> PC;
            int *addrPtr = (int*) malloc(sizeof(int));
            *addrPtr = cpu -> PC2;

            DictionaryEntry *currentInstruction = getValueChainByDictionaryKey (instructionCache2, addrPtr);

            char *instruction_str = (char *) malloc (sizeof(char) * MAX_LINE);
            strcpy (instruction_str, ((char*)currentInstruction -> value -> value));

            printf ("Fetched %d:%s\n", *addrPtr, instruction_str);

            addDictionaryEntry (cpu -> fetchBufferResult2, addrPtr, instruction_str);

            //check if in BTB
            if (cpu -> branchTargetBuffer2 != NULL){
                DictionaryEntry *BTBEntry = getValueChainByDictionaryKey(cpu -> branchTargetBuffer2, addrPtr);

                if (BTBEntry != NULL){

                    if (*((int*)BTBEntry -> key) == *addrPtr){
                        printf("Instruction %d is a branch in the BranchTargetBuffer with ", *addrPtr);
                        int targetAddress = *((int*)BTBEntry -> value -> value);

                        printf("target address %d.\n", targetAddress);
                        cpu -> PC2 = targetAddress;
                        return 1;
                    }
                }
            }
            cpu -> PC2 = cpu -> PC2 + 4;
        }
    }
    else{
        cpu -> stallNextFetch2 = 0;
        printf("Fetching stall in this cycle...\n");
    }
    return 1;
}


//Decode instruction unit
int decodeInstructionsUnit(){
    while(cpu -> fetchBuffer -> head!= NULL){
        if ((cpu->instructionQueue->count + cpu->instructionQueueResult->count) >= cpu->instructionQueue->size){
            printf("Cannot decode all fetched instructions because the instruction queue is full.\n");
            cpu -> stallNextFetch = 1;
            return 1;
        }else{
            DictionaryEntry *instructionEntry;
            Instruction *instruction;
            instructionEntry = popDictionaryEntry(cpu -> fetchBuffer);
            instruction = decodeInstruction(instructionEntry -> value -> value, *((int*)instructionEntry -> key));
            instruction->isProg2 = 0;
            enqueueCircular(cpu -> instructionQueueResult, instruction);	
        }

    }
    return 1;
}


//Decode instruction unit for part 2
int decodeInstructionsUnit2(){
    while(cpu -> fetchBuffer2 -> head!= NULL){
        if ((cpu->instructionQueue2->count + cpu->instructionQueueResult2->count) >= cpu->instructionQueue2->size){
            printf("Cannot decode all fetched instructions because the instruction queue is full.\n");
            cpu -> stallNextFetch2 = 1;
            return 1;
        }else{
            DictionaryEntry *instructionEntry;
            Instruction *instruction;
            instructionEntry = popDictionaryEntry(cpu -> fetchBuffer2);
            instruction = decodeInstruction(instructionEntry -> value -> value, *((int*)instructionEntry -> key));
            instruction->isProg2 = 1;
            enqueueCircular(cpu -> instructionQueueResult2, instruction);
        }

    }
    return 1;
}

//Update fetch buffer
void updateFetchBuffer(){
    DictionaryEntry *instructionEntry;
    while((instructionEntry = popDictionaryEntry(cpu -> fetchBufferResult)) != NULL){
        appendDictionaryEntry(cpu -> fetchBuffer, instructionEntry);
    }
    return;
}

//Update fetch buffer for part 2
void updateFetchBuffer2(){
    DictionaryEntry *instructionEntry;
    while((instructionEntry = popDictionaryEntry(cpu -> fetchBufferResult2)) != NULL){
        appendDictionaryEntry(cpu -> fetchBuffer2, instructionEntry);
    }
    return;
}

//Update instruction queue
void updateInstructionQueue(){
    Instruction *instruction;
    while((instruction = dequeueCircular(cpu->instructionQueueResult))!= NULL){
        enqueueCircular(cpu->instructionQueue, instruction);
    }
    return;
}

//Update instruction queue
void updateInstructionQueue2(){
    Instruction *instruction;
    while((instruction = dequeueCircular(cpu->instructionQueueResult2))!= NULL){
        enqueueCircular(cpu->instructionQueue2, instruction);
    }
    return;
}

//Update reservation stations
void updateReservationStations(){
    DictionaryEntry *RSEntry;
    while((RSEntry = popDictionaryEntry(cpu -> resStaIntResult)) != NULL){
        appendDictionaryEntry(cpu -> resStaInt, RSEntry);
    }
    while((RSEntry = popDictionaryEntry(cpu -> resStaMultResult)) != NULL){
        appendDictionaryEntry(cpu -> resStaMult, RSEntry);
    }
    while((RSEntry = popDictionaryEntry(cpu -> loadBufferResult)) != NULL){
        appendDictionaryEntry(cpu -> loadBuffer, RSEntry);
    }
    while((RSEntry = popDictionaryEntry(cpu -> storeBufferResult)) != NULL){
        appendDictionaryEntry(cpu -> storeBuffer, RSEntry);
    }
    while ((RSEntry = popDictionaryEntry (cpu -> loadBufferResult)) != NULL) {
        appendDictionaryEntry (cpu -> loadBuffer, RSEntry);
    }
    while ((RSEntry = popDictionaryEntry (cpu -> storeBufferResult)) != NULL) {
        appendDictionaryEntry (cpu -> storeBuffer, RSEntry);
    }
    while((RSEntry = popDictionaryEntry(cpu -> resStaFPaddResult)) != NULL){
        appendDictionaryEntry(cpu -> resStaFPadd, RSEntry);
    }
    while((RSEntry = popDictionaryEntry(cpu -> resStaFPmultResult)) != NULL){
        appendDictionaryEntry(cpu -> resStaFPmult, RSEntry);
    }
    while((RSEntry = popDictionaryEntry(cpu -> resStaFPdivResult)) != NULL){
        appendDictionaryEntry(cpu -> resStaFPdiv, RSEntry);
    }
    while((RSEntry = popDictionaryEntry(cpu -> resStaBUResult)) != NULL){
        appendDictionaryEntry(cpu -> resStaBU, RSEntry);
    }
    return;
}

//Decode an instruction
Instruction * decodeInstruction(char *instruction_str, int instructionAddress){
    Instruction *instruction;

    char *token = (char *) malloc (sizeof(char) * MAX_LINE);

    OpCode op;

    int rd;
    int rs;
    int rt;

    int rsValue;
    int rtValue;

    int fd;
    int fs;
    int ft;

    double fsValue;
    double ftValue;

    int immediate;

    int target;


	op = NOOP, rd = -1, rs = -1, rt = -1, rsValue = -1, rtValue = -1, fd = -1, fs = -1, ft = -1, fsValue = -1, ftValue = -1, immediate = 0, target = 0;

	token = (char *) malloc (sizeof(char) * MAX_LINE);

	strcpy (token, instruction_str);

	token = strtok(token, " ,()\t\n");

	if(strcmp(token, "AND") == 0) {
		op = AND;

		token = strtok(NULL, " ,()RF\t\n");
		rd = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		rs = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		rt = atoi(token);
	} else if(strcmp(token, "ANDI") == 0) {
		op = ANDI;

		token = strtok(NULL, " ,()RF\t\n");
		rd = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		rs = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		immediate = atoi(token);
	} else if(strcmp(token, "OR") == 0) {
		op = OR;

		token = strtok(NULL, " ,()RF\t\n");
		rd = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		rs = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		rt = atoi(token);
	} else if(strcmp(token, "ORI") == 0) {
		op = ORI;

		token = strtok(NULL, " ,()RF\t\n");
		rd = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		rs = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		immediate = atoi(token);
	} else if(strcmp(token, "SLT") == 0) {
		op = SLT;
		token = strtok(NULL, " ,()RF\t\n");
		rd = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		rs = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		rt = atoi(token);
	} else if(strcmp(token, "SLTI") == 0) {
		op = SLTI;

		token = strtok(NULL, " ,()RF\t\n");
		rd = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		rs = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		immediate = atoi(token);
	} else if(strcmp(token, "DADD") == 0) {
		op = DADD;

		token = strtok(NULL, " ,()RF\t\n");
		rd = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		rs = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		rt = atoi(token);
	} else if(strcmp(token, "DADDI") == 0) {
		op = DADDI;

		token = strtok(NULL, " ,()RF\t\n");
		rd = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		rs = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");

		immediate = atoi(token);
	} else if(strcmp(token, "DSUB") == 0) {
		op = DSUB;

		token = strtok(NULL, " ,()RF\t\n");
		rd = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		rs = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		rt = atoi(token);
	} else if(strcmp(token, "DMUL") == 0) {
		op = DMUL;

		token = strtok(NULL, " ,()RF\t\n");
		rd = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		rs = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		rt = atoi(token);
	} else if(strcmp(token, "LD") == 0) {
		op = LD;

		token = strtok(NULL, " ,()RF\t\n");
		rt = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		immediate = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		rs = atoi(token);
	} else if(strcmp(token, "SD") == 0) {
		op = SD;

		token = strtok(NULL, " ,()RF\t\n");
		rt = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		immediate = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		rs = atoi(token);
	} else if(strcmp(token, "L.D") == 0) {
		op = L_D;

		token = strtok(NULL, " ,()RF\t\n");
		ft = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		immediate = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		rs = atoi(token);
	} else if(strcmp(token, "S.D") == 0) {
		op = S_D;

		token = strtok(NULL, " ,()RF\t\n");
		ft = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		immediate = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		rs = atoi(token);
	} else if(strcmp(token, "ADD.D") == 0) {
		op = ADD_D;

		token = strtok(NULL, " ,()RF\t\n");
		fd = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		fs = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		ft = atoi(token);
	} else if(strcmp(token, "SUB.D") == 0) {
		op = SUB_D;

		token = strtok(NULL, " ,()RF\t\n");
		fd = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		fs = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		ft = atoi(token);
	} else if(strcmp(token, "MUL.D") == 0) {
		op = MUL_D;

		token = strtok(NULL, " ,()RF\t\n");
		fd = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		fs = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		ft = atoi(token);
	} else if(strcmp(token, "DIV.D") == 0) {
		op = DIV_D;

		token = strtok(NULL, " ,()RF\t\n");
		fd = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		fs = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		ft = atoi(token);
	} else if(strcmp(token, "BEQZ") == 0) {
		op = BEQZ;

		token = strtok(NULL, " ,()RF\t\n");
		rs = atoi(token);

		token = strtok(NULL, " ,()\t\n");
	} else if(strcmp(token, "BNEZ") == 0) {
		op = BNEZ;

		token = strtok(NULL, " ,()RF\t\n");
		rs = atoi(token);

		token = strtok(NULL, " ,()\t\n");
	} else if(strcmp(token, "BEQ") == 0) {
		op = BEQ;

		token = strtok(NULL, " ,()RF\t\n");
		rs = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		rt = atoi(token);

		token = strtok(NULL, " ,()\t\n");
	} else if(strcmp(token, "BNE") == 0) {
		op = BNE;

		token = strtok(NULL, " ,()RF\t\n");
		rs = atoi(token);

		token = strtok(NULL, " ,()RF\t\n");
		rt = atoi(token);

		token = strtok(NULL, " ,()\t\n");
	} else {
		printf("Invalid instruction %s...\n", instruction_str);
		exit (EXIT_FAILURE);
	}

	if (op == BEQZ || op == BNEZ || op == BEQ || op == BNE) {
		DictionaryEntry *codeLabel = getValueChainByDictionaryKey (codeLabels, (void *) token);

		if (codeLabel == NULL) {
			printf("Invalid code label cannot be resolved...\n");
			exit (EXIT_FAILURE);
		} else {
			target = *((int*)codeLabel -> value -> value);
		}
	}

	instruction = (Instruction *) malloc (sizeof(Instruction));

	instruction -> op = op;

	instruction -> rd = rd;
	instruction -> rs = rs;
	instruction -> rt = rt;

	instruction -> rsValue = rsValue;
	instruction -> rtValue = rtValue;

	instruction -> fd = fd;
	instruction -> fs = fs;
	instruction -> ft = ft;

	instruction -> fsValue = fsValue;
	instruction -> ftValue = ftValue;

	instruction -> immediate = immediate;

	instruction -> target = target;

	instruction -> address = instructionAddress;

	printf("Decoded %d:%s -> %s, rd=%d, rs=%d, rt=%d, fd=%d, fs=%d, ft=%d, immediate=%d, target=%d\n", instruction -> address, instruction_str,
		 getOpcodeString ((int) op), rd, rs, rt, fd, fs, ft, immediate, target);

    return instruction;
}

//Check if renaming registers is full
int renameRegIsFull(Dictionary *renameReg, int d){
    int counterRenameReg;
    counterRenameReg = countDictionaryLen(renameReg);
    if (counterRenameReg == numberRenameReg){
        DictionaryEntry *renameRegDest = getValueChainByDictionaryKey(renameReg, &d);
        if (renameRegDest == NULL){
            printf("Stall during IssueUnit because renaming register is full.\n");
            return 1;
        }
    }
    return 0;
}

//Add instruction to reservation stations of type integer
int addInstruction2RSint(Dictionary *renameRegInt, Dictionary *resSta, Dictionary *resStaResult,
                      char* rsType, int maxLenRS, Instruction *instruction, RegStatus **IntRegStatus){
    int counterUnit;
    int counterUnitResult;
    RSint* RS = (RSint*) malloc (sizeof(RSint));
    counterUnit = countDictionaryLen(resSta);
    counterUnitResult = countDictionaryLen(resStaResult);
    if (maxLenRS - counterUnit - counterUnitResult > 0){
        int DestROBnum = cpu -> reorderBuffer->tail;
        RegStatus *RegStatusEntry = IntRegStatus[instruction->rs];
        if (RegStatusEntry->busy == 1){
            int robNum = RegStatusEntry -> reorderNum;
            if (((ROB *)cpu -> reorderBuffer -> items[robNum]) -> isReady == 1){
                DictionaryEntry *renameRegIntEntry = getValueChainByDictionaryKey(renameRegInt, &(RegStatusEntry -> reorderNum));
                RS -> Vj = *((int *)renameRegIntEntry->value->value);
                RS -> Qj = -1;
                RS->isReady = 1;
            }
            else{
                RS -> Qj = robNum;
                RS->isReady = 0;
            }
        }else{
            RS -> Vj = cpu->integerRegisters[instruction->rs]->data;
            RS -> Qj = -1;
            RS->isReady = 1;
        }
        if (instruction->rt >= 0){
            RegStatus *RegStatusEntry = IntRegStatus[instruction->rt];
            if (RegStatusEntry->busy == 1){
                int robNum = RegStatusEntry -> reorderNum;
                if (((ROB *)cpu -> reorderBuffer -> items[robNum]) -> isReady == 1){
                    DictionaryEntry *renameRegIntEntry = getValueChainByDictionaryKey(renameRegInt, &(RegStatusEntry -> reorderNum));
                    RS -> Vk = *((int *)renameRegIntEntry->value->value);
                    RS -> Qk = -1;
                    if(RS->isReady == 1)
                        RS->isReady = 1;
                }
                else{
                    RS -> Qk = robNum;
                    RS->isReady = 0;
                }
            }else{
                RS -> Vk = cpu->integerRegisters[instruction->rt]->data;
                RS -> Qk = -1;
                if(RS->isReady == 1)
                    RS->isReady = 1;
            }
        }else{
            RS -> Qk = -1;
        }
        //Append to reservation stations
        KeyRS *keyRS = (KeyRS*)malloc(sizeof(KeyRS));
        keyRS->reorderNum = DestROBnum;
        keyRS->progNum = instruction->isProg2 + 1;
        RS->Dest = DestROBnum;
        RS->instruction = instruction;
        RS->isExecuting = 0;
        addDictionaryEntry(resStaResult, keyRS, RS);
        //Update register status
        RegStatusEntry = IntRegStatus[instruction->rd];
        RegStatusEntry->busy = 1;
        RegStatusEntry->reorderNum = DestROBnum;
        //Add to renaming registers
        void *valuePtr = malloc(sizeof(int));
	    *((int*)valuePtr) = 0;
        removeDictionaryEntriesByKey(renameRegInt, &(RS->Dest));
        addDictionaryEntry(renameRegInt, &(RS->Dest), valuePtr);
        printf("Issued instruction %d: %s\n", instruction->address, getOpcodeString ((int) instruction->op));
        return 1;

    }else{
        cpu -> stallFullRS ++;
        printf("Stall during IssueUnit because reservation stations %s is full.\n", rsType);
        return 0;
    }
}

//Add instruction to reservation stations of type floating pointer
int addInstruction2RSfloat(Dictionary *renameRegFP, Dictionary *resSta, Dictionary *resStaResult,
                      char* rsType, int maxLenRS, Instruction *instruction, RegStatus **FPRegStatus){
    int counterUnit;
    int counterUnitResult;
    RSfloat* RS = (RSfloat*) malloc (sizeof(RSfloat));
    counterUnit = countDictionaryLen(resSta);
    counterUnitResult = countDictionaryLen(resStaResult);
    if (maxLenRS - counterUnit - counterUnitResult > 0){
        int DestROBnum = cpu -> reorderBuffer->tail;
        RegStatus *RegStatusEntry = FPRegStatus[instruction->fs];
        if (RegStatusEntry->busy == 1){
            int robNum = RegStatusEntry -> reorderNum;
            if (((ROB *)cpu -> reorderBuffer -> items[robNum]) -> isReady == 1){
                DictionaryEntry *renameRegFPEntry = getValueChainByDictionaryKey(renameRegFP, &(RegStatusEntry -> reorderNum));
                RS -> Vj = *((double *)renameRegFPEntry->value->value);
                RS -> Qj = -1;
                RS->isReady = 1;
            }
            else{
                RS -> Qj = robNum;
                RS->isReady = 0;
            }
        }else{
            RS -> Vj = cpu->floatingPointRegisters[instruction->fs]->data;
            RS -> Qj = -1;
            RS->isReady = 1;
        }
        if (instruction->ft >= 0){
            RegStatus *RegStatusEntry = FPRegStatus[instruction->ft];
            if (RegStatusEntry->busy == 1){
                int robNum = RegStatusEntry -> reorderNum;
                if (((ROB *)cpu -> reorderBuffer -> items[robNum]) -> isReady == 1){
                    DictionaryEntry *renameRegFPEntry = getValueChainByDictionaryKey(renameRegFP, &(RegStatusEntry -> reorderNum));
                    RS -> Vk = *((double *)renameRegFPEntry->value->value);
                    RS -> Qk = -1;
                    if(RS->isReady == 1)
                        RS->isReady = 1;
                }
                else{
                    RS -> Qk = robNum;
                    RS->isReady = 0;
                }
            }else{
                RS -> Vk = cpu->floatingPointRegisters[instruction->ft]->data;
                RS -> Qk = -1;
                if(RS->isReady == 1)
                    RS->isReady = 1;
            }
        }else{
            RS -> Qk = -1;
        }
         //Append to reservation stations
        KeyRS *keyRS = (KeyRS*)malloc(sizeof(KeyRS));
        keyRS->reorderNum = DestROBnum;
        keyRS->progNum = instruction->isProg2 + 1;
        RS->Dest = DestROBnum;
        RS->instruction = instruction;
        RS->isExecuting = 0;
        addDictionaryEntry(resStaResult, keyRS, RS);
        //Update register status
        RegStatusEntry = FPRegStatus[instruction->fd];
        RegStatusEntry->busy = 1;
        RegStatusEntry->reorderNum = DestROBnum;
        //Add to renaming registers
        void *valuePtr = malloc(sizeof(double));
	    *((double*)valuePtr) = 0;
        removeDictionaryEntriesByKey(renameRegFP, &(RS->Dest));
        addDictionaryEntry(renameRegFP, &(RS->Dest), valuePtr);
        printf("Issued instruction %d: %s\n", instruction->address, getOpcodeString ((int) instruction->op));
        return 1;
    }else{
        cpu -> stallFullRS ++;
        printf("Stall during IssueUnit because reservation stations %s is full.\n", rsType);
        return 0;
    }
}

//Add instruction to reservation stations of type branch
int addInstruction2RSbranch(Dictionary *renameRegInt, Dictionary *resSta, Dictionary *resStaResult, char* rsType, int maxLenRS,
                            Instruction *instruction, RegStatus **IntRegStatus){
    int counterUnit;
    int counterUnitResult;
    RSint* RS = (RSint*) malloc (sizeof(RSint));
    counterUnit = countDictionaryLen(resSta);
    counterUnitResult = countDictionaryLen(resStaResult);
    if (maxLenRS - counterUnit - counterUnitResult > 0){
        int DestROBnum = cpu -> reorderBuffer->tail;
        RegStatus *RegStatusEntry = IntRegStatus[instruction->rs];
        if (RegStatusEntry->busy == 1){
            int robNum = RegStatusEntry -> reorderNum;
            if (((ROB *)cpu -> reorderBuffer -> items[robNum]) -> isReady == 1){
                DictionaryEntry *renameRegIntEntry = getValueChainByDictionaryKey(renameRegInt, &(RegStatusEntry -> reorderNum));
                RS -> Vj = *((int *)renameRegIntEntry->value->value);
                RS -> Qj = -1;
                RS->isReady = 1;
            }
            else{
                RS -> Qj = robNum;
                RS->isReady = 0;
            }
        }else{
            RS -> Vj = cpu->integerRegisters[instruction->rs]->data;
            RS -> Qj = -1;
            RS->isReady = 1;
        }
        if (instruction->rt >= 0){
            RegStatus *RegStatusEntry = IntRegStatus[instruction->rt];
            if (RegStatusEntry->busy == 1){
                int robNum = RegStatusEntry -> reorderNum;
                if (((ROB *)cpu -> reorderBuffer -> items[robNum]) -> isReady == 1){
                    DictionaryEntry *renameRegIntEntry = getValueChainByDictionaryKey(renameRegInt, &(RegStatusEntry -> reorderNum));
                    RS -> Vk = *((int *)renameRegIntEntry->value->value);
                    RS -> Qk = -1;
                    if(RS->isReady == 1)
                        RS->isReady = 1;
                }
                else{
                    RS -> Qk = robNum;
                    RS->isReady = 0;
                }
            }else{
                RS -> Vk = cpu->integerRegisters[instruction->rt]->data;
                RS -> Qk = -1;
                if(RS->isReady == 1)
                    RS->isReady = 1;
            }
        }else{
            RS -> Qk = -1;
        }
        //Append to reservation stations
        KeyRS *keyRS = (KeyRS*)malloc(sizeof(KeyRS));
        keyRS->reorderNum = DestROBnum;
        keyRS->progNum = instruction->isProg2 + 1;
        RS->Dest = DestROBnum;
        RS->instruction = instruction;
        RS->isExecuting = 0;
        addDictionaryEntry(resStaResult, keyRS, RS);
        printf("Issued instruction %d: %s\n", instruction->address, getOpcodeString ((int) instruction->op));
        return 1;

    }else{
        cpu -> stallFullRS ++;
        printf("Stall during IssueUnit because reservation stations %s is full.\n", rsType);
        return 0;
    }
}

/**
 * Takes an instruction and adds it to the given reservation station.
 * @return int 1 if successfully added, 0 otherwise.
 */
int addLoadStore2Buffer(Dictionary *LOrSBuffer, Dictionary *LOrSBufferResult,
                         char *buffType, int maxLenBuff, Instruction *instruction) {
    int counterUnit;
    int counterUnitResult;
    RSmem* RS = (RSmem*) malloc (sizeof(RSmem));
    counterUnit = countDictionaryLen(LOrSBuffer);
    counterUnitResult = countDictionaryLen(LOrSBufferResult);
//    if (strcmp(buffType, "Load") == 0 && countDictionaryLen (cpu -> storeBuffer) != 0) {
//        printf("Stall Load during Issue Unit because of earlier store in queue.\n");
//        return 0;
//    }
    if (maxLenBuff - counterUnit - counterUnitResult > 0){
        RS -> isReady = 1; //Will be set to 0 later if necessary
        int DestROBnum = cpu -> reorderBuffer->tail;
        RegStatus *RegStatusEntry = cpu -> IntRegStatus[instruction->rs];
        if (RegStatusEntry -> busy == 1) {
            int robNum = RegStatusEntry -> reorderNum;
            if (((ROB *)cpu -> reorderBuffer -> items[robNum]) -> isReady == 1){
                DictionaryEntry *renameRegIntEntry = getValueChainByDictionaryKey(cpu -> renameRegInt, &(RegStatusEntry -> reorderNum));
                RS -> Vj = *((int *)renameRegIntEntry -> value -> value);
                RS -> Qj = -1;
            }
            else{
                RS -> Qj = robNum;
                RS->isReady = 0;
            }
        }else{
            RS -> Vj = cpu->integerRegisters[instruction->rs]->data;
            RS -> Qj = -1;
        }
        RS->Dest = DestROBnum;
        RS->instruction = instruction;
        if (strcmp(buffType, "Store") == 0) {
            if (instruction -> op == S_D) {
                RegStatusEntry = cpu -> FPRegStatus[instruction -> ft];
                if (RegStatusEntry -> busy == 1){
                    int robNum = RegStatusEntry -> reorderNum;
                    if (((ROB *)cpu -> reorderBuffer -> items[robNum]) -> isReady == 1){
                        DictionaryEntry *renameRegFloatEntry = getValueChainByDictionaryKey (cpu -> renameRegFP, &instruction->ft);
                        RS -> fpVk = *((int *)renameRegFloatEntry -> value -> value);
                        RS -> Qk = -1;
                    }
                    else{
                        RS -> Qk = robNum;
                        RS->isReady = 0;
                    }
                }else{
                    RS -> fpVk = cpu -> floatingPointRegisters[instruction->ft] -> data;
                    RS -> Qk = -1;
                }
            } else {
                RegStatusEntry = cpu -> IntRegStatus[instruction -> rt];
                if (RegStatusEntry -> busy == 1){
                    int robNum = RegStatusEntry -> reorderNum;
                    if (((ROB *)cpu -> reorderBuffer -> items[robNum]) -> isReady == 1){
                        DictionaryEntry *renameRegIntEntry = getValueChainByDictionaryKey (cpu -> renameRegInt, &instruction->rt);
                        RS -> iVk = *((int *)renameRegIntEntry -> value -> value);
                        RS -> Qk = -1;
                    }
                    else{
                        RS -> Qk = robNum;
                        RS->isReady = 0;
                    }
                }else{
                    RS -> iVk = cpu -> integerRegisters[instruction->rt] -> data;
                    RS -> Qk = -1;
                }
            }
        }
        RS -> address = -1;
        RS->isExecuting = 0;
        KeyRS *keyRS = (KeyRS*)malloc(sizeof(KeyRS));
        keyRS->reorderNum = DestROBnum;
        keyRS->progNum = instruction->isProg2 + 1;
        addDictionaryEntry(LOrSBufferResult, keyRS, RS);
        //Add to renaming registers and update Register Status if load
        if (strcmp(buffType, "Load") == 0) {
            if (instruction -> op == L_D) {
                RegStatusEntry = cpu -> FPRegStatus[instruction -> ft];
                RegStatusEntry->busy = 1;
        		RegStatusEntry->reorderNum = DestROBnum;
                void *valuePtr = malloc(sizeof(double));
                *((double*)valuePtr) = 0;
                removeDictionaryEntriesByKey(cpu -> renameRegFP, &(RS -> Dest));
                addDictionaryEntry(cpu -> renameRegFP, &(RS -> Dest), valuePtr);
            } else {
                RegStatusEntry = cpu -> FPRegStatus[instruction -> rt];
                RegStatusEntry->busy = 1;
        		RegStatusEntry->reorderNum = DestROBnum;
                void *valuePtr = malloc(sizeof(int));
                *((int*)valuePtr) = 0;
                removeDictionaryEntriesByKey(cpu -> renameRegInt, &(RS->Dest));
                addDictionaryEntry(cpu -> renameRegInt, &(RS->Dest), valuePtr);
            }

        }
        printf("Issued instruction %d: %s\n", instruction->address, getOpcodeString ((int) instruction->op));
        return 1;
    } else {
        cpu -> stallFullRS ++;
        printf("Stall during IssueUnit because %s buffer is full.\n", buffType);
        return 0;
    }
}

//Add instruction to reservation stations of type integer for part 2
int addInstruction2RSint2(Dictionary *renameRegInt, Dictionary *resSta, Dictionary *resStaResult,
                      char* rsType, int maxLenRS, Instruction *instruction, RegStatus **IntRegStatus){
    int counterUnit;
    int counterUnitResult;
    RSint* RS = (RSint*) malloc (sizeof(RSint));
    counterUnit = countDictionaryLen(resSta);
    counterUnitResult = countDictionaryLen(resStaResult);
    if (maxLenRS - counterUnit - counterUnitResult > 0){
        int DestROBnum = cpu -> reorderBuffer2->tail;
        RegStatus *RegStatusEntry = IntRegStatus[instruction->rs];
        if (RegStatusEntry->busy == 1){
            int robNum = RegStatusEntry -> reorderNum;
            if (((ROB *)cpu -> reorderBuffer2 -> items[robNum]) -> isReady == 1){
                DictionaryEntry *renameRegIntEntry = getValueChainByDictionaryKey(renameRegInt, &(RegStatusEntry -> reorderNum));
                RS -> Vj = *((int *)renameRegIntEntry->value->value);
                RS -> Qj = -1;
                RS->isReady = 1;
            }
            else{
                RS -> Qj = robNum;
                RS->isReady = 0;
            }
        }else{
            RS -> Vj = cpu->integerRegisters2[instruction->rs]->data;
            RS -> Qj = -1;
            RS->isReady = 1;
        }
        if (instruction->rt >= 0){
            RegStatus *RegStatusEntry = IntRegStatus[instruction->rt];
            if (RegStatusEntry->busy == 1){
                int robNum = RegStatusEntry -> reorderNum;
                if (((ROB *)cpu -> reorderBuffer2 -> items[robNum]) -> isReady == 1){
                    DictionaryEntry *renameRegIntEntry = getValueChainByDictionaryKey(renameRegInt, &(RegStatusEntry -> reorderNum));
                    RS -> Vk = *((int *)renameRegIntEntry->value->value);
                    RS -> Qk = -1;
                    if(RS->isReady == 1)
                        RS->isReady = 1;
                }
                else{
                    RS -> Qk = robNum;
                    RS->isReady = 0;
                }
            }else{
                RS -> Vk = cpu->integerRegisters2[instruction->rt]->data;
                RS -> Qk = -1;
                if(RS->isReady == 1)
                    RS->isReady = 1;
            }
        }else{
            RS -> Qk = -1;
        }
        //Append to reservation stations
        KeyRS *keyRS = (KeyRS*)malloc(sizeof(KeyRS));
        keyRS->reorderNum = DestROBnum;
        keyRS->progNum = instruction->isProg2 + 1;
        RS->Dest = DestROBnum;
        RS->instruction = instruction;
        RS->isExecuting = 0;
        addDictionaryEntry(resStaResult, keyRS, RS);
        //Update register status
        RegStatusEntry = IntRegStatus[instruction->rd];
        RegStatusEntry->busy = 1;
        RegStatusEntry->reorderNum = DestROBnum;
        //Add to renaming registers
        void *valuePtr = malloc(sizeof(int));
	    *((int*)valuePtr) = 0;
        removeDictionaryEntriesByKey(renameRegInt, &(RS->Dest));
        addDictionaryEntry(renameRegInt, &(RS->Dest), valuePtr);
        printf("Issued instruction %d: %s\n", instruction->address, getOpcodeString ((int) instruction->op));
        return 1;

    }else{
        cpu -> stallFullRS ++;
        printf("Stall during IssueUnit because reservation stations %s is full.\n", rsType);
        return 0;
    }
}

//Add instruction to reservation stations of type floating pointer for part 2
int addInstruction2RSfloat2(Dictionary *renameRegFP, Dictionary *resSta, Dictionary *resStaResult,
                      char* rsType, int maxLenRS, Instruction *instruction, RegStatus **FPRegStatus){
    int counterUnit;
    int counterUnitResult;
    RSfloat* RS = (RSfloat*) malloc (sizeof(RSfloat));
    counterUnit = countDictionaryLen(resSta);
    counterUnitResult = countDictionaryLen(resStaResult);
    if (maxLenRS - counterUnit - counterUnitResult > 0){
        int DestROBnum = cpu -> reorderBuffer2->tail;
        RegStatus *RegStatusEntry = FPRegStatus[instruction->fs];
        if (RegStatusEntry->busy == 1){
            int robNum = RegStatusEntry -> reorderNum;
            if (((ROB *)cpu -> reorderBuffer2 -> items[robNum]) -> isReady == 1){
                DictionaryEntry *renameRegFPEntry = getValueChainByDictionaryKey(renameRegFP, &(RegStatusEntry -> reorderNum));
                RS -> Vj = *((double *)renameRegFPEntry->value->value);
                RS -> Qj = -1;
                RS->isReady = 1;
            }
            else{
                RS -> Qj = robNum;
                RS->isReady = 0;
            }
        }else{
            RS -> Vj = cpu->floatingPointRegisters2[instruction->fs]->data;
            RS -> Qj = -1;
            RS->isReady = 1;
        }
        if (instruction->ft >= 0){
            RegStatus *RegStatusEntry = FPRegStatus[instruction->ft];
            if (RegStatusEntry->busy == 1){
                int robNum = RegStatusEntry -> reorderNum;
                if (((ROB *)cpu -> reorderBuffer2 -> items[robNum]) -> isReady == 1){
                    DictionaryEntry *renameRegFPEntry = getValueChainByDictionaryKey(renameRegFP, &(RegStatusEntry -> reorderNum));
                    RS -> Vk = *((double *)renameRegFPEntry->value->value);
                    RS -> Qk = -1;
                    if(RS->isReady == 1)
                        RS->isReady = 1;
                }
                else{
                    RS -> Qk = robNum;
                    RS->isReady = 0;
                }
            }else{
                RS -> Vk = cpu->floatingPointRegisters2[instruction->ft]->data;
                RS -> Qk = -1;
                if(RS->isReady == 1)
                    RS->isReady = 1;
            }
        }else{
            RS -> Qk = -1;
        }
         //Append to reservation stations
        KeyRS *keyRS = (KeyRS*)malloc(sizeof(KeyRS));
        keyRS->reorderNum = DestROBnum;
        keyRS->progNum = instruction->isProg2 + 1;
        RS->Dest = DestROBnum;
        RS->instruction = instruction;
        RS->isExecuting = 0;
        addDictionaryEntry(resStaResult, &keyRS, RS);
        //Update register status
        RegStatusEntry = FPRegStatus[instruction->fd];
        RegStatusEntry->busy = 1;
        RegStatusEntry->reorderNum = DestROBnum;
        //Add to renaming registers
        void *valuePtr = malloc(sizeof(double));
	    *((double*)valuePtr) = 0;
        removeDictionaryEntriesByKey(renameRegFP, &(RS->Dest));
        addDictionaryEntry(renameRegFP, &(RS->Dest), valuePtr);
        printf("Issued instruction %d: %s\n", instruction->address, getOpcodeString ((int) instruction->op));
        return 1;
    }else{
        cpu -> stallFullRS ++;
        printf("Stall during IssueUnit because reservation stations %s is full.\n", rsType);
        return 0;
    }
}

//Add instruction to reservation stations of type branch for part 2
int addInstruction2RSbranch2(Dictionary *renameRegInt, Dictionary *resSta, Dictionary *resStaResult, char* rsType, int maxLenRS,
                            Instruction *instruction, RegStatus **IntRegStatus){
    int counterUnit;
    int counterUnitResult;
    RSint* RS = (RSint*) malloc (sizeof(RSint));
    counterUnit = countDictionaryLen(resSta);
    counterUnitResult = countDictionaryLen(resStaResult);
    if (maxLenRS - counterUnit - counterUnitResult > 0){
        int DestROBnum = cpu -> reorderBuffer2->tail;
        RegStatus *RegStatusEntry = IntRegStatus[instruction->rs];
        if (RegStatusEntry->busy == 1){
            int robNum = RegStatusEntry -> reorderNum;
            if (((ROB *)cpu -> reorderBuffer2 -> items[robNum]) -> isReady == 1){
                DictionaryEntry *renameRegIntEntry = getValueChainByDictionaryKey(renameRegInt, &(RegStatusEntry -> reorderNum));
                RS -> Vj = *((int *)renameRegIntEntry->value->value);
                RS -> Qj = -1;
                RS->isReady = 1;
            }
            else{
                RS -> Qj = robNum;
                RS->isReady = 0;
            }
        }else{
            RS -> Vj = cpu->integerRegisters2[instruction->rs]->data;
            RS -> Qj = -1;
            RS->isReady = 1;
        }
        if (instruction->rt >= 0){
            RegStatus *RegStatusEntry = IntRegStatus[instruction->rt];
            if (RegStatusEntry->busy == 1){
                int robNum = RegStatusEntry -> reorderNum;
                if (((ROB *)cpu -> reorderBuffer2 -> items[robNum]) -> isReady == 1){
                    DictionaryEntry *renameRegIntEntry = getValueChainByDictionaryKey(renameRegInt, &(RegStatusEntry -> reorderNum));
                    RS -> Vk = *((int *)renameRegIntEntry->value->value);
                    RS -> Qk = -1;
                    if(RS->isReady == 1)
                        RS->isReady = 1;
                }
                else{
                    RS -> Qk = robNum;
                    RS->isReady = 0;
                }
            }else{
                RS -> Vk = cpu->integerRegisters2[instruction->rt]->data;
                RS -> Qk = -1;
                if(RS->isReady == 1)
                    RS->isReady = 1;
            }
        }else{
            RS -> Qk = -1;
        }
        //Append to reservation stations
        KeyRS *keyRS = (KeyRS*)malloc(sizeof(KeyRS));
        keyRS->reorderNum = DestROBnum;
        keyRS->progNum = instruction->isProg2 + 1;
        RS->Dest = DestROBnum;
        RS->instruction = instruction;
        RS->isExecuting = 0;
        addDictionaryEntry(resStaResult, keyRS, RS);
        printf("Issued instruction %d: %s\n", instruction->address, getOpcodeString ((int) instruction->op));
        return 1;

    }else{
        cpu -> stallFullRS ++;
        printf("Stall during IssueUnit because reservation stations %s is full.\n", rsType);
        return 0;
    }
}

/**
 * Takes an instruction and adds it to the given reservation station for part 2
 * @return int 1 if successfully added, 0 otherwise.
 */
int addLoadStore2Buffer2(Dictionary *LOrSBuffer, Dictionary *LOrSBufferResult,
                         char *buffType, int maxLenBuff, Instruction *instruction) {
    int counterUnit;
    int counterUnitResult;
    RSmem* RS = (RSmem*) malloc (sizeof(RSmem));
    counterUnit = countDictionaryLen(LOrSBuffer);
    counterUnitResult = countDictionaryLen(LOrSBufferResult);
//    if (strcmp(buffType, "Load") == 0 && countDictionaryLen (cpu -> storeBuffer) != 0) {
//        printf("Stall Load during Issue Unit because of earlier store in queue.\n");
//        return 0;
//    }
    if (maxLenBuff - counterUnit - counterUnitResult > 0){
        RS -> isReady = 1; //Will be set to 0 later if necessary
        int DestROBnum = cpu -> reorderBuffer2->tail;
        RegStatus *RegStatusEntry = cpu -> IntRegStatus2[instruction->rs];
        if (RegStatusEntry -> busy == 1) {
            int robNum = RegStatusEntry -> reorderNum;
            if (((ROB *)cpu -> reorderBuffer2 -> items[robNum]) -> isReady == 1){
                DictionaryEntry *renameRegIntEntry = getValueChainByDictionaryKey(cpu -> renameRegInt2, &(RegStatusEntry -> reorderNum));
                RS -> Vj = *((int *)renameRegIntEntry -> value -> value);
                RS -> Qj = -1;
            }
            else{
                RS -> Qj = robNum;
                RS->isReady = 0;
            }
        }else{
            RS -> Vj = cpu->integerRegisters2[instruction->rs]->data;
            RS -> Qj = -1;
        }
        RS->Dest = DestROBnum;
        RS->instruction = instruction;
        if (strcmp(buffType, "Store") == 0) {
            if (instruction -> op == S_D) {
                RegStatusEntry = cpu -> FPRegStatus2[instruction -> ft];
                if (RegStatusEntry -> busy == 1){
                    int robNum = RegStatusEntry -> reorderNum;
                    if (((ROB *)cpu -> reorderBuffer2 -> items[robNum]) -> isReady == 1){
                        DictionaryEntry *renameRegFloatEntry = getValueChainByDictionaryKey (cpu -> renameRegFP2, &instruction->ft);
                        RS -> fpVk = *((int *)renameRegFloatEntry -> value -> value);
                        RS -> Qk = -1;
                    }
                    else{
                        RS -> Qk = robNum;
                        RS->isReady = 0;
                    }
                }else{
                    RS -> fpVk = cpu -> floatingPointRegisters2[instruction->ft] -> data;
                    RS -> Qk = -1;
                }
            } else {
                RegStatusEntry = cpu -> IntRegStatus2[instruction -> rt];
                if (RegStatusEntry -> busy == 1){
                    int robNum = RegStatusEntry -> reorderNum;
                    if (((ROB *)cpu -> reorderBuffer2 -> items[robNum]) -> isReady == 1){
                        DictionaryEntry *renameRegIntEntry = getValueChainByDictionaryKey (cpu -> renameRegInt2, &instruction->rt);
                        RS -> iVk = *((int *)renameRegIntEntry -> value -> value);
                        RS -> Qk = -1;
                    }
                    else{
                        RS -> Qk = robNum;
                        RS->isReady = 0;
                    }
                }else{
                    RS -> iVk = cpu -> integerRegisters2[instruction->rt] -> data;
                    RS -> Qk = -1;
                }
            }
        }
        RS -> address = -1;
        RS->isExecuting = 0;
        KeyRS *keyRS = (KeyRS*)malloc(sizeof(KeyRS));
        keyRS->reorderNum = RS->Dest;
        keyRS->progNum = instruction->isProg2 + 1;
        addDictionaryEntry(LOrSBufferResult, keyRS, RS);
        //Add to renaming registers and update Register Status if load
        if (strcmp(buffType, "Load") == 0) {
            if (instruction -> op == L_D) {
                RegStatusEntry = cpu -> FPRegStatus2[instruction -> ft];
                RegStatusEntry->busy = 1;
        		RegStatusEntry->reorderNum = DestROBnum;
                void *valuePtr = malloc(sizeof(double));
                *((double*)valuePtr) = 0;
                removeDictionaryEntriesByKey(cpu -> renameRegFP2, &(RS -> Dest));
                addDictionaryEntry(cpu -> renameRegFP2, &(RS -> Dest), valuePtr);
            } else {
                RegStatusEntry = cpu -> FPRegStatus2[instruction -> rt];
                RegStatusEntry->busy = 1;
        		RegStatusEntry->reorderNum = DestROBnum;
                void *valuePtr = malloc(sizeof(int));
                *((int*)valuePtr) = 0;
                removeDictionaryEntriesByKey(cpu -> renameRegInt2, &(RS->Dest));
                addDictionaryEntry(cpu -> renameRegInt2, &(RS->Dest), valuePtr);
            }

        }
        printf("Issued instruction %d: %s\n", instruction->address, getOpcodeString ((int) instruction->op));
        return 1;
    } else {
        cpu -> stallFullRS ++;
        printf("Stall during IssueUnit because %s buffer is full.\n", buffType);
        return 0;
    }
}


//Issue an instruction
int issueInstruction(Instruction *instruction){
    if (isFullCircularQueue(cpu -> reorderBuffer)){
        cpu -> stallFullROB ++;
        printf("Stall during IssueUnit because reorder buffer is full.\n");
        return 0;
    }
    int issued = 0;
    int renameRegFull = 1;
    char* rsType;
    switch (instruction->op) {
        case ANDI:
        case AND:
        case ORI:
        case OR:
        case SLTI:
        case SLT:
        case DADDI:
        case DADD:
        case DSUB:
            renameRegFull = renameRegIsFull(cpu->renameRegInt, instruction -> rd);
            if (renameRegFull!=1){
                rsType = "INT";
                issued = addInstruction2RSint(cpu->renameRegInt, cpu->resStaInt, cpu->resStaIntResult, rsType, numberRSint,
                                            instruction, cpu->IntRegStatus);
            }
            break;
        case DMUL:
            renameRegFull = renameRegIsFull(cpu->renameRegInt, instruction -> rd);
            if (renameRegFull!=1){
                rsType = "MULT";
                issued = addInstruction2RSint(cpu->renameRegInt, cpu->resStaMult, cpu->resStaMultResult,
                                           rsType, numberRSmult, instruction, cpu->IntRegStatus);
            }
            break;
        case ADD_D:
        case SUB_D:
            renameRegFull = renameRegIsFull(cpu->renameRegFP, instruction -> fd);
            if (renameRegFull!=1){
                char* rsType = "FPadd";
                issued = addInstruction2RSfloat(cpu->renameRegFP, cpu->resStaFPadd, cpu->resStaFPaddResult,
                                               rsType, numberRSfpAdd, instruction, cpu->FPRegStatus);
            }
            break;
        case MUL_D:
            renameRegFull = renameRegIsFull(cpu->renameRegFP, instruction -> fd);
            if (renameRegFull!=1){
                rsType = "FPmult";
                issued = addInstruction2RSfloat(cpu->renameRegFP, cpu->resStaFPmult, cpu->resStaFPmultResult,
                                               rsType, numberRSfpMult, instruction, cpu->FPRegStatus);
            }
            break;
        case DIV_D:
            renameRegFull = renameRegIsFull(cpu->renameRegFP, instruction -> fd);
            if (renameRegFull!=1){
                rsType = "FPdiv";
                issued = addInstruction2RSfloat(cpu->renameRegFP, cpu->resStaFPdiv, cpu->resStaFPdivResult,
                                               rsType, numberRSfpDiv, instruction, cpu->FPRegStatus);
            }
            break;
        case L_D:
        	renameRegFull = renameRegIsFull(cpu->renameRegFP, instruction -> ft);
            if (renameRegFull!=1){
                rsType = "Load";
                issued = addLoadStore2Buffer(cpu->loadBuffer, cpu->loadBufferResult,
                         rsType, numberBufferLoad, instruction);
            }
            break;

        case LD:
        	renameRegFull = renameRegIsFull(cpu->renameRegInt, instruction -> rt);
            if (renameRegFull!=1){
                rsType = "Load";
                issued = addLoadStore2Buffer(cpu->loadBuffer, cpu->loadBufferResult,
                         rsType, numberBufferLoad, instruction);
            }
            break;
        case SD:
        case S_D:
            rsType = "Store";
            issued = addLoadStore2Buffer(cpu->storeBuffer, cpu->storeBufferResult,
                     rsType, numberBufferStore, instruction);
            break;
        case BNE:
        case BNEZ:
        case BEQ:
        case BEQZ:
            rsType = "BU";
            issued = addInstruction2RSbranch(cpu->renameRegInt, cpu->resStaBU, cpu->resStaBUResult, rsType, numberRSbu,
                                             instruction, cpu->IntRegStatus);
            break;
        default:
            break;
    }
    if (issued == 1){
        enqueueCircular(cpu->reorderBuffer, InitializeROBEntry(instruction));
        return 1;
    }else
        return 0;
}

//Issue an instruction for part 2
int issueInstruction2(Instruction *instruction){
    if (isFullCircularQueue(cpu -> reorderBuffer2)){
        cpu -> stallFullROB2 ++;
        printf("Stall during IssueUnit because reorder buffer is full.\n");
        return 0;
    }
    int issued = 0;
    int renameRegFull = 1;
    char* rsType;
    switch (instruction->op) {
        case ANDI:
        case AND:
        case ORI:
        case OR:
        case SLTI:
        case SLT:
        case DADDI:
        case DADD:
        case DSUB:
            renameRegFull = renameRegIsFull(cpu->renameRegInt2, instruction -> rd);
            if (renameRegFull!=1){
                rsType = "INT";
                issued = addInstruction2RSint2(cpu->renameRegInt2, cpu->resStaInt, cpu->resStaIntResult, rsType, numberRSint,
                                            instruction, cpu->IntRegStatus2);
            }
            break;
        case DMUL:
            renameRegFull = renameRegIsFull(cpu->renameRegInt2, instruction -> rd);
            if (renameRegFull!=1){
                rsType = "MULT";
                issued = addInstruction2RSint2(cpu->renameRegInt2, cpu->resStaMult, cpu->resStaMultResult,
                                           rsType, numberRSmult, instruction, cpu->IntRegStatus2);
            }
            break;
        case ADD_D:
        case SUB_D:
            renameRegFull = renameRegIsFull(cpu->renameRegFP2, instruction -> fd);
            if (renameRegFull!=1){
                char* rsType = "FPadd";
                issued = addInstruction2RSfloat2(cpu->renameRegFP2, cpu->resStaFPadd, cpu->resStaFPaddResult,
                                               rsType, numberRSfpAdd, instruction, cpu->FPRegStatus2);
            }
            break;
        case MUL_D:
            renameRegFull = renameRegIsFull(cpu->renameRegFP2, instruction -> fd);
            if (renameRegFull!=1){
                rsType = "FPmult";
                issued = addInstruction2RSfloat2(cpu->renameRegFP2, cpu->resStaFPmult, cpu->resStaFPmultResult,
                                               rsType, numberRSfpMult, instruction, cpu->FPRegStatus2);
            }
            break;
        case DIV_D:
            renameRegFull = renameRegIsFull(cpu->renameRegFP2, instruction -> fd);
            if (renameRegFull!=1){
                rsType = "FPdiv";
                issued = addInstruction2RSfloat2(cpu->renameRegFP2, cpu->resStaFPdiv, cpu->resStaFPdivResult,
                                               rsType, numberRSfpDiv, instruction, cpu->FPRegStatus2);
            }
            break;
        case L_D:
        	renameRegFull = renameRegIsFull(cpu->renameRegFP2, instruction -> ft);
            if (renameRegFull!=1){
                rsType = "Load";
                issued = addLoadStore2Buffer2(cpu->loadBuffer, cpu->loadBufferResult,
                         rsType, numberBufferLoad, instruction);
            }
            break;

        case LD:
        	renameRegFull = renameRegIsFull(cpu->renameRegInt2, instruction -> rt);
            if (renameRegFull!=1){
                rsType = "Load";
                issued = addLoadStore2Buffer2(cpu->loadBuffer, cpu->loadBufferResult,
                         rsType, numberBufferLoad, instruction);
            }
            break;
        case SD:
        case S_D:
            rsType = "Store";
            issued = addLoadStore2Buffer2(cpu->storeBuffer, cpu->storeBufferResult,
                     rsType, numberBufferStore, instruction);
            break;
        case BNE:
        case BNEZ:
        case BEQ:
        case BEQZ:
            rsType = "BU";
            issued = addInstruction2RSbranch2(cpu->renameRegInt2, cpu->resStaBU, cpu->resStaBUResult, rsType, numberRSbu,
                                             instruction, cpu->IntRegStatus2);
            break;
        default:
            break;
    }
    if (issued == 1){
        enqueueCircular(cpu->reorderBuffer2, InitializeROBEntry2(instruction));
        return 1;
    }else
        return 0;
}

//Issue Unit, return the number of issued instructions in this cycle
int issueUnit(int NW){
    int i;
    int issued;
    Instruction *instruction;
    for(i=0; i<NW; i++){
        if((instruction = getHeadCircularQueue(cpu -> instructionQueue))!= NULL){
            issued = issueInstruction(instruction);
            if (issued == 0)
                return i;
            else
                dequeueCircular(cpu -> instructionQueue);
        }
        else
            return i;
    }
    return i;
}

//Issue Unit for part 2
int issueUnit2(int NW){
    int i;
    int issued;
    Instruction *instruction;
    for(i=0; i<NW; i++){
        if((instruction = getHeadCircularQueue(cpu -> instructionQueue2))!= NULL){
            issued = issueInstruction2(instruction);
            if (issued == 0)
                return i;
            else
                dequeueCircular(cpu -> instructionQueue2);
        }
        else
            return i;
    }
    return i;
}

/**
 * Method that simulates pipelined Unit.
 * @return pointer to output instruction of the pipeline.
 */

CompletedInstruction *executePipelinedUnit (CircularQueue *pipeline) {
    CompletedInstruction *output = (CompletedInstruction *)dequeueCircular (pipeline);

    if (pipeline -> count < pipeline -> size - 1) {
        enqueueCircular (pipeline, NULL);
    }

    return output;
}


/**
 * Method that simulates FPdiv unit.
 * @return Pointer to the output instruction of the pipeline. NULL in case of bubbles or while executing.
 */

CompletedInstruction *executeFPDivUnit (CircularQueue *pipeline) {
    CompletedInstruction *output;

    output = dequeueCircular (pipeline);

    if (output != NULL) {
        cpu -> FPdivPipelineBusy = 0;
    }
    if (pipeline -> count < pipeline -> size - 1) {
        enqueueCircular (pipeline, NULL);
    }

    return output;
}

//Execution
CompletedInstruction **execute(int NB){
    Instruction *instruction = malloc(sizeof(Instruction));
    void *valuePtr = malloc(sizeof(double));
    void *addrPtr = malloc(sizeof(int));
  	DictionaryEntry *dataCacheElement;
  	CompletedInstruction *instructionAndResult = malloc(sizeof(CompletedInstruction));
    RSint *rsint;
    RSfloat *rsfloat;
    RSmem *rsmem;
    DictionaryValue *dictVal;
    //Array for instructions moving from Reservation Stations to execution units. Contains DictionaryValues.
    DictionaryEntry **instructionsToExec = malloc(sizeof(DictionaryEntry *)*8);
    //Array for outputs of Units. See Unit enum in DataTypes.h
    static CompletedInstruction *unitOutputs[7];
    int i, j;
    char *pipelineString;
    //variables for loads
    RSmem *RS;
    int instructionFoundOrBubble;
    DictionaryEntry *dictEntry;
    CircularQueue *buff;
    int loadStallROBNumber = -1; //needed to stall step 1 of load pipeline
    //Temp pipelines to hold changes during execution
    CompletedInstruction *INTPipelineTemp = NULL;
    CompletedInstruction *MULTPipelineTemp = NULL;
    CompletedInstruction *LoadPipelineTemp = NULL;
    CompletedInstruction *StorePipelineTemp = NULL;
    int storeFirst = 0; //1 if store first
    CompletedInstruction *FPaddPipelineTemp = NULL;
    CompletedInstruction *FPmultPipelineTemp = NULL;
    CompletedInstruction *FPdivPipelineTemp = NULL;
    CompletedInstruction *BUPipelineTemp = NULL;
    KeyRS *key = malloc(sizeof(KeyRS));


    dictEntry = (DictionaryEntry *)cpu -> resStaInt -> head;
    dictVal = checkReservationStation (dictEntry, 0);
    if (dictVal != NULL) {
        rsint = (RSint *)dictVal -> value;
        key -> reorderNum = rsint -> Dest;
        key -> progNum = rsint -> instruction -> isProg2 + 1;
        instructionsToExec[0] = getValueChainByDictionaryKey (cpu -> resStaInt, key);
    } else {
        instructionsToExec[0] = NULL;
    }
    dictEntry = (DictionaryEntry *)cpu -> resStaMult -> head;
    dictVal = checkReservationStation (dictEntry, 0);
    if (dictVal != NULL) {
        rsint = (RSint *)(dictVal -> value);
        key -> reorderNum = rsint -> Dest;
        key -> progNum = rsint -> instruction -> isProg2 + 1;
        instructionsToExec[1] = getValueChainByDictionaryKey (cpu -> resStaMult, key);
    } else {
        instructionsToExec[1] = NULL;
    }
    dictEntry = (DictionaryEntry *)cpu -> loadBuffer -> head;
    dictVal = checkReservationStation (dictEntry, 3);
    if (dictVal != NULL) {
        rsmem = (RSmem *)(dictVal -> value);
        key -> reorderNum = rsmem -> Dest;
        key -> progNum = rsmem -> instruction -> isProg2 + 1;
        instructionsToExec[2] = getValueChainByDictionaryKey (cpu -> loadBuffer, key);
    } else {
        instructionsToExec[2] = NULL;
    }
    dictEntry = (DictionaryEntry *)cpu -> storeBuffer -> head;
    dictVal = checkReservationStation (dictEntry, 2);
    if (dictVal != NULL) {
        rsmem = (RSmem *)(dictVal -> value);
        key -> reorderNum = rsmem -> Dest;
        key -> progNum = rsmem -> instruction -> isProg2 + 1;
        instructionsToExec[3] = getValueChainByDictionaryKey (cpu -> storeBuffer, key);
    } else {
        instructionsToExec[3] = NULL;
    }
    dictEntry = (DictionaryEntry *)cpu -> resStaFPadd -> head;
    dictVal = checkReservationStation (dictEntry, 1);
    if (dictVal != NULL) {
        rsfloat = (RSfloat *)(dictVal -> value);
        key -> reorderNum = rsfloat -> Dest;
        key -> progNum = rsfloat -> instruction -> isProg2 + 1;
        instructionsToExec[4] = getValueChainByDictionaryKey (cpu -> resStaFPadd, key);
    } else {
        instructionsToExec[4] = NULL;
    }
    dictEntry = (DictionaryEntry *)cpu -> resStaFPmult -> head;
    dictVal = checkReservationStation (dictEntry, 1);
    if (dictVal != NULL) {
        rsfloat = (RSfloat *)(dictVal -> value);
        key -> reorderNum = rsfloat -> Dest;
        key -> progNum = rsfloat -> instruction -> isProg2 + 1;
        instructionsToExec[5] = getValueChainByDictionaryKey (cpu -> resStaFPmult, key);
    } else {
        instructionsToExec[5] = NULL;
    }
    if (!(cpu -> FPdivPipelineBusy)) {
        dictEntry = (DictionaryEntry *)cpu -> resStaFPdiv -> head;
        dictVal = checkReservationStation (dictEntry, 1);
        if (dictVal != NULL) {
            rsfloat = (RSfloat *)(dictVal -> value);
            key -> reorderNum = rsfloat -> Dest;
            key -> progNum = rsfloat -> instruction -> isProg2 + 1;
            instructionsToExec[6] = getValueChainByDictionaryKey (cpu -> resStaFPdiv, key);
        } else {
            instructionsToExec[6] = NULL;
        }
    }
    dictEntry = (DictionaryEntry *)cpu -> resStaBU -> head;
    dictVal = checkReservationStation (dictEntry, 0);
    if (dictVal != NULL) {
        rsint = (RSint *)(dictVal -> value);
        key -> reorderNum = rsint -> Dest;
        key -> progNum = rsint -> instruction -> isProg2 + 1;
        instructionsToExec[7] = getValueChainByDictionaryKey (cpu -> resStaBU, key);
    } else {
        instructionsToExec[7] = NULL;
    }

    for (i = 0; i < 8; i++) {
        if (instructionsToExec[i] == NULL) { //if reservation station did not provide instruction
            continue;
        }
		//printf("instruction exceuting has unit number %d\n", i);
        if (i < 2 || i > 6) {
            rsint = (RSint *)((DictionaryEntry *)instructionsToExec[i] -> value -> value);
            instruction = rsint -> instruction;
			printf("instruction exceuting has address %d and ROB %d\n", instruction ->address, rsint -> Dest);
        } else if (i == 2 || i == 3) {
            rsmem = (RSmem *)(((DictionaryEntry *)instructionsToExec[i]) -> value -> value);
            instruction = rsmem -> instruction;
			printf("instruction exceuting has address %d and ROB %d\n", instruction ->address, rsmem -> Dest);
        } else {
            rsfloat = (RSfloat *)(((DictionaryEntry *)instructionsToExec[i]) -> value -> value);
            instruction = rsfloat -> instruction;
			printf("instruction exceuting has address %d and ROB %d\n", instruction ->address, rsfloat -> Dest);
        }
        instructionAndResult -> instruction = instruction;

        switch (instruction->op) {
            case ANDI:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj & instruction->immediate;
                instructionAndResult -> ROB_number = rsint -> Dest;
                INTPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(INTPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "INT";
                printPipeline(instruction, pipelineString, 1);
                break;
            case AND:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj & rsint -> Vk;
                instructionAndResult -> ROB_number = rsint -> Dest;
                INTPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(INTPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "INT";
                printPipeline(instruction, pipelineString, 1);
                break;
            case ORI:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj | instruction->immediate;
                instructionAndResult -> ROB_number = rsint -> Dest;
                INTPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(INTPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "INT";
                printPipeline(instruction, pipelineString, 1);
                break;
            case OR:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj | rsint -> Vk;
                instructionAndResult -> ROB_number = rsint -> Dest;
                INTPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(INTPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "INT";
                printPipeline(instruction, pipelineString, 1);
                break;
            case SLTI:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj < instruction->immediate ? 1 : 0;
                instructionAndResult -> ROB_number = rsint -> Dest;
                INTPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(INTPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "INT";
                printPipeline(instruction, pipelineString, 1);
                break;
            case SLT:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj < rsint -> Vk ? 1 : 0;
                instructionAndResult -> ROB_number = rsint -> Dest;
                INTPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(INTPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "INT";
                printPipeline(instruction, pipelineString, 1);
                break;
            case DADDI:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj + instruction->immediate;
                instructionAndResult -> ROB_number = rsint -> Dest;
                INTPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(INTPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "INT";
                printPipeline(instruction, pipelineString, 1);
                break;
            case DADD:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj + rsint -> Vk;
                instructionAndResult -> ROB_number = rsint -> Dest;
                INTPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(INTPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "INT";
                printPipeline(instruction, pipelineString, 1);
                break;
            case DSUB:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj - rsint -> Vk;
                instructionAndResult -> ROB_number = rsint -> Dest;
                INTPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(INTPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "INT";
                printPipeline(instruction, pipelineString, 1);
                break;
            case DMUL:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj * rsint -> Vk;
                instructionAndResult -> ROB_number = rsint -> Dest;
                MULTPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(MULTPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "MULT";
                printPipeline(instruction, pipelineString, 1);
                break;
            case ADD_D:
                rsfloat -> isExecuting = 1;
                instructionAndResult -> fpResult = rsfloat -> Vj + rsfloat -> Vk;
                instructionAndResult -> ROB_number = rsfloat -> Dest;
                FPaddPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(FPaddPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "FPadd";
                printPipeline(instruction, pipelineString, 1);
                break;
            case SUB_D:
                rsfloat -> isExecuting = 1;
                instructionAndResult -> fpResult = rsfloat -> Vj - rsfloat -> Vk;
                instructionAndResult -> ROB_number = rsfloat -> Dest;
                FPaddPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(FPaddPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "FPadd";
                printPipeline(instruction, pipelineString, 1);
                break;
            case MUL_D:
                rsfloat -> isExecuting = 1;
                instructionAndResult -> fpResult = rsfloat -> Vj * rsfloat -> Vk;
                instructionAndResult -> ROB_number = rsfloat -> Dest;
                FPmultPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(FPmultPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "FPmult";
                printPipeline(instruction, pipelineString, 1);
                break;
            case DIV_D:
                rsfloat -> isExecuting = 1;
                instructionAndResult -> fpResult = rsfloat -> Vj / rsfloat -> Vk;
                instructionAndResult -> ROB_number = rsfloat -> Dest;
                FPdivPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(FPdivPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                cpu -> FPdivPipelineBusy = 1;
                pipelineString = "FPdiv";
                printPipeline(instruction, pipelineString, 1);
                break;
            case L_D:
                //Two-step
                //First calculate address for earliest load that needs it
                if (rsmem -> isExecuting == 0){
                    rsmem -> isExecuting = 1;
                    rsmem -> address = rsmem -> Vj + instruction->immediate;
                    loadStallROBNumber = rsmem -> Dest;
                    pipelineString = "Load/Store";
                    printPipeline(instruction, pipelineString, 1);
                } else {
                    rsmem = NULL;
                }
                //Then check if load can execute (i.e. no stores ahead of it in ROB with same address)
                //Buff is reorder buffer but just for readability
                buff = cpu -> reorderBuffer;
                dictEntry = cpu -> loadBuffer -> head;
                instructionFoundOrBubble = 0;
                while (!instructionFoundOrBubble) { //1 for instruction, 2 for bubble
                    if (dictEntry == NULL) {
                        instructionFoundOrBubble = 2;
                    } else {
                        RS = (RSmem *)((DictionaryEntry *)dictEntry -> value -> value);
                        if (RS -> isReady && RS -> address != -1) {
                            for (j = 0; j < buff -> count && j < ((RS->Dest - buff->head)%buff->size) && j != -1; j++) {
                                if (((ROB *)(buff -> items[(buff -> head + j) % (buff->size)]))-> DestAddr == RS -> address) {
                                    j = -2; //break out of for loop
                                }
                            }
                            if (j != -1 && RS != rsmem && RS -> isExecuting != 2) {
                                instructionFoundOrBubble = 1;
                                rsmem = RS;
                            } else {
                                dictEntry = dictEntry -> next;
                            }
                        } else {
                            dictEntry = dictEntry -> next;
                        }
                    }
                }
                if (instructionFoundOrBubble == 1 && instructionsToExec[3] == NULL) {
                    rsmem -> isExecuting = 2;
                    * ((int*)addrPtr) = rsmem -> address;
                    dataCacheElement = getValueChainByDictionaryKey(dataCache, addrPtr);
                    if (dataCacheElement != NULL) {
                        valuePtr = dataCacheElement->value->value;

                    } else {
                        *((double *)valuePtr) = 0.0;
                    }
                    instructionAndResult -> fpResult = *((double*)valuePtr);
                    instructionAndResult -> ROB_number = rsmem -> Dest;
                    LoadPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                    memcpy(LoadPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                    pipelineString = "MEM";
                    printPipeline(instruction, pipelineString, 1);
                }
                break;
            case LD:
                //Two-step
                //First calculate address for earliest load that needs it
                if (rsmem -> isExecuting == 0){
                    rsmem -> isExecuting = 1;
                    rsmem -> address = rsmem -> Vj + instruction->immediate;
                    loadStallROBNumber = rsmem -> Dest;
                    pipelineString = "Load/Store";
                    printPipeline(instruction, pipelineString, 1);
                } else {
                    rsmem = NULL;
                }
                //Then check if load can execute (i.e. no stores ahead of it in ROB with same address)
                //Buff is reorder buffer but just for readability
                buff = cpu -> reorderBuffer;
                dictEntry = cpu -> loadBuffer -> head;
                instructionFoundOrBubble = 0;
                while (!instructionFoundOrBubble) { //1 for instruction, 2 for bubble
                    RS = (RSmem *)((DictionaryEntry *)dictEntry -> value -> value);
                    if (RS == NULL) {
                        instructionFoundOrBubble = 2;
                        continue;
                    }
                    if (RS -> isReady && RS -> address != -1) {
                        for (j = 0; j < buff -> count && j < ((RS->Dest - buff->head)%buff->size) && j != -1; j++) {
                            if (((ROB *)(buff -> items[(buff -> head + j) % (buff->size)])) -> DestAddr == RS -> address) {
                                j = -1; //break out of for loop
                            }
                        }
                        if (j != -1 && RS != rsmem && RS -> isExecuting != 2) {
                            instructionFoundOrBubble = 1;
                            rsmem = RS;
                        } else {
                            dictEntry = dictEntry -> next;
                        }
                    } else {
                        dictEntry = dictEntry -> next;
                    }
                }
                if (instructionFoundOrBubble == 1 && instructionsToExec[3] == NULL) {
                    rsmem -> isExecuting = 2;
                    * ((int*)addrPtr) = rsmem -> address;
                    dataCacheElement = getValueChainByDictionaryKey(dataCache, addrPtr);
                    if (dataCacheElement != NULL) {
                        valuePtr = dataCacheElement->value->value;
                    } else {
                        * ((int *)valuePtr) = 0;
                    }
                    instructionAndResult -> intResult = (int)*((double*)valuePtr);
                    instructionAndResult -> ROB_number = rsmem -> Dest;
                    LoadPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                    memcpy(LoadPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                    pipelineString = "MEM";
                    printPipeline(instruction, pipelineString, 1);
                }
                break;
            case SD:
                rsmem -> isExecuting = 1;
                rsmem -> address = rsmem -> Vj + instruction->immediate;
                instructionAndResult -> address = rsmem -> address;
                instructionAndResult -> intResult = rsmem -> iVk;
                instructionAndResult -> ROB_number = rsmem -> Dest;
                StorePipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(StorePipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                storeFirst = 1;
                pipelineString = "Load/Store";
                printPipeline(instruction, pipelineString, 1);
                break;
            case S_D:
                rsmem -> isExecuting = 1;
                rsmem -> address = rsmem -> Vj + instruction->immediate;
                instructionAndResult -> address = rsmem -> address;
                instructionAndResult -> fpResult = rsmem -> fpVk;
                instructionAndResult -> ROB_number = rsmem -> Dest;
                StorePipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(StorePipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                storeFirst = 1;
                pipelineString = "Load/Store";
                printPipeline(instruction, pipelineString, 1);
                break;
            case BNE:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj != rsint -> Vk ? 0 : -1;
                instructionAndResult -> ROB_number = rsint -> Dest;
                BUPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(BUPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "BU";
                printPipeline(instruction, pipelineString, 1);
                break;
            case BNEZ:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj != 0 ? 0 : -1;
                instructionAndResult -> ROB_number = rsint -> Dest;
                BUPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(BUPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "BU";
                printPipeline(instruction, pipelineString, 1);
                break;
            case BEQ:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj == rsint -> Vk ? 0 : -1;
                instructionAndResult -> ROB_number = rsint -> Dest;
                BUPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(BUPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "BU";
                printPipeline(instruction, pipelineString, 1);
                break;
            case BEQZ:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj == 0 ? 0 : -1;
                instructionAndResult -> ROB_number = rsint -> Dest;
                BUPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(BUPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "BU";
                printPipeline(instruction, pipelineString, 1);
                break;
            default:
                break;
        }
    }

    //Take outputs from Units, but only as many as can be accepted by WriteBack Buffer
    int maxOutput = NB - (countDictionaryLen (cpu -> WriteBackBuffer));
	//printf("max outputs - %d\n", maxOutput);
    i = 0;
    if (i < maxOutput) {
        unitOutputs[INT] = executePipelinedUnit (cpu -> INTPipeline);
        if (unitOutputs[INT] != NULL) {
            i++;
            key -> reorderNum = unitOutputs[INT] -> ROB_number;
            key -> progNum = unitOutputs[INT] -> instruction -> isProg2 + 1;
            removeDictionaryEntriesByKey (cpu -> renameRegInt, &(unitOutputs[INT] -> ROB_number));
            addDictionaryEntry (cpu -> renameRegInt, &(unitOutputs[INT] -> ROB_number), &(unitOutputs[INT] -> intResult));
            removeDictionaryEntriesByKey (cpu -> resStaInt, key);
            pipelineString = "INT";
            printPipeline(unitOutputs[INT], pipelineString, 0);
        }
        if (INTPipelineTemp != NULL) {
            enqueueCircular (cpu -> INTPipeline, INTPipelineTemp);
        }
    } else {
        if (INTPipelineTemp != NULL) {
            key -> reorderNum = INTPipelineTemp -> ROB_number;
            key -> progNum = INTPipelineTemp -> instruction -> isProg2 + 1;
            RSint *stalled = (RSint *)(getValueChainByDictionaryKey(cpu -> resStaInt, key) -> value -> value);
            stalled -> isExecuting = 0;
        }
    }
    if (i < maxOutput) {
        unitOutputs[MULT] = executePipelinedUnit (cpu -> MULTPipeline);
        if (unitOutputs[MULT] != NULL) {
            i++;
            key -> reorderNum = unitOutputs[MULT] -> ROB_number;
            key -> progNum = unitOutputs[MULT] -> instruction -> isProg2 + 1;
            removeDictionaryEntriesByKey (cpu -> renameRegInt, &(unitOutputs[MULT] -> ROB_number));
            addDictionaryEntry (cpu -> renameRegInt, &(unitOutputs[MULT] -> ROB_number), &(unitOutputs[MULT] -> intResult));
            removeDictionaryEntriesByKey (cpu -> resStaMult, key);
            pipelineString = "MULT";
            printPipeline(unitOutputs[MULT], pipelineString, 0);
        }
        if (MULTPipelineTemp != NULL) {
            enqueueCircular (cpu -> MULTPipeline, MULTPipelineTemp);
        }
    } else {
        if (MULTPipelineTemp != NULL) {
            key -> reorderNum = MULTPipelineTemp -> ROB_number;
            key -> progNum = MULTPipelineTemp -> instruction -> isProg2 + 1;
            RSint *stalled = (RSint *)(getValueChainByDictionaryKey(cpu -> resStaMult, key) -> value -> value);
            stalled -> isExecuting = 0;
        }
    }
    if (i < maxOutput) {
        unitOutputs[LS] = executePipelinedUnit (cpu -> LoadStorePipeline);
        if (unitOutputs[LS] != NULL) {
            i++;
            key -> reorderNum = unitOutputs[LS] -> ROB_number;
            key -> progNum = unitOutputs[LS] -> instruction -> isProg2 + 1;
            if (unitOutputs[LS] -> instruction -> op == L_D) {
                removeDictionaryEntriesByKey (cpu -> renameRegFP, &(unitOutputs[LS] -> ROB_number));
                addDictionaryEntry (cpu -> renameRegFP, &(unitOutputs[LS] -> ROB_number), &(unitOutputs[LS] -> fpResult));
                removeDictionaryEntriesByKey (cpu -> loadBuffer, key);
            } else if (unitOutputs[LS] -> instruction -> op == LD) {
                removeDictionaryEntriesByKey (cpu -> renameRegInt, &(unitOutputs[LS] -> ROB_number));
                addDictionaryEntry (cpu -> renameRegInt, &(unitOutputs[LS] -> ROB_number), &(unitOutputs[LS] -> intResult));
                removeDictionaryEntriesByKey (cpu -> loadBuffer, key);
            } else if (unitOutputs[LS] -> instruction -> op == S_D) {
                removeDictionaryEntriesByKey (cpu -> renameRegFP, &(unitOutputs[LS] -> ROB_number));
                addDictionaryEntry (cpu -> renameRegFP, &(unitOutputs[LS] -> ROB_number), &(unitOutputs[LS] -> fpResult));
                removeDictionaryEntriesByKey (cpu -> storeBuffer, key);
            } else {
                removeDictionaryEntriesByKey (cpu -> renameRegInt, &(unitOutputs[LS] -> ROB_number));
                addDictionaryEntry (cpu -> renameRegInt, &(unitOutputs[LS] -> ROB_number), &(unitOutputs[LS] -> intResult));
                removeDictionaryEntriesByKey (cpu -> storeBuffer, key);
            }
            pipelineString = "Load/Store";
            printPipeline(unitOutputs[LS], pipelineString, 0);
        }
        if (!storeFirst && LoadPipelineTemp != NULL) {
            enqueueCircular (cpu -> LoadStorePipeline, LoadPipelineTemp);
        } else if (StorePipelineTemp != NULL) {
            enqueueCircular (cpu -> LoadStorePipeline, StorePipelineTemp);
        }
    } else {
        if (loadStallROBNumber != -1) {
            key -> reorderNum = loadStallROBNumber;
            key -> progNum = ((RSmem *)(((DictionaryEntry *)instructionsToExec[2]) -> value -> value)) -> instruction -> isProg2 + 1;
            RSmem *stalled = (RSmem *)(getValueChainByDictionaryKey(cpu -> loadBuffer, key) -> value -> value);
            stalled -> isExecuting = 0;
        }
        if (LoadPipelineTemp != NULL) {
            key -> reorderNum = LoadPipelineTemp -> ROB_number;
            key -> progNum = LoadPipelineTemp -> instruction -> isProg2 + 1;
            RSmem *stalled = (RSmem *)(getValueChainByDictionaryKey(cpu -> loadBuffer, &(LoadPipelineTemp -> ROB_number)) -> value -> value);
            stalled -> isExecuting = 1;
        }
        if (StorePipelineTemp != NULL) {
            key -> reorderNum = StorePipelineTemp -> ROB_number;
            key -> progNum = StorePipelineTemp -> instruction -> isProg2 + 1;
            RSmem *stalled = (RSmem *)(getValueChainByDictionaryKey(cpu -> storeBuffer, &(StorePipelineTemp -> ROB_number)) -> value -> value);
            stalled -> isExecuting = 0;
        }
    }
    if (i < maxOutput) {
        unitOutputs[FPadd] = executePipelinedUnit (cpu -> FPaddPipeline);
        if (unitOutputs[FPadd] != NULL) {
            i++;
            key -> reorderNum = unitOutputs[FPadd] -> ROB_number;
            key -> progNum = unitOutputs[FPadd] -> instruction -> isProg2 + 1;
            removeDictionaryEntriesByKey (cpu -> renameRegFP, &(unitOutputs[FPadd] -> ROB_number));
            addDictionaryEntry (cpu -> renameRegFP, &(unitOutputs[FPadd] -> ROB_number), &(unitOutputs[FPadd] -> fpResult));
            removeDictionaryEntriesByKey (cpu -> resStaFPadd, key);
            pipelineString = "FPadd";
            printPipeline(unitOutputs[FPadd], pipelineString, 0);
        }
        if (FPaddPipelineTemp != NULL) {
            enqueueCircular (cpu -> FPaddPipeline, FPaddPipelineTemp);
        }
    } else {
        if (FPaddPipelineTemp != NULL) {
            key -> reorderNum = FPaddPipelineTemp -> ROB_number;
            key -> progNum = FPaddPipelineTemp -> instruction -> isProg2 + 1;
            RSfloat *stalled = (RSfloat *)(getValueChainByDictionaryKey(cpu -> resStaFPadd, key) -> value -> value);
            stalled -> isExecuting = 0;
        }
    }
    if (i < maxOutput) {
        unitOutputs[FPmult] = executePipelinedUnit (cpu -> FPmultPipeline);
        if (unitOutputs[FPmult] != NULL) {
            i++;
            key -> reorderNum = unitOutputs[FPmult] -> ROB_number;
            key -> progNum = unitOutputs[FPmult] -> instruction -> isProg2 + 1;
            removeDictionaryEntriesByKey (cpu -> renameRegFP, &(unitOutputs[FPmult] -> ROB_number));
            addDictionaryEntry (cpu -> renameRegFP, &(unitOutputs[FPmult] -> ROB_number), &(unitOutputs[FPmult] -> fpResult));
            removeDictionaryEntriesByKey (cpu -> resStaFPmult, key);
            pipelineString = "FPmult";
            printPipeline(unitOutputs[FPmult], pipelineString, 0);
        }
        if (FPmultPipelineTemp != NULL) {
            enqueueCircular (cpu -> FPmultPipeline, FPmultPipelineTemp);
        }
    } else {
        if (FPmultPipelineTemp != NULL) {
            key -> reorderNum = FPmultPipelineTemp -> ROB_number;
            key -> progNum = FPmultPipelineTemp -> instruction -> isProg2 + 1;
            RSfloat *stalled = (RSfloat *)(getValueChainByDictionaryKey(cpu -> resStaFPmult, key) -> value -> value);
            stalled -> isExecuting = 0;
        }
    }
    if (i < maxOutput) {
        unitOutputs[FPdiv] = executeFPDivUnit (cpu -> FPdivPipeline);
        if (unitOutputs[FPdiv] != NULL) {
            i++;
            key -> reorderNum = unitOutputs[FPdiv] -> ROB_number;
            key -> progNum = unitOutputs[FPdiv] -> instruction -> isProg2 + 1;
            removeDictionaryEntriesByKey (cpu -> renameRegFP, &(unitOutputs[FPdiv] -> ROB_number));
            addDictionaryEntry (cpu -> renameRegFP, &(unitOutputs[FPdiv] -> ROB_number), &(unitOutputs[FPdiv] -> fpResult));
            removeDictionaryEntriesByKey (cpu -> resStaFPdiv, key);
            pipelineString = "FPdiv";
            printPipeline(unitOutputs[FPdiv], pipelineString, 0);
        }
        if (FPdivPipelineTemp != NULL) {
            enqueueCircular (cpu -> FPdivPipeline, FPdivPipelineTemp);
        }
    } else {
        if (FPdivPipelineTemp != NULL) {
            key -> reorderNum = FPdivPipelineTemp -> ROB_number;
            key -> progNum = FPdivPipelineTemp -> instruction -> isProg2 + 1;
            RSfloat *stalled = (RSfloat *)(getValueChainByDictionaryKey(cpu -> resStaFPdiv, key) -> value -> value);
            stalled -> isExecuting = 0;
            cpu -> FPdivPipelineBusy = 0;
        }
    }
    if (i < maxOutput) {
        unitOutputs[BU] = executePipelinedUnit (cpu -> BUPipeline);
        if (unitOutputs[BU] != NULL) {
            i++;
            key -> reorderNum = unitOutputs[BU] -> ROB_number;
            key -> progNum = unitOutputs[BU] -> instruction -> isProg2 + 1;
            removeDictionaryEntriesByKey (cpu -> renameRegInt, &(unitOutputs[BU] -> ROB_number));
            addDictionaryEntry (cpu -> renameRegInt, &(unitOutputs[BU] -> ROB_number), &(unitOutputs[BU] -> intResult));
            removeDictionaryEntriesByKey (cpu -> resStaBU, key);
            branchHelper (unitOutputs[BU]);
            pipelineString = "BU";
            printPipeline(unitOutputs[BU], pipelineString, 0);
        }
        if (BUPipelineTemp != NULL) {
            enqueueCircular (cpu -> BUPipeline, BUPipelineTemp);
        }
    } else {
        if (BUPipelineTemp != NULL) {
            key -> reorderNum = BUPipelineTemp -> ROB_number;
            key -> progNum = BUPipelineTemp -> instruction -> isProg2 + 1;
            RSint *stalled = (RSint *)(getValueChainByDictionaryKey(cpu -> resStaBU, key) -> value -> value);
            stalled -> isExecuting = 0;
        }
    }

    return unitOutputs;

}

//Execution for SMT
CompletedInstruction **execute2(int NB) {
    Instruction *instruction = malloc(sizeof(Instruction));
    void *valuePtr = malloc(sizeof(double));
    void *addrPtr = malloc(sizeof(int));
  	DictionaryEntry *dataCacheElement;
  	CompletedInstruction *instructionAndResult = malloc(sizeof(CompletedInstruction));
    RSint *rsint;
    RSfloat *rsfloat;
    RSmem *rsmem;
    DictionaryValue *dictVal;
    //Array for instructions moving from Reservation Stations to execution units. Contains DictionaryValues.
    DictionaryEntry **instructionsToExec = malloc(sizeof(DictionaryEntry *)*8);
    //Array for outputs of Units. See Unit enum in DataTypes.h
    static CompletedInstruction *unitOutputs[7];
    int i, j;
    char *pipelineString;
    //variables for loads
    RSmem *RS;
    int instructionFoundOrBubble;
    DictionaryEntry *dictEntry;
    CircularQueue *buff;
    int loadStallROBNumber = -1; //needed to stall step 1 of load pipeline
    //Temp pipelines to hold changes during execution
    CompletedInstruction *INTPipelineTemp = NULL;
    CompletedInstruction *MULTPipelineTemp = NULL;
    CompletedInstruction *LoadPipelineTemp = NULL;
    CompletedInstruction *StorePipelineTemp = NULL;
    int storeFirst = 0; //1 if store first
    CompletedInstruction *FPaddPipelineTemp = NULL;
    CompletedInstruction *FPmultPipelineTemp = NULL;
    CompletedInstruction *FPdivPipelineTemp = NULL;
    CompletedInstruction *BUPipelineTemp = NULL;
    KeyRS *key = malloc(sizeof(KeyRS));

    dictEntry = (DictionaryEntry *)cpu -> resStaInt -> head;
    dictVal = checkReservationStation (dictEntry, 0);
    if (dictVal != NULL) {
        rsint = (RSint *)dictVal -> value;
        key -> reorderNum = rsint -> Dest;
        key -> progNum = rsint -> instruction -> isProg2 + 1;
        instructionsToExec[0] = getValueChainByDictionaryKey (cpu -> resStaInt, key);
    } else {
        instructionsToExec[0] = NULL;
    }
    dictEntry = (DictionaryEntry *)cpu -> resStaMult -> head;
    dictVal = checkReservationStation (dictEntry, 0);
    if (dictVal != NULL) {
        rsint = (RSint *)(dictVal -> value);
        key -> reorderNum = rsint -> Dest;
        key -> progNum = rsint -> instruction -> isProg2 + 1;
        instructionsToExec[1] = getValueChainByDictionaryKey (cpu -> resStaMult, &(rsint -> Dest));
    } else {
        instructionsToExec[1] = NULL;
    }
    dictEntry = (DictionaryEntry *)cpu -> loadBuffer -> head;
    dictVal = checkReservationStation (dictEntry, 3);
    if (dictVal != NULL) {
        rsmem = (RSmem *)(dictVal -> value);
        key -> reorderNum = rsmem -> Dest;
        key -> progNum = rsmem -> instruction -> isProg2 + 1;
        instructionsToExec[2] = getValueChainByDictionaryKey (cpu -> loadBuffer, key);
    } else {
        instructionsToExec[2] = NULL;
    }
    dictEntry = (DictionaryEntry *)cpu -> storeBuffer -> head;
    dictVal = checkReservationStation (dictEntry, 2);
    if (dictVal != NULL) {
        rsmem = (RSmem *)(dictVal -> value);
        key -> reorderNum = rsmem -> Dest;
        key -> progNum = rsmem -> instruction -> isProg2 + 1;
        instructionsToExec[3] = getValueChainByDictionaryKey (cpu -> storeBuffer, key);
    } else {
        instructionsToExec[3] = NULL;
    }
    dictEntry = (DictionaryEntry *)cpu -> resStaFPadd -> head;
    dictVal = checkReservationStation (dictEntry, 1);
    if (dictVal != NULL) {
        rsfloat = (RSfloat *)(dictVal -> value);
        key -> reorderNum = rsfloat -> Dest;
        key -> progNum = rsfloat -> instruction -> isProg2 + 1;
        instructionsToExec[4] = getValueChainByDictionaryKey (cpu -> resStaFPadd, key);
    } else {
        instructionsToExec[4] = NULL;
    }
    dictEntry = (DictionaryEntry *)cpu -> resStaFPmult -> head;
    dictVal = checkReservationStation (dictEntry, 1);
    if (dictVal != NULL) {
        rsfloat = (RSfloat *)(dictVal -> value);
        key -> reorderNum = rsfloat -> Dest;
        key -> progNum = rsfloat -> instruction -> isProg2 + 1;
        instructionsToExec[5] = getValueChainByDictionaryKey (cpu -> resStaFPmult, key);
    } else {
        instructionsToExec[5] = NULL;
    }
    if (!(cpu -> FPdivPipelineBusy)) {
        dictEntry = (DictionaryEntry *)cpu -> resStaFPdiv -> head;
        dictVal = checkReservationStation (dictEntry, 1);
        if (dictVal != NULL) {
            rsfloat = (RSfloat *)(dictVal -> value);
            key -> reorderNum = rsfloat -> Dest;
            key -> progNum = rsfloat -> instruction -> isProg2 + 1;
            instructionsToExec[6] = getValueChainByDictionaryKey (cpu -> resStaFPdiv, key);
        } else {
            instructionsToExec[6] = NULL;
        }
    }
    dictEntry = (DictionaryEntry *)cpu -> resStaBU -> head;
    dictVal = checkReservationStation (dictEntry, 0);
    if (dictVal != NULL) {
        rsint = (RSint *)(dictVal -> value);
        key -> reorderNum = rsint -> Dest;
        key -> progNum = rsint -> instruction -> isProg2 + 1;
        instructionsToExec[7] = getValueChainByDictionaryKey (cpu -> resStaBU, key);
    } else {
        instructionsToExec[7] = NULL;
    }

    for (i = 0; i < 8; i++) {
        if (instructionsToExec[i] == NULL) { //if reservation station did not provide instruction
            continue;
        }
        if (i < 2 || i > 6) {
            rsint = (RSint *)((DictionaryEntry *)instructionsToExec[i] -> value -> value);
            instruction = rsint -> instruction;
			printf("instruction exceuting has address %d in program %d and ROB %d\n", instruction ->address, instruction->isProg2+1, rsint -> Dest);
        } else if (i == 2 || i == 3) {
            rsmem = (RSmem *)(((DictionaryEntry *)instructionsToExec[i]) -> value -> value);
            instruction = rsmem -> instruction;
			printf("instruction exceuting has address %d in program %d and ROB %d\n", instruction ->address, instruction->isProg2+1, rsmem -> Dest);
        } else {
            rsfloat = (RSfloat *)(((DictionaryEntry *)instructionsToExec[i]) -> value -> value);
            instruction = rsfloat -> instruction;
			printf("instruction exceuting has address %d in program %d and ROB %d\n", instruction ->address, instruction->isProg2+1, rsfloat -> Dest);
        }
		//printf("instruction exceuting has address %d and ROB %d\n", instruction ->address, rsint -> Dest);
        instructionAndResult -> instruction = instruction;
        switch (instruction->op) {
            case ANDI:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj & instruction->immediate;
                instructionAndResult -> ROB_number = rsint -> Dest;
                INTPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(INTPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "INT";
                printPipeline(instruction, pipelineString, 1);
                break;
            case AND:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj & rsint -> Vk;
                instructionAndResult -> ROB_number = rsint -> Dest;
                INTPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(INTPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "INT";
                printPipeline(instruction, pipelineString, 1);
                break;
            case ORI:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj | instruction->immediate;
                instructionAndResult -> ROB_number = rsint -> Dest;
                INTPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(INTPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "INT";
                printPipeline(instruction, pipelineString, 1);
                break;
            case OR:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj | rsint -> Vk;
                instructionAndResult -> ROB_number = rsint -> Dest;
                INTPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(INTPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "INT";
                printPipeline(instruction, pipelineString, 1);
                break;
            case SLTI:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj < instruction->immediate ? 1 : 0;
                instructionAndResult -> ROB_number = rsint -> Dest;
                INTPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(INTPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "INT";
                printPipeline(instruction, pipelineString, 1);
                break;
            case SLT:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj < rsint -> Vk ? 1 : 0;
                instructionAndResult -> ROB_number = rsint -> Dest;
                INTPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(INTPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "INT";
                printPipeline(instruction, pipelineString, 1);
                break;
            case DADDI:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj + instruction->immediate;
                instructionAndResult -> ROB_number = rsint -> Dest;
                INTPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(INTPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "INT";
                printPipeline(instruction, pipelineString, 1);
                break;
            case DADD:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj + rsint -> Vk;
                instructionAndResult -> ROB_number = rsint -> Dest;
                INTPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(INTPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "INT";
                printPipeline(instruction, pipelineString, 1);
                break;
            case DSUB:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj - rsint -> Vk;
                instructionAndResult -> ROB_number = rsint -> Dest;
                INTPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(INTPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "INT";
                printPipeline(instruction, pipelineString, 1);
                break;
            case DMUL:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj * rsint -> Vk;
                instructionAndResult -> ROB_number = rsint -> Dest;
                MULTPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(MULTPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "MULT";
                printPipeline(instruction, pipelineString, 1);
                break;
            case ADD_D:
                rsfloat -> isExecuting = 1;
                instructionAndResult -> fpResult = rsfloat -> Vj + rsfloat -> Vk;
                instructionAndResult -> ROB_number = rsfloat -> Dest;
                FPaddPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(FPaddPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "FPadd";
                printPipeline(instruction, pipelineString, 1);
                break;
            case SUB_D:
                rsfloat -> isExecuting = 1;
                instructionAndResult -> fpResult = rsfloat -> Vj - rsfloat -> Vk;
                instructionAndResult -> ROB_number = rsfloat -> Dest;
                FPaddPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(FPaddPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "FPadd";
                printPipeline(instruction, pipelineString, 1);
                break;
            case MUL_D:
                rsfloat -> isExecuting = 1;
                instructionAndResult -> fpResult = rsfloat -> Vj * rsfloat -> Vk;
                instructionAndResult -> ROB_number = rsfloat -> Dest;
                FPmultPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(FPmultPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "FPmult";
                printPipeline(instruction, pipelineString, 1);
                break;
            case DIV_D:
                rsfloat -> isExecuting = 1;
                instructionAndResult -> fpResult = rsfloat -> Vj / rsfloat -> Vk;
                instructionAndResult -> ROB_number = rsfloat -> Dest;
                FPdivPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(FPdivPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                cpu -> FPdivPipelineBusy = 1;
                pipelineString = "FPdiv";
                printPipeline(instruction, pipelineString, 1);
                break;
            case L_D:
                //Two-step
                //First calculate address for earliest load that needs it
                if (rsmem -> isExecuting == 0){
                    rsmem -> isExecuting = 1;
                    rsmem -> address = rsmem -> Vj + instruction->immediate;
                    loadStallROBNumber = rsmem -> Dest;
                    pipelineString = "Load/Store";
                    printPipeline(instruction, pipelineString, 1);
                } else {
                    rsmem = NULL;
                }
                //Then check if load can execute (i.e. no stores ahead of it in ROB with same address)
                //Buff is reorder buffer but just for readability
                if (instruction -> isProg2) {
                    buff = cpu -> reorderBuffer2;
                } else {
                    buff = cpu -> reorderBuffer;
                }
                dictEntry = cpu -> loadBuffer -> head;
                instructionFoundOrBubble = 0;
                while (!instructionFoundOrBubble) { //1 for instruction, 2 for bubble
                    if (dictEntry == NULL) {
                        instructionFoundOrBubble = 2;
                    } else {
                        RS = (RSmem *)((DictionaryEntry *)dictEntry -> value -> value);
                        if (RS -> isReady && RS -> address != -1) {
                            for (j = 0; j < buff -> count && j < ((RS->Dest - buff->head)%buff->size) && j != -1; j++) {
                                if (((ROB *)(buff -> items[(buff -> head + j) % (buff->size)]))-> DestAddr == RS -> address) {
                                    j = -2; //break out of for loop
                                }
                            }
                            if (j != -1 && RS != rsmem && RS -> isExecuting != 2) {
                                instructionFoundOrBubble = 1;
                                rsmem = RS;
                            } else {
                                dictEntry = dictEntry -> next;
                            }
                        } else {
                            dictEntry = dictEntry -> next;
                        }
                    }
                }
                if (instructionFoundOrBubble == 1 && instructionsToExec[3] == NULL) {
                    rsmem -> isExecuting = 2;
                    * ((int*)addrPtr) = rsmem -> address;
                    if (instruction -> isProg2) {
                        dataCacheElement = getValueChainByDictionaryKey(dataCache2, addrPtr);
                        if (dataCacheElement != NULL) {
                            valuePtr = dataCacheElement->value->value;
                        } else {
                            *((double *)valuePtr) = 0.0;
                        }
                    } else {
                        dataCacheElement = getValueChainByDictionaryKey(dataCache, addrPtr);
                        if (dataCacheElement != NULL) {
                            valuePtr = dataCacheElement->value->value;
                        } else {
                            *((double *)valuePtr) = 0.0;
                        }
                    }
                    instructionAndResult -> fpResult = *((double*)valuePtr);
                    instructionAndResult -> ROB_number = rsmem -> Dest;
                    LoadPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                    memcpy(LoadPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                    pipelineString = "MEM";
                    printPipeline(instruction, pipelineString, 1);
                }
                break;
            case LD:
                //Two-step
                //First calculate address for earliest load that needs it
                if (rsmem -> isExecuting == 0){
                    rsmem -> isExecuting = 1;
                    rsmem -> address = rsmem -> Vj + instruction->immediate;
                    loadStallROBNumber = rsmem -> Dest;
                    pipelineString = "Load/Store";
                    printPipeline(instruction, pipelineString, 1);
                } else {
                    rsmem = NULL;
                }
                //Then check if load can execute (i.e. no stores ahead of it in ROB with same address)
                //Buff is reorder buffer but just for readability
                if (instruction -> isProg2) {
                    buff = cpu -> reorderBuffer2;
                } else {
                    buff = cpu -> reorderBuffer;
                }
                dictEntry = cpu -> loadBuffer -> head;
                instructionFoundOrBubble = 0;
                while (!instructionFoundOrBubble) { //1 for instruction, 2 for bubble
                    RS = (RSmem *)((DictionaryEntry *)dictEntry -> value -> value);
                    if (RS == NULL) {
                        instructionFoundOrBubble = 2;
                        continue;
                    }
                    if (RS -> isReady && RS -> address != -1) {
                        for (j = 0; j < buff -> count && j < ((RS->Dest - buff->head)%buff->size) && j != -1; j++) {
                            if (((ROB *)(buff -> items[(buff -> head + j) % (buff->size)])) -> DestAddr == RS -> address) {
                                j = -1; //break out of for loop
                            }
                        }
                        if (j != -1 && RS != rsmem && RS -> isExecuting != 2) {
                            instructionFoundOrBubble = 1;
                            rsmem = RS;
                        } else {
                            dictEntry = dictEntry -> next;
                        }
                    } else {
                        dictEntry = dictEntry -> next;
                    }
                }
                if (instructionFoundOrBubble == 1 && instructionsToExec[3] == NULL) {
                    rsmem -> isExecuting = 2;
                    * ((int*)addrPtr) = rsmem -> address;
                    if (instruction -> isProg2) {
                        dataCacheElement = getValueChainByDictionaryKey(dataCache2, addrPtr);
                        if (dataCacheElement != NULL) {
                            valuePtr = dataCacheElement->value->value;
                        } else {
                            *((int *)valuePtr) = 0;
                        }
                    } else {
                        dataCacheElement = getValueChainByDictionaryKey(dataCache, addrPtr);
                        if (dataCacheElement != NULL) {
                            valuePtr = dataCacheElement->value->value;
                        } else {
                            *((int *)valuePtr) = 0;
                        }
                    }
                    instructionAndResult -> intResult = (int)*((double*)valuePtr);
                    instructionAndResult -> ROB_number = rsmem -> Dest;
                    LoadPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                    memcpy(LoadPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                    pipelineString = "MEM";
                    printPipeline(instruction, pipelineString, 1);
                }
                break;
            case SD:
                rsmem -> isExecuting = 1;
                rsmem -> address = rsmem -> Vj + instruction->immediate;
                instructionAndResult -> address = rsmem -> address;
                instructionAndResult -> intResult = rsmem -> iVk;
                instructionAndResult -> ROB_number = rsmem -> Dest;
                StorePipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(StorePipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                storeFirst = 1;
                pipelineString = "Load/Store";
                printPipeline(instruction, pipelineString, 1);
                break;
            case S_D:
                rsmem -> isExecuting = 1;
                rsmem -> address = rsmem -> Vj + instruction->immediate;
                instructionAndResult -> address = rsmem -> address;
                instructionAndResult -> fpResult = rsmem -> fpVk;
                instructionAndResult -> ROB_number = rsmem -> Dest;
                StorePipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(StorePipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                storeFirst = 1;
                pipelineString = "Load/Store";
                printPipeline(instruction, pipelineString, 1);
                break;
            case BNE:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj != rsint -> Vk ? 0 : -1;
                instructionAndResult -> ROB_number = rsint -> Dest;
                BUPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(BUPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "BU";
                printPipeline(instruction, pipelineString, 1);
                break;
            case BNEZ:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj != 0 ? 0 : -1;
                instructionAndResult -> ROB_number = rsint -> Dest;
                BUPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(BUPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "BU";
                printPipeline(instruction, pipelineString, 1);
                break;
            case BEQ:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj == rsint -> Vk ? 0 : -1;
                instructionAndResult -> ROB_number = rsint -> Dest;
                BUPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(BUPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "BU";
                printPipeline(instruction, pipelineString, 1);
                break;
            case BEQZ:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj == 0 ? 0 : -1;
                instructionAndResult -> ROB_number = rsint -> Dest;
                BUPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(BUPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "BU";
                printPipeline(instruction, pipelineString, 1);
                break;
            default:
                break;
        }
    }
    //TODO: check right WB buffers?
    //Take outputs from Units, but only as many as can be accepted by WriteBack Buffer
    int maxOutput = NB - (countDictionaryLen (cpu -> WriteBackBuffer));
    i = 0;
    if (i < maxOutput) {
        unitOutputs[INT] = executePipelinedUnit (cpu -> INTPipeline);
        if (unitOutputs[INT] != NULL) {
            i++;
            key -> reorderNum = unitOutputs[INT] -> ROB_number;
            key -> progNum = unitOutputs[INT] -> instruction -> isProg2 + 1;
            if (unitOutputs[INT] -> instruction -> isProg2) {
                removeDictionaryEntriesByKey (cpu -> renameRegInt2, &(unitOutputs[INT] -> ROB_number));
                addDictionaryEntry (cpu -> renameRegInt2, &(unitOutputs[INT] -> ROB_number), &(unitOutputs[INT] -> intResult));
            } else {
                removeDictionaryEntriesByKey (cpu -> renameRegInt, &(unitOutputs[INT] -> ROB_number));
                addDictionaryEntry (cpu -> renameRegInt, &(unitOutputs[INT] -> ROB_number), &(unitOutputs[INT] -> intResult));
            }
            removeDictionaryEntriesByKey (cpu -> resStaInt, key);
            pipelineString = "INT";
            printPipeline(unitOutputs[INT], pipelineString, 0);
        }
        if (INTPipelineTemp != NULL) {
            enqueueCircular (cpu -> INTPipeline, INTPipelineTemp);
        }
    } else {
        if (INTPipelineTemp != NULL) {
            key -> reorderNum = INTPipelineTemp -> ROB_number;
            key -> progNum = INTPipelineTemp -> instruction -> isProg2 + 1;
            RSint *stalled = (RSint *)(getValueChainByDictionaryKey(cpu -> resStaInt, key) -> value -> value);
            stalled -> isExecuting = 0;
        }
    }
    if (i < maxOutput) {
        unitOutputs[MULT] = executePipelinedUnit (cpu -> MULTPipeline);
        if (unitOutputs[MULT] != NULL) {
            i++;
            key -> reorderNum = unitOutputs[MULT] -> ROB_number;
            key -> progNum = unitOutputs[MULT] -> instruction -> isProg2 + 1;
            if (unitOutputs[MULT] -> instruction -> isProg2) {
                removeDictionaryEntriesByKey (cpu -> renameRegInt2, &(unitOutputs[MULT] -> ROB_number));
                addDictionaryEntry (cpu -> renameRegInt2, &(unitOutputs[MULT] -> ROB_number), &(unitOutputs[MULT] -> intResult));
            } else {
                removeDictionaryEntriesByKey (cpu -> renameRegInt, &(unitOutputs[MULT] -> ROB_number));
                addDictionaryEntry (cpu -> renameRegInt, &(unitOutputs[MULT] -> ROB_number), &(unitOutputs[MULT] -> intResult));
            }
            removeDictionaryEntriesByKey (cpu -> resStaMult, key);
            pipelineString = "MULT";
            printPipeline(unitOutputs[MULT], pipelineString, 0);
        }
        if (MULTPipelineTemp != NULL) {
            enqueueCircular (cpu -> MULTPipeline, MULTPipelineTemp);
        }
    } else {
        if (MULTPipelineTemp != NULL) {
            key -> reorderNum = MULTPipelineTemp -> ROB_number;
            key -> progNum = MULTPipelineTemp -> instruction -> isProg2 + 1;
            RSint *stalled = (RSint *)(getValueChainByDictionaryKey(cpu -> resStaMult, key) -> value -> value);
            stalled -> isExecuting = 0;
        }
    }
    if (i < maxOutput) {
        unitOutputs[LS] = executePipelinedUnit (cpu -> LoadStorePipeline);
        if (unitOutputs[LS] != NULL) {
            i++;
            key -> reorderNum = unitOutputs[LS] -> ROB_number;
            key -> progNum = unitOutputs[LS] -> instruction -> isProg2 + 1;
            if (unitOutputs[LS] -> instruction -> op == L_D) {
                if (unitOutputs[LS] -> instruction -> isProg2) {
                    removeDictionaryEntriesByKey (cpu -> renameRegFP2, &(unitOutputs[LS] -> ROB_number));
                    addDictionaryEntry (cpu -> renameRegFP2, &(unitOutputs[LS] -> ROB_number), &(unitOutputs[LS] -> fpResult));
                } else {
                    removeDictionaryEntriesByKey (cpu -> renameRegFP, &(unitOutputs[LS] -> ROB_number));
                    addDictionaryEntry (cpu -> renameRegFP, &(unitOutputs[LS] -> ROB_number), &(unitOutputs[LS] -> fpResult));
                }
                removeDictionaryEntriesByKey (cpu -> loadBuffer, key);
            } else if (unitOutputs[LS] -> instruction -> op == LD) {
                if (unitOutputs[LS] -> instruction -> isProg2) {
                    removeDictionaryEntriesByKey (cpu -> renameRegInt2, &(unitOutputs[LS] -> ROB_number));
                    addDictionaryEntry (cpu -> renameRegInt2, &(unitOutputs[LS] -> ROB_number), &(unitOutputs[LS] -> intResult));
                } else {
                    removeDictionaryEntriesByKey (cpu -> renameRegInt, &(unitOutputs[LS] -> ROB_number));
                    addDictionaryEntry (cpu -> renameRegInt, &(unitOutputs[LS] -> ROB_number), &(unitOutputs[LS] -> intResult));
                }
                removeDictionaryEntriesByKey (cpu -> loadBuffer, key);
            } else if (unitOutputs[LS] -> instruction -> op == S_D) {
                if (unitOutputs[LS] -> instruction -> isProg2) {
                    removeDictionaryEntriesByKey (cpu -> renameRegFP2, &(unitOutputs[LS] -> ROB_number));
                    addDictionaryEntry (cpu -> renameRegFP2, &(unitOutputs[LS] -> ROB_number), &(unitOutputs[LS] -> fpResult));
                } else {
                    removeDictionaryEntriesByKey (cpu -> renameRegFP, &(unitOutputs[LS] -> ROB_number));
                    addDictionaryEntry (cpu -> renameRegFP, &(unitOutputs[LS] -> ROB_number), &(unitOutputs[LS] -> fpResult));
                }
                removeDictionaryEntriesByKey (cpu -> storeBuffer, key);
            } else {
                 if (unitOutputs[LS] -> instruction -> isProg2) {
                    removeDictionaryEntriesByKey (cpu -> renameRegInt2, &(unitOutputs[LS] -> ROB_number));
                    addDictionaryEntry (cpu -> renameRegInt2, &(unitOutputs[LS] -> ROB_number), &(unitOutputs[LS] -> intResult));
                } else {
                    removeDictionaryEntriesByKey (cpu -> renameRegInt, &(unitOutputs[LS] -> ROB_number));
                    addDictionaryEntry (cpu -> renameRegInt, &(unitOutputs[LS] -> ROB_number), &(unitOutputs[LS] -> intResult));
                }
                removeDictionaryEntriesByKey (cpu -> storeBuffer, key);
            }
            pipelineString = "Load/Store";
            printPipeline(unitOutputs[LS], pipelineString, 0);
        }
        if (!storeFirst && LoadPipelineTemp != NULL) {
            enqueueCircular (cpu -> LoadStorePipeline, LoadPipelineTemp);
        } else if (StorePipelineTemp != NULL) {
            enqueueCircular (cpu -> LoadStorePipeline, StorePipelineTemp);
        }
    } else {
        if (loadStallROBNumber != -1) {
            key -> reorderNum = loadStallROBNumber;
            key -> progNum = ((RSmem *)(((DictionaryEntry *)instructionsToExec[2]) -> value -> value)) -> instruction -> isProg2 + 1;
            RSmem *stalled = (RSmem *)(getValueChainByDictionaryKey(cpu -> loadBuffer, key) -> value -> value);
            stalled -> isExecuting = 0;
        }
        if (LoadPipelineTemp != NULL) {
            key -> reorderNum = LoadPipelineTemp -> ROB_number;
            key -> progNum = LoadPipelineTemp -> instruction -> isProg2 + 1;
            RSmem *stalled = (RSmem *)(getValueChainByDictionaryKey(cpu -> loadBuffer, key) -> value -> value);
            stalled -> isExecuting = 1;
        }
        if (StorePipelineTemp != NULL) {
            key -> reorderNum = StorePipelineTemp -> ROB_number;
            key -> progNum = StorePipelineTemp -> instruction -> isProg2 + 1;
            RSmem *stalled = (RSmem *)(getValueChainByDictionaryKey(cpu -> storeBuffer, key) -> value -> value);
            stalled -> isExecuting = 0;
        }
    }
    if (i < maxOutput) {
        unitOutputs[FPadd] = executePipelinedUnit (cpu -> FPaddPipeline);
        if (unitOutputs[FPadd] != NULL) {
            i++;
            key -> reorderNum = unitOutputs[FPadd] -> ROB_number;
            key -> progNum = unitOutputs[FPadd] -> instruction -> isProg2 + 1;
            if (unitOutputs[FPadd] -> instruction -> isProg2) {
                removeDictionaryEntriesByKey (cpu -> renameRegFP2, &(unitOutputs[FPadd] -> ROB_number));
                addDictionaryEntry (cpu -> renameRegFP2, &(unitOutputs[FPadd] -> ROB_number), &(unitOutputs[FPadd] -> fpResult));
            } else {
                removeDictionaryEntriesByKey (cpu -> renameRegFP, &(unitOutputs[FPadd] -> ROB_number));
                addDictionaryEntry (cpu -> renameRegFP, &(unitOutputs[FPadd] -> ROB_number), &(unitOutputs[FPadd] -> fpResult));
            }
            removeDictionaryEntriesByKey (cpu -> resStaFPadd, key);
            pipelineString = "FPadd";
            printPipeline(unitOutputs[FPadd], pipelineString, 0);
        }
        if (FPaddPipelineTemp != NULL) {
            enqueueCircular (cpu -> FPaddPipeline, FPaddPipelineTemp);
        }
    } else {
        if (FPaddPipelineTemp != NULL) {
            key -> reorderNum = FPaddPipelineTemp -> ROB_number;
            key -> progNum = FPaddPipelineTemp -> instruction -> isProg2 + 1;
            RSfloat *stalled = (RSfloat *)(getValueChainByDictionaryKey(cpu -> resStaFPadd, key) -> value -> value);
            stalled -> isExecuting = 0;
        }
    }
    if (i < maxOutput) {
        unitOutputs[FPmult] = executePipelinedUnit (cpu -> FPmultPipeline);
        if (unitOutputs[FPmult] != NULL) {
            i++;
            key -> reorderNum = unitOutputs[FPmult] -> ROB_number;
            key -> progNum = unitOutputs[FPmult] -> instruction -> isProg2 + 1;
            if (unitOutputs[FPmult] -> instruction -> isProg2) {
                removeDictionaryEntriesByKey (cpu -> renameRegFP2, &(unitOutputs[FPmult] -> ROB_number));
                addDictionaryEntry (cpu -> renameRegFP2, &(unitOutputs[FPmult] -> ROB_number), &(unitOutputs[FPmult] -> fpResult));
            } else {
                removeDictionaryEntriesByKey (cpu -> renameRegFP, &(unitOutputs[FPmult] -> ROB_number));
                addDictionaryEntry (cpu -> renameRegFP, &(unitOutputs[FPmult] -> ROB_number), &(unitOutputs[FPmult] -> fpResult));
            }
            removeDictionaryEntriesByKey (cpu -> resStaFPmult, key);
            pipelineString = "FPmult";
            printPipeline(unitOutputs[FPmult], pipelineString, 0);
        }
        if (FPmultPipelineTemp != NULL) {
            enqueueCircular (cpu -> FPmultPipeline, FPmultPipelineTemp);
        }
    } else {
        if (FPmultPipelineTemp != NULL) {
            key -> reorderNum = FPmultPipelineTemp -> ROB_number;
            key -> progNum = FPmultPipelineTemp -> instruction -> isProg2 + 1;
            RSfloat *stalled = (RSfloat *)(getValueChainByDictionaryKey(cpu -> resStaFPmult, key) -> value -> value);
            stalled -> isExecuting = 0;
        }
    }
    if (i < maxOutput) {
        unitOutputs[FPdiv] = executeFPDivUnit (cpu -> FPdivPipeline);
        if (unitOutputs[FPdiv] != NULL) {
            i++;
            key -> reorderNum = unitOutputs[FPdiv] -> ROB_number;
            key -> progNum = unitOutputs[FPdiv] -> instruction -> isProg2 + 1;
            if (unitOutputs[FPdiv] -> instruction -> isProg2) {
                removeDictionaryEntriesByKey (cpu -> renameRegFP2, &(unitOutputs[FPdiv] -> ROB_number));
                addDictionaryEntry (cpu -> renameRegFP2, &(unitOutputs[FPdiv] -> ROB_number), &(unitOutputs[FPdiv] -> fpResult));
            } else {
                removeDictionaryEntriesByKey (cpu -> renameRegFP, &(unitOutputs[FPdiv] -> ROB_number));
                addDictionaryEntry (cpu -> renameRegFP, &(unitOutputs[FPdiv] -> ROB_number), &(unitOutputs[FPdiv] -> fpResult));
            }
            removeDictionaryEntriesByKey (cpu -> resStaFPdiv, key);
            pipelineString = "FPdiv";
            printPipeline(unitOutputs[FPdiv], pipelineString, 0);
        }
        if (FPdivPipelineTemp != NULL) {
            enqueueCircular (cpu -> FPdivPipeline, FPdivPipelineTemp);
        }
    } else {
        if (FPdivPipelineTemp != NULL) {
            key -> reorderNum = FPdivPipelineTemp -> ROB_number;
            key -> progNum = FPdivPipelineTemp -> instruction -> isProg2 + 1;
            RSfloat *stalled = (RSfloat *)(getValueChainByDictionaryKey(cpu -> resStaFPdiv, &(FPdivPipelineTemp -> ROB_number)) -> value -> value);
            stalled -> isExecuting = 0;
            cpu -> FPdivPipelineBusy = 0;
        }
    }
    if (i < maxOutput) {
        unitOutputs[BU] = executePipelinedUnit (cpu -> BUPipeline);
        if (unitOutputs[BU] != NULL) {
            i++;
            key -> reorderNum = unitOutputs[BU] -> ROB_number;
            key -> progNum = unitOutputs[BU] -> instruction -> isProg2 + 1;
            if (unitOutputs[BU] -> instruction -> isProg2) {
                removeDictionaryEntriesByKey (cpu -> renameRegInt2, &(unitOutputs[BU] -> ROB_number));
                addDictionaryEntry (cpu -> renameRegInt2, &(unitOutputs[BU] -> ROB_number), &(unitOutputs[BU] -> intResult));
            } else {
                removeDictionaryEntriesByKey (cpu -> renameRegInt, &(unitOutputs[BU] -> ROB_number));
                addDictionaryEntry (cpu -> renameRegInt, &(unitOutputs[BU] -> ROB_number), &(unitOutputs[BU] -> intResult));
            }
            removeDictionaryEntriesByKey (cpu -> resStaBU, key);
            branchHelper (unitOutputs[BU]);
            pipelineString = "BU";
            printPipeline(unitOutputs[BU], pipelineString, 0);
        }
        if (BUPipelineTemp != NULL) {
            enqueueCircular (cpu -> BUPipeline, BUPipelineTemp);
        }
    } else {
        if (BUPipelineTemp != NULL) {
            key -> reorderNum = BUPipelineTemp -> ROB_number;
            key -> progNum = BUPipelineTemp -> instruction -> isProg2 + 1;
            RSint *stalled = (RSint *)(getValueChainByDictionaryKey(cpu -> resStaBU, &(BUPipelineTemp -> ROB_number)) -> value -> value);
            stalled -> isExecuting = 0;
        }
    }

    return unitOutputs;
}


//Initialize ROB struct
ROB * InitializeROBEntry(Instruction * instructionP)
{

	ROB * ROBEntry ;
	OpCode op;
	
	ROBEntry = (ROB*) malloc (sizeof(ROB));	
	ROBEntry -> isStore = 0;
	ROBEntry -> DestRenameReg = cpu->reorderBuffer -> tail;
	ROBEntry -> isBranch = 0 ;
	ROBEntry -> isCorrectPredict = 1;
	ROBEntry -> DestAddr = 0;
	op = instructionP -> op;
	
	  switch (op) {
        case ANDI:
			ROBEntry->DestReg = instructionP->rd;
			ROBEntry -> isINT = 1;
            break;
        case AND:
            ROBEntry->DestReg = instructionP->rd;
			ROBEntry -> isINT = 1;
            break;
        case ORI:
            ROBEntry->DestReg = instructionP->rd;
		ROBEntry -> isINT = 1;
            break;
        case OR:
            ROBEntry->DestReg = instructionP->rd;
		ROBEntry -> isINT = 1;
            break;
        case SLTI:
            ROBEntry->DestReg = instructionP->rd;
		ROBEntry -> isINT = 1;
            break;
        case SLT:
          ROBEntry->DestReg = instructionP->rd;
		ROBEntry -> isINT = 1;
            break;
        case DADDI:
            ROBEntry->DestReg = instructionP->rd;
		ROBEntry -> isINT = 1;
            break;
        case DADD:
            ROBEntry->DestReg = instructionP->rd;
		ROBEntry -> isINT = 1;
            break;
        case DSUB:
		ROBEntry->DestReg = instructionP->rd;
		ROBEntry -> isINT = 1;
            break;
        case DMUL:
		ROBEntry->DestReg = instructionP->rd;
		ROBEntry -> isINT = 1;
            break;
        case ADD_D:
		ROBEntry->DestReg = instructionP->fd;
			ROBEntry -> isINT = 0;
            break;
        case SUB_D:
			ROBEntry->DestReg = instructionP->fd;
			ROBEntry -> isINT = 0;
            break;
        case MUL_D:
			ROBEntry->DestReg = instructionP->fd;
			ROBEntry -> isINT = 0;
            break;
        case DIV_D:
			ROBEntry->DestReg = instructionP->fd;
			ROBEntry -> isINT = 0;
            break;
        case L_D:
		ROBEntry->DestReg = instructionP->ft;
			ROBEntry -> isINT = 0;
            break;
        case LD:
			ROBEntry->DestReg = instructionP->rt;
			ROBEntry -> isINT = 1;
            break;
        case SD:
            ROBEntry->DestReg = -1;
			ROBEntry -> isINT = 1;
			ROBEntry -> isStore = 1;
			ROBEntry -> DestAddr = 0;

		
            break;
        case S_D:
			ROBEntry->DestReg = -1;
			ROBEntry -> isINT = 0;
			ROBEntry -> isStore = 1;
			ROBEntry -> DestAddr = 0;

		
            break;
        case BNE:

				ROBEntry->DestReg = -1;
				ROBEntry -> isINT = 0;
				ROBEntry -> isBranch = 1;
            break;
        case BNEZ:

        case BEQ:

        case BEQZ:
				ROBEntry->DestReg = -1;
				ROBEntry -> isINT = 0;
				ROBEntry->isBranch = 1;
				cpu->isAfterBranch = 1;
            break;
        default:
            break;
    }
	
	ROBEntry->instruction = instructionP;
	ROBEntry->state = "I";
	ROBEntry->isReady = 0;
	ROBEntry->isAfterBranch = cpu->isAfterBranch;
	return ROBEntry;
}


//Initialize ROB struct for part 2
ROB * InitializeROBEntry2(Instruction * instructionP)
{

	ROB * ROBEntry ;
	OpCode op;

	ROBEntry = (ROB*) malloc (sizeof(ROB));
	ROBEntry -> isStore = 0;
	ROBEntry -> DestRenameReg = cpu->reorderBuffer2 -> tail;
	ROBEntry -> isBranch = 0 ;
	ROBEntry -> isCorrectPredict = 1;
	ROBEntry -> DestAddr = 0;
	op = instructionP -> op;

	  switch (op) {
        case ANDI:
			ROBEntry->DestReg = instructionP->rd;
			ROBEntry -> isINT = 1;
            break;
        case AND:
            ROBEntry->DestReg = instructionP->rd;
			ROBEntry -> isINT = 1;
            break;
        case ORI:
            ROBEntry->DestReg = instructionP->rd;
		ROBEntry -> isINT = 1;
            break;
        case OR:
            ROBEntry->DestReg = instructionP->rd;
		ROBEntry -> isINT = 1;
            break;
        case SLTI:
            ROBEntry->DestReg = instructionP->rd;
		ROBEntry -> isINT = 1;
            break;
        case SLT:
          ROBEntry->DestReg = instructionP->rd;
		ROBEntry -> isINT = 1;
            break;
        case DADDI:
            ROBEntry->DestReg = instructionP->rd;
		ROBEntry -> isINT = 1;
            break;
        case DADD:
            ROBEntry->DestReg = instructionP->rd;
		ROBEntry -> isINT = 1;
            break;
        case DSUB:
		ROBEntry->DestReg = instructionP->rd;
		ROBEntry -> isINT = 1;
            break;
        case DMUL:
		ROBEntry->DestReg = instructionP->rd;
		ROBEntry -> isINT = 1;
            break;
        case ADD_D:
		ROBEntry->DestReg = instructionP->fd;
			ROBEntry -> isINT = 0;
            break;
        case SUB_D:
			ROBEntry->DestReg = instructionP->fd;
			ROBEntry -> isINT = 0;
            break;
        case MUL_D:
			ROBEntry->DestReg = instructionP->fd;
			ROBEntry -> isINT = 0;
            break;
        case DIV_D:
			ROBEntry->DestReg = instructionP->fd;
			ROBEntry -> isINT = 0;
            break;
        case L_D:
		ROBEntry->DestReg = instructionP->ft;
			ROBEntry -> isINT = 0;
            break;
        case LD:
			ROBEntry->DestReg = instructionP->rt;
			ROBEntry -> isINT = 1;
            break;
        case SD:
            ROBEntry->DestReg = -1;
			ROBEntry -> isINT = 1;
			ROBEntry -> isStore = 1;
			ROBEntry -> DestAddr = 0;


            break;
        case S_D:
			ROBEntry->DestReg = -1;
			ROBEntry -> isINT = 0;
			ROBEntry -> isStore = 1;
			ROBEntry -> DestAddr = 0;


            break;
        case BNE:

				ROBEntry->DestReg = -1;
				ROBEntry -> isINT = 0;
				ROBEntry -> isBranch = 1;
            break;
        case BNEZ:

        case BEQ:

        case BEQZ:
				ROBEntry->DestReg = -1;
				ROBEntry -> isINT = 0;
				ROBEntry->isBranch = 1;
				cpu->isAfterBranch2 = 1;
            break;
        default:
            break;
    }

	ROBEntry->instruction = instructionP;
	ROBEntry->state = "I";
	ROBEntry->isReady = 0;
	ROBEntry->isAfterBranch = cpu->isAfterBranch2;
	return ROBEntry;
}


//commit instructions or flush ROB
int Commit(int NC, int NR, int returncount)
{
	// commit instructions from ROB
	ROB * ROBEntry;
	RegStatus *RegStatusEntry;
	void *valuePtr = malloc(sizeof(double));
	int robnum;
	int rcount = returncount;
		ROBEntry = cpu -> reorderBuffer -> items[cpu->reorderBuffer ->head];
//		while(ROBEntry != NULL && NC != 0)
		while (cpu->reorderBuffer->count != 0 && NC != 0)
		{
				//printf("Checking instruction %d for commiting\n", ROBEntry -> instruction -> address);
				if((strcmp(ROBEntry -> state, "W") == 0) && ROBEntry -> isReady == 1)
				{
					ROBEntry = dequeueCircular(cpu -> reorderBuffer);
					//printf("Checked instruction %d for commiting\n", ROBEntry -> instruction -> address);
					if(ROBEntry -> isINT == 1 && ROBEntry -> isStore == 0 && ROBEntry -> isBranch == 0){
							int DestRenameReg, DestVal, DestReg;
							DestRenameReg = ROBEntry -> DestRenameReg;
							DestReg = ROBEntry -> DestReg;
							DictionaryEntry * Current = getValueChainByDictionaryKey(cpu -> renameRegInt, &DestRenameReg);
							DestVal = *((int *)Current -> value -> value);
							printf("%d\n", DestVal );
							cpu -> integerRegisters [DestReg] -> data = DestVal;
							RegStatusEntry = cpu -> IntRegStatus[DestReg];
							robnum = (cpu->reorderBuffer -> head - 1)%cpu->reorderBuffer->size;
							//printf("reg status rob muber - %d\t Commit instruction ROB number - %d\n",RegStatusEntry -> reorderNum, cpu->reorderBuffer->head);
							if(RegStatusEntry -> reorderNum == robnum){
								RegStatusEntry->busy = 0;
							}
							removeDictionaryEntriesByKey(cpu -> renameRegInt, &DestRenameReg);
							printf("Committed instruction %d in integer register number %d with value %d \n", ROBEntry -> instruction -> address, DestReg, DestVal);
							NC --;
							rcount++;
					}
					else if(ROBEntry -> isINT == 0 && ROBEntry -> isStore == 0 && ROBEntry -> isBranch == 0){
						int DestRenameReg, DestReg; float DestVal;
							DestRenameReg = ROBEntry -> DestRenameReg;
							DestReg = ROBEntry -> DestReg;
							DictionaryEntry * Current = getValueChainByDictionaryKey(cpu -> renameRegFP , &DestRenameReg);
							DestVal = *((double *)Current -> value -> value);
							printf("%f\n", DestVal );
							cpu -> floatingPointRegisters [DestReg] -> data = DestVal;
							RegStatusEntry = cpu -> FPRegStatus[DestReg];
							robnum = (cpu->reorderBuffer -> head - 1)%cpu->reorderBuffer->size;
							if(RegStatusEntry -> reorderNum == robnum){
								RegStatusEntry->busy = 0;
							}
							removeDictionaryEntriesByKey(cpu -> renameRegFP, &DestRenameReg);
							printf("Committed instruction %d in floating point register number %d with value %f\n", ROBEntry -> instruction -> address, DestReg, DestVal);
							NC --;
							rcount++;
					}
					else if(ROBEntry -> isStore == 1)
					{
					
						if(ROBEntry -> isINT == 1){
							int DestVal, DestRenameReg;
							DestRenameReg = ROBEntry -> DestRenameReg;
							DictionaryEntry * Current = getValueChainByDictionaryKey(cpu -> renameRegInt, &DestRenameReg);
							DestVal = *((int *)Current -> value -> value);
							removeDictionaryEntriesByKey (dataCache, &(ROBEntry -> DestAddr));
							*((int*)valuePtr) = DestVal; // value from rename register ;
							addDictionaryEntry (dataCache, &(ROBEntry -> DestAddr), valuePtr);
							printf("Committed instruction SD %d in memory address %d \n", ROBEntry -> instruction -> address, ROBEntry -> DestAddr);
							NC --;
							rcount++;
						}
						else if(ROBEntry -> isINT == 0){
							float DestVal; int DestRenameReg;
							DestRenameReg = ROBEntry -> DestRenameReg;
							DictionaryEntry * Current = getValueChainByDictionaryKey(cpu -> renameRegFP, &DestRenameReg);
							DestVal = *((double *)Current -> value -> value);
							removeDictionaryEntriesByKey (dataCache, &(ROBEntry -> DestAddr));
							*((double*)valuePtr) = (double) DestVal; // value from rename register ;
							addDictionaryEntry (dataCache, &(ROBEntry -> DestAddr), valuePtr);
							printf("Committed instruction S_D %d in memory address %d \n", ROBEntry -> instruction -> address, ROBEntry -> DestAddr);							
							//DestVal = 0;
							NC --;
							rcount++;
						}
					}
					else{
						//Branch
						if(ROBEntry ->isBranch == 1 ){
						if( ROBEntry -> isCorrectPredict == 0){
							// move head to isafterbranch == 0
							int i = 0;
							ROB *ROBentrySecond = cpu -> reorderBuffer-> items[(cpu->reorderBuffer->head + i)%cpu->reorderBuffer->size];
							int robnum = (cpu->reorderBuffer->head + i)%cpu->reorderBuffer->size;
							while(ROBentrySecond != NULL){
								ROB *ROBentrySecond = cpu -> reorderBuffer-> items[(cpu->reorderBuffer->head + i)%cpu->reorderBuffer->size];
								robnum = (cpu->reorderBuffer->head + i)%cpu->reorderBuffer->size;
								if(ROBentrySecond != NULL){
									if(robnum == cpu -> reorderBuffer -> tail){
										printf("ROB is empty now\n");
										//cpu -> reorderBuffer -> head = cpu -> reorderBuffer -> tail;
										cpu -> reorderBuffer = createCircularQueue(NR);
										break;
									}
									if(ROBentrySecond -> isAfterBranch == 0)
									{
										cpu -> reorderBuffer -> head = (cpu->reorderBuffer->head + i)%cpu->reorderBuffer->size;
										cpu -> reorderBuffer -> count = cpu -> reorderBuffer -> count - i;
										printf("Branch mispredicted so flushed ROB.\n");
										break;
									}
									else{
										KeyRS *robnumkey = (KeyRS *)malloc(sizeof(KeyRS));
										robnumkey -> reorderNum = robnum;
										robnumkey -> progNum = 1;
										if(cpu -> WriteBackBuffer != NULL){
											removeDictionaryEntriesByKey(cpu -> WriteBackBuffer, robnumkey); 
										}
										if(getValueChainByDictionaryKey(cpu -> renameRegInt, &(robnum))  != NULL){
											removeDictionaryEntriesByKey(cpu -> renameRegInt, &(robnum)); 
										}
										else if(getValueChainByDictionaryKey(cpu -> renameRegFP, &(robnum)) != NULL){
											removeDictionaryEntriesByKey(cpu -> renameRegFP, &(robnum)); 
										}
										
										//go to next
									}
									i++;
								}
								else{
									cpu -> reorderBuffer -> head = (cpu->reorderBuffer->head + i)%cpu->reorderBuffer->size;
									cpu -> reorderBuffer -> count = cpu -> reorderBuffer -> count - i;
									printf("Branch mispredicted so flushed ROB.\n");
									break;
								}
							}
						}
						printf("Committed branch instruction %d\n", ROBEntry -> instruction -> address);
						
						}
						NC--;
						rcount++;
					}
				}
		else{
		
			break;
		}
		
		ROBEntry = cpu -> reorderBuffer-> items[cpu->reorderBuffer->head];
		}
	return rcount;
}



//for prog 2
//commit instructions or flush ROB
int Commit2(int NC, int NR, int returncount)
{
	// commit instructions from ROB
	ROB * ROBEntry;
	RegStatus *RegStatusEntry;
	void *valuePtr = malloc(sizeof(double));
	int robnum;
		ROBEntry = cpu -> reorderBuffer2 -> items[cpu->reorderBuffer2 ->head];
//		while(ROBEntry != NULL && NC != 0)
		while (cpu->reorderBuffer2->count != 0 && NC != 0 )
		{
				//printf("Checking instruction %d for commiting\n", ROBEntry -> instruction -> address);
				if((strcmp(ROBEntry -> state, "W") == 0) && ROBEntry -> isReady == 1)
				{
					ROBEntry = dequeueCircular(cpu -> reorderBuffer2);
					//printf("Checked instruction %d for commiting\n", ROBEntry -> instruction -> address);
					if(ROBEntry -> isINT == 1 && ROBEntry -> isStore == 0 && ROBEntry -> isBranch == 0){
							int DestRenameReg, DestVal, DestReg;
							DestRenameReg = ROBEntry -> DestRenameReg;
							DestReg = ROBEntry -> DestReg;
							DictionaryEntry * Current = getValueChainByDictionaryKey(cpu -> renameRegInt2, &DestRenameReg);
							DestVal = *((int *)Current -> value -> value);
							cpu -> integerRegisters2 [DestReg] -> data = DestVal;
							RegStatusEntry = cpu -> IntRegStatus2[DestReg];
							robnum = (cpu->reorderBuffer2 -> head - 1)%cpu->reorderBuffer2->size;
							//printf("reg status rob muber - %d\t Commit instruction ROB number - %d\n",RegStatusEntry -> reorderNum, cpu->reorderBuffer->head);
							if(RegStatusEntry -> reorderNum == robnum){
								RegStatusEntry->busy = 0;
							}
							removeDictionaryEntriesByKey(cpu -> renameRegInt2, &DestRenameReg);
							printf("Committed instruction %d in integer register number %d with value %d \n", ROBEntry -> instruction -> address, DestReg, DestVal);
							NC --;
							returncount++;
					}
					else if(ROBEntry -> isINT == 0 && ROBEntry -> isStore == 0 && ROBEntry -> isBranch == 0){
						int DestRenameReg, DestReg; float DestVal;
							DestRenameReg = ROBEntry -> DestRenameReg;
							DestReg = ROBEntry -> DestReg;
							DictionaryEntry * Current = getValueChainByDictionaryKey(cpu -> renameRegFP2 , &DestRenameReg);
							DestVal = *((double *)Current -> value -> value);
							cpu -> floatingPointRegisters2 [DestReg] -> data = DestVal;
							RegStatusEntry = cpu -> FPRegStatus2[DestReg];
							robnum = (cpu->reorderBuffer2 -> head - 1)%cpu->reorderBuffer2->size;
							if(RegStatusEntry -> reorderNum == robnum){
								RegStatusEntry->busy = 0;
							}
							removeDictionaryEntriesByKey(cpu -> renameRegFP2, &DestRenameReg);
							printf("Committed instruction %d in floating point register number %d with value %f\n", ROBEntry -> instruction -> address, DestReg, DestVal);
							NC --;
							returncount++;
					}
					else if(ROBEntry -> isStore == 1)
					{
						if(ROBEntry -> isINT == 1){
							int DestVal, DestRenameReg;
							DestRenameReg = ROBEntry -> DestRenameReg;
							DictionaryEntry * Current = getValueChainByDictionaryKey(cpu -> renameRegInt2, &DestRenameReg);
							DestVal = *((int *)Current -> value -> value);
							removeDictionaryEntriesByKey (dataCache2, &(ROBEntry -> DestAddr));
							*((int*)valuePtr) = DestVal; // value from rename register ;
							addDictionaryEntry (dataCache2, &(ROBEntry -> DestAddr), valuePtr);
							//removeDictionaryEntriesByKey(cpu -> renameRegInt, &DestRenameReg);
							printf("Committed instruction SD %d in memory address %d with value %d \n", ROBEntry -> instruction -> address, ROBEntry -> DestAddr, DestVal);
							NC --;
							returncount++;
						}
						else if(ROBEntry -> isINT == 0){
							float DestVal; int DestRenameReg;
							DestRenameReg = ROBEntry -> DestRenameReg;
							DictionaryEntry * Current = getValueChainByDictionaryKey(cpu -> renameRegFP2, &DestRenameReg);
							DestVal = *((double *)Current -> value -> value);
							removeDictionaryEntriesByKey (dataCache2, &(ROBEntry -> DestAddr));
							*((double*)valuePtr) = (double) DestVal; // value from rename register ;
							addDictionaryEntry (dataCache2, &(ROBEntry -> DestAddr), valuePtr);
							//removeDictionaryEntriesByKey(cpu -> renameRegInt, &DestRenameReg);
							printf("Committed instruction S_D %d in memory address %d with value %f \n", ROBEntry -> instruction -> address, ROBEntry -> DestAddr, DestVal);							
							//DestVal = 0;
							NC --;
							returncount++;
						}
					}
					else{
						//Branch
						if(ROBEntry ->isBranch == 1 ){
						if( ROBEntry -> isCorrectPredict == 0){
							// move head to isafterbranch == 0
							int i = 0;
							ROB *ROBentrySecond = cpu -> reorderBuffer2-> items[(cpu->reorderBuffer2->head + i)%cpu->reorderBuffer2->size];
							int robnum = (cpu->reorderBuffer2->head + i)%cpu->reorderBuffer2->size;
							while(ROBentrySecond != NULL){
								ROB *ROBentrySecond = cpu -> reorderBuffer2-> items[(cpu->reorderBuffer2->head + i)%cpu->reorderBuffer2->size];
								robnum = (cpu->reorderBuffer2->head + i)%cpu->reorderBuffer2->size;
								if(ROBentrySecond != NULL){
									if(robnum == cpu -> reorderBuffer2 -> tail){
										printf("ROB is empty now\n");
										//cpu -> reorderBuffer2 -> head = cpu -> reorderBuffer2 -> tail;
										cpu -> reorderBuffer2 = createCircularQueue(NR);
										break;
									}
									if(ROBentrySecond -> isAfterBranch == 0)
									{
										cpu -> reorderBuffer2 -> head = (cpu->reorderBuffer2->head + i)%cpu->reorderBuffer2->size;
										cpu -> reorderBuffer2 -> count = cpu -> reorderBuffer2 -> count - i;
										printf("Branch mispredicted so flushed ROB.\n");
										break;
									}
									else{
										KeyRS *robnumkey = (KeyRS *)malloc(sizeof(KeyRS));
										robnumkey -> reorderNum = robnum;
										robnumkey -> progNum = 1;
										if(cpu -> WriteBackBuffer != NULL){
											removeDictionaryEntriesByKey(cpu -> WriteBackBuffer, robnumkey); 
										}
										if(getValueChainByDictionaryKey(cpu -> renameRegInt, &(robnum))  != NULL){
											removeDictionaryEntriesByKey(cpu -> renameRegInt, &(robnum)); 
										}
										else if(getValueChainByDictionaryKey(cpu -> renameRegFP, &(robnum)) != NULL){
											removeDictionaryEntriesByKey(cpu -> renameRegFP, &(robnum)); 
										}
										
										//go to next
									}
									i++;
								}
								else{
									cpu -> reorderBuffer2 -> head = (cpu->reorderBuffer2->head + i)%cpu->reorderBuffer2->size;
									cpu -> reorderBuffer2 -> count = cpu -> reorderBuffer2 -> count - i;
									printf("Branch mispredicted so flushed ROB.\n");
									break;
								}
							}
						}
						printf("Committed branch instruction %d\n", ROBEntry -> instruction -> address);
						
						}
						NC--;
						returncount++;
					}
				}
		else{
		
			break;
		}
		
		ROBEntry = cpu -> reorderBuffer2-> items[cpu->reorderBuffer2->head];
		}
return returncount;
}



// update RES with output from execution
void updateOutputRES(CompletedInstruction *instruction){
	printf("Updating reservation stations with ROB - %d\n", instruction -> ROB_number);
	int robnumber = instruction -> ROB_number;
	DictionaryEntry *current;
	RSint *RSint;
	RSfloat *RSfloat;
	RSmem *RSmem;
	int isProg2;
	 switch (instruction -> instruction -> op) {
        case ANDI:
        case AND:
        case ORI:
        case OR:
        case SLTI:
        case SLT:
        case DADDI:
        case DADD:
        case DSUB:
		case DMUL:
		case BNE:
        case BNEZ:
        case BEQ:
        case BEQZ:
		case LD:
		case SD :
			for (current = cpu -> resStaBU -> head; current != NULL; current = current -> next){
				RSint = current -> value -> value;
				isProg2 = RSint->instruction->isProg2;
				if(isProg2 == 0){
					if (RSint -> isReady == 0){
						if(RSint -> Qj == robnumber){
								RSint -> Vj = instruction -> intResult;
								RSint -> Qj = -1;
							}
						if(RSint -> Qk == robnumber){
							RSint -> Vk = instruction -> intResult;
							RSint -> Qk = -1;
						}

						if ((RSint -> Qj == -1) && (RSint -> Qk == -1)){
							RSint -> isReady = 1;
						}
					}
				}
			}
            for (current = cpu -> resStaInt -> head; current != NULL; current = current -> next){
                RSint = current -> value -> value;
				isProg2 = RSint->instruction-> isProg2;
				if(isProg2 == 0){
					if (RSint -> isReady == 0){
						if(RSint -> Qj == robnumber){
							RSint -> Vj = instruction -> intResult;
							RSint -> Qj = -1;
						}
						if(RSint -> Qk == robnumber){
							RSint -> Vk = instruction -> intResult;
							 RSint -> Qk = -1;
						}
						 if ((RSint -> Qj == -1) && (RSint -> Qk == -1)){
								RSint -> isReady = 1;
						}
					}
				}
			}
         	for (current = cpu -> resStaMult -> head; current != NULL; current = current -> next){
				RSint = current -> value -> value;
				isProg2 = RSint ->instruction-> isProg2;
				if(isProg2 == 0){
					if (RSint -> isReady == 0){
						if(RSint -> Qj == robnumber){
							RSint -> Vj = instruction -> intResult;
							RSint -> Qj = -1;
						}
						if(RSint -> Qk == robnumber){
							RSint -> Vk = instruction -> intResult;
							RSint -> Qk = -1;
						}
						if ((RSint -> Qj == -1) && (RSint -> Qk == -1)){
								RSint -> isReady = 1;
						}
					}
				}
			}
			for (current = cpu -> storeBuffer -> head; current != NULL; current = current -> next){
					RSmem = current -> value -> value;
					isProg2 = RSmem ->instruction-> isProg2;
					if(isProg2 == 0){
						if (RSmem -> isReady == 0){
							if(RSmem -> Qj == robnumber){
								RSmem -> Vj = instruction -> intResult;
								RSmem -> Qj = -1;						
							}
							if(RSmem -> Qk == robnumber){
								RSmem -> iVk = instruction -> intResult;
								RSmem -> Qk = -1;
							}
							if ((RSmem -> Qj == -1) && (RSmem -> Qk == -1)){
								RSmem -> isReady = 1;
							}
						}
					}
			}
            for (current = cpu -> loadBuffer -> head; current != NULL; current = current -> next){
                RSmem = current -> value -> value;
				isProg2 = RSmem ->instruction-> isProg2;
				if(isProg2 == 0){
					if (RSmem -> isReady == 0){
						if(RSmem -> Qj == robnumber){
							RSmem -> Vj = instruction -> intResult;
							RSmem -> Qj = -1;		
						}
						if (RSmem -> Qj == -1){
									RSmem -> isReady = 1;
						}
					}
				}
            }
            break;
        case ADD_D:
        case SUB_D:
		case MUL_D:
		case DIV_D:
		case S_D:
		case L_D:
				for (current = cpu -> resStaFPadd -> head; current != NULL; current = current -> next){
					RSfloat = current -> value -> value;
					isProg2 = RSfloat ->instruction-> isProg2;
					if(isProg2 == 0){
						if (RSfloat -> isReady == 0){
							if(RSfloat -> Qj == robnumber){
								RSfloat -> Vj = instruction -> fpResult;	
								RSfloat -> Qj = -1;
							}
							if(RSfloat -> Qk == robnumber){
								RSfloat -> Vk = instruction -> fpResult;
								RSfloat -> Qk = -1;
							}
							if ((RSfloat -> Qj == -1) && (RSfloat -> Qk == -1)){
									RSfloat -> isReady = 1;
							}
						}
					}
				}
				for (current = cpu -> loadBuffer -> head; current != NULL; current = current -> next){
				RSmem = current -> value -> value;
				isProg2 = RSmem ->instruction-> isProg2;
				if(isProg2 == 0){
				 if (RSmem -> isReady == 0){
					if(RSmem -> Qj == robnumber){
								RSmem -> Vj = instruction -> intResult;
								RSmem -> Qj = -1;							
							}
							if (RSmem -> Qj == -1){
									RSmem -> isReady = 1;
						}
				 }
				}
				}
				for (current = cpu -> resStaFPmult -> head; current != NULL; current = current -> next){
				RSfloat = current -> value -> value;
				isProg2 = RSfloat ->instruction-> isProg2;
					if(isProg2 == 0){
					if (RSfloat -> isReady == 0){
						if(RSfloat -> Qj == robnumber){
							RSfloat -> Vj = instruction -> fpResult;
							RSfloat -> Qj = -1;						
						}
						if(RSfloat -> Qk == robnumber){
							RSfloat -> Vk = instruction -> fpResult;
							RSfloat -> Qk = -1;
							
						}
						if ((RSfloat -> Qj == -1) && (RSfloat -> Qk == -1)){
									RSfloat -> isReady = 1;
						}
					}
					}
				}  
			for (current = cpu -> resStaFPdiv -> head; current != NULL; current = current -> next){
					RSfloat = current -> value -> value;
					isProg2 = RSfloat ->instruction-> isProg2;
					if(isProg2 == 0){
						if (RSfloat -> isReady == 0){
							if(RSfloat -> Qj == robnumber){
								RSfloat -> Vj = instruction -> fpResult;
								RSfloat -> Qj = -1;						
							}
							if(RSfloat -> Qk == robnumber){
								RSfloat -> Vk = instruction -> fpResult;
								RSfloat -> Qk = -1;
							}
							if ((RSfloat -> Qj == -1) && (RSfloat -> Qk == -1)){
										RSfloat -> isReady = 1;
								}
						}
					}
				}
			
            for (current = cpu -> storeBuffer -> head; current != NULL; current = current -> next){
					RSmem = current -> value -> value;
					isProg2 = RSmem ->instruction-> isProg2;
					if(isProg2 == 0){
					 if (RSmem -> isReady == 0){
						if(RSmem -> Qj == robnumber){
							RSmem -> fpVk = instruction -> fpResult;	
							RSmem -> Qj = -1;
						}
						if(RSmem -> Qk == robnumber){
							RSmem -> fpVk = instruction -> fpResult;
							RSmem -> Qk = -1;
						}
						if ((RSmem -> Qj == -1) && (RSmem -> Qk == -1)){
									RSmem -> isReady = 1;
							}
					 }
				}
				}
            break;
     
        default:
            break;

	 }
}



//for prog 2
// update RES with output from execution
void updateOutputRES2(CompletedInstruction *instruction){
	printf("Updating reservation stations with ROB - %d\n", instruction -> ROB_number);
	int robnumber = instruction -> ROB_number;
	DictionaryEntry *current;
	RSint *RSint;
	RSfloat *RSfloat;
	RSmem *RSmem;
	int isProg2;
	 switch (instruction -> instruction -> op) {
        case ANDI:
        case AND:
        case ORI:
        case OR:
        case SLTI:
        case SLT:
        case DADDI:
        case DADD:
        case DSUB:
		case DMUL:
		case BNE:
        case BNEZ:
        case BEQ:
        case BEQZ:
		case LD:
		case SD :
			for (current = cpu -> resStaBU -> head; current != NULL; current = current -> next){
				RSint = current -> value -> value;
				isProg2 = RSint->instruction->isProg2;
				if(isProg2 == 1){
					if (RSint -> isReady == 0){
						if(RSint -> Qj == robnumber){
								RSint -> Vj = instruction -> intResult;
								RSint -> Qj = -1;
							}
						if(RSint -> Qk == robnumber){
							RSint -> Vk = instruction -> intResult;
							RSint -> Qk = -1;
						}

						if ((RSint -> Qj == -1) && (RSint -> Qk == -1)){
							RSint -> isReady = 1;
						}
					}
				}
			}
            for (current = cpu -> resStaInt -> head; current != NULL; current = current -> next){
                RSint = current -> value -> value;
				isProg2 = RSint->instruction-> isProg2;
				if(isProg2 == 1){
					if (RSint -> isReady == 0){
						if(RSint -> Qj == robnumber){
							RSint -> Vj = instruction -> intResult;
							RSint -> Qj = -1;
						}
						if(RSint -> Qk == robnumber){
							RSint -> Vk = instruction -> intResult;
							 RSint -> Qk = -1;
						}
						 if ((RSint -> Qj == -1) && (RSint -> Qk == -1)){
								RSint -> isReady = 1;
						}
					}
				}
			}
         	for (current = cpu -> resStaMult -> head; current != NULL; current = current -> next){
				RSint = current -> value -> value;
				isProg2 = RSint ->instruction-> isProg2;
				if(isProg2 == 1){
					if (RSint -> isReady == 0){
						if(RSint -> Qj == robnumber){
							RSint -> Vj = instruction -> intResult;
							RSint -> Qj = -1;
						}
						if(RSint -> Qk == robnumber){
							RSint -> Vk = instruction -> intResult;
							RSint -> Qk = -1;
						}
						if ((RSint -> Qj == -1) && (RSint -> Qk == -1)){
								RSint -> isReady = 1;
						}
					}
				}
			}
			for (current = cpu -> storeBuffer -> head; current != NULL; current = current -> next){
					RSmem = current -> value -> value;
					isProg2 = RSmem ->instruction-> isProg2;
					if(isProg2 == 1){
						if (RSmem -> isReady == 0){
							if(RSmem -> Qj == robnumber){
								RSmem -> Vj = instruction -> intResult;
								RSmem -> Qj = -1;						
							}
							if(RSmem -> Qk == robnumber){
								RSmem -> iVk = instruction -> intResult;
								RSmem -> Qk = -1;
							}
							if ((RSmem -> Qj == -1) && (RSmem -> Qk == -1)){
								RSmem -> isReady = 1;
							}
						}
					}
			}
            for (current = cpu -> loadBuffer -> head; current != NULL; current = current -> next){
                RSmem = current -> value -> value;
				isProg2 = RSmem ->instruction-> isProg2;
				if(isProg2 == 1){
					if (RSmem -> isReady == 0){
						if(RSmem -> Qj == robnumber){
							RSmem -> Vj = instruction -> intResult;
							RSmem -> Qj = -1;		
						}
						if (RSmem -> Qj == -1){
									RSmem -> isReady = 1;
						}
					}
				}
            }
            break;
        case ADD_D:
        case SUB_D:
		case MUL_D:
		case DIV_D:
		case S_D:
		case L_D:
				for (current = cpu -> resStaFPadd -> head; current != NULL; current = current -> next){
					RSfloat = current -> value -> value;
					isProg2 = RSfloat ->instruction-> isProg2;
					if(isProg2 == 1){
						if (RSfloat -> isReady == 0){
							if(RSfloat -> Qj == robnumber){
								RSfloat -> Vj = instruction -> fpResult;	
								RSfloat -> Qj = -1;
							}
							if(RSfloat -> Qk == robnumber){
								RSfloat -> Vk = instruction -> fpResult;
								RSfloat -> Qk = -1;
							}
							if ((RSfloat -> Qj == -1) && (RSfloat -> Qk == -1)){
									RSfloat -> isReady = 1;
							}
						}
					}
				}
				for (current = cpu -> loadBuffer -> head; current != NULL; current = current -> next){
				RSmem = current -> value -> value;
				isProg2 = RSmem ->instruction-> isProg2;
				if(isProg2 == 1){
				 if (RSmem -> isReady == 0){
					if(RSmem -> Qj == robnumber){
								RSmem -> Vj = instruction -> intResult;
								RSmem -> Qj = -1;							
							}
							if (RSmem -> Qj == -1){
									RSmem -> isReady = 1;
						}
				 }
				}
				}
				for (current = cpu -> resStaFPmult -> head; current != NULL; current = current -> next){
				RSfloat = current -> value -> value;
				isProg2 = RSfloat ->instruction-> isProg2;
					if(isProg2 == 1){
					if (RSfloat -> isReady == 0){
						if(RSfloat -> Qj == robnumber){
							RSfloat -> Vj = instruction -> fpResult;
							RSfloat -> Qj = -1;						
						}
						if(RSfloat -> Qk == robnumber){
							RSfloat -> Vk = instruction -> fpResult;
							RSfloat -> Qk = -1;
							
						}
						if ((RSfloat -> Qj == -1) && (RSfloat -> Qk == -1)){
									RSfloat -> isReady = 1;
						}
					}
					}
				}  
			for (current = cpu -> resStaFPdiv -> head; current != NULL; current = current -> next){
					RSfloat = current -> value -> value;
					isProg2 = RSfloat ->instruction-> isProg2;
					if(isProg2 == 1){
						if (RSfloat -> isReady == 0){
							if(RSfloat -> Qj == robnumber){
								RSfloat -> Vj = instruction -> fpResult;
								RSfloat -> Qj = -1;						
							}
							if(RSfloat -> Qk == robnumber){
								RSfloat -> Vk = instruction -> fpResult;
								RSfloat -> Qk = -1;
							}
							if ((RSfloat -> Qj == -1) && (RSfloat -> Qk == -1)){
										RSfloat -> isReady = 1;
								}
						}
					}
				}
			
            for (current = cpu -> storeBuffer -> head; current != NULL; current = current -> next){
					RSmem = current -> value -> value;

					isProg2 = RSmem ->instruction-> isProg2;
					if(isProg2 == 1){
					 if (RSmem -> isReady == 0){
						if(RSmem -> Qj == robnumber){
							RSmem -> fpVk = instruction -> fpResult;	
							RSmem -> Qj = -1;
						}
						if(RSmem -> Qk == robnumber){
							RSmem -> fpVk = instruction -> fpResult;
							RSmem -> Qk = -1;
						}
						if ((RSmem -> Qj == -1) && (RSmem -> Qk == -1)){
									RSmem -> isReady = 1;
							}
					 }
				}
				}
            break;
     
        default:
            break;

	 }
}

void updateOutputRESresult(CompletedInstruction *instruction) {
    int robnumber = instruction -> ROB_number;
	RSint *RSint;
	RSfloat *RSfloat;
	RSmem *RSmem;
	DictionaryEntry *tempEntry;
	int isProg2;
	switch (instruction -> instruction -> op) {
        case ANDI:
        case AND:
        case ORI:
        case OR:
        case SLTI:
        case SLT:
        case DADDI:
        case DADD:
        case DSUB:
		case DMUL:
		case BNE:
        case BNEZ:
        case BEQ:
        case BEQZ:
		case LD:
		case SD :
		    for (tempEntry = cpu -> resStaBUResult -> head; tempEntry != NULL; tempEntry = tempEntry -> next){
                if (tempEntry != NULL){
                    RSint = tempEntry -> value -> value;
					isProg2 = RSint ->instruction-> isProg2;
					if(isProg2 == 0){
						if(RSint -> Qj == robnumber){
								RSint -> Vj = instruction -> intResult;
								RSint -> Qj = -1;
							}
						if(RSint -> Qk == robnumber){
							RSint -> Vk = instruction -> intResult;
							RSint -> Qk = -1;
						}

						if ((RSint -> Qj == -1) && (RSint -> Qk == -1)){
							RSint -> isReady = 1;
						}
					}
                }
            }
            for (tempEntry = cpu -> resStaIntResult -> head; tempEntry != NULL; tempEntry = tempEntry -> next){
                if (tempEntry != NULL){
                    RSint = tempEntry -> value -> value;
					isProg2 = RSint ->instruction-> isProg2;
					if(isProg2 == 0){
                    if(RSint -> Qj == robnumber){
                            RSint -> Vj = instruction -> intResult;
                            RSint -> Qj = -1;
                        }
                    if(RSint -> Qk == robnumber){
                        RSint -> Vk = instruction -> intResult;
                        RSint -> Qk = -1;
                    }

                    if ((RSint -> Qj == -1) && (RSint -> Qk == -1)){
                        RSint -> isReady = 1;
                    }
					}
                }
            }
            for (tempEntry = cpu -> resStaMultResult -> head; tempEntry != NULL; tempEntry = tempEntry -> next){
                if (tempEntry != NULL){
                    RSint = tempEntry -> value -> value;
					isProg2 = RSint ->instruction-> isProg2;
					if(isProg2 == 0){
                    if(RSint -> Qj == robnumber){
                            RSint -> Vj = instruction -> intResult;
                            RSint -> Qj = -1;
                        }
                    if(RSint -> Qk == robnumber){
                        RSint -> Vk = instruction -> intResult;
                        RSint -> Qk = -1;
                    }

                    if ((RSint -> Qj == -1) && (RSint -> Qk == -1)){
                        RSint -> isReady = 1;
                    }
					}
                }
            }
            for (tempEntry = cpu -> storeBufferResult -> head; tempEntry != NULL; tempEntry = tempEntry -> next){
                if (tempEntry != NULL) {
                    RSmem = tempEntry -> value -> value;
					isProg2 = RSmem ->instruction-> isProg2;
					if(isProg2 == 0){
                    if(RSmem -> Qj == robnumber){
                        RSmem -> Vj = instruction -> intResult;
                        RSmem -> Qj = -1;
                    }
                    if(RSmem -> Qk == robnumber){
                        RSmem -> iVk = instruction -> intResult;
                        RSmem -> Qk = -1;
                    }
                    if ((RSmem -> Qj == -1) && (RSmem -> Qk == -1)){
                                RSmem -> isReady = 1;
                    }
                }
				}
            }
            for (tempEntry = cpu -> loadBufferResult -> head; tempEntry != NULL; tempEntry = tempEntry -> next){
                if (tempEntry != NULL) {
                    RSmem = tempEntry -> value -> value;
					isProg2 = RSmem ->instruction-> isProg2;
					if(isProg2 == 0){
                    if(RSmem -> Qj == robnumber){
                        RSmem -> Vj = instruction -> intResult;
                        RSmem -> Qj = -1;
                    }
                    if ((RSmem -> Qj == -1)){
                        RSmem -> isReady = 1;
                    }
					}
                }
            }
            break;
        case ADD_D:
        case SUB_D:
		case MUL_D:
		case DIV_D:
		case S_D:
		case L_D:
		    for (tempEntry = cpu -> resStaFPaddResult -> head; tempEntry != NULL; tempEntry = tempEntry -> next){
                if (tempEntry != NULL) {
                    RSfloat = tempEntry -> value -> value;
					isProg2 = RSfloat ->instruction-> isProg2;
					if(isProg2 == 0){
                    if(RSfloat -> Qj == robnumber){
                        RSfloat -> Vj = instruction -> fpResult;
                        RSfloat -> Qj = -1;
                    }
                    if(RSfloat -> Qk == robnumber){
                        RSfloat -> Vk = instruction -> fpResult;
                        RSfloat -> Qk = -1;
                    }
                    if ((RSfloat -> Qj == -1) && (RSfloat -> Qk == -1)){
                                RSfloat -> isReady = 1;
                        }
					}
                }
            }
            for (tempEntry = cpu -> loadBufferResult -> head; tempEntry != NULL; tempEntry = tempEntry -> next){
                if (tempEntry != NULL) {
                    RSmem = tempEntry -> value -> value;
					isProg2 = RSmem ->instruction-> isProg2;
					if(isProg2 == 0){
                    if(RSmem -> Qj == robnumber){
                        RSmem -> Vj = instruction -> intResult;
                        RSmem -> Qj = -1;
                    }
                    if ((RSmem -> Qj == -1)){
                                RSmem -> isReady = 1;
                    }
					}
                }
            }
            for (tempEntry = cpu -> resStaFPmultResult -> head; tempEntry != NULL; tempEntry = tempEntry -> next){
                tempEntry = cpu -> resStaFPmultResult -> head;
                if (tempEntry != NULL) {
                    RSfloat = tempEntry -> value -> value;
					isProg2 = RSfloat ->instruction-> isProg2;
					if(isProg2 == 0){
                    if(RSfloat -> Qj == robnumber){
                        RSfloat -> Vj = instruction -> fpResult;
                        RSfloat -> Qj = -1;
                    }
                    if(RSfloat -> Qk == robnumber){
                        RSfloat -> Vk = instruction -> fpResult;
                        RSfloat -> Qk = -1;
                    }
                    if ((RSfloat -> Qj == -1) && (RSfloat -> Qk == -1)){
                                RSfloat -> isReady = 1;
                        }
					}
                }
            }
            for (tempEntry = cpu -> resStaFPdivResult -> head; tempEntry != NULL; tempEntry = tempEntry -> next){
                if (tempEntry != NULL) {
                    RSfloat = tempEntry -> value -> value;
					isProg2 = RSfloat ->instruction-> isProg2;
					if(isProg2 == 0){
                    if(RSfloat -> Qj == robnumber){
                        RSfloat -> Vj = instruction -> fpResult;
                        RSfloat -> Qj = -1;
                    }
                    if(RSfloat -> Qk == robnumber){
                        RSfloat -> Vk = instruction -> fpResult;
                        RSfloat -> Qk = -1;
                    }
                    if ((RSfloat -> Qj == -1) && (RSfloat -> Qk == -1)){
                                RSfloat -> isReady = 1;
                        }
					}
                }
            }
            for (tempEntry = cpu -> storeBufferResult -> head; tempEntry != NULL; tempEntry = tempEntry -> next){
                if (tempEntry != NULL) {
                    RSmem = tempEntry -> value -> value;
					isProg2 = RSmem ->instruction-> isProg2;
					if(isProg2 == 0){
                    if(RSmem -> Qj == robnumber){
                        RSmem -> Vj = instruction -> intResult;
                        RSmem -> Qj = -1;
                    }
                    if(RSmem -> Qk == robnumber){
                        RSmem -> fpVk = instruction -> fpResult;
                        RSmem -> Qk = -1;
                    }
                    if ((RSmem -> Qj == -1) && (RSmem -> Qk == -1)){
                                RSmem -> isReady = 1;
                    }
					}
                }
            }
            break;
        default:
            break;
	 }
}


//for prog 2
void updateOutputRESresult2(CompletedInstruction *instruction) {
    int robnumber = instruction -> ROB_number;
	RSint *RSint;
	RSfloat *RSfloat;
	RSmem *RSmem;
	DictionaryEntry *tempEntry;
	int isProg2;
	switch (instruction -> instruction -> op) {
        case ANDI:
        case AND:
        case ORI:
        case OR:
        case SLTI:
        case SLT:
        case DADDI:
        case DADD:
        case DSUB:
		case DMUL:
		case BNE:
        case BNEZ:
        case BEQ:
        case BEQZ:
		case LD:
		case SD :
		    for (tempEntry = cpu -> resStaBUResult -> head; tempEntry != NULL; tempEntry = tempEntry -> next){
                if (tempEntry != NULL){
                    RSint = tempEntry -> value -> value;
					isProg2 = RSint ->instruction-> isProg2;
					if(isProg2 == 1){
						if(RSint -> Qj == robnumber){
								RSint -> Vj = instruction -> intResult;
								RSint -> Qj = -1;
							}
						if(RSint -> Qk == robnumber){
							RSint -> Vk = instruction -> intResult;
							RSint -> Qk = -1;
						}

						if ((RSint -> Qj == -1) && (RSint -> Qk == -1)){
							RSint -> isReady = 1;
						}
					}
                }
            }
            for (tempEntry = cpu -> resStaIntResult -> head; tempEntry != NULL; tempEntry = tempEntry -> next){
                if (tempEntry != NULL){
                    RSint = tempEntry -> value -> value;
					isProg2 = RSint ->instruction-> isProg2;
					if(isProg2 == 1){
                    if(RSint -> Qj == robnumber){
                            RSint -> Vj = instruction -> intResult;
                            RSint -> Qj = -1;
                        }
                    if(RSint -> Qk == robnumber){
                        RSint -> Vk = instruction -> intResult;
                        RSint -> Qk = -1;
                    }

                    if ((RSint -> Qj == -1) && (RSint -> Qk == -1)){
                        RSint -> isReady = 1;
                    }
					}
                }
            }
            for (tempEntry = cpu -> resStaMultResult -> head; tempEntry != NULL; tempEntry = tempEntry -> next){
                if (tempEntry != NULL){
                    RSint = tempEntry -> value -> value;
					isProg2 = RSint ->instruction-> isProg2;
					if(isProg2 == 1){
                    if(RSint -> Qj == robnumber){
                            RSint -> Vj = instruction -> intResult;
                            RSint -> Qj = -1;
                        }
                    if(RSint -> Qk == robnumber){
                        RSint -> Vk = instruction -> intResult;
                        RSint -> Qk = -1;
                    }

                    if ((RSint -> Qj == -1) && (RSint -> Qk == -1)){
                        RSint -> isReady = 1;
                    }
					}
                }
            }
            for (tempEntry = cpu -> storeBufferResult -> head; tempEntry != NULL; tempEntry = tempEntry -> next){
                if (tempEntry != NULL) {
                    RSmem = tempEntry -> value -> value;
					isProg2 = RSmem ->instruction-> isProg2;
					if(isProg2 == 1){
                    if(RSmem -> Qj == robnumber){
                        RSmem -> Vj = instruction -> intResult;
                        RSmem -> Qj = -1;
                    }
                    if(RSmem -> Qk == robnumber){
                        RSmem -> iVk = instruction -> intResult;
                        RSmem -> Qk = -1;
                    }
                    if ((RSmem -> Qj == -1) && (RSmem -> Qk == -1)){
                                RSmem -> isReady = 1;
                    }
                }
				}
            }
            for (tempEntry = cpu -> loadBufferResult -> head; tempEntry != NULL; tempEntry = tempEntry -> next){
                if (tempEntry != NULL) {
                    RSmem = tempEntry -> value -> value;
					isProg2 = RSmem ->instruction-> isProg2;
					if(isProg2 == 1){
                    if(RSmem -> Qj == robnumber){
                        RSmem -> Vj = instruction -> intResult;
                        RSmem -> Qj = -1;
                    }
                    if ((RSmem -> Qj == -1)){
                        RSmem -> isReady = 1;
                    }
					}
                }
            }
            break;
        case ADD_D:
        case SUB_D:
		case MUL_D:
		case DIV_D:
		case S_D:
		case L_D:
		    for (tempEntry = cpu -> resStaFPaddResult -> head; tempEntry != NULL; tempEntry = tempEntry -> next){
                if (tempEntry != NULL) {
                    RSfloat = tempEntry -> value -> value;
					isProg2 = RSfloat ->instruction-> isProg2;
					if(isProg2 == 1){
                    if(RSfloat -> Qj == robnumber){
                        RSfloat -> Vj = instruction -> fpResult;
                        RSfloat -> Qj = -1;
                    }
                    if(RSfloat -> Qk == robnumber){
                        RSfloat -> Vk = instruction -> fpResult;
                        RSfloat -> Qk = -1;
                    }
                    if ((RSfloat -> Qj == -1) && (RSfloat -> Qk == -1)){
                                RSfloat -> isReady = 1;
                        }
					}
                }
            }
            for (tempEntry = cpu -> loadBufferResult -> head; tempEntry != NULL; tempEntry = tempEntry -> next){
                if (tempEntry != NULL) {
                    RSmem = tempEntry -> value -> value;
					isProg2 = RSmem ->instruction-> isProg2;
					if(isProg2 == 1){
                    if(RSmem -> Qj == robnumber){
                        RSmem -> Vj = instruction -> intResult;
                        RSmem -> Qj = -1;
                    }
                    if ((RSmem -> Qj == -1)){
                                RSmem -> isReady = 1;
                    }
					}
                }
            }
            for (tempEntry = cpu -> resStaFPmultResult -> head; tempEntry != NULL; tempEntry = tempEntry -> next){
                tempEntry = cpu -> resStaFPmultResult -> head;
                if (tempEntry != NULL) {
                    RSfloat = tempEntry -> value -> value;
					isProg2 = RSfloat ->instruction-> isProg2;
					if(isProg2 == 1){
                    if(RSfloat -> Qj == robnumber){
                        RSfloat -> Vj = instruction -> fpResult;
                        RSfloat -> Qj = -1;
                    }
                    if(RSfloat -> Qk == robnumber){
                        RSfloat -> Vk = instruction -> fpResult;
                        RSfloat -> Qk = -1;
                    }
                    if ((RSfloat -> Qj == -1) && (RSfloat -> Qk == -1)){
                                RSfloat -> isReady = 1;
                        }
					}
                }
            }
            for (tempEntry = cpu -> resStaFPdivResult -> head; tempEntry != NULL; tempEntry = tempEntry -> next){
                if (tempEntry != NULL) {
                    RSfloat = tempEntry -> value -> value;
					isProg2 = RSfloat ->instruction-> isProg2;
					if(isProg2 == 1){
                    if(RSfloat -> Qj == robnumber){
                        RSfloat -> Vj = instruction -> fpResult;
                        RSfloat -> Qj = -1;
                    }
                    if(RSfloat -> Qk == robnumber){
                        RSfloat -> Vk = instruction -> fpResult;
                        RSfloat -> Qk = -1;
                    }
                    if ((RSfloat -> Qj == -1) && (RSfloat -> Qk == -1)){
                                RSfloat -> isReady = 1;
                        }
					}
                }
            }
            for (tempEntry = cpu -> storeBufferResult -> head; tempEntry != NULL; tempEntry = tempEntry -> next){
                if (tempEntry != NULL) {
                    RSmem = tempEntry -> value -> value;
					isProg2 = RSmem ->instruction-> isProg2;
					if(isProg2 == 1){
                    if(RSmem -> Qj == robnumber){
                        RSmem -> Vj = instruction -> intResult;
                        RSmem -> Qj = -1;
                    }
                    if(RSmem -> Qk == robnumber){
                        RSmem -> fpVk = instruction -> fpResult;
                        RSmem -> Qk = -1;
                    }
                    if ((RSmem -> Qj == -1) && (RSmem -> Qk == -1)){
                                RSmem -> isReady = 1;
                    }
					}
                }
            }
            break;
        default:
            break;
	 }
}



// insert ouput results into write back buffer and update rename register
void insertintoWriteBackBuffer(int NB)
{
	//int	*ROB_number = (int*) malloc(sizeof(int));
	KeyRS *ROB_number = (KeyRS *) malloc(sizeof(KeyRS));
	CompletedInstruction *instruction;
	CompletedInstruction **unitOutputs;
	unitOutputs = execute(NB);
	printf("Execution Complete ---------------\n");
	printf("Write Back Start ---------------\n");
	if(cpu -> WriteBackBuffer == NULL)
	{
		cpu -> WriteBackBuffer = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
	}
//	//TODO: don't return here, this is for testing execute(2)
//	return;
	if(unitOutputs != NULL){
		if(unitOutputs[INT] != NULL){
			instruction = unitOutputs[INT];
			//*ROB_number = instruction->ROB_number;
			ROB_number->reorderNum = instruction->ROB_number;
			ROB_number->progNum = instruction ->instruction -> isProg2 + 1;
			addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
			int* intresult = &(instruction -> intResult);
			printf ("Added instruction %d to WriteBack Buffer with ROB no - %d  Output - %d\n",  instruction -> instruction -> address, instruction -> ROB_number, *intresult);	
		}
		if(unitOutputs[MULT] != NULL){

			instruction = unitOutputs[MULT];
			ROB_number->reorderNum = instruction->ROB_number;
			ROB_number->progNum = instruction ->instruction -> isProg2 + 1;
			addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
			int* intresult = &(instruction -> intResult);
			printf ("Added instruction %d to WriteBack Buffer with ROB no - %d  Output - %d\n",  instruction -> instruction -> address, instruction -> ROB_number, *intresult);	
		}
		if(unitOutputs[FPadd] != NULL){
			instruction = unitOutputs[FPadd];
			ROB_number->reorderNum = instruction->ROB_number;
			ROB_number->progNum = instruction ->instruction -> isProg2 + 1;
			addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
			double *fpresult = &(instruction -> fpResult);
			printf ("Added instruction %d to WriteBack Buffer with ROB no - %d  Output - %f\n",  instruction -> instruction -> address, instruction -> ROB_number, *fpresult);	
		}
		if(unitOutputs[FPmult]!= NULL){
			instruction = unitOutputs[FPmult];
			ROB_number->reorderNum = instruction->ROB_number;
			ROB_number->progNum = instruction ->instruction -> isProg2 + 1;
			double *fpresult = &(instruction -> fpResult);
			addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
			printf ("Added instruction %d to WriteBack Buffer with ROB no - %d  Output - %f\n",  instruction -> instruction -> address, instruction -> ROB_number,*fpresult);	
		}
		if(unitOutputs[FPdiv] != NULL){
			instruction = unitOutputs[FPdiv];
			ROB_number->reorderNum = instruction->ROB_number;
			ROB_number->progNum = instruction ->instruction -> isProg2 + 1;
			addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
			double *fpresult = &(instruction -> fpResult);
			printf ("Added instruction %d to WriteBack Buffer with ROB no - %d  Output - %f\n",  instruction -> instruction -> address, instruction -> ROB_number, *fpresult);	
		}
		if(unitOutputs[BU] != NULL){
				
			instruction = unitOutputs[BU];
			ROB_number->reorderNum = instruction->ROB_number;
			ROB_number->progNum = instruction ->instruction -> isProg2 + 1;
			addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
			int* intresult = &(instruction -> intResult);
			printf ("Added instruction %d to WriteBack Buffer with ROB no - %d  Output - %d\n",  instruction -> instruction -> address, instruction -> ROB_number,  *intresult);	
		}
		if(unitOutputs[LS] != NULL){
			instruction = unitOutputs[LS];
			OpCode op = instruction -> instruction -> op;
			if((strcmp(getOpcodeString (op) ,"SD") == 0))
			{
				ROB_number->reorderNum = instruction->ROB_number;
				ROB_number->progNum = instruction ->instruction -> isProg2 + 1;
				addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
				int* intresult = &(instruction -> intResult);
				printf ("Added instruction %d to WriteBack Buffer with ROB no - %d  Output - %d\n",  instruction -> instruction -> address, instruction -> ROB_number, *intresult);	
		
			}
			else if((strcmp(getOpcodeString (op), "S_D") == 0))
			{
				ROB_number->reorderNum = instruction->ROB_number;
				ROB_number->progNum = instruction ->instruction -> isProg2 + 1;
				addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
				double *fpresult = &(instruction -> fpResult);
				printf ("Added instruction %d to WriteBack Buffer with ROB no - %d  Output - %f\n",  instruction -> instruction -> address, instruction -> ROB_number, *fpresult);
			}
			if((strcmp(getOpcodeString (op) ,"LD") == 0))
			{
				ROB_number->reorderNum = instruction->ROB_number;
				ROB_number->progNum = instruction ->instruction -> isProg2 + 1;
				addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
				int* intresult = &(instruction -> intResult);
				printf ("Added instruction %d to WriteBack Buffer with ROB no - %d  Output - %d\n",  instruction -> instruction -> address, instruction -> ROB_number,  *intresult);	
		
			}
			else if((strcmp(getOpcodeString (op), "L_D") == 0))
			{
				ROB_number->reorderNum = instruction->ROB_number;
				ROB_number->progNum = instruction ->instruction -> isProg2 + 1;
				addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
				double *fpresult = &(instruction -> fpResult);
				printf ("Added instruction %d to WriteBack Buffer with ROB no - %d  Outputt - %f\n",  instruction -> instruction -> address, instruction -> ROB_number, *fpresult);
			}
		}
	}
	//printf("Completed write back\n");
}

// for prog 2
// insert ouput results into write back buffer and update rename register
void insertintoWriteBackBuffer2(int NB)
{
	//int	*ROB_number = (int*) malloc(sizeof(int));
	KeyRS *ROB_number = (KeyRS *) malloc(sizeof(KeyRS));
	CompletedInstruction *instruction;
	CompletedInstruction **unitOutputs;
	unitOutputs = execute2(NB);
	printf("Execution Complete ---------------\n");
	printf("Write Back Start ---------------\n");
	if(cpu -> WriteBackBuffer == NULL)
	{
		cpu -> WriteBackBuffer = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
	}
//	//TODO: don't return here, this is for testing execute(2)
//	return;
	if(unitOutputs != NULL){
		if(unitOutputs[INT] != NULL){
			instruction = unitOutputs[INT];
			//*ROB_number = instruction->ROB_number;
			ROB_number->reorderNum = instruction->ROB_number;
			ROB_number->progNum = instruction ->instruction -> isProg2 + 1;
			addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
			int* intresult = &(instruction -> intResult);
			printf ("Added instruction %d to WriteBack Buffer with ROB no - %d  Output - %d\n",  instruction -> instruction -> address, instruction -> ROB_number, *intresult);	
		}
		if(unitOutputs[MULT] != NULL){

			instruction = unitOutputs[MULT];
			ROB_number->reorderNum = instruction->ROB_number;
			ROB_number->progNum = instruction ->instruction -> isProg2 + 1;
			addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
			int* intresult = &(instruction -> intResult);
			printf ("Added instruction %d to WriteBack Buffer with ROB no - %d  Output - %d\n",  instruction -> instruction -> address, instruction -> ROB_number, *intresult);	
		}
		if(unitOutputs[FPadd] != NULL){
			instruction = unitOutputs[FPadd];
			ROB_number->reorderNum = instruction->ROB_number;
			ROB_number->progNum = instruction ->instruction -> isProg2 + 1;
			addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
			double *fpresult = &(instruction -> fpResult);
			printf ("Added instruction %d to WriteBack Buffer with ROB no - %d  Output - %f\n",  instruction -> instruction -> address, instruction -> ROB_number, *fpresult);	
		}
		if(unitOutputs[FPmult]!= NULL){
			instruction = unitOutputs[FPmult];
			ROB_number->reorderNum = instruction->ROB_number;
			ROB_number->progNum = instruction ->instruction -> isProg2 + 1;
			double *fpresult = &(instruction -> fpResult);
			addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
			printf ("Added instruction %d to WriteBack Buffer with ROB no - %d  Output - %f\n",  instruction -> instruction -> address, instruction -> ROB_number,*fpresult);	
		}
		if(unitOutputs[FPdiv] != NULL){
			instruction = unitOutputs[FPdiv];
			ROB_number->reorderNum = instruction->ROB_number;
			ROB_number->progNum = instruction ->instruction -> isProg2 + 1;
			addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
			double *fpresult = &(instruction -> fpResult);
			printf ("Added instruction %d to WriteBack Buffer with ROB no - %d  Output - %f\n",  instruction -> instruction -> address, instruction -> ROB_number, *fpresult);	
		}
		if(unitOutputs[BU] != NULL){
				
			instruction = unitOutputs[BU];
			ROB_number->reorderNum = instruction->ROB_number;
			ROB_number->progNum = instruction ->instruction -> isProg2 + 1;
			addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
			int* intresult = &(instruction -> intResult);
			printf ("Added instruction %d to WriteBack Buffer with ROB no - %d  Output - %d\n",  instruction -> instruction -> address, instruction -> ROB_number,  *intresult);	
		}
		if(unitOutputs[LS] != NULL){
			instruction = unitOutputs[LS];
			OpCode op = instruction -> instruction -> op;
			if((strcmp(getOpcodeString (op) ,"SD") == 0))
			{
				ROB_number->reorderNum = instruction->ROB_number;
				ROB_number->progNum = instruction ->instruction -> isProg2 + 1;
				addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
				int* intresult = &(instruction -> intResult);
				printf ("Added instruction %d to WriteBack Buffer with ROB no - %d  Output - %d\n",  instruction -> instruction -> address, instruction -> ROB_number, *intresult);	
		
			}
			else if((strcmp(getOpcodeString (op), "S_D") == 0))
			{
				ROB_number->reorderNum = instruction->ROB_number;
				ROB_number->progNum = instruction ->instruction -> isProg2 + 1;
				addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
				double *fpresult = &(instruction -> fpResult);
				printf ("Added instruction %d to WriteBack Buffer with ROB no - %d  Output - %f\n",  instruction -> instruction -> address, instruction -> ROB_number, *fpresult);
			}
			if((strcmp(getOpcodeString (op) ,"LD") == 0))
			{
				ROB_number->reorderNum = instruction->ROB_number;
				ROB_number->progNum = instruction ->instruction -> isProg2 + 1;
				addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
				int* intresult = &(instruction -> intResult);
				printf ("Added instruction %d to WriteBack Buffer with ROB no - %d  Output - %d\n",  instruction -> instruction -> address, instruction -> ROB_number,  *intresult);	
		
			}
			else if((strcmp(getOpcodeString (op), "L_D") == 0))
			{
				ROB_number->reorderNum = instruction->ROB_number;
				ROB_number->progNum = instruction ->instruction -> isProg2 + 1;
				addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
				double *fpresult = &(instruction -> fpResult);
				printf ("Added instruction %d to WriteBack Buffer with ROB no - %d  Outputt - %f\n",  instruction -> instruction -> address, instruction -> ROB_number, *fpresult);
			}
		}
	}
	//printf("Completed write back\n");
}


// returns count of instructions which can be committed
int commitInstuctionCount(){
	ROB *ROBentry;
	int count = 0 , i = 0;
	if (cpu -> reorderBuffer == NULL){
		return 0;
	}
	else {
		ROBentry = cpu-> reorderBuffer -> items[cpu->reorderBuffer -> head];
		//while(ROBentry != NULL){
		while (i < cpu->reorderBuffer->count) {
			if((strcmp(ROBentry -> state, "W") == 0) && ROBentry -> isReady == 1)
			{
				count ++;
			}
			else{
				break;
			}
			i++;
			ROBentry = cpu->reorderBuffer -> items[(cpu->reorderBuffer -> head + i)%cpu->reorderBuffer->size];
		}
	}
	return count;
}

// returns count of instructions which can be committed
int commitInstuctionCount2(){
	ROB *ROBentry;
	int count = 0 , i = 0;
	if (cpu -> reorderBuffer2 == NULL){
		return 0;
	}
	else {
		ROBentry = cpu-> reorderBuffer2 -> items[cpu->reorderBuffer2 -> head];
		//while(ROBentry != NULL){
		while (i < cpu->reorderBuffer2->count) {
			if((strcmp(ROBentry -> state, "W") == 0) && ROBentry -> isReady == 1)
			{
				count ++;
			}
			else{
				break;
			}
			i++;
			ROBentry = cpu->reorderBuffer2 -> items[(cpu->reorderBuffer2 -> head + i)%cpu->reorderBuffer2->size];
		}
	}
	return count;
}

// write back to RES
int writeBackUnit(int NB, int returncount){
	// update outputs to ROB and Reservation stations
	//int i;
	DictionaryEntry *current;
	CompletedInstruction *instruction;
	ROB *ROBentry;
	int isprog;
	KeyRS *key = (KeyRS *)malloc(sizeof(KeyRS));

	for (current = cpu -> WriteBackBuffer -> head; current != NULL && NB >= 0; current = current -> next){
      
			instruction = (CompletedInstruction *)current -> value -> value;
			isprog = instruction -> instruction -> isProg2;
			printf("current instruction is from program - %d\n", isprog);
			if(isprog == 0){
				printf("instruction in Writeback has ROB2_number  - %d\n", instruction ->ROB_number);
				int j = 0;
				key -> reorderNum = instruction ->ROB_number;
				key -> progNum =  instruction-> instruction -> isProg2 + 1;
				if (cpu -> reorderBuffer != NULL){
					ROBentry = cpu-> reorderBuffer -> items[cpu->reorderBuffer -> head];
					//while(ROBentry != NULL){
					while (j < cpu->reorderBuffer->count) {
						if(ROBentry -> DestRenameReg == instruction -> ROB_number){
							ROBentry -> state = "W";
							ROBentry -> isReady = 1;
							if(ROBentry -> isBranch == 1){
								ROBentry -> isCorrectPredict = instruction -> isCorrectPredict;
							}
							if(ROBentry -> isStore == 1){
								ROBentry -> DestAddr = instruction -> address;
								printf("store address is updated in ROB.\n");
							}
							printf("instruction %d  with ROB_number - %d updated in reorder buffer \n", ROBentry -> instruction -> address, instruction -> ROB_number);
						}
						
						j++;
						ROBentry = cpu->reorderBuffer -> items[(cpu->reorderBuffer -> head + j)%cpu->reorderBuffer->size];
					}
				}
			updateOutputRES(instruction);
			updateOutputRESresult(instruction);
			removeDictionaryEntriesByKey(cpu -> WriteBackBuffer, key);
			returncount++;
			}
		}
	return returncount;
}

// for prog 2
// write back to RES
int writeBackUnit2(int NB, int returncount){
	// update outputs to ROB and Reservation stations
	//int i;
	DictionaryEntry *current;
	CompletedInstruction *instruction;
	ROB *ROBentry;
	int isprog;
	//KeyRS *key = (KeyRS *) malloc(sizeof(KeyRS));
	
	for (current = cpu -> WriteBackBuffer -> head; current != NULL && NB >= 0; current = current -> next){
      
			instruction = (CompletedInstruction *)current -> value -> value;
			isprog = instruction -> instruction -> isProg2;
			printf("current instruction is from program - %d\n", isprog);
			if(isprog == 1){
				
				printf("instruction in Writeback has ROB2_number  - %d\n", instruction ->ROB_number);
				int j = 0;
				if (cpu -> reorderBuffer2 != NULL){
					ROBentry = cpu-> reorderBuffer2 -> items[cpu->reorderBuffer2 -> head];
					//while(ROBentry != NULL){
					while (j < cpu->reorderBuffer2->count) {
						if(ROBentry -> DestRenameReg == instruction -> ROB_number){
							ROBentry -> state = "W";
							ROBentry -> isReady = 1;
							if(ROBentry -> isBranch == 1){
								ROBentry -> isCorrectPredict = instruction -> isCorrectPredict;
							}
							if(ROBentry -> isStore == 1){
								ROBentry -> DestAddr = instruction -> address;
								printf("store address is updated in ROB.\n");
							}
							printf("instruction %d  with ROB_number - %d updated in reorder buffer \n", ROBentry -> instruction -> address, instruction -> ROB_number);
						}
						
						j++;
						ROBentry = cpu->reorderBuffer2 -> items[(cpu->reorderBuffer2 -> head + j)%cpu->reorderBuffer2->size];
					}
				}
			updateOutputRES2(instruction);
			updateOutputRESresult2(instruction);
				returncount++;
			}
		}
	return returncount;
}
	
	


// manage commit and write back
int CommitUnit(int NB, int NR)
{
	int wb_count, commit_count, returncount = 0;
	wb_count = countDictionaryLen(cpu -> WriteBackBuffer);
	commit_count = commitInstuctionCount();
	
	printf("Write Back and Commit---------------\n");
	printf("commit count - %d, wb count - %d and NB - %d \t NR - %d\n", commit_count, wb_count, NB, NR);
	if(wb_count == 0 && commit_count == 0){
		printf("No instruction in Writeback and in ROB for Commit.\n");
	}
	else if(wb_count == 0 || commit_count >= NB)
	{
		returncount = Commit(NB, NR, returncount);
	}
	else if(commit_count == 0 || wb_count >= NB ){
		returncount = writeBackUnit(NB, returncount);
	}
	else{
			returncount = Commit(commit_count, NR, returncount);
			returncount = writeBackUnit(NB - commit_count, returncount);
		}
		
		printf("returncount from prog 1 commit unit is %d\n", returncount);
		return returncount;
		// divide NB
}

// manage commit and write back
int CommitUnit2(int NB, int NR)
{
	int wb_count, commit_count, returncount = 0;
	wb_count = countDictionaryLen(cpu -> WriteBackBuffer);
	commit_count = commitInstuctionCount2();
	
	printf("Write Back and Commit---------------\n");
	printf("commit count - %d, wb count - %d\n", commit_count, wb_count);
	if(wb_count == 0 && commit_count == 0){
		printf("No instruction in Writeback and in ROB for Commit.\n");
	}
	else if(wb_count == 0 || commit_count >= NB)
	{
		returncount = Commit(NB, NR, returncount);
	}
	else if(commit_count == 0 || wb_count >= NB ){
		returncount = writeBackUnit2(NB, returncount);
	}
	else{
			returncount = Commit(commit_count, NR, returncount);
			returncount = writeBackUnit2(NB - commit_count, returncount);
		}
		return returncount;
		// divide NB
}
/**
 * Method that simulates the looping cycle-wise
 * @return: When the simulator stops
 */
int runClockCycle (int NF, int NW, int NB, int NR) {
    int isEnd;

	cpu -> cycle++; //increment cycle counter

	printf ("\nCycle %d\n", cpu -> cycle);

    fetchMultiInstructionUnit(NF);

    printf("Finished fetching.\n");

    decodeInstructionsUnit();

    printf("Finished decoding.\n");

    issueUnit(NW);

    printf("Finished issue.\n");
    
	printf("Execution -----------\n");
	
	insertintoWriteBackBuffer(NB);
	//printf("Write Back Finish ---------------\n");
	CommitUnit(NB, NR);
	printDataCache ();
	updateFetchBuffer();
    updateInstructionQueue();
    updateReservationStations();

    printf("Finished update.\n");

    isEnd = checkEnd();

	if(isEnd==1){
	    printf("Processor has finished working in %d cycle(s).\n", cpu -> cycle);
	    printf("Stalls due to full Reservation Stations: %d\n", cpu -> stallFullRS);
	    printf("Stalls due to full Reorder Buffer: %d\n", cpu -> stallFullROB);
	    return 0;
	}else
	    return 1;

}

/**
 * Method that simulates the looping cycle-wise for part 2
 * @return: When the simulator stops
 */
int runClockCycle2 (int NF, int NW, int NB, int NR) {
    int isEnd1;
    int isEnd2;

	cpu -> cycle++; //increment cycle counter

	printf ("\nCycle %d\n", cpu -> cycle);

    printf("Fetch begins ------------\n");
	int countInstQueue1;
	int countInstQueue2;
	int countFetchBuffer1;
	int countFetchBuffer2;

	countInstQueue1 = getCountCircularQueue(cpu->instructionQueue);
	countInstQueue2 = getCountCircularQueue(cpu->instructionQueue2);
	countFetchBuffer1 = countDictionaryLen(cpu->fetchBuffer);
	countFetchBuffer2 = countDictionaryLen(cpu->fetchBuffer2);

	if((countInstQueue1 + countFetchBuffer1) > (countInstQueue2 + countFetchBuffer2)){//fetch the one with fewer entries in instruction queue
	    printf("Fetch instructions in program 2.\n");
	    fetchMultiInstructionUnit2(NF);
	    cpu->nextCycleDecodeProgram = 2;

	}else{
	    if(countInstQueue1 < countInstQueue2){
            printf("Fetch instructions in program 1.\n");
            fetchMultiInstructionUnit(NF);
            cpu->nextCycleDecodeProgram = 1;

	    }else{
	        if (cpu->cycle%2 == 0){//Give priority to thread 2 in odd cycles
	            printf("Fetch instructions in program 2.\n");
                fetchMultiInstructionUnit2(NF);
                cpu->nextCycleDecodeProgram = 2;
	        }else{
                printf("Fetch instructions in program 1.\n");
                fetchMultiInstructionUnit(NF);
                cpu->nextCycleDecodeProgram = 1;
	        }
	    }
    }
    printf("Fetch finished -----------\n");

    printf("Decode begins -----------\n");
    if (cpu->lastCycleFetchProgram == 1){
        printf("Decode instructions in program 1.\n");
        decodeInstructionsUnit();
    }else{
        if (cpu->lastCycleFetchProgram == 2){
            printf("Decode instructions in program 2.\n");
            decodeInstructionsUnit2();
        }
    }

    printf("Decode finished -----------\n");

    printf("Issue begins -----------\n");
    int numIssued;
    if (cpu->cycle%2 == 0){//Give priority to thread 2 in odd cycles
        printf("Issue instructions in program 2.\n");
        numIssued = issueUnit2(NW);
        if (NW > numIssued){
            printf("Issue instructions in program 1.\n");
            issueUnit(NW - numIssued);
        }
    }else{
        printf("Issue instructions in program 1.\n");
        numIssued = issueUnit(NW);
        if (NW > numIssued){
            printf("Issue instructions in program 2.\n");
            issueUnit2(NW - numIssued);
        }
    }
    printf("Issue finished -----------\n");



	printf("Execution -----------\n");
	insertintoWriteBackBuffer2(NB);
	//printf("Write Back Finish ---------------\n");
	//CommitUnit(NB, NR);
	int numCDB;
    if (cpu->cycle%2 == 0){//Give priority to thread 2 in odd cycles
        printf("Commit instructions in program 2.\n");
        numCDB = CommitUnit2(NB, NR);
        if (NB > numCDB){
            printf("Commit instructions in program 1.\n");
            CommitUnit(NB - numCDB, NR);
        }
    }else{
        printf("Commit instructions in program 1.\n");
        numCDB = CommitUnit(NB, NR);
        if (NB > numCDB){
            printf("Commit instructions in program 2.\n");
            CommitUnit2(NB - numCDB, NR);
        }
    }
   


    updateFetchBuffer();
    updateInstructionQueue();
    updateFetchBuffer2();
    updateInstructionQueue2();
    updateReservationStations();
    printf("Finished update.\n");
    cpu->lastCycleFetchProgram = cpu->nextCycleDecodeProgram;
    cpu->nextCycleDecodeProgram = 0;

    isEnd1 = checkEnd();
    isEnd2 = checkEnd2();

	if((isEnd1==1) & (isEnd2 == 1)){
	    printf("Processor has finished working in %d cycle(s).\n", cpu -> cycle);
	    printf("Stalls due to full Reservation Stations: %d\n", cpu -> stallFullRS);
	    printf("Stalls due to full Reorder Buffer: %d\n", cpu -> stallFullROB);
	    return 0;
	}else
	    return 1;

}

/**
 * Helper method for BHT hash table to calculate hash value from PC
 * @param PCHash
 * @return
 */
int getHashCodeFromPCHash (void *PCHash) {
	return *((int*)PCHash);
}

int getHashCodeFromInstructionAddress(void *InstructionAddress){
    return *((int*)InstructionAddress);
}

int compareDecodedInstructions(void *decodedInstruction1, void *decodedInstruction2){
    return ((Instruction *)decodedInstruction1) -> address - ((Instruction *)decodedInstruction2) -> address;
}

int getHashCodeFromBranchAddress(void *branchAddress){
    int fullAddr = *((int*)branchAddress);
    int k;
    int hashAddr = 0;
    int base = 2;
    int i;
    for(i = 0; i < 4; i++){
        if (i == 0)
            base = 1;
        else
            base *= 2;
        k = fullAddr >> (i + 3);
        if (k & 1)
            hashAddr += 1 * base;
    }
    return hashAddr;
}

int compareTargetAddress(void *targetAddress1, void *targetAddress2){
    return *((int*)targetAddress1)  - *((int*)targetAddress2);
}
//This function is used for reservation stations and load/write buffers
int getHashCodeFromROBNumber (void *ROBNumber) {
    KeyRS* keyRS = (KeyRS*)ROBNumber;
    if (keyRS->progNum == 1){
        return keyRS->reorderNum;
    }
    else{
        return -(keyRS->reorderNum + 1); //+1 is to avoid the case both of them are zero
    }
}

int compareROBNumber (void *ROBNumber1, void *ROBNumber2) {
    KeyRS* keyRS1 = (KeyRS*)ROBNumber1;
    KeyRS* keyRS2 = (KeyRS*)ROBNumber2;
    if(keyRS1->progNum == keyRS2->progNum){
        return keyRS1->reorderNum - keyRS2->reorderNum;
    }else{
        return keyRS1->reorderNum + keyRS2->reorderNum + 1;
    }
}

int getHashCodeFromRegNumber (void *RegNumber) {
    return *((int*)RegNumber);
}

int compareRegNumber (void *RegNumber1, void *RegNumber2) {
    return *((int *)RegNumber1) - *((int *)RegNumber2);
}

/*
 * Check reservation station for earliest ready value.
 * @param int selectRS 0 to for RSint, 1 for RSfloat, 2 for Store, 3 for Load
 */
DictionaryValue *checkReservationStation(DictionaryEntry *dictEntry, int selectRS) {
    int loadStep1First = 1;
    DictionaryEntry *temp = dictEntry;
    while (dictEntry != NULL) {
        if (selectRS == 1) {
            RSfloat *RS = (RSfloat *)((DictionaryEntry *)dictEntry -> value -> value);
            if (RS -> isReady && !RS -> isExecuting) {
                return ((DictionaryEntry *)dictEntry) -> value;
            } else {
                dictEntry = dictEntry -> next;
            }
        } else if (selectRS == 2) {
            RSmem *RS = (RSmem *)((DictionaryEntry *)dictEntry -> value -> value);
            if (RS -> isReady && !RS -> isExecuting) {
                return ((DictionaryEntry *)dictEntry) -> value;
            } else {
                dictEntry = dictEntry -> next;
            }
        } else if (selectRS == 3) {
            RSmem *RS = (RSmem *)((DictionaryEntry *)dictEntry -> value -> value);
            if (!loadStep1First && RS -> isReady) {
                return ((DictionaryEntry *)dictEntry) -> value;
            }
            if (RS -> isReady && RS -> address == -1) {
                return ((DictionaryEntry *)dictEntry) -> value;
            } else {
                dictEntry = dictEntry -> next;
                if (dictEntry == NULL && loadStep1First) {
                    loadStep1First = 0;
                    dictEntry = temp;
                }
            }
        } else {
            RSint *RS = (RSint *)((DictionaryEntry *)dictEntry -> value -> value);
            if (RS -> isReady && !RS -> isExecuting) {
                return ((DictionaryEntry *)dictEntry) -> value;
            } else {
                dictEntry = dictEntry -> next;
            }
        }
    }
    return NULL;
}

/**
 * Prints information about instruction entering or leaving a pipeline.
 * @param Instruction *instruction The instruction entering or leaving.
 * @param char *pipeline String name of pipeline.
 * @param int entering 1 if entering the pipeline, 0 if exiting.
 */
void printPipeline(void *instruction, char *pipeline, int entering) {
    if (entering) {
        Instruction *inst = (Instruction *)instruction;
        printf("Instruction %s at address %d in program %d entered %s unit.\n",
         getOpcodeString ((int) (inst -> op)), inst -> address, inst -> isProg2 + 1 ,pipeline);
    } else {
        CompletedInstruction *inst = (CompletedInstruction *)instruction;
        Instruction *i = inst -> instruction;
        char *instructionString = malloc (90 * sizeof(char));
        sprintf (instructionString, "%d: %s, rd=%d, rs=%d, rt=%d, fd=%d, fs=%d, ft=%d, immediate=%d, target=%d",
         i -> address, getOpcodeString ((int) i -> op), i-> rd, i-> rs, i-> rt, i-> fd, i-> fs, i-> ft, i-> immediate,
          i-> target);
        printf("%s unit output Program %d PC %s with ROB number %d\n", pipeline, i -> isProg2 + 1, instructionString, inst -> ROB_number);
        free (instructionString);
    }
}

//Flush instructions queue and fetch buffer if mis-predicted branch
void flushInstructionQueueFetchBuffer(int NI){
    cpu -> fetchBuffer = createDictionary (getHashCodeFromPCHash, compareInstructions);
	cpu -> fetchBufferResult = createDictionary (getHashCodeFromPCHash, compareInstructions);
	cpu -> instructionQueue = createCircularQueue(NI);
	cpu -> instructionQueueResult = createCircularQueue(NI);
	//Set flag to 0
	cpu->isAfterBranch = 0;
	cpu -> stallNextFetch = 0;
    }

//Flush instructions queue and fetch buffer if mis-predicted branch for part 2
void flushInstructionQueueFetchBuffer2(int NI){
    cpu -> fetchBuffer2 = createDictionary (getHashCodeFromPCHash, compareInstructions);
	cpu -> fetchBufferResult2 = createDictionary (getHashCodeFromPCHash, compareInstructions);
	cpu -> instructionQueue2 = createCircularQueue(NI);
	cpu -> instructionQueueResult2 = createCircularQueue(NI);
	//Set flag to 0
	cpu->isAfterBranch2 = 0;
	cpu -> stallNextFetch2 = 0;
    }

/*
 * Determines if branch was predicted correctly or not and updates CPU accordingly.
 */
void branchHelper (CompletedInstruction *instructionAndResult) {
    int NI = cpu -> instructionQueue -> size;
    int *targetAddress = &(instructionAndResult -> instruction -> target);
    if (instructionAndResult -> instruction -> isProg2) {
        DictionaryEntry *BTBEntry = getValueChainByDictionaryKey (cpu -> branchTargetBuffer2, &(instructionAndResult->instruction->address));
        if (instructionAndResult -> intResult == 0) { //branch taken
            if (BTBEntry == NULL) { //predicted not taken
                addDictionaryEntry (cpu -> branchTargetBuffer2, &(instructionAndResult -> instruction -> address),
                 targetAddress);
                instructionAndResult -> isCorrectPredict = 0;
                flushInstructionQueueFetchBuffer2 (NI);
                cpu -> PC2 = *targetAddress;
                printf("Branch taken but predicted as not taken in program 2.\n");
            } else { //predicted taken
                if (*(int *)(BTBEntry -> value -> value) == *targetAddress) {
                    instructionAndResult -> isCorrectPredict = 1;
                    printf("Branch taken and predicted as taken in program 2.\n");
                } else {
                    removeDictionaryEntriesByKey (cpu -> branchTargetBuffer2, &(instructionAndResult -> instruction -> address));
                    addDictionaryEntry (cpu -> branchTargetBuffer2, &(instructionAndResult -> instruction -> address),
                     targetAddress);
                    instructionAndResult -> isCorrectPredict = 0;
                    flushInstructionQueueFetchBuffer2 (NI);
                    cpu -> PC2 = *targetAddress;
                    printf("Branch taken and predicted as not taken in program 2.\n");
                }
            }
        } else { //branch not taken
            if (BTBEntry != NULL) { //predicted taken
                removeDictionaryEntriesByKey (cpu -> branchTargetBuffer2, &(instructionAndResult -> instruction -> address));
                instructionAndResult -> isCorrectPredict = 0;
                flushInstructionQueueFetchBuffer2 (NI);
                cpu -> PC2 = instructionAndResult -> instruction -> address + 4;
                printf("Branch not taken but predicted as taken in program 2.\n");
            } else { //predicted not taken
                printf("Branch not taken and predicted as not taken in program 2.\n");
                instructionAndResult -> isCorrectPredict = 1;
            }
        }
    } else {
        DictionaryEntry *BTBEntry = getValueChainByDictionaryKey (cpu -> branchTargetBuffer, &(instructionAndResult->instruction->address));
        if (instructionAndResult -> intResult == 0) { //branch taken
            if (BTBEntry == NULL) { //predicted not taken
                addDictionaryEntry (cpu -> branchTargetBuffer, &(instructionAndResult -> instruction -> address),
                 targetAddress);
                instructionAndResult -> isCorrectPredict = 0;
                flushInstructionQueueFetchBuffer (NI);
                cpu -> PC = *targetAddress;
                printf("Branch taken but predicted as not taken\n");
            } else { //predicted taken
                if (*(int *)(BTBEntry -> value -> value) == *targetAddress) {
                    instructionAndResult -> isCorrectPredict = 1;
                     printf("Branch taken and predicted as taken\n");
                } else {
                    removeDictionaryEntriesByKey (cpu -> branchTargetBuffer, &(instructionAndResult -> instruction -> address));
                    addDictionaryEntry (cpu -> branchTargetBuffer, &(instructionAndResult -> instruction -> address),
                     targetAddress);
                    instructionAndResult -> isCorrectPredict = 0;
                    flushInstructionQueueFetchBuffer (NI);
                    cpu -> PC = *targetAddress;
                    printf("Branch taken and predicted as not taken\n");
                }
            }
        } else { //branch not taken
            if (BTBEntry != NULL) { //predicted taken
                removeDictionaryEntriesByKey (cpu -> branchTargetBuffer, &(instructionAndResult -> instruction -> address));
                instructionAndResult -> isCorrectPredict = 0;
                flushInstructionQueueFetchBuffer (NI);
                cpu -> PC = instructionAndResult -> instruction -> address + 4;
                printf("Branch not taken but predicted as taken\n");
            } else { //predicted not taken
                printf("Branch not taken and predicted as not taken.\n");
                instructionAndResult -> isCorrectPredict = 1;
            }
        }
    }
}

//Determine if the run cycle should be ended
int checkEnd(){
    //check whether PC exceeds last instruction in cache
    int fetchEnd = 0;
    int robCount, iQueueCount;
    if (cpu -> PC >= (instructionCacheBaseAddress + (cacheLineSize * numberOfInstruction))) {
        fetchEnd = 1;
    }
    //Check whether all instructions in ROB have been committed
    iQueueCount = getCountCircularQueue(cpu->instructionQueue);
    robCount = getCountCircularQueue(cpu->reorderBuffer);

    if((fetchEnd==1)&&(robCount==0)&&iQueueCount == 0){
        return 1;
    }else{
        return 0;
    }
}

//Determine if the run cycle should be ended for part 2
int checkEnd2(){
    //check whether PC exceeds last instruction in cache
    int fetchEnd = 0;
    int robCount, iQueueCount;
    if (cpu -> PC2 >= (instructionCacheBaseAddress + (cacheLineSize * numberOfInstruction2))) {
        fetchEnd = 1;
    }
    //Check whether all instructions in ROB have been committed
    iQueueCount = getCountCircularQueue(cpu->instructionQueue2);
    robCount = getCountCircularQueue(cpu->reorderBuffer2);

    if((fetchEnd==1)&&(robCount==0)&&iQueueCount == 0){
        return 1;
    }else{
        return 0;
    }
}



