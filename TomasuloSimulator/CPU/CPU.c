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
int addInstruction2RSint(Dictionary *renameReg, Dictionary *resSta, Dictionary *resStaResult,
                      char* rsType, int maxLenRS, Instruction *instruction);
int addInstruction2RSfloat(Dictionary *renameReg, Dictionary *resSta, Dictionary *resStaResult,
                      char* rsType, int maxLenRS, Instruction *instruction);
int issueInstruction(Instruction *instruction);
void issueUnit(int NW);


ROB * InitializeROBEntry(Instruction * instructionP);


/**
 * This method initializes CPU data structures and all its data members
 */
void initializeCPU (int NI, int NR) {
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
    //Initialize load/store buffers
//    cpu -> loadBuffer = createCircularQueue(4);
//    cpu -> storeBuffer = createCircularQueue(6);
    //Initialize Reorder buffer
    cpu -> reorderBuffer = createCircularQueue(NR);
    //Initialize Stall counters
    cpu -> stallFullROB = 0;
    cpu -> stallFullRS = 0;
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
//    while((RSEntry = popDictionaryEntry(cpu -> resStaLoadResult)) != NULL){
//        appendDictionaryEntry(cpu -> resStaLoad, RSEntry);
//    }
//    while((RSEntry = popDictionaryEntry(cpu -> resStaStoreResult)) != NULL){
//        appendDictionaryEntry(cpu -> resStaStore, RSEntry);
//    }
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
int addInstruction2RSint(Dictionary *renameReg, Dictionary *resSta, Dictionary *resStaResult,
                      char* rsType, int maxLenRS, Instruction *instruction){
    int counterUnit;
    int counterUnitResult;
    RSint* RS = (RSint*) malloc (sizeof(RSint));
    counterUnit = countDictionaryLen(resSta);
    counterUnitResult = countDictionaryLen(resStaResult);
    if (maxLenRS - counterUnit - counterUnitResult > 0){
        int DestROBnum = cpu -> reorderBuffer->tail;
        DictionaryEntry *renameRegEntry = getValueChainByDictionaryKey(renameReg, &instruction->rs);
        if (renameRegEntry != NULL){
            int robNum = ((RenameReg *)renameRegEntry -> value -> value) -> reorderNum;
            if (((ROB *)cpu -> reorderBuffer -> items[robNum]) -> isReady == 1){
                RS -> Vj = ((ROB *)cpu -> reorderBuffer -> items[robNum]) -> DestValueIntReg;
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
            DictionaryEntry *renameRegEntry = getValueChainByDictionaryKey(renameReg, &instruction->rt);
            if (renameRegEntry!=NULL){
                int robNum = ((RenameReg *)renameRegEntry -> value -> value) -> reorderNum;
                if (((ROB *)cpu -> reorderBuffer -> items[robNum]) -> isReady == 1){
                    RS -> Vk = ((ROB *)cpu -> reorderBuffer -> items[robNum]) -> DestValueIntReg;
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
        //Add to renaming registers
        RenameReg *renamingReg = (RenameReg*)malloc(sizeof(RenameReg));
        renamingReg -> reorderNum = DestROBnum;
        renamingReg -> busy = 1;
        removeDictionaryEntriesByKey(renameReg, &(instruction->rd));
        addDictionaryEntry(renameReg, &(instruction->rd), renamingReg);
        printf("Issued instruction %d: %s\n", instruction->address, getOpcodeString ((int) instruction->op));
        return 1;

    }else{
        cpu -> stallFullRS ++;
        printf("Stall during IssueUnit because reservation stations %s is full.\n", rsType);
        return 0;
    }
}

//Add instruction to reservation stations of type floating pointer
int addInstruction2RSfloat(Dictionary *renameReg, Dictionary *resSta, Dictionary *resStaResult,
                      char* rsType, int maxLenRS, Instruction *instruction){
    int counterUnit;
    int counterUnitResult;
    RSfloat* RS = (RSfloat*) malloc (sizeof(RSfloat));
    counterUnit = countDictionaryLen(resSta);
    counterUnitResult = countDictionaryLen(resStaResult);
    if (maxLenRS - counterUnit - counterUnitResult > 0){
        int DestROBnum = cpu -> reorderBuffer->tail;
        DictionaryEntry *renameRegEntry = getValueChainByDictionaryKey(renameReg, &instruction->fs);
        if (renameRegEntry != NULL){
            int robNum = ((RenameReg *)renameRegEntry -> value -> value) -> reorderNum;
            if (((ROB *)cpu -> reorderBuffer -> items[robNum]) -> isReady == 1){
                RS -> Vj = ((ROB *)cpu -> reorderBuffer -> items[robNum]) -> DestValueIntReg;
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
            DictionaryEntry *renameRegEntry = getValueChainByDictionaryKey(renameReg, &instruction->ft);
            if (renameRegEntry!=NULL){
                int robNum = ((RenameReg *)renameRegEntry -> value -> value) -> reorderNum;
                if (((ROB *)cpu -> reorderBuffer -> items[robNum]) -> isReady == 1){
                    RS -> Vk = ((ROB *)cpu -> reorderBuffer -> items[robNum]) -> DestValueIntReg;
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
        RS->Dest = DestROBnum;
        RS->instruction = instruction;
        addDictionaryEntry(resStaResult, &(RS->Dest), RS);
        RenameReg *renamingReg = (RenameReg*)malloc(sizeof(RenameReg));
        renamingReg -> reorderNum = DestROBnum;
        renamingReg -> busy = 1;
        removeDictionaryEntriesByKey(renameReg, &instruction->fd);
        addDictionaryEntry(renameReg, &instruction->fd, renamingReg);
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
int addLoadStore2Buffer(Dictionary *renameReg, Dictionary *LOrSBuffer, Dictionary *LOrSBufferResult,
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
        RSmem *bufferItem;
        int i;
        RS -> isReady = 1; //Will be set to 0 later if necessary
        DictionaryEntry *renameRegEntry;
        int DestROBnum = cpu -> reorderBuffer->tail;
        renameRegEntry = getValueChainByDictionaryKey(renameReg, &instruction->rs);
        if (renameRegEntry != NULL){
            int robNum = ((RegStatus *)renameRegEntry -> value -> value) -> reorderNum;
            if (((ROB *)cpu -> reorderBuffer -> items[robNum]) -> isReady == 1){
                RS -> Vj = ((ROB *)cpu -> reorderBuffer -> items[robNum]) -> DestValueIntReg;
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
         else if (strcmp(buffType, "Store") == 0) {
            if (instruction -> op == S_D) {
                renameRegEntry = getValueChainByDictionaryKey (renameReg, &instruction->ft);
                if (renameRegEntry!=NULL){
                    int robNum = ((RegStatus *)renameRegEntry -> value -> value) -> reorderNum;
                    if (((ROB *)cpu -> reorderBuffer -> items[robNum]) -> isReady == 1){
                        RS -> fpVk = ((ROB *)cpu -> reorderBuffer -> items[robNum]) -> DestValueFloatReg;
                        RS -> Qk = -1;
                    }
                    else{
                        RS -> Qk = robNum;
                        RS->isReady = 0;
                    }
                }else{
                    RS -> Vk = cpu->floatingPointRegisters[instruction->ft]->data;
                    RS -> Qk = -1;
                }
            } else {
                renameRegEntry = getValueChainByDictionaryKey(renameReg, &instruction->rt);
                if (renameRegEntry!=NULL){
                    int robNum = ((RenameReg *)renameRegEntry -> value -> value) -> reorderNum;
                    if (((ROB *)cpu -> reorderBuffer -> items[robNum]) -> isReady == 1){
                        RS -> iVk = ((ROB *)cpu -> reorderBuffer -> items[robNum]) -> DestValueIntReg;
                        RS -> Qk = -1;
                        if(RS->isReady == 1)
                            RS->isReady = 1;
                    }
                    else{
                        RS -> Qk = robNum;
                        RS->isReady = 0;
                    }
                }else{
                    RS -> iVk = cpu->integerRegisters[instruction->rt]->data;
                    RS -> Qk = -1;
                }
            }
        } else {
            printf(buffType);
        }
        RS -> address = -1;
        appendDictionaryEntry(LOrSBufferResult, &(RS->Dest), RS);
        //Add to renaming registers if load
        if (strcmp(buffType, "Load") == 0) {
            RegStatus *renamingReg = (RenameReg*)malloc(sizeof(RenameReg));
            renamingReg -> reorderNum = DestROBnum;
            renamingReg -> busy = 1;
            if (instruction -> op == L_D) {
                removeDictionaryEntriesByKey(renameReg, &(instruction->ft));
                addDictionaryEntry(renameReg, &(instruction->ft), renamingReg);
            } else {
                removeDictionaryEntriesByKey(renameReg, &(instruction->rt));
                addDictionaryEntry(renameReg, &(instruction->rt), renamingReg);
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
                                            instruction);
            }
            break;
        case DMUL:
            renameRegFull = renameRegIsFull(cpu->renameRegInt, instruction -> rd);
            if (renameRegFull!=1){
                char* rsType = "MULT";
                issued = addInstruction2RSint(cpu->renameRegInt, cpu->resStaMult, cpu->resStaMultResult,
                                           rsType, numberRSmult, instruction);
            }
            break;
        case ADD_D:
        case SUB_D:
            renameRegFull = renameRegIsFull(cpu->renameRegFP, instruction -> fd);
            if (renameRegFull!=1){
                char* rsType = "FPadd";
                issued = addInstruction2RSfloat(cpu->renameRegFP, cpu->resStaFPadd, cpu->resStaFPaddResult,
                                               rsType, numberRSfpAdd, instruction);
            }
            break;
        case MUL_D:
            renameRegFull = renameRegIsFull(cpu->renameRegFP, instruction -> fd);
            if (renameRegFull!=1){
                char* rsType = "FPmult";
                issued = addInstruction2RSfloat(cpu->renameRegFP, cpu->resStaFPmult, cpu->resStaFPmultResult,
                                               rsType, numberRSfpMult, instruction);
            }
            break;
        case DIV_D:
            renameRegFull = renameRegIsFull(cpu->renameRegFP, instruction -> fd);
            if (renameRegFull!=1){
                char* rsType = "FPdiv";
                issued = addInstruction2RSfloat(cpu->renameRegFP, cpu->resStaFPdiv, cpu->resStaFPdivResult,
                                               rsType, numberRSfpDiv, instruction);
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
    int i, j, index, LSbufferInsert;
    char *pipelineString;
    RSmem *bufferItem;
    //variables for loads
    RSmem *RSint;
    int instructionFoundOrBubble;
    DictionaryEntry *dictEntry;
    CircularQueue *buff;
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
                //Two-step
                //First calculate address for earliest load that needs it
                rsmem -> address = rsmem -> Vj + instruction->immediate;
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
                            if ((ROB *)(buff -> items[(buff -> head + i) % (buff->size)])-> DestAddr == RS -> address) {
                                dictEntry = dictEntry -> next;
                                i = -1; //break out of for loop
                            }
                            bufferItem = bufferItem -> next;
                        }
                        if (i != -1) {
                            instructionFoundOrBubble = 1;
                            rsmem = RS;
                        }
                    } else {
                        dictEntry = dictEntry -> next;
                    }
                }
                * ((int*)addrPtr) = rsmem -> address;
                dataCacheElement = getValueChainByDictionaryKey(dataCache, addrPtr);
                valuePtr = dataCacheElement->value->value;
                instructionAndResult -> fpResult = *((double*)valuePtr);
                instructionAndResult -> ROB_number = rsmem -> Dest;
                LoadPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(LoadPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "Load/Store";
                printPipeline(instruction, pipelineString, 1);
                break;
            case LD:
                //Two-step
                //First calculate address for earliest load that needs it
                rsmem -> address = rsmem -> Vj + instruction->immediate;
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
                            if ((ROB *)(buff -> items[(buff -> head + i) % (buff->size)])-> DestAddr == RS -> address) {
                                dictEntry = dictEntry -> next;
                                i = -1; //break out of for loop
                            }
                            bufferItem = bufferItem -> next;
                        }
                        if (i != -1) {
                            instructionFoundOrBubble = 1;
                            rsmem = RS;
                        }
                    } else {
                        dictEntry = dictEntry -> next;
                    }
                }
                * ((int*)addrPtr) = rsmem -> address;
                dataCacheElement = getValueChainByDictionaryKey(dataCache, addrPtr);
                valuePtr = dataCacheElement->value->value;
                instructionAndResult -> intResult = (int)*((double*)valuePtr);
                instructionAndResult -> ROB_number = rsmem -> Dest;
                LoadPipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(LoadPipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "Load/Store";
                printPipeline(instruction, pipelineString, 1);
                break;
            case SD:
                rsmem -> address = rsmem -> Vj + instruction->immediate;
                instructionAndResult -> address = rsmem -> address;
                instructionAndResult -> intResult = rsmem -> iVk;
                StorePipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(StorePipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "Load/Store";
                printPipeline(instruction, pipelineString, 1);
                break;
            case S_D:
                rsmem -> address = rsmem -> Vj + instruction->immediate;
                instructionAndResult -> address = rsmem -> address;
                instructionAndResult -> fpResult = rsmem -> fpVk;
                StorePipelineTemp = malloc(sizeof(CompletedInstruction)*2);
                memcpy(StorePipelineTemp, instructionAndResult, sizeof(CompletedInstruction));
                pipelineString = "Load/Store";
                printPipeline(instruction, pipelineString, 1);
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
        getValueChainByDictionaryKey (cpu -> renameRegInt, unitOutputs[INT] -> instruction)
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
		ROBEntry->DestReg = instructionP->fd;
			ROBEntry -> isINT = 0;
            break;
        case LD:
			ROBEntry->DestReg = instructionP->rd;
			ROBEntry -> isINT = 1;
            break;
        case SD:
			ROBEntry->DestReg = instructionP->rs;
			ROBEntry -> isINT = 1;
			ROBEntry -> isStore = 1;
            break;
        case S_D:
			ROBEntry->DestReg = instructionP->fs;
			ROBEntry -> isINT = 0;
			ROBEntry -> isStore = 1;
            break;
        case BNE:
				ROBEntry->DestReg = -1;
				ROBEntry -> isINT = 0;
            break;
        case BNEZ:
				ROBEntry->DestReg = -1;
				ROBEntry -> isINT = 0;
            break;
        case BEQ:
				ROBEntry->DestReg = -1;
				ROBEntry -> isINT = 0;
            break;
        case BEQZ:
				ROBEntry->DestReg = -1;
				ROBEntry -> isINT = 0;
            break;
        default:
				ROBEntry->DestReg = -1;
				ROBEntry -> isINT = 0;
            break;
    }
	
	ROBEntry->instruction = instructionP;
	ROBEntry->state = "I";
	ROBEntry->DestValueIntReg = 0;
	ROBEntry -> DestValueFloatReg = 0.0;
	ROBEntry->isReady = 0;
	ROBEntry -> DestAddr = 0;
	return ROBEntry;
}




/**
 * Method that simulates the looping cycle-wise
 * @return: When the simulator stops
 */
int runClockCycle (int NF, int NI, int NW) {

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
 * @param int selectRS 0 to for RSint, 1 for RSfloat, 2 for Store, 4 for step 2 of load
 */
DictionaryValue *checkReservationStation(DictionaryEntry *dictEntry, int selectRS) {
    while (dictEntry != NULL) {
        if (selectRS == 1) {
            RSfloat *RS = (RSfloat *)((DictionaryEntry *)dictEntry -> value -> value);
            if (RS -> isReady) {
                return ((DictionaryEntry *)dictEntry) -> value;
            } else {
                dictEntry = dictEntry -> next;
            }
        } else if (selectRS == 2) {
            RSmem *RS = (RSmem *)((DictionaryEntry *)dictEntry -> value -> value);
            if (RS -> isReady) {
                return ((DictionaryEntry *)dictEntry) -> value;
            } else {
                dictEntry = dictEntry -> next;
            }
        } else if (selectRS == 3) {
            RSmem *RS = (RSmem *)((DictionaryEntry *)dictEntry -> value -> value);
            if (RS -> isReady && RS -> address == -1) {
                return ((DictionaryEntry *)dictEntry) -> value;
            } else {
                dictEntry = dictEntry -> next;
            }
        }
        } else if (selectRS == 4) {
            RSmem *RS = (RSmem *)((DictionaryEntry *)dictEntry -> value -> value);
            if (RS -> isReady && RS -> address != -1) {
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


