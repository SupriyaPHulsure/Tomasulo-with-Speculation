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

int fetchMultiInstructionUnit(int NF);
Instruction * decodeInstruction(char *instruction_str, int instructionAddress);
int decodeInstructionsUnit(int NI);
void initializeCPU (int NI);


/**
 * This method initializes CPU data structures and all its data members
 */
void initializeCPU (int NI) {
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

	cpu -> fetchBuffer = createDictionary (getHashCodeFromPCHash, compareInstructions);
	cpu -> fetchBufferResult = createDictionary (getHashCodeFromPCHash, compareInstructions);
	cpu -> instructionQueue = createCircularQueue(NI);
	cpu -> instructionQueueResult = createCircularQueue(NI);
	cpu -> branchTargetBuffer = createDictionary(getHashCodeFromBranchAddress, compareTargetAddress);

	cpu -> stallNextFetch = 0;
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
        if (isFullCircularQueue(cpu -> instructionQueueResult)){
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
    return ;
}

//TODO: update instruction queue

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

/**
 * Method that simulates pipelined Unit.
 * @return pointer to output instruction of the pipeline.
 */
CompletedInstruction *executePipelinedUnit (CircularQueue *pipeline) {
    CompletedInstruction *output = dequeueCircular (pipeline);

    if (pipeline -> count < pipeline -> size) {
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
CompletedInstruction **execute(Instruction * instruction){
    void *valuePtr = malloc(sizeof(double));
    void *addrPtr = malloc(sizeof(int));
  	DictionaryEntry *dataCacheElement;
  	CompletedInstruction *instructionAndResult = malloc(sizeof(CompletedInstruction));
    instructionAndResult -> instruction = instruction;

    //Array for outputs of Units. See Unit enum in DataTypes.h
    static CompletedInstruction *unitOutputs[7];

    //pipelined
    switch (instruction->op) {
        case ANDI:
            instructionAndResult -> intResult = cpu->integerRegisters[instruction->rs]->data & instruction->immediate;
            enqueueCircular (cpu -> INTPipeline, instructionAndResult);
            break;
        case AND:
            instructionAndResult -> intResult = cpu->integerRegisters[instruction->rs]->data & cpu->integerRegisters[instruction->rt]->data;
            enqueueCircular (cpu -> INTPipeline, instructionAndResult);
            break;
        case ORI:
            instructionAndResult -> intResult = cpu->integerRegisters[instruction->rs]->data | instruction->immediate;
            enqueueCircular (cpu -> INTPipeline, instructionAndResult);
            break;
        case OR:
            instructionAndResult -> intResult = cpu->integerRegisters[instruction->rs]->data | cpu->integerRegisters[instruction->rt]->data;
            enqueueCircular (cpu -> INTPipeline, instructionAndResult);
            break;
        case SLTI:
            instructionAndResult -> intResult = cpu->integerRegisters[instruction->rs]->data < instruction->immediate ? 1 : 0;
            enqueueCircular (cpu -> INTPipeline, instructionAndResult);
            break;
        case SLT:
            instructionAndResult -> intResult = cpu->integerRegisters[instruction->rs]->data < cpu->integerRegisters[instruction->rt]->data ? 1 : 0;
            enqueueCircular (cpu -> INTPipeline, instructionAndResult);
            break;
        case DADDI:
            instructionAndResult -> intResult = cpu->integerRegisters[instruction->rs]->data + instruction->immediate;
            enqueueCircular (cpu -> INTPipeline, instructionAndResult);
            break;
        case DADD:
            instructionAndResult -> intResult = cpu->integerRegisters[instruction->rs]->data + cpu->integerRegisters[instruction->rt]->data;
            enqueueCircular (cpu -> INTPipeline, instructionAndResult);
            break;
        case DSUB:
            instructionAndResult -> intResult = cpu->integerRegisters[instruction->rs]->data - cpu->integerRegisters[instruction->rt]->data;
            enqueueCircular (cpu -> INTPipeline, instructionAndResult);
            break;
        case DMUL:
            instructionAndResult -> intResult = cpu->integerRegisters[instruction->rs]->data * cpu->integerRegisters[instruction->rt]->data;
            enqueueCircular (cpu -> MULTPipeline, instructionAndResult);
            break;
        case ADD_D:
            instructionAndResult -> fpResult = cpu->floatingPointRegisters[instruction->fs]->data + cpu->floatingPointRegisters[instruction->ft]->data;
            enqueueCircular (cpu -> FPaddPipeline, instructionAndResult);
            break;
        case SUB_D:
            instructionAndResult -> fpResult = cpu->floatingPointRegisters[instruction->fs]->data - cpu->floatingPointRegisters[instruction->ft]->data;
            enqueueCircular (cpu -> FPaddPipeline, instructionAndResult);
            break;
        case MUL_D:
            instructionAndResult -> fpResult = cpu->floatingPointRegisters[instruction->fs]->data * cpu->floatingPointRegisters[instruction->ft]->data;
            enqueueCircular (cpu -> FPmultPipeline, instructionAndResult);
            break;
        case DIV_D:
            instructionAndResult -> fpResult = cpu->floatingPointRegisters[instruction->fs]->data / cpu->floatingPointRegisters[instruction->ft]->data;
            enqueueCircular (cpu -> FPdivPipeline, instructionAndResult);
            cpu -> FPdivPipelineBusy = 1;
            break;
        case L_D:
            cpu->memoryAddress = cpu->integerRegisters[instruction->rs]->data + instruction->immediate;
            void *addrPtr = malloc(sizeof(int));
            * ((int*)addrPtr) = cpu->memoryAddress;
            dataCacheElement = getValueChainByDictionaryKey(dataCache, addrPtr);
            valuePtr = dataCacheElement->value->value;
            instructionAndResult -> fpResult = *((double*)valuePtr);
            enqueueCircular (cpu -> LoadStorePipeline, instructionAndResult);
            break;
        case LD:
            cpu->memoryAddress = cpu->integerRegisters[instruction->rs]->data + instruction->immediate;
            * ((int*)addrPtr) = cpu->memoryAddress;
            dataCacheElement = getValueChainByDictionaryKey(dataCache, addrPtr);
            valuePtr = dataCacheElement->value->value;
            instructionAndResult -> intResult = (int)*((double*)valuePtr);
            enqueueCircular (cpu -> LoadStorePipeline, instructionAndResult);
            break;
        case SD:
            instructionAndResult -> address = cpu->integerRegisters[instruction->rs]->data + instruction->immediate;
            instructionAndResult -> intResult = cpu->integerRegisters[instruction->rt]->data;
            enqueueCircular (cpu -> LoadStorePipeline, instructionAndResult);
            break;
        case S_D:
            instructionAndResult -> address = cpu->integerRegisters[instruction->rs]->data + instruction->immediate;
            instructionAndResult -> intResult = cpu->floatingPointRegisters[instruction->ft]->data;
            enqueueCircular (cpu -> LoadStorePipeline, instructionAndResult);
            break;
        case BNE:
            instructionAndResult -> intResult = cpu->integerRegisters[instruction->rs]->data != cpu->integerRegisters[instruction->rt]->data ? 0 : -1;
            enqueueCircular (cpu -> BUPipeline, instructionAndResult);
            break;
        case BNEZ:
            instructionAndResult -> intResult = cpu->integerRegisters[instruction->rs]->data != 0 ? 0 : -1;
            enqueueCircular (cpu -> BUPipeline, instructionAndResult);
            break;
        case BEQ:
            instructionAndResult -> intResult = cpu->integerRegisters[instruction->rs]->data == cpu->integerRegisters[instruction->rt]->data ? 0 : -1;
            enqueueCircular (cpu -> BUPipeline, instructionAndResult);
            break;
        case BEQZ:
            instructionAndResult -> intResult = cpu->integerRegisters[instruction->rs]->data == 0 ? 0 : -1;
            enqueueCircular (cpu -> BUPipeline, instructionAndResult);
            break;
        default:
            break;
    }

    //Take outputs from Units
    unitOutputs[INT] = executePipelinedUnit (cpu -> INTPipeline);
    unitOutputs[MULT] = executePipelinedUnit (cpu -> MULTPipeline);
    unitOutputs[LS] = executePipelinedUnit (cpu -> LoadStorePipeline);
    unitOutputs[FPadd] = executePipelinedUnit (cpu -> FPaddPipeline);
    unitOutputs[FPmult] = executePipelinedUnit (cpu -> FPmultPipeline);
    unitOutputs[FPdiv] = executeFPDivUnit (cpu -> FPdivPipeline);
    unitOutputs[BU] = executePipelinedUnit (cpu -> BUPipeline);

    return unitOutputs;

//	switch (instruction -> op) {
//			case ANDI:
//				cpu -> intResult = cpu -> integerRegisters [instruction -> rs] -> data & instruction -> immediate;
//				cpu -> integerRegisters [instruction -> rd] -> data = cpu -> intResult;
//				cpu -> PC = cpu -> PC + 4;
//				break;
//			case AND:
//                                cpu -> intResult = cpu -> integerRegisters [instruction -> rs] -> data & cpu -> integerRegisters [instruction -> rt] -> data;
//				cpu -> integerRegisters [instruction -> rd] -> data = cpu -> intResult;
//				cpu -> PC = cpu -> PC + 4;
//                                break;
//			case ORI:
//				cpu -> intResult = cpu -> integerRegisters [instruction -> rs] -> data | instruction -> immediate;
//				cpu -> integerRegisters [instruction -> rd] -> data = cpu -> intResult;
//				cpu -> PC = cpu -> PC + 4;
//				break;
//                        case OR:
//				cpu -> intResult = cpu -> integerRegisters [instruction -> rs] -> data | cpu -> integerRegisters [instruction -> rt] -> data;
//				cpu -> integerRegisters [instruction -> rd] -> data = cpu -> intResult;
//				cpu -> PC = cpu -> PC + 4;
//                                break;
//			case SLTI:
//				cpu -> intResult = cpu -> integerRegisters [instruction -> rs] -> data < instruction -> immediate ? 1 : 0;
//				cpu -> integerRegisters [instruction -> rd] -> data = cpu -> intResult;
//				cpu -> PC = cpu -> PC + 4;
//				break;
//                        case SLT:
//				cpu -> intResult = cpu -> integerRegisters [instruction -> rs] -> data < cpu -> integerRegisters [instruction -> rt] -> data ? 1 : 0;
//				cpu -> integerRegisters [instruction -> rd] -> data = cpu -> intResult;
//				cpu -> PC = cpu -> PC + 4;
//                                break;
//			case DADDI:
//				cpu -> intResult = cpu -> integerRegisters [instruction -> rs] -> data + instruction -> immediate;
//				cpu -> integerRegisters [instruction -> rd] -> data = cpu -> intResult;
//				cpu -> PC = cpu -> PC + 4;
//				break;
//                        case DADD:
//				cpu -> intResult = cpu -> integerRegisters [instruction -> rs] -> data + cpu -> integerRegisters [instruction -> rt] -> data;
//				cpu -> integerRegisters [instruction -> rd] -> data = cpu -> intResult;
//				cpu -> PC = cpu -> PC + 4;
//                                break;
//                        case DSUB:
//				cpu -> intResult = cpu -> integerRegisters [instruction -> rs] -> data - cpu -> integerRegisters [instruction -> rt] -> data;
//				cpu -> integerRegisters [instruction -> rd] -> data = cpu -> intResult;
//				cpu -> PC = cpu -> PC + 4;
//                                break;
//			case DMUL:
//				cpu -> intResult = cpu -> integerRegisters [instruction -> rs] -> data * cpu -> integerRegisters [instruction -> rt] -> data;
//				cpu -> integerRegisters [instruction -> rd] -> data = cpu -> intResult;
//				cpu -> PC = cpu -> PC + 4;
//                                break;
//                        case ADD_D:
//                                cpu -> fpResult = cpu -> floatingPointRegisters [instruction -> fs] -> data + cpu -> floatingPointRegisters [instruction -> ft] -> data;
//				cpu -> floatingPointRegisters [instruction -> fd] -> data = cpu -> fpResult;
//				cpu -> PC = cpu -> PC + 4;
//                                break;
//                        case SUB_D:
//				cpu -> fpResult = cpu -> floatingPointRegisters [instruction -> fs] -> data - cpu -> floatingPointRegisters [instruction -> ft] -> data;
//				cpu -> floatingPointRegisters [instruction -> fd] -> data = cpu -> fpResult;
//				cpu -> PC = cpu -> PC + 4;
//                                break;
//                        case MUL_D:
//				cpu -> fpResult = cpu -> floatingPointRegisters [instruction -> fs] -> data * cpu -> floatingPointRegisters [instruction -> ft] -> data;
//				cpu -> floatingPointRegisters [instruction -> fd] -> data = cpu -> fpResult;
//				cpu -> PC = cpu -> PC + 4;
//                                break;
//                        case DIV_D:
//				cpu -> fpResult = cpu -> floatingPointRegisters [instruction -> fs] -> data / cpu -> floatingPointRegisters [instruction -> ft] -> data;
//				cpu -> floatingPointRegisters [instruction -> fd] -> data = cpu -> fpResult;
//				cpu -> PC = cpu -> PC + 4;
//                                break;
//                        case L_D:
//				cpu -> memoryAddress = cpu -> integerRegisters [instruction -> rs] -> data + instruction -> immediate;
//
//
//				*((int*)addrPtr) = cpu -> memoryAddress;
//				dataCacheElement = getValueChainByDictionaryKey (dataCache, addrPtr);
//
//				valuePtr = dataCacheElement -> value -> value;
//				cpu -> fpResult = *((double*)valuePtr);
//
//				cpu -> floatingPointRegisters [instruction -> ft] -> data = cpu -> fpResult;
//				cpu -> PC = cpu -> PC + 4;
//				break;
//			case LD:
//				cpu -> memoryAddress = cpu -> integerRegisters [instruction -> rs] -> data + instruction -> immediate;
//
//				*((int*)addrPtr) = cpu -> memoryAddress;
//				dataCacheElement = getValueChainByDictionaryKey (dataCache, addrPtr);
//
//				valuePtr = dataCacheElement -> value -> value;
//				cpu -> intResult =  (int) *((double*)valuePtr);
//
//				cpu -> integerRegisters [instruction -> rt] -> data = cpu -> intResult;
//				cpu -> PC = cpu -> PC + 4;
//                                break;
//                        case SD:
//                                cpu -> memoryAddress = cpu -> integerRegisters [instruction -> rs] -> data + instruction -> immediate;
//                                cpu -> intResult = cpu -> integerRegisters [instruction -> rt] -> data ;
//
//				*((int*)addrPtr) = cpu -> memoryAddress;
//				removeDictionaryEntriesByKey (dataCache, addrPtr);
//
//				*((double*)valuePtr) = cpu -> intResult;
//				addDictionaryEntry (dataCache, addrPtr, valuePtr);
//				cpu -> PC = cpu -> PC + 4;
//                                break;
//                        case S_D:
//                                cpu -> memoryAddress = cpu -> integerRegisters [instruction -> rs] -> data  + instruction -> immediate;
//                                cpu -> fpResult = cpu -> floatingPointRegisters [instruction -> ft] -> data ;
//
//				*((int*)addrPtr) = cpu -> memoryAddress;
//                                removeDictionaryEntriesByKey (dataCache, addrPtr);
//
//                                *((double*)valuePtr) = (double) cpu -> fpResult;
//                                addDictionaryEntry (dataCache, addrPtr, valuePtr);
//				cpu -> PC = cpu -> PC + 4;
//                                break;
//			case BNE:
//                                cpu -> intResult = cpu -> integerRegisters [instruction -> rs] -> data != cpu -> integerRegisters [instruction -> rt] -> data ? 0 : -1;
//				if (cpu -> intResult == 0) {
//					cpu -> PC = instruction -> target;
//				} else {
//					cpu -> PC = cpu -> PC + 4;
//				}
//                                break;
//                        case BNEZ:
//                                cpu -> intResult = cpu -> integerRegisters [instruction -> rs] -> data != 0 ? 0 : -1;
//				if (cpu -> intResult == 0) {
//                                        cpu -> PC = instruction -> target;
//                                } else {
//					cpu -> PC = cpu -> PC + 4;
//				}
//                                break;
//                        case BEQ:
//                                cpu -> intResult = cpu -> integerRegisters [instruction -> rs] -> data == cpu -> integerRegisters [instruction -> rt] -> data ? 0 : -1;
//				if (cpu -> intResult == 0) {
//                                        cpu -> PC = instruction -> target;
//                                } else {
//					cpu -> PC = cpu -> PC + 4;
//				}
//                                break;
//                        case BEQZ:
//                                cpu -> intResult = cpu -> integerRegisters [instruction -> rs] -> data == 0 ? 0 : -1;
//				if (cpu -> intResult == 0) {
//                                        cpu -> PC = instruction -> target;
//                                } else {
//					cpu -> PC = cpu -> PC + 4;
//				}
//                                break;
//                        default:
//                                break;
//                }

}


/**
 * Method that simulates the looping cycle-wise
 * @return: When the simulator stops
 */
int runClockCycle (int NF, int NI) {

	cpu -> cycle++; //increment cycle counter

	printf ("\nCycle %d\n", cpu -> cycle);

    fetchMultiInstructionUnit(NF);

    decodeInstructionsUnit(NI);

    updateFetchBuffer();

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


