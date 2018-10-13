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
int addInstruction2RSbranch(Dictionary *renameRegInt, Dictionary *resSta, Dictionary *resStaResult,
                            char* rsType, int maxLenRS, Instruction *instruction, RegStatus **IntRegStatus);
int issueInstruction(Instruction *instruction);
void issueUnit(int NW);

ROB * InitializeROBEntry(Instruction * instruction);

CompletedInstruction **execute();

void insertintoWriteBackBuffer();
void writeBackUnit(int NB);
void updateOutputRES(CompletedInstruction *instruction);
int commitInstuctionCount();
void Commit(int NC);
void CommitUnit(int NB);



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
    //Initialize Reorder buffer
    cpu -> reorderBuffer = createCircularQueue(NR);

    //Initialize Stall counters
    cpu -> stallFullROB = 0;
    cpu -> stallFullRS = 0;

	cpu -> reorderBufferResult = createCircularQueue(NR);
	cpu -> CDBBuffer = createCircularQueue(NB);
	cpu -> WriteBackBuffer = createDictionary(getHashCodeFromROBNumber, compareROBNumber);
	cpu -> WriteBackBufferResult = createDictionary(getHashCodeFromROBNumber, compareROBNumber);

    //Initialize Flag of instructions after branch
    cpu -> isAfterBranch = 0;

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
        }else{
            RS -> Qk = -1;
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
        RS->Dest = DestROBnum;
        RS->instruction = instruction;
        addDictionaryEntry(resStaResult, &(RS->Dest), RS);
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
    if (strcmp(buffType, "Load") == 0 && countDictionaryLen (cpu -> storeBuffer) != 0) {
        printf("Stall Load during Issue Unit because of earlier store in queue.\n");
        return 0;
    }
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
        addDictionaryEntry(LOrSBufferResult, &(RS->Dest), RS);
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
            issued = addLoadStore2Buffer(cpu->loadBuffer, cpu->loadBufferResult,
                     rsType, numberBufferLoad, instruction);           
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
    DictionaryValue *dictVal;
    //Array for instructions moving from Reservation Stations to execution units. Contains DictionaryValues.
    DictionaryEntry **instructionsToExec = malloc(sizeof(DictionaryEntry *)*8);
    //Array for outputs of Units. See Unit enum in DataTypes.h
    static CompletedInstruction *unitOutputs[7];
    int i;
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



    dictEntry = (DictionaryEntry *)cpu -> resStaInt -> head;
    dictVal = checkReservationStation (dictEntry, 0);
    if (dictVal != NULL) {
        rsint = (RSint *)dictVal -> value;
        instructionsToExec[0] = getValueChainByDictionaryKey (cpu -> resStaInt, &(rsint -> Dest));
    } else {
        instructionsToExec[0] = NULL;
    }
    dictEntry = (DictionaryEntry *)cpu -> resStaMult -> head;
    dictVal = checkReservationStation (dictEntry, 0);
    if (dictVal != NULL) {
        rsint = (RSint *)(dictVal -> value);
        instructionsToExec[1] = getValueChainByDictionaryKey (cpu -> resStaMult, &(rsint -> Dest));
    } else {
        instructionsToExec[1] = NULL;
    }
    dictEntry = (DictionaryEntry *)cpu -> loadBuffer -> head;
    dictVal = checkReservationStation (dictEntry, 3);
    if (dictVal != NULL) {
        rsmem = (RSmem *)(dictVal -> value);
        instructionsToExec[2] = getValueChainByDictionaryKey (cpu -> loadBuffer, &(rsmem -> Dest));
    } else {
        instructionsToExec[2] = NULL;
    }
    dictEntry = (DictionaryEntry *)cpu -> storeBuffer -> head;
    dictVal = checkReservationStation (dictEntry, 2);
    if (dictVal != NULL) {
        rsmem = (RSmem *)(dictVal -> value);
        instructionsToExec[3] = getValueChainByDictionaryKey (cpu -> storeBuffer, &(rsmem -> Dest));
    } else {
        instructionsToExec[3] = NULL;
    }
    dictEntry = (DictionaryEntry *)cpu -> resStaFPadd -> head;
    dictVal = checkReservationStation (dictEntry, 1);
    if (dictVal != NULL) {
        rsfloat = (RSfloat *)(dictVal -> value);
        instructionsToExec[4] = getValueChainByDictionaryKey (cpu -> resStaFPadd, &(rsfloat -> Dest));
    } else {
        instructionsToExec[4] = NULL;
    }
    dictEntry = (DictionaryEntry *)cpu -> resStaFPmult -> head;
    dictVal = checkReservationStation (dictEntry, 1);
    if (dictVal != NULL) {
        rsfloat = (RSfloat *)(dictVal -> value);
        instructionsToExec[5] = getValueChainByDictionaryKey (cpu -> resStaFPmult, &(rsfloat -> Dest));
    } else {
        instructionsToExec[5] = NULL;
    }
    if (!(cpu -> FPdivPipelineBusy)) {
        dictEntry = (DictionaryEntry *)cpu -> resStaFPdiv -> head;
        dictVal = checkReservationStation (dictEntry, 1);
        if (dictVal != NULL) {
            rsfloat = (RSfloat *)(dictVal -> value);
            instructionsToExec[6] = getValueChainByDictionaryKey (cpu -> resStaFPdiv, &(rsfloat -> Dest));
        } else {
            instructionsToExec[6] = NULL;
        }
    }
    dictEntry = (DictionaryEntry *)cpu -> resStaBU -> head;
    dictVal = checkReservationStation (dictEntry, 0);
    if (dictVal != NULL) {
        rsint = (RSint *)(dictVal -> value);
        instructionsToExec[7] = getValueChainByDictionaryKey (cpu -> resStaBU, &(rsint -> Dest));
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
                        continue;
                    }
                    RS = (RSmem *)((DictionaryEntry *)dictEntry -> value -> value);
                    if (RS -> isReady && RS -> address != -1) {
                        for (i = 0; i < buff -> count && i != -1; i++) {
                            if (((ROB *)(buff -> items[(buff -> head + i) % (buff->size)]))-> DestAddr == RS -> address) {
                                dictEntry = dictEntry -> next;
                                i = -1; //break out of for loop
                            }
                        }
                        if (i != -1 && RS != rsmem && RS -> isExecuting != 2) {
                            instructionFoundOrBubble = 1;
                            rsmem = RS;
                            rsmem -> isExecuting = 2;
                        } else {
                            dictEntry = dictEntry -> next;
                        }
                    } else {
                        dictEntry = dictEntry -> next;
                    }
                }
                if (instructionFoundOrBubble == 1) {
                    * ((int*)addrPtr) = rsmem -> address;
                    dataCacheElement = getValueChainByDictionaryKey(dataCache, addrPtr);
                    valuePtr = dataCacheElement->value->value;
                    instructionAndResult -> fpResult = *((double*)valuePtr);
                    instructionAndResult -> ROB_number = rsmem -> Dest;
                    LoadPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                    memcpy(LoadPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                }
                pipelineString = "Load/Store";
                printPipeline(instruction, pipelineString, 1);
                break;
            case LD:
                //Two-step
                //First calculate address for earliest load that needs it
                if (rsmem -> isExecuting == 0){
                    rsmem -> isExecuting = 1;
                    rsmem -> address = rsmem -> Vj + instruction->immediate;
                    loadStallROBNumber = rsmem -> Dest;
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
                        for (i = 0; i < buff -> count && i != -1; i++) {
                            if (((ROB *)(buff -> items[(buff -> head + i) % (buff->size)])) -> DestAddr == RS -> address) {
                                dictEntry = dictEntry -> next;
                                i = -1; //break out of for loop
                            }
                        }
                        if (i != -1 && RS != rsmem && RS -> isExecuting != 2) {
                            instructionFoundOrBubble = 1;
                            rsmem = RS;
                            rsmem -> isExecuting = 2;
                        } else {
                            dictEntry = dictEntry -> next;
                        }
                    } else {
                        dictEntry = dictEntry -> next;
                    }
                }
                if (instructionFoundOrBubble == 1) {
                    * ((int*)addrPtr) = rsmem -> address;
                    dataCacheElement = getValueChainByDictionaryKey(dataCache, addrPtr);
                    valuePtr = dataCacheElement->value->value;
                    instructionAndResult -> intResult = (int)*((double*)valuePtr);
                    instructionAndResult -> ROB_number = rsmem -> Dest;
                    LoadPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                    memcpy(LoadPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                }
                pipelineString = "Load/Store";
                printPipeline(instruction, pipelineString, 1);
                break;
            case SD:
                rsmem -> isExecuting = 1;
                rsmem -> address = rsmem -> Vj + instruction->immediate;
                instructionAndResult -> address = rsmem -> address;
                instructionAndResult -> intResult = rsmem -> iVk;
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
                StorePipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(StorePipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                storeFirst = 1;
                pipelineString = "Load/Store";
                printPipeline(instruction, pipelineString, 1);
                break;
            case BNE:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj != rsint -> Vk ? 0 : -1;
                BUPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(BUPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "BU";
                printPipeline(instruction, pipelineString, 1);
                break;
            case BNEZ:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj != 0 ? 0 : -1;
                BUPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(BUPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "BU";
                printPipeline(instruction, pipelineString, 1);
                break;
            case BEQ:
                rsint -> isExecuting = 1;
                instructionAndResult -> intResult = rsint -> Vj == rsint -> Vk ? 0 : -1;
                BUPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(BUPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "BU";
                printPipeline(instruction, pipelineString, 1);
                break;
            case BEQZ:
                rsint -> isExecuting = 1;
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

    //Take outputs from Units, but only as many as can be accepted by WriteBack Buffer
    int maxOutput = (cpu -> CDBBuffer -> size) - countDictionaryLen (cpu -> WriteBackBuffer);
    i = 0;
    if (i < maxOutput) {
        unitOutputs[INT] = executePipelinedUnit (cpu -> INTPipeline);
        if (unitOutputs[INT] != NULL) {
            i++;
            removeDictionaryEntriesByKey (cpu -> renameRegInt, &(unitOutputs[INT] -> ROB_number));
            addDictionaryEntry (cpu -> renameRegInt, &(unitOutputs[INT] -> ROB_number), &(unitOutputs[INT] -> intResult));
            removeDictionaryEntriesByKey (cpu -> resStaInt, &(unitOutputs[INT] -> ROB_number));
            pipelineString = "INT";
            printPipeline(unitOutputs[INT], pipelineString, 0);
        }
        if (INTPipelineTemp != NULL) {
            enqueueCircular (cpu -> INTPipeline, INTPipelineTemp);
        }
    } else {
        if (INTPipelineTemp != NULL) {
            RSint *stalled = (RSint *)(getValueChainByDictionaryKey(cpu -> resStaInt, &(INTPipelineTemp -> ROB_number)) -> value -> value);
            stalled -> isExecuting = 0;
        }
    }
    if (i < maxOutput) {
        unitOutputs[MULT] = executePipelinedUnit (cpu -> MULTPipeline);
        if (unitOutputs[MULT] != NULL) {
            i++;
            removeDictionaryEntriesByKey (cpu -> renameRegInt, &(unitOutputs[MULT] -> ROB_number));
            addDictionaryEntry (cpu -> renameRegInt, &(unitOutputs[MULT] -> ROB_number), &(unitOutputs[MULT] -> intResult));
            removeDictionaryEntriesByKey (cpu -> resStaMult, &(unitOutputs[MULT] -> ROB_number));
            pipelineString = "MULT";
            printPipeline(unitOutputs[MULT], pipelineString, 0);
        }
        if (MULTPipelineTemp != NULL) {
            enqueueCircular (cpu -> MULTPipeline, MULTPipelineTemp);
        }
    } else {
        if (MULTPipelineTemp != NULL) {
            RSint *stalled = (RSint *)(getValueChainByDictionaryKey(cpu -> resStaMult, &(MULTPipelineTemp -> ROB_number)) -> value -> value);
            stalled -> isExecuting = 0;
        }
    }
    if (i < maxOutput) {
        unitOutputs[LS] = executePipelinedUnit (cpu -> LoadStorePipeline);
        if (unitOutputs[LS] != NULL) {
            i++;
            if (unitOutputs[LS] -> instruction -> op == L_D) {
                removeDictionaryEntriesByKey (cpu -> renameRegFP, &(unitOutputs[LS] -> ROB_number));
                addDictionaryEntry (cpu -> renameRegFP, &(unitOutputs[LS] -> ROB_number), &(unitOutputs[LS] -> fpResult));
                removeDictionaryEntriesByKey (cpu -> loadBuffer, &(unitOutputs[LS] -> ROB_number));
            } else if (unitOutputs[LS] -> instruction -> op == LD) {
                removeDictionaryEntriesByKey (cpu -> renameRegInt, &(unitOutputs[LS] -> ROB_number));
                addDictionaryEntry (cpu -> renameRegInt, &(unitOutputs[LS] -> ROB_number), &(unitOutputs[LS] -> intResult));
                removeDictionaryEntriesByKey (cpu -> loadBuffer, &(unitOutputs[LS] -> ROB_number));
            } else {
                removeDictionaryEntriesByKey (cpu -> storeBuffer, &(unitOutputs[LS] -> ROB_number));
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
            RSmem *stalled = (RSmem *)(getValueChainByDictionaryKey(cpu -> loadBuffer, &(loadStallROBNumber)) -> value -> value);
            stalled -> isExecuting = 0;
        } else if (StorePipelineTemp != NULL) {
            RSmem *stalled = (RSmem *)(getValueChainByDictionaryKey(cpu -> storeBuffer, &(StorePipelineTemp -> ROB_number)) -> value -> value);
            stalled -> isExecuting = 0;
        }
    }
    if (i < maxOutput) {
        unitOutputs[FPadd] = executePipelinedUnit (cpu -> FPaddPipeline);
        if (unitOutputs[FPadd] != NULL) {
            i++;
            removeDictionaryEntriesByKey (cpu -> renameRegFP, &(unitOutputs[FPadd] -> ROB_number));
            addDictionaryEntry (cpu -> renameRegFP, &(unitOutputs[FPadd] -> ROB_number), &(unitOutputs[FPadd] -> intResult));
            removeDictionaryEntriesByKey (cpu -> resStaFPadd, &(unitOutputs[FPadd] -> ROB_number));
            pipelineString = "FPadd";
            printPipeline(unitOutputs[FPadd], pipelineString, 0);
        }
        if (FPaddPipelineTemp != NULL) {
            enqueueCircular (cpu -> FPaddPipeline, FPaddPipelineTemp);
        }
    } else {
        if (FPaddPipelineTemp != NULL) {
            RSfloat *stalled = (RSfloat *)(getValueChainByDictionaryKey(cpu -> resStaFPadd, &(FPaddPipelineTemp -> ROB_number)) -> value -> value);
            stalled -> isExecuting = 0;
        }
    }
    if (i < maxOutput) {
        unitOutputs[FPmult] = executePipelinedUnit (cpu -> FPmultPipeline);
        if (unitOutputs[FPmult] != NULL) {
            i++;
            removeDictionaryEntriesByKey (cpu -> renameRegFP, &(unitOutputs[FPmult] -> ROB_number));
            addDictionaryEntry (cpu -> renameRegFP, &(unitOutputs[FPmult] -> ROB_number), &(unitOutputs[FPmult] -> intResult));
            removeDictionaryEntriesByKey (cpu -> resStaFPmult, &(unitOutputs[FPmult] -> ROB_number));
            pipelineString = "FPmult";
            printPipeline(unitOutputs[FPmult], pipelineString, 0);
        }
        if (FPmultPipelineTemp != NULL) {
            enqueueCircular (cpu -> FPmultPipeline, FPmultPipelineTemp);
        }
    } else {
        if (FPmultPipelineTemp != NULL) {
            RSfloat *stalled = (RSfloat *)(getValueChainByDictionaryKey(cpu -> resStaFPmult, &(FPmultPipelineTemp -> ROB_number)) -> value -> value);
            stalled -> isExecuting = 0;
        }
    }
    if (i < maxOutput) {
        unitOutputs[FPdiv] = executeFPDivUnit (cpu -> FPdivPipeline);
        if (unitOutputs[FPdiv] != NULL) {
            i++;
            removeDictionaryEntriesByKey (cpu -> renameRegFP, &(unitOutputs[FPdiv] -> ROB_number));
            addDictionaryEntry (cpu -> renameRegFP, &(unitOutputs[FPdiv] -> ROB_number), &(unitOutputs[FPdiv] -> intResult));
            removeDictionaryEntriesByKey (cpu -> resStaFPdiv, &(unitOutputs[FPdiv] -> ROB_number));
            pipelineString = "FPdiv";
            printPipeline(unitOutputs[FPdiv], pipelineString, 0);
        }
        if (FPdivPipelineTemp != NULL) {
            enqueueCircular (cpu -> FPdivPipeline, FPdivPipelineTemp);
        }
    } else {
        if (FPdivPipelineTemp != NULL) {
            RSfloat *stalled = (RSfloat *)(getValueChainByDictionaryKey(cpu -> resStaFPdiv, &(FPdivPipelineTemp -> ROB_number)) -> value -> value);
            stalled -> isExecuting = 0;
            cpu -> FPdivPipelineBusy = 0;
        }
    }
    if (i < maxOutput) {
        unitOutputs[BU] = executePipelinedUnit (cpu -> BUPipeline);
        if (unitOutputs[BU] != NULL) {
            i++;
            removeDictionaryEntriesByKey (cpu -> renameRegInt, &(unitOutputs[BU] -> ROB_number));
            addDictionaryEntry (cpu -> renameRegInt, &(unitOutputs[BU] -> ROB_number), &(unitOutputs[BU] -> intResult));
            removeDictionaryEntriesByKey (cpu -> resStaInt, &(unitOutputs[BU] -> ROB_number));
            branchHelper (instructionAndResult);
            pipelineString = "BU";
            printPipeline(unitOutputs[BU], pipelineString, 0);
        }
        if (BUPipelineTemp != NULL) {
            enqueueCircular (cpu -> BUPipeline, BUPipelineTemp);
        }
    } else {
        if (BUPipelineTemp != NULL) {
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
	ROBEntry -> isAfterBranch = 0;
	ROBEntry -> isCorrectPredict = 1;
	
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
	ROBEntry -> DestAddr = 0;
	ROBEntry->isAfterBranch = cpu->isAfterBranch;
	return ROBEntry;
}

// manage commit and write back
void CommitUnit(int NB)
{
	int wb_count, commit_count;
	wb_count = countDictionaryLen(cpu -> WriteBackBuffer);
	commit_count = commitInstuctionCount();
	printf("commit count - %d, wb count - %d\n", commit_count, wb_count);
	if(wb_count == 0 && commit_count == 0){
		printf("No instruction in Writeback and for Commit.\n");
	}
	else if(wb_count == 0)
	{
		Commit(NB);
	}
	else if(commit_count == 0){
		writeBackUnit(NB);
	}
	else{
		Commit(commit_count);
		writeBackUnit(NB - commit_count);
		// divide NB
	}
}

//commit instructions or flush ROB
void Commit(int NC)
{
	// commit instructions from ROB
	ROB * ROBEntry;
	RegStatus *RegStatusEntry ;
	void *valuePtr = malloc(sizeof(double));
		ROBEntry = getHeadCircularQueue(cpu -> reorderBuffer);
		while(ROBEntry != NULL && NC != 0)
		{
				if((strcmp(ROBEntry -> state, "W") == 0) && ROBEntry -> isReady == 1)
				{
					ROBEntry = dequeueCircular(cpu -> reorderBuffer);
					if(ROBEntry -> isINT == 1 && ROBEntry -> isStore == 0 && ROBEntry -> isBranch == 0){
							int DestRenameReg, DestVal, DestReg;
							DestRenameReg = ROBEntry -> DestRenameReg;
							DestReg = ROBEntry -> DestReg;
							DictionaryEntry * Current = getValueChainByDictionaryKey(cpu -> renameRegInt, &DestRenameReg);
							DestVal = *((int *)Current -> value -> value);
							//DestVal = 0;
							cpu -> integerRegisters [DestReg] -> data = DestVal;
							RegStatusEntry = cpu -> IntRegStatus[DestReg];
							RegStatusEntry->busy = 0;
							removeDictionaryEntriesByKey(cpu -> renameRegInt, &DestRenameReg);
							printf("Committed instruction %d in integer register %d with value %d \n", ROBEntry -> instruction -> address, DestReg, DestVal);
							NC --;
					}
					else if(ROBEntry -> isINT == 0 && ROBEntry -> isStore == 0 && ROBEntry -> isBranch == 0){
						int DestRenameReg, DestReg; float DestVal;
							DestRenameReg = ROBEntry -> DestRenameReg;
							DestReg = ROBEntry -> DestReg;
							DictionaryEntry * Current = getValueChainByDictionaryKey(cpu -> renameRegFP , &DestRenameReg);
							DestVal = *((double *)Current -> value -> value);
							//DestVal = 0.00;
							cpu -> floatingPointRegisters [DestReg] -> data = DestVal;
							RegStatusEntry = cpu -> FPRegStatus[DestReg];
							RegStatusEntry->busy = 0;
							NC --;
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
							*((int*)valuePtr) = DestVal; // value from rename register ;
							addDictionaryEntry (dataCache, &(ROBEntry -> DestAddr), valuePtr);
							removeDictionaryEntriesByKey(cpu -> renameRegInt, &DestRenameReg);
							//DestVal = 0.00;
							NC --;
						}
						else if(ROBEntry -> isINT == 0){
							float DestVal; int DestRenameReg;
							DestRenameReg = ROBEntry -> DestRenameReg;
							DictionaryEntry * Current = getValueChainByDictionaryKey(cpu -> renameRegFP, &DestRenameReg);
							DestVal = *((double *)Current -> value -> value);
							//*((int*)addrPtr) = ROBEntry -> DestAddr;
							removeDictionaryEntriesByKey (dataCache, &(ROBEntry -> DestAddr));
							*((double*)valuePtr) = (double) DestVal; // value from rename register ;
							addDictionaryEntry (dataCache, &(ROBEntry -> DestAddr), valuePtr);
							removeDictionaryEntriesByKey(cpu -> renameRegInt, &DestRenameReg);						
							//DestVal = 0;
							NC --;
						}
					}
					else{
						//Branch
						if(ROBEntry ->isBranch == 1 && ROBEntry -> isCorrectPredict == 0){
							while(ROBEntry != NULL || ROBEntry -> isAfterBranch == 0){
								if(ROBEntry -> isAfterBranch == 0){
									ROBEntry = dequeueCircular(cpu -> reorderBuffer);
								}
							}
							
						}
						NC--;
					}
				}
		
		else{
			printf("ROB not Commit\n");
			break;
		}
		//i++;
		
		
		}
		printf("Completed Commit\n");
}


// write back to RES
void writeBackUnit(int NB){
	// update outputs to ROB and Reservation stations
	int i;
	DictionaryEntry *head;
	CompletedInstruction *instruction;
	ROB *ROBentry;
	for(i = 0; i< NB; i++){
		head = popDictionaryEntry(cpu -> WriteBackBuffer);
		if(head != NULL){
			instruction = (CompletedInstruction *)head -> value -> value;
			//printf("instruction in Writeback has ROB_number - %d\n", instruction ->ROB_number);
			int j = 0;
			if (cpu -> reorderBuffer != NULL){
				ROBentry = cpu-> reorderBuffer -> items[cpu->reorderBuffer -> head];
				while(ROBentry != NULL){
					//printf("ROB instruction DestRenameReg - %d\n",  ROBentry -> DestRenameReg);
					if(ROBentry -> DestRenameReg == instruction -> ROB_number){
						ROBentry -> state = "W";
						ROBentry -> isReady = 1;
					}
					j++;
					ROBentry = cpu->reorderBuffer -> items[j];
				}			
			}
			updateOutputRES(instruction);
		}
	}
}


// update RES with output from execution
void updateOutputRES(CompletedInstruction *instruction){
	printf("RES update opcode - %d\t %d\n", instruction -> instruction -> op, instruction -> ROB_number);
	int robnumber = instruction -> ROB_number;
	DictionaryEntry *current;
	RSint *RSint;
	RSfloat *RSfloat;
	RSmem *RSmem;
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
				if (RSint -> isReady == 0){
                    if(RSint -> Qj == robnumber){
                            RSint -> Vj = instruction -> intResult;
                            RSint -> Qj = -1;
                        }
                    if(RSint -> Qk == robnumber){
                        RSint -> Vk = instruction -> intResult;
                        RSint -> Qk = -1;
                    }

                    if ((RSint -> Qj = -1) & (RSint -> Qk = -1)){
                        RSint -> isReady = 1;
                    }
				}
			}
            for (current = cpu -> resStaInt -> head; current != NULL; current = current -> next){
                RSint = current -> value -> value;
                if(RSint -> Qj == robnumber){
                    RSint -> Vj = instruction -> intResult;
                    printf("Qj rob number matches\n");
                }
                if(RSint -> Qk == robnumber){
                    RSint -> Vk = instruction -> intResult;
                    printf("Qk rob number matches\n");
                }
            }
   
         	for (current = cpu -> resStaMult -> head; current != NULL; current = current -> next){
				RSint = current -> value -> value;
				if(RSint -> Qj == robnumber){
					RSint -> Vj = instruction -> intResult;
						printf("mult Qj rob number matches\n");
				}
				if(RSint -> Qk == robnumber){
					RSint -> Vk = instruction -> intResult;
					printf(" mult Qk rob number matches\n");
				}
			}
			for (current = cpu -> storeBuffer -> head; current != NULL; current = current -> next){
					RSmem = current -> value -> value;
					if(RSmem -> Qj == robnumber){
						RSmem -> iVk = instruction -> intResult;	
					}
					if(RSmem -> Qk == robnumber){
						RSmem -> iVk = instruction -> intResult;
					}
			}
            for (current = cpu -> loadBuffer -> head; current != NULL; current = current -> next){
                RSmem = current -> value -> value;
                if(RSmem -> Qj == robnumber){
                    RSmem -> iVk = instruction -> intResult;
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
					if(RSfloat -> Qj == robnumber){
						RSfloat -> Vj = instruction -> fpResult;	
					}
					if(RSfloat -> Qk == robnumber){
						RSfloat -> Vk = instruction -> fpResult;
					}
				}
				for (current = cpu -> loadBuffer -> head; current != NULL; current = current -> next){
				RSmem = current -> value -> value;
				if(RSmem -> Qj == robnumber){
							RSmem -> fpVk = instruction -> fpResult;	
						}
				}
				for (current = cpu -> resStaFPmult -> head; current != NULL; current = current -> next){
				RSfloat = current -> value -> value;
				if(RSfloat -> Qj == robnumber){
						RSfloat -> Vj = instruction -> fpResult;	
					}
					if(RSfloat -> Qk == robnumber){
						RSfloat -> Vk = instruction -> fpResult;
					}
			}  
			for (current = cpu -> resStaFPdiv -> head; current != NULL; current = current -> next){
					RSfloat = current -> value -> value;
					if(RSfloat -> Qj == robnumber){
						RSfloat -> Vj = instruction -> fpResult;	
					}
					if(RSfloat -> Qk == robnumber){
						RSfloat -> Vk = instruction -> fpResult;
					}
				}
			
            for (current = cpu -> storeBuffer -> head; current != NULL; current = current -> next){
					RSmem = current -> value -> value;
					if(RSmem -> Qj == robnumber){
						RSmem -> fpVk = instruction -> fpResult;	
					}
					if(RSmem -> Qk == robnumber){
						RSmem -> fpVk = instruction -> fpResult;
					}
				}
            break;
     
        default:
            break;

	 }
}

// insert ouput results into write back buffer and update rename register
void insertintoWriteBackBuffer()
{
	int	*ROB_number = (int*) malloc(sizeof(int));
	CompletedInstruction *instruction;
	CompletedInstruction **unitOutputs;
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
		int* intresult = &(instruction -> intResult);
		addDictionaryEntry (cpu -> renameRegInt, ROB_number, intresult);
		printf ("Added to  WriteBack Buffer %d: ROB - %d  Output - %d\n",  instruction -> instruction -> address, instruction -> ROB_number, *intresult);	
	}
	if(unitOutputs[MULT] != NULL){
			
		instruction = unitOutputs[MULT];
		*ROB_number = instruction->ROB_number;
		addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
		removeDictionaryEntriesByKey (cpu -> renameRegInt, ROB_number);
		int* intresult = &(instruction -> intResult);
		addDictionaryEntry (cpu -> renameRegInt, ROB_number, intresult);
		printf ("Added to  WriteBack Buffer %d: ROB - %d Output - %d\n",  instruction -> instruction -> address, instruction -> ROB_number, *intresult);	
	}
	if(unitOutputs[FPadd] != NULL){
		instruction = unitOutputs[FPadd];
		*ROB_number = instruction->ROB_number;
		addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
		removeDictionaryEntriesByKey (cpu -> renameRegFP , ROB_number);
		double *fpresult = &(instruction -> fpResult);
		addDictionaryEntry (cpu -> renameRegFP, ROB_number, fpresult);
		printf ("Added to  WriteBack Buffer %d: ROB - %d Output - %f\n",  instruction -> instruction -> address, instruction -> ROB_number, *fpresult);	
	}
	if(unitOutputs[FPmult]!= NULL){
		instruction = unitOutputs[FPmult];
		*ROB_number = instruction->ROB_number;
		removeDictionaryEntriesByKey (cpu -> renameRegFP, ROB_number);
		double *fpresult = &(instruction -> fpResult);
		addDictionaryEntry (cpu -> renameRegFP, ROB_number, fpresult);
		addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
		printf ("Added to  WriteBack Buffer %d: ROB - %d Output - %f\n",  instruction -> instruction -> address, instruction -> ROB_number,*fpresult);	
	}
	if(unitOutputs[FPdiv] != NULL){
		instruction = unitOutputs[FPdiv];
		*ROB_number = instruction->ROB_number;
		addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
		removeDictionaryEntriesByKey (cpu -> renameRegFP, ROB_number);
		double *fpresult = &(instruction -> fpResult);
		addDictionaryEntry (cpu -> renameRegFP, ROB_number, fpresult);
		printf ("Added to  WriteBack Buffer %d: ROB - %d Output - %f\n",  instruction -> instruction -> address, instruction -> ROB_number, *fpresult);	
	}
	if(unitOutputs[BU] != NULL){
			
		instruction = unitOutputs[BU];
		*ROB_number = instruction->ROB_number;
		addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
		removeDictionaryEntriesByKey (cpu -> renameRegInt, ROB_number);
		int* intresult = &(instruction -> intResult);
		addDictionaryEntry (cpu -> renameRegInt, ROB_number, intresult);
		printf ("Added to  WriteBack Buffer %d: ROB - %d Output - %d\n",  instruction -> instruction -> address, instruction -> ROB_number,  *intresult);	
	}
	if(unitOutputs[LS] != NULL){
		instruction = unitOutputs[LS];
		OpCode op = instruction -> instruction -> op;
		if((strcmp(getOpcodeString (op) ,"SD") == 0))
		{
			*ROB_number = instruction->ROB_number;
			addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
			removeDictionaryEntriesByKey (cpu -> renameRegInt, ROB_number);
			int* intresult = &(instruction -> intResult);
			addDictionaryEntry (cpu -> renameRegInt, ROB_number, intresult);
			printf ("Added to  WriteBack Buffer %d: ROB - %d Output - %d\n",  instruction -> instruction -> address, instruction -> ROB_number, *intresult);	
	
		}
		else if((strcmp(getOpcodeString (op), "S_D") == 0))
		{
			*ROB_number = instruction->ROB_number;
			addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
			removeDictionaryEntriesByKey (cpu -> renameRegFP, ROB_number);
			double *fpresult = &(instruction -> fpResult);
			addDictionaryEntry (cpu -> renameRegFP, ROB_number, fpresult);
			
			printf ("Added to  WriteBack Buffer %d: ROB - %d Output - %f\n",  instruction -> instruction -> address, instruction -> ROB_number, *fpresult);
		}
		if((strcmp(getOpcodeString (op) ,"LD") == 0))
		{
			*ROB_number = instruction->ROB_number;
			addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
			removeDictionaryEntriesByKey (cpu -> renameRegInt, ROB_number);
			int* intresult = &(instruction -> intResult);
			addDictionaryEntry (cpu -> renameRegInt, ROB_number, intresult);
			printf ("Added to  WriteBack Buffer %d: LD  - ROB - %d Output - %d\n",  instruction -> instruction -> address, instruction -> ROB_number,  *intresult);	
	
		}
		else if((strcmp(getOpcodeString (op), "L_D") == 0))
		{
			*ROB_number = instruction->ROB_number;
			addDictionaryEntry (cpu -> WriteBackBuffer, ROB_number, instruction);
			removeDictionaryEntriesByKey (cpu -> renameRegFP, ROB_number);
			//*((double*)valuePtr) = (double) instruction -> fpResult;
			double *fpresult = &(instruction -> fpResult);
			addDictionaryEntry (cpu -> renameRegFP, ROB_number, fpresult);
			printf ("Added to  WriteBack Buffer %d: L_D -  ROB - %d Output - %f\n",  instruction -> instruction -> address, instruction -> ROB_number, *fpresult);
		}
		
	}
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
		while(ROBentry != NULL){
			if((strcmp(ROBentry -> state, "W") == 0) && ROBentry -> isReady == 1)
			{
				count ++;
			}
			else{
				break;
			}
			i++;
			ROBentry = cpu->reorderBuffer -> items[i];
		}
	}
	return count;
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
    
	insertintoWriteBackBuffer();
	CommitUnit(NB);

	updateFetchBuffer();
    updateInstructionQueue();
    updateReservationStations();

    printf("Finished update.\n");
	


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

void flushInstructionQueueFetchBuffer(int NI){
    cpu -> fetchBuffer = createDictionary (getHashCodeFromPCHash, compareInstructions);
	cpu -> fetchBufferResult = createDictionary (getHashCodeFromPCHash, compareInstructions);
	cpu -> instructionQueue = createCircularQueue(NI);
	cpu -> instructionQueueResult = createCircularQueue(NI);
	//Set flag to 0
	cpu->isAfterBranch = 0;
    }

/*
 * Determines if branch was predicted correctly or not and updates CPU accordingly.
 */
void branchHelper (CompletedInstruction *instructionAndResult) {
    int NI = cpu -> instructionQueue -> size;
    int *targetAddress = &(instructionAndResult -> instruction -> target);
    DictionaryEntry *BTBEntry = getValueChainByDictionaryKey (cpu -> branchTargetBuffer, targetAddress);
    if (instructionAndResult -> intResult == 0) { //branch taken
        if (BTBEntry == NULL) { //predicted not taken
            addDictionaryEntry (cpu -> branchTargetBuffer, &(instructionAndResult -> instruction -> address),
             targetAddress);
            instructionAndResult -> isCorrectPredict = 0;
            flushInstructionQueueFetchBuffer (NI);
            cpu -> PC = *targetAddress;
        } else { //predicted taken
            if (*(int *)(BTBEntry -> value -> value) == *targetAddress) {
                instructionAndResult -> isCorrectPredict = 1;
            } else {
                removeDictionaryEntriesByKey (cpu -> branchTargetBuffer, &(instructionAndResult -> instruction -> address));
                addDictionaryEntry (cpu -> branchTargetBuffer, &(instructionAndResult -> instruction -> address),
                 targetAddress);
                 instructionAndResult -> isCorrectPredict = 0;
                 flushInstructionQueueFetchBuffer (NI);
                 cpu -> PC = *targetAddress;
            }
        }
    } else { //branch not taken
        if (BTBEntry != NULL) { //predicted taken
            removeDictionaryEntriesByKey (cpu -> branchTargetBuffer, &(instructionAndResult -> instruction -> address));
            instructionAndResult -> isCorrectPredict = 0;
            flushInstructionQueueFetchBuffer (NI);
            cpu -> PC = instructionAndResult -> instruction -> address + 4;
        } else { //predicted not taken
            instructionAndResult -> isCorrectPredict = 1;
        }
    }
}


