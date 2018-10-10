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

int fetchMultiInstructionUnit(int NF);
Instruction * decodeInstruction(char *instruction_str, int instructionAddress);
int decodeInstructionsUnit(int NI);
void updateFetchBuffer();
void updateInstructionQueue();
void updateReservationStations();
int renameRegIsFull(Dictionary *renameReg, int d);
int addInstruction2RSint(Dictionary *renameRegInt, Dictionary *resSta, Dictionary *resStaResult,
                      char* rsType, int maxLenRS, Instruction *instruction, RegStatus **IntRegStatus);
int addInstruction2RSfloat(Dictionary *renameRegFP, Dictionary *resSta, Dictionary *resStaResult,
                      char* rsType, int maxLenRS, Instruction *instruction, RegStatus **FPRegStatus);
int issueInstruction(Instruction *instruction);
void issueUnit(int NW);

ROB * InitializeROBEntry(Instruction * instruction);
void addMultipleEntriestoROB(NW);
void addSingleEntrytoROB(Instruction *instructionL);
void insertintoWriteBackBuffer();
CompletedInstruction **execute();
void Commit(int NB);
void writeBackUnit(int NB);



/**
 * This method initializes CPU data structures and all its data members
 */
void initializeCPU (int NI, int NR, int NB) {
	int i;

	cpu = (CPU *) malloc (sizeof(CPU));

	cpu -> cycle = 0;
	cpu -> PC = instructionCacheBaseAddress;

	 //initialize integer registers
	cpu -> integerRegisters = (INTReg **) malloc (sizeof(INTReg *) * numberOfIntRegisters);

	for (i = 0; i < numberOfIntRegisters; i++) {
		cpu -> integerRegisters [i] = (INTReg *) malloc (sizeof(INTReg));
		cpu -> integerRegisters [i] -> data = 0;
		cpu -> integerRegisters [i] -> intResult = 0;
 	}

	 //initialize floating point registers
	cpu -> floatingPointRegisters = (FPReg **) malloc (sizeof(FPReg *) * numberOfFPRegisters);

	for (i = 0; i < numberOfFPRegisters; i++) {
		cpu -> floatingPointRegisters [i] = (FPReg *) malloc (sizeof(FPReg));
		cpu -> floatingPointRegisters [i] -> data = 0.0;
		cpu -> floatingPointRegisters [i] -> fpResult = 0;
 	}

	cpu -> intDestReg = 0;
	cpu -> intResult = 0;

	cpu -> fpDestReg = 0;
	cpu -> fpResult = 0;

	//initialize integer registers status
	cpu -> IntRegStatus = (RegStatus **) malloc (sizeof(RegStatus *) * numberOfIntRegisters);

	for (i = 0; i < numberOfIntRegisters; i++) {
		cpu -> IntRegStatus [i] = (RegStatus *) malloc (sizeof(RegStatus));
		cpu -> IntRegStatus [i] -> reorderNum = -1;
		cpu -> IntRegStatus [i] -> busy = 0;
 	}

 	//initialize floating point registers status
	cpu -> FPRegStatus = (RegStatus **) malloc (sizeof(RegStatus *) * numberOfFPRegisters);

	for (i = 0; i < numberOfFPRegisters; i++) {
		cpu -> FPRegStatus [i] = (RegStatus *) malloc (sizeof(RegStatus));
		cpu -> FPRegStatus [i] -> reorderNum = -1;
		cpu -> FPRegStatus [i] -> busy = 0;
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
	//Initialize BTB
	cpu -> branchTargetBuffer = createDictionary(getHashCodeFromBranchAddress, compareTargetAddress);
    //Flag for next fetch unit
	cpu -> stallNextFetch = 0;
    //Initialize renaming register
	cpu -> renameRegInt = createDictionary(getHashCodeFromRegNumber, compareRegNumber);
	cpu -> renameRegFP = createDictionary(getHashCodeFromRegNumber, compareRegNumber);

    //Initialize reservation stations
    cpu -> resStaInt = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaMult = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaLoad = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaStore = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaFPadd = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaFPmult = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaFPdiv = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaBU = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaIntResult = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaMultResult = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaLoadResult = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaStoreResult = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaFPaddResult = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaFPmultResult = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaFPdivResult = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    cpu -> resStaBUResult = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
    //Initialize load/store buffers
    cpu -> loadBuffer = createCircularQueue(4);
    cpu -> storeBuffer = createCircularQueue(6);
    //Initialize Reorder buffer
    cpu -> reorderBuffer = createCircularQueue(NR);

    //Initialize Stall counters
    cpu -> stallFullROB = 0;
    cpu -> stallFullRS = 0;

	cpu -> reorderBufferResult = createCircularQueue(NR);
	cpu -> CDBBuffer = createCircularQueue(NB);
	cpu -> WriteBackBuffer = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
	cpu -> WriteBackBufferResult = createDictionary(getHashCodeFromROBNumber, compareROBNumber);

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
                        printf("Instruction %d is a branch in the BranchTargetBuffer with", *addrPtr);
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

//Decode instruction unit
int decodeInstructionsUnit(int NI){
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
            enqueueCircular(cpu -> instructionQueueResult, instruction);	
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

//Update instruction queue
void updateInstructionQueue(){
    Instruction *instruction;
    while((instruction = dequeueCircular(cpu->instructionQueueResult))!= NULL){
        enqueueCircular(cpu->instructionQueue, instruction);
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
    while((RSEntry = popDictionaryEntry(cpu -> resStaLoadResult)) != NULL){
        appendDictionaryEntry(cpu -> resStaLoad, RSEntry);
    }
    while((RSEntry = popDictionaryEntry(cpu -> resStaStoreResult)) != NULL){
        appendDictionaryEntry(cpu -> resStaStore, RSEntry);
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
        }
        //Append to reservation stations
        RS->Dest = DestROBnum;
        RS->instruction = instruction;
        addDictionaryEntry(resStaResult, &(RS->Dest), RS);
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
        }
         //Append to reservation stations
        RS->Dest = DestROBnum;
        RS->instruction = instruction;
        addDictionaryEntry(resStaResult, &(RS->Dest), RS);
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

//Issue an instruction
int issueInstruction(Instruction *instruction){
    if (isFullCircularQueue(cpu -> reorderBuffer)){
        cpu -> stallFullROB ++;
        printf("Stall during IssueUnit because reorder buffer is full.\n");
        return 0;
    }
    int issued = 0;
    int renameRegFull = 1;
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
                char* rsType = "INT";
                issued = addInstruction2RSint(cpu->renameRegInt, cpu->resStaInt, cpu->resStaIntResult, rsType, numberRSint,
                                            instruction, cpu->IntRegStatus);
            }
            break;
        case DMUL:
            renameRegFull = renameRegIsFull(cpu->renameRegInt, instruction -> rd);
            if (renameRegFull!=1){
                char* rsType = "MULT";
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
                char* rsType = "FPmult";
                issued = addInstruction2RSfloat(cpu->renameRegFP, cpu->resStaFPmult, cpu->resStaFPmultResult,
                                               rsType, numberRSfpMult, instruction, cpu->FPRegStatus);
            }
            break;
        case DIV_D:
            renameRegFull = renameRegIsFull(cpu->renameRegFP, instruction -> fd);
            if (renameRegFull!=1){
                char* rsType = "FPdiv";
                issued = addInstruction2RSfloat(cpu->renameRegFP, cpu->resStaFPdiv, cpu->resStaFPdivResult,
                                               rsType, numberRSfpDiv, instruction, cpu->FPRegStatus);
            }
            break;
        case L_D:
        case LD:
            //TODO:merge with Dillon's code
        case SD:
        case S_D:
            //Do not need to check renameRegFull
            //TODO: merge with Dillon's code
            break;
        case BNE:
        case BNEZ:
        case BEQ:
        case BEQZ:
            //char* rsType = "BU";
            //RSint *rsInt = (RSint*) malloc (sizeof(RSint));
            //rsHasPlace = addInstruction2RS(cpu->renameRegInt, rsInt, cpu->resStaBU, cpu->resStaBUResult, cpu->integerRegisters,
            //                               instruction -> rs, instruction->rt，instruction->rd, rsType, numberRSbu);
            //TODO
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

void issueUnit(int NW){
    int i;
    int issued;
    Instruction *instruction;
    for(i=0; i<NW; i++){
        if((instruction = getHeadCircularQueue(cpu -> instructionQueue))!= NULL){
            issued = issueInstruction(instruction);
            if (issued == 0)
                return;
            else
                dequeueCircular(cpu -> instructionQueue);
        }
        else
            return;
    }
}



/**
 * Takes an instruction and adds it to the given reservation station.
 * @return int 1 if successfully added, 0 otherwise.
 */
int addLoadStoreToResSta(Instruction *instruction, Dictionary *reservationStation, int ROB_number) {

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
CompletedInstruction **execute(){
    Instruction *instruction = malloc(sizeof(Instruction));
    void *valuePtr = malloc(sizeof(double));
    void *addrPtr = malloc(sizeof(int));
  	DictionaryEntry *dataCacheElement;
  	CompletedInstruction *instructionAndResult = malloc(sizeof(CompletedInstruction));
    RSint *rsint;
    RSfloat *rsfloat;
    RSmem *rsmem;
    DictionaryEntry *dictEntry;
    DictionaryValue *dictVal;
    //Array for instructions moving from Reservation Stations to execution units. Contains DictionaryValues.
    DictionaryEntry **instructionsToExec = malloc(sizeof(DictionaryEntry *)*8);
    //Array for outputs of Units. See Unit enum in DataTypes.h
    static CompletedInstruction *unitOutputs[7];
    int i, j, bufferCount, index, LSbufferInsert;
    char *pipelineString;
    RSmem *bufferItem;
    //Temp pipelines to hold changes during execution
    CompletedInstruction *INTPipelineTemp = NULL;
    CompletedInstruction *MULTPipelineTemp = NULL;
    CompletedInstruction *LoadPipelineTemp = NULL;
    CompletedInstruction *StorePipelineTemp = NULL;
    int loadStoreFirst = 0; //1 if load first, 2 if store first
    CompletedInstruction *FPaddPipelineTemp = NULL;
    CompletedInstruction *FPmultPipelineTemp = NULL;
    CompletedInstruction *FPdivPipelineTemp = NULL;
    CompletedInstruction *BUPipelineTemp = NULL;



    dictEntry = (DictionaryEntry *)cpu -> resStaInt -> head;
    dictVal = checkReservationStation (dictEntry, 0);
    if (dictVal != NULL) {
        rsint = (RSint *)(dictVal -> value);
        instructionsToExec[0] = getValueChainByDictionaryKey (cpu -> resStaInt, &(rsint -> Dest));
        removeDictionaryEntriesByKey (cpu -> resStaInt, &(rsint -> Dest));
    } else {
        instructionsToExec[0] = NULL;
    }
    dictEntry = (DictionaryEntry *)cpu -> resStaMult -> head;
    dictVal = checkReservationStation (dictEntry, 0);
    if (dictVal != NULL) {
        rsint = (RSint *)(dictVal -> value);
        instructionsToExec[1] = getValueChainByDictionaryKey (cpu -> resStaMult, &(rsint -> Dest));
        removeDictionaryEntriesByKey (cpu -> resStaMult, &(rsint -> Dest));
    } else {
        instructionsToExec[1] = NULL;
    }
    dictEntry = (DictionaryEntry *)cpu -> resStaLoad -> head;
    dictVal = checkReservationStation (dictEntry, 2);
    if (dictVal != NULL) {
        rsmem = (RSmem *)(dictVal -> value);
        instructionsToExec[2] = getValueChainByDictionaryKey (cpu -> resStaLoad, &(rsmem -> Dest));
        removeDictionaryEntriesByKey (cpu -> resStaLoad, &(rsmem -> Dest));
    } else {
        instructionsToExec[2] = NULL;
    }
    dictEntry = (DictionaryEntry *)cpu -> resStaStore -> head;
    dictVal = checkReservationStation (dictEntry, 2);
    if (dictVal != NULL) {
        rsmem = (RSmem *)(dictVal -> value);
        instructionsToExec[3] = getValueChainByDictionaryKey (cpu -> resStaStore, &(rsmem -> Dest));
        removeDictionaryEntriesByKey (cpu -> resStaStore, &(rsmem -> Dest));
    } else {
        instructionsToExec[3] = NULL;
    }
    dictEntry = (DictionaryEntry *)cpu -> resStaFPadd -> head;
    dictVal = checkReservationStation (dictEntry, 1);
    if (dictVal != NULL) {
        rsfloat = (RSfloat *)(dictVal -> value);
        instructionsToExec[4] = getValueChainByDictionaryKey (cpu -> resStaFPadd, &(rsfloat -> Dest));
        removeDictionaryEntriesByKey (cpu -> resStaFPadd, &(rsfloat -> Dest));
    } else {
        instructionsToExec[4] = NULL;
    }
    dictEntry = (DictionaryEntry *)cpu -> resStaFPmult -> head;
    dictVal = checkReservationStation (dictEntry, 1);
    if (dictVal != NULL) {
        rsfloat = (RSfloat *)(dictVal -> value);
        instructionsToExec[5] = getValueChainByDictionaryKey (cpu -> resStaFPmult, &(rsfloat -> Dest));
        removeDictionaryEntriesByKey (cpu -> resStaFPmult, &(rsfloat -> Dest));
    } else {
        instructionsToExec[5] = NULL;
    }
    if (!(cpu -> FPdivPipelineBusy)) {
        dictEntry = (DictionaryEntry *)cpu -> resStaFPdiv -> head;
        dictVal = checkReservationStation (dictEntry, 1);
        if (dictVal != NULL) {
            rsfloat = (RSfloat *)(dictVal -> value);
            instructionsToExec[6] = getValueChainByDictionaryKey (cpu -> resStaFPdiv, &(rsfloat -> Dest));
            removeDictionaryEntriesByKey (cpu -> resStaFPdiv, &(rsfloat -> Dest));
        } else {
            instructionsToExec[6] = NULL;
        }
    }
    dictEntry = (DictionaryEntry *)cpu -> resStaBU -> head;
    dictVal = checkReservationStation (dictEntry, 0);
    if (dictVal != NULL) {
        rsint = (RSint *)(dictVal -> value);
        instructionsToExec[7] = getValueChainByDictionaryKey (cpu -> resStaBU, &(rsint -> Dest));
        removeDictionaryEntriesByKey (cpu -> resStaBU, &(rsint -> Dest));
    } else {
        instructionsToExec[7] = NULL;
    }

    for (i = 0; i < 8; i++) {
        if (instructionsToExec[i] == NULL) { //if reservation station did not provide instruction
            continue;
        }
        if (i < 2 || i > 6) {
            rsint = (RSint *)(((DictionaryEntry *)instructionsToExec[i]) -> value -> value);
            instruction = rsint -> instruction;
        } else if (i == 2 || i == 3) {
            rsmem = (RSmem *)(((DictionaryEntry *)instructionsToExec[i]) -> value -> value);
            instruction = rsmem -> instruction;
        } else {
            rsfloat = (RSfloat *)(((DictionaryEntry *)instructionsToExec[i]) -> value -> value);
            instruction = rsfloat -> instruction;
        }
        instructionAndResult -> instruction = instruction;
        switch (instruction->op) {
            case ANDI:
                instructionAndResult -> intResult = rsint -> Vj & instruction->immediate;
                instructionAndResult -> ROB_number = rsint -> Dest;
                INTPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(INTPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "INT";
                printPipeline(instruction, pipelineString, 1);
                break;
            case AND:
                instructionAndResult -> intResult = rsint -> Vj & rsint -> Vk;
                instructionAndResult -> ROB_number = rsint -> Dest;
                INTPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(INTPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "INT";
                printPipeline(instruction, pipelineString, 1);
                break;
            case ORI:
                instructionAndResult -> intResult = rsint -> Vj | instruction->immediate;
                instructionAndResult -> ROB_number = rsint -> Dest;
                INTPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(INTPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "INT";
                printPipeline(instruction, pipelineString, 1);
                break;
            case OR:
                instructionAndResult -> intResult = rsint -> Vj | rsint -> Vk;
                instructionAndResult -> ROB_number = rsint -> Dest;
                INTPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(INTPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "INT";
                printPipeline(instruction, pipelineString, 1);
                break;
            case SLTI:
                instructionAndResult -> intResult = rsint -> Vj < instruction->immediate ? 1 : 0;
                instructionAndResult -> ROB_number = rsint -> Dest;
                INTPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(INTPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "INT";
                printPipeline(instruction, pipelineString, 1);
                break;
            case SLT:
                instructionAndResult -> intResult = rsint -> Vj < rsint -> Vk ? 1 : 0;
                instructionAndResult -> ROB_number = rsint -> Dest;
                INTPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(INTPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "INT";
                printPipeline(instruction, pipelineString, 1);
                break;
            case DADDI:
                instructionAndResult -> intResult = rsint -> Vj + instruction->immediate;
                instructionAndResult -> ROB_number = rsint -> Dest;
                INTPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(INTPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "INT";
                printPipeline(instruction, pipelineString, 1);
                break;
            case DADD:
                instructionAndResult -> intResult = rsint -> Vj + rsint -> Vk;
                instructionAndResult -> ROB_number = rsint -> Dest;
                INTPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(INTPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "INT";
                printPipeline(instruction, pipelineString, 1);
                break;
            case DSUB:
                instructionAndResult -> intResult = rsint -> Vj - rsint -> Vk;
                instructionAndResult -> ROB_number = rsint -> Dest;
                INTPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(INTPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "INT";
                printPipeline(instruction, pipelineString, 1);
                break;
            case DMUL:
                instructionAndResult -> intResult = rsint -> Vj * rsint -> Vk;
                instructionAndResult -> ROB_number = rsint -> Dest;
                MULTPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(MULTPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "MULT";
                printPipeline(instruction, pipelineString, 1);
                break;
            case ADD_D:
                instructionAndResult -> fpResult = rsfloat -> Vj + rsfloat -> Vk;
                instructionAndResult -> ROB_number = rsfloat -> Dest;
                FPaddPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(FPaddPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "FPadd";
                printPipeline(instruction, pipelineString, 1);
                break;
            case SUB_D:
                instructionAndResult -> fpResult = rsfloat -> Vj - rsfloat -> Vk;
                instructionAndResult -> ROB_number = rsfloat -> Dest;
                FPaddPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(FPaddPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "FPadd";
                printPipeline(instruction, pipelineString, 1);
                break;
            case MUL_D:
                instructionAndResult -> fpResult = rsfloat -> Vj * rsfloat -> Vk;
                instructionAndResult -> ROB_number = rsfloat -> Dest;
                FPmultPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(FPmultPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "FPmult";
                printPipeline(instruction, pipelineString, 1);
                break;
            case DIV_D:
                instructionAndResult -> fpResult = rsfloat -> Vj / rsfloat -> Vk;
                instructionAndResult -> ROB_number = rsfloat -> Dest;
                FPdivPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(FPdivPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                cpu -> FPdivPipelineBusy = 1;
                pipelineString = "FPdiv";
                printPipeline(instruction, pipelineString, 1);
                break;
            case L_D:
                LSbufferInsert = 1;
                rsmem -> address = rsmem -> Vj + instruction->immediate;
                cpu->memoryAddress = rsmem -> address;
                * ((int*)addrPtr) = cpu->memoryAddress;
                bufferCount = getCountCircularQueue (cpu -> storeBuffer);
                for (j = 0; j < bufferCount; j++) {
                    index = (cpu -> storeBuffer -> head + j) % cpu -> storeBuffer -> size;
                    bufferItem = (RSmem *)(cpu -> storeBuffer -> items[index]);
                    if (bufferItem -> address == rsmem -> address) {
                        LSbufferInsert = 0;
                    }
                }
                if (LSbufferInsert && !loadStoreFirst) {
                    loadStoreFirst = 1;
                    enqueueCircular (cpu -> loadBuffer, rsmem);
                    dataCacheElement = getValueChainByDictionaryKey(dataCache, addrPtr);
                    valuePtr = dataCacheElement->value->value;
                    instructionAndResult -> fpResult = *((double*)valuePtr);
                    instructionAndResult -> ROB_number = rsmem -> Dest;
                    LoadPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                    memcpy(LoadPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                } else {
                    addDictionaryEntry (cpu -> resStaLoad, &(rsmem -> Dest), rsmem);
                }
                break;
            case LD:
                LSbufferInsert = 1;
                rsmem -> address = rsmem -> Vj + instruction->immediate;
                cpu->memoryAddress = rsmem -> address;
                * ((int*)addrPtr) = cpu->memoryAddress;
                bufferCount = getCountCircularQueue (cpu -> storeBuffer);
                for (j = 0; j < bufferCount; j++) {
                    index = (cpu -> storeBuffer -> head + j) % cpu -> storeBuffer -> size;
                    bufferItem = (RSmem *)(cpu -> storeBuffer -> items[index]);
                    if (bufferItem -> address == rsmem -> address) {
                        LSbufferInsert = 0;
                    }
                }
                if (LSbufferInsert && !loadStoreFirst) {
                    loadStoreFirst = 1;
                    enqueueCircular (cpu -> loadBuffer, rsmem);
                    dataCacheElement = getValueChainByDictionaryKey(dataCache, addrPtr);
                    valuePtr = dataCacheElement->value->value;
                    instructionAndResult -> intResult = (int)*((double*)valuePtr);
                    instructionAndResult -> ROB_number = rsmem -> Dest;
                    LoadPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                    memcpy(LoadPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                } else {
                    addDictionaryEntry (cpu -> resStaLoad, &(rsmem -> Dest), rsmem);
                }
                break;
            case SD:
                LSbufferInsert = 1;
                rsmem -> address = rsmem -> Vj + instruction->immediate;
                bufferCount = getCountCircularQueue (cpu -> loadBuffer);
                for (j = 0; j < bufferCount; j++) {
                    index = (cpu -> loadBuffer -> head + j) % cpu -> loadBuffer -> size;
                    bufferItem = (RSmem *)(cpu -> loadBuffer -> items[index]);
                    if (bufferItem -> address == rsmem -> address) {
                        LSbufferInsert = 0;
                    }
                }
                if (LSbufferInsert) {
                    bufferCount = getCountCircularQueue (cpu -> storeBuffer);
                    for (j = 0; j < bufferCount; j++) {
                        index = (cpu -> storeBuffer -> head + j) % cpu -> storeBuffer -> size;
                        bufferItem = (RSmem *)(cpu -> storeBuffer -> items[index]);
                        if (bufferItem -> address == rsmem -> address) {
                            LSbufferInsert = 0;
                        }
                    }
                }
                if (LSbufferInsert && !loadStoreFirst) {
                    loadStoreFirst = 2;
                    enqueueCircular (cpu -> storeBuffer, rsmem);
                    instructionAndResult -> address = rsmem -> address;
                    instructionAndResult -> intResult = rsmem -> iVk;
                    StorePipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                    memcpy(StorePipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                } else {
                    addDictionaryEntry (cpu -> resStaStore, &(rsmem -> Dest), rsmem);
                }
                break;
            case S_D:
                LSbufferInsert = 1;
                rsmem -> address = rsmem -> Vj + instruction->immediate;
                bufferCount = getCountCircularQueue (cpu -> loadBuffer);
                for (j = 0; j < bufferCount; j++) {
                    index = (cpu -> loadBuffer -> head + j) % cpu -> loadBuffer -> size;
                    bufferItem = (RSmem *)(cpu -> loadBuffer -> items[index]);
                    if (bufferItem -> address == rsmem -> address) {
                        LSbufferInsert = 0;
                    }
                }
                if (LSbufferInsert ) {
                    bufferCount = getCountCircularQueue (cpu -> storeBuffer);
                    for (j = 0; j < bufferCount; j++) {
                        index = (cpu -> storeBuffer -> head + j) % cpu -> storeBuffer -> size;
                        bufferItem = (RSmem *)(cpu -> storeBuffer -> items[index]);
                        if (bufferItem -> address == rsmem -> address) {
                            LSbufferInsert = 0;
                        }
                    }
                }
                if (LSbufferInsert && !loadStoreFirst) {
                    loadStoreFirst = 2;
                    enqueueCircular (cpu -> storeBuffer, rsmem);
                    instructionAndResult -> address = rsmem -> address;
                    instructionAndResult -> fpResult = rsmem -> fpVk;
                    StorePipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                    memcpy(StorePipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                } else {
                    addDictionaryEntry (cpu -> resStaStore, &(rsmem -> Dest), rsmem);
                }
                break;
            case BNE:
                instructionAndResult -> intResult = rsint -> Vj != rsint -> Vk ? 0 : -1;
                BUPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(BUPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "BU";
                printPipeline(instruction, pipelineString, 1);
                break;
            case BNEZ:
                instructionAndResult -> intResult = rsint -> Vj != 0 ? 0 : -1;
                BUPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(BUPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "BU";
                printPipeline(instruction, pipelineString, 1);
                break;
            case BEQ:
                instructionAndResult -> intResult = rsint -> Vj == rsint -> Vk ? 0 : -1;
                BUPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(BUPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "BU";
                printPipeline(instruction, pipelineString, 1);
                break;
            case BEQZ:
                instructionAndResult -> intResult = rsint -> Vj == 0 ? 0 : -1;
                BUPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(BUPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "BU";
                printPipeline(instruction, pipelineString, 1);
                break;
            default:
                break;
        }
    }

    //Take outputs from Units
    unitOutputs[INT] = executePipelinedUnit (cpu -> INTPipeline);
    if (unitOutputs[INT] != NULL) {
        pipelineString = "INT";
        printPipeline(unitOutputs[INT], pipelineString, 0);
    }
    if (INTPipelineTemp != NULL) {
        enqueueCircular (cpu -> INTPipeline, INTPipelineTemp);
    }
    unitOutputs[MULT] = executePipelinedUnit (cpu -> MULTPipeline);
    if (unitOutputs[MULT] != NULL) {
        pipelineString = "MULT";
        printPipeline(unitOutputs[MULT], pipelineString, 0);
    }
    if (MULTPipelineTemp != NULL) {
        enqueueCircular (cpu -> MULTPipeline, MULTPipelineTemp);
    }
    unitOutputs[LS] = executePipelinedUnit (cpu -> LoadStorePipeline);
    if (unitOutputs[LS] != NULL) {
        if (unitOutputs[LS] -> instruction -> op == LD || unitOutputs[LS] -> instruction -> op == L_D) {
            dequeueCircular (cpu -> loadBuffer);
        } else if (unitOutputs[LS] -> instruction -> op == SD || unitOutputs[LS] -> instruction -> op == S_D) {
            dequeueCircular (cpu -> storeBuffer);
        } else {
            printf ("Error: Opcode = %d", unitOutputs[LS] -> instruction -> op); //sanity check
        }
        pipelineString = "Load/Store";
        printPipeline(unitOutputs[LS], pipelineString, 0);
    }
    if (loadStoreFirst == 1) {
        enqueueCircular (cpu -> LoadStorePipeline, LoadPipelineTemp);
    } else if (loadStoreFirst == 2) {
        enqueueCircular (cpu -> LoadStorePipeline, StorePipelineTemp);
    }
    unitOutputs[FPadd] = executePipelinedUnit (cpu -> FPaddPipeline);
    if (unitOutputs[FPadd] != NULL) {
        pipelineString = "FPadd";
        printPipeline(unitOutputs[FPadd], pipelineString, 0);
    }
    if (FPaddPipelineTemp != NULL) {
        enqueueCircular (cpu -> FPaddPipeline, FPaddPipelineTemp);
    }
    unitOutputs[FPmult] = executePipelinedUnit (cpu -> FPmultPipeline);
    if (unitOutputs[FPmult] != NULL) {
        pipelineString = "FPmult";
        printPipeline(unitOutputs[FPmult], pipelineString, 0);
    }
    if (FPmultPipelineTemp != NULL) {
        enqueueCircular (cpu -> FPmultPipeline, FPmultPipelineTemp);
    }
    unitOutputs[FPdiv] = executeFPDivUnit (cpu -> FPdivPipeline);
    if (unitOutputs[FPdiv] != NULL) {
        pipelineString = "FPdiv";
        printPipeline(unitOutputs[FPdiv], pipelineString, 0);
    }
    if (FPdivPipelineTemp != NULL) {
        enqueueCircular (cpu -> FPdivPipeline, FPdivPipelineTemp);
    }
    unitOutputs[BU] = executePipelinedUnit (cpu -> BUPipeline);
    if (unitOutputs[BU] != NULL) {
        pipelineString = "BU";
        printPipeline(unitOutputs[BU], pipelineString, 0);
    }
    if (BUPipelineTemp != NULL) {
        enqueueCircular (cpu -> BUPipeline, BUPipelineTemp);
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
	ROBEntry -> DestRenameReg = 0;
	ROBEntry -> isBranch = 0 ;
	ROBEntry -> isAfterBranch = 0;
	ROBEntry -> isCorrectPredicted = 0;
	
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
			ROBEntry->DestReg = instructionP->rs;
			ROBEntry -> isINT = 1;
			ROBEntry -> isStore = 1;
            break;
        case S_D:
			ROBEntry->DestReg = instructionP->rs;
			ROBEntry -> isINT = 0;
			ROBEntry -> isStore = 1;
            break;
        case BNE:
				ROBEntry->DestReg = -1;
				ROBEntry -> isINT = 0;
				ROBEntry -> isBranch = 1;
            break;
        case BNEZ:
				ROBEntry->DestReg = -1;
				ROBEntry -> isINT = 0;
				ROBEntry -> isBranch = 1;
            break;
        case BEQ:
				ROBEntry->DestReg = -1;
				ROBEntry -> isINT = 0;
				ROBEntry -> isBranch = 1;
            break;
        case BEQZ:
				ROBEntry->DestReg = -1;
				ROBEntry -> isINT = 0;
				ROBEntry -> isBranch = 1;
            break;
        default:
				ROBEntry -> DestReg = -1;
				ROBEntry -> isINT = 0;
            break;
    }
	
	ROBEntry->instruction = instructionP;
	ROBEntry->state = "I";
	ROBEntry->isReady = 0;
	ROBEntry -> DestAddr = 0;
	return ROBEntry;
}


// AddtoROB
void addSingleEntrytoROB(Instruction *instructionL){
	if (isFullCircularQueue(cpu->reorderBuffer) == 1)
				{
						printf("Stall the issue beacause ROB is full\n");
				}
				else{
					enqueueCircular(cpu->reorderBuffer, InitializeROBEntry(instructionL));
				/* 	ROB * drob = (ROB *) malloc (sizeof(ROB));
					drob = dequeueCircular(cpu->reorderBuffer); */				
		} 
}

/// addall issued istructions to ROB from instruction queue
void addMultipleEntriestoROB(int NW)
{
	int i;
	Instruction *instruction = (Instruction *) malloc (sizeof(Instruction));
	for(i =0; i<NW; i++){
		if((instruction = dequeueCircular(cpu -> instructionQueue)) != NULL)
		{
			if (isFullCircularQueue(cpu->reorderBuffer) == 1)
				{
						printf(" ROB is full\n");
				}
				else{
					enqueueCircular(cpu->reorderBuffer, InitializeROBEntry(instruction));
						
			}
		}		
	}
}


void Commitunit(int NB)
{
	// commit instructions from ROB
	ROB * ROBEntry;
	void *valuePtr = malloc(sizeof(double));
	if(getCountCircularQueue(cpu -> reorderBuffer) > 0){
		ROBEntry = getHeadCircularQueue(cpu -> reorderBuffer);
		if(ROBEntry != NULL){
			if((strcmp(ROBEntry -> state, "W") == 0) && ROBEntry -> isReady == 1)
			{
				if(ROBEntry -> isINT == 1 && ROBEntry -> isStore == 0 && ROBEntry -> isBranch == 0){
						int DestRenameReg, DestVal, DestReg;
						DestRenameReg = ROBEntry -> DestRenameReg;
						DestReg = ROBEntry -> DestReg;
						DictionaryEntry * Current = getValueChainByDictionaryKey(cpu -> renameRegInt, &DestRenameReg);
						DestVal = *((int *)Current -> value -> value);
						//DestVal = 0;
						cpu -> integerRegisters [DestReg] -> data = DestVal;
						NB --;
				}
				else if(ROBEntry -> isINT == 0 && ROBEntry -> isStore == 0 && ROBEntry -> isBranch == 0){
					int DestRenameReg, DestReg; float DestVal;
						DestRenameReg = ROBEntry -> DestRenameReg;
						DestReg = ROBEntry -> DestReg;
						DictionaryEntry * Current = getValueChainByDictionaryKey(cpu -> renameRegInt, &DestRenameReg);
						DestVal = *((int *)Current -> value -> value);
						//DestVal = 0.00;
						cpu -> floatingPointRegisters [DestReg] -> data = DestVal;
						NB --;
				}
				else if(ROBEntry -> isStore == 1)
				{
					if(ROBEntry -> isINT == 1){
						int DestVal, DestRenameReg;
						DestRenameReg = ROBEntry -> DestRenameReg;
						DictionaryEntry * Current = getValueChainByDictionaryKey(cpu -> renameRegInt, &DestRenameReg);
						DestVal = *((int *)Current -> value -> value);
						
						//*((int*)addrPtr) = ROBEntry -> DestAddr;
						removeDictionaryEntriesByKey (dataCache, &(ROBEntry -> DestAddr));
						*((double*)valuePtr) = DestVal; // value from rename register ;
						addDictionaryEntry (dataCache, &(ROBEntry -> DestAddr), valuePtr); 
						//DestVal = 0.00;
						NB --;
					}
					else if(ROBEntry -> isINT == 0){
						float DestVal; int DestRenameReg;
						DestRenameReg = ROBEntry -> DestRenameReg;
						DictionaryEntry * Current = getValueChainByDictionaryKey(cpu -> renameRegInt, &DestRenameReg);
						DestVal = *((int *)Current -> value -> value);
						
						//*((int*)addrPtr) = ROBEntry -> DestAddr;
						removeDictionaryEntriesByKey (dataCache, &(ROBEntry -> DestAddr));
						*((double*)valuePtr) = (double) DestVal; // value from rename register ;
						addDictionaryEntry (dataCache, &(ROBEntry -> DestAddr), valuePtr); 
						//DestVal = 0;
						NB --;
					}
				}
				else{
					//Branch
					// check iscorrectpredicted then move head to isafterbranch = 0;
				}
			}
		}
	}
	writeBackUnit(NB);
}


void writeBackUnit(int NB){
	// you can take upto max 4 or NB instructions from WB and  update them to ROB and Reservation stations
	
}


// TODO : update result in ROB
void insertintoWriteBackBuffer()
{
	int	*ROB_number = (int*) malloc(sizeof(int));
	CompletedInstruction *instruction;
	CompletedInstruction **unitOutputs;
	void *valuePtr = malloc(sizeof(double));
	unitOutputs = execute();
	if(cpu -> WriteBackBuffer == NULL)
	{
		cpu -> WriteBackBuffer = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
	}
	if(unitOutputs[INT] != NULL){	
		instruction = unitOutputs[INT];
		*ROB_number = instruction->ROB_number;
		addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
		removeDictionaryEntriesByKey (cpu -> renameRegInt, ROB_number);
		*((double*)valuePtr) = instruction -> intResult;
		addDictionaryEntry (cpu -> renameRegInt, ROB_number, valuePtr);
		printf ("Added to  WriteBack Buffer %d: %d\n",  instruction -> instruction -> address, instruction -> ROB_number);	
	}
	if(unitOutputs[MULT] != NULL){
			
		instruction = unitOutputs[MULT];
		*ROB_number = instruction->ROB_number;
		addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
		removeDictionaryEntriesByKey (cpu -> renameRegInt, ROB_number);
		*((double*)valuePtr) = instruction -> intResult;
		addDictionaryEntry (cpu -> renameRegInt, ROB_number, valuePtr);
		printf ("Added to  WriteBack Buffer %d: %d\n",  instruction -> instruction -> address, instruction -> ROB_number);	
	}
	if(unitOutputs[FPadd] != NULL){
		instruction = unitOutputs[FPadd];
		*ROB_number = instruction->ROB_number;
		addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
		removeDictionaryEntriesByKey (cpu -> renameRegInt, ROB_number);
		*((double*)valuePtr) = (double)instruction -> intResult;
		addDictionaryEntry (cpu -> renameRegInt, ROB_number, valuePtr);
		printf ("Added to  WriteBack Buffer %d: %d\n",  instruction -> instruction -> address, instruction -> ROB_number);	
	}
	if(unitOutputs[FPmult]!= NULL){
		instruction = unitOutputs[FPmult];
		*ROB_number = instruction->ROB_number;
		removeDictionaryEntriesByKey (cpu -> renameRegInt, ROB_number);
		*((double*)valuePtr) = (double)instruction -> intResult;
		addDictionaryEntry (cpu -> renameRegInt, ROB_number, valuePtr);
		addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
		printf ("Added to  WriteBack Buffer %d: %d\n",  instruction -> instruction -> address, instruction -> ROB_number);	
	}
	if(unitOutputs[FPdiv] != NULL){
		instruction = unitOutputs[FPdiv];
		*ROB_number = instruction->ROB_number;
		addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
		removeDictionaryEntriesByKey (cpu -> renameRegInt, ROB_number);
		*((double*)valuePtr) = (double)instruction -> intResult;
		addDictionaryEntry (cpu -> renameRegInt, ROB_number, valuePtr);
		printf ("Added to  WriteBack Buffer %d: %d\n",  instruction -> instruction -> address, instruction -> ROB_number);	
	}
	if(unitOutputs[LS] != NULL){
		instruction = unitOutputs[LS];
		OpCode op = instruction -> instruction -> op;
		if((strcmp(getOpcodeString (op) ,"SD") == 0))
		{
			*ROB_number = instruction->ROB_number;
			addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
			removeDictionaryEntriesByKey (cpu -> renameRegInt, ROB_number);
			*((double*)valuePtr) = instruction -> intResult;
			addDictionaryEntry (cpu -> renameRegInt, ROB_number, valuePtr);
			printf ("Added to  WriteBack Buffer %d: %d\n",  instruction -> instruction -> address, instruction -> ROB_number);	
	
		}
		else if((strcmp(getOpcodeString (op), "S_D") == 0))
		{
			*ROB_number = instruction->ROB_number;
			addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
			removeDictionaryEntriesByKey (cpu -> renameRegInt, ROB_number);
			*((double*)valuePtr) = (double) instruction -> intResult;
			addDictionaryEntry (cpu -> renameRegInt, ROB_number, valuePtr);
			printf ("Added to  WriteBack Buffer %d: %d\n",  instruction -> instruction -> address, instruction -> ROB_number);
		}
	}
}



/**
 * Method that simulates the looping cycle-wise
 * @return: When the simulator stops
 */
int runClockCycle (int NF, int NI, int NW, int NB) {

	cpu -> cycle++; //increment cycle counter

	printf ("\nCycle %d\n", cpu -> cycle);

    fetchMultiInstructionUnit(NF);

    printf("Finished fetching.\n");

    decodeInstructionsUnit(NI); 

    printf("Finished decoding.\n");

    issueUnit(NW);

    printf("Finished issue.\n");

    execute();

    printf ("Finished execute.\n");

    updateFetchBuffer();
    updateInstructionQueue();
    updateReservationStations();

    printf("Finished update.\n");

	
	
	

	
	addMultipleEntriestoROB(NW);

	
	//printROB();
    //execute(instruction);



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

int getHashCodeFromROBNumber (void *ROBNumber) {
    return *((int*)ROBNumber);
}

int compareROBNumber (void *ROBNumber1, void *ROBNumber2) {
    return *((int *)ROBNumber1) - *((int *)ROBNumber2);
}

int getHashCodeFromRegNumber (void *RegNumber) {
    return *((int*)RegNumber);
}

int compareRegNumber (void *RegNumber1, void *RegNumber2) {
    return *((int *)RegNumber1) - *((int *)RegNumber2);
}

DictionaryValue *checkReservationStation(DictionaryEntry *dictEntry, int isFloat) {
    while (dictEntry != NULL) {
        if (isFloat == 1) {
            RSfloat *RS = (RSfloat *)((DictionaryEntry *)dictEntry -> value -> value);
            if (RS -> isReady) {
                return ((DictionaryEntry *)dictEntry) -> value;
            } else {
                dictEntry = dictEntry -> next;
            }
        } else if (isFloat == 2) {
            RSmem *RS = (RSmem *)((DictionaryEntry *)dictEntry -> value -> value);
            if (RS -> isReady) {
                return ((DictionaryEntry *)dictEntry) -> value;
            } else {
                dictEntry = dictEntry -> next;
            }
        } else {
            RSint *RS = (RSint *)((DictionaryEntry *)dictEntry -> value -> value);
            if (RS -> isReady) {
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
        printf("Instruction %s at address %d entered %s unit.\n",
         getOpcodeString ((int) (inst -> op)), inst -> address, pipeline);
    } else {
        CompletedInstruction *inst = (CompletedInstruction *)instruction;
        Instruction *i = inst -> instruction;
        char *instructionString = malloc (90 * sizeof(char));
        sprintf (instructionString, "%d: %s, rd=%d, rs=%d, rt=%d, fd=%d, fs=%d, ft=%d, immediate=%d, target=%d",
         i -> address, getOpcodeString ((int) i -> op), i-> rd, i-> rs, i-> rt, i-> fd, i-> fs, i-> ft, i-> immediate,
          i-> target);
        printf("%s unit output PC %s with ROB number %d\n", pipeline, instructionString, inst -> ROB_number);
        free (instructionString);
    }
}


