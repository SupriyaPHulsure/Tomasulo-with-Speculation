/*
 * DataTypes.h
 *
 *  Created on: Sep 29, 2015
 *      Author: DebashisGanguly
 */

#ifndef GLOBAL_DATATYPES_H_
#define GLOBAL_DATATYPES_H_

#include "./ADT/CircularQueue.h"
#include "./ADT/Dictionary.h"

typedef enum _opCode {
	AND,
	ANDI,
	OR,
	ORI,
	SLT,
	SLTI,
	DADD,
	DADDI,
	DSUB,
	DMUL,
	LD,
	SD,
	L_D,
	S_D,
	ADD_D,
	SUB_D,
	MUL_D,
	DIV_D,
	BEQZ,
	BNEZ,
	BEQ,
	BNE,
	NOOP
} OpCode;

static inline char *getOpcodeString (int opcode) {
	char *opcodeString[23] = {"AND", "ANDI", "OR", "ORI", "SLT", "SLTI", "DADD", "DADDI", "DSUB", "DMUL", "LD", "SD", "L_D", "S_D", "ADD_D", "SUB_D", "MUL_D", "DIV_D", "BEQZ", "BNEZ", "BEQ", "BNE", "NOOP"};

	return opcodeString[opcode];
}

//data structure for decoded instruction
typedef struct _instruction {
	OpCode op;
	int address;

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

} Instruction;

//data structure to bind instruction and its result in pipeline
typedef struct _completedInstruction {
    Instruction *instruction;
    int intResult;
    double fpResult;
    int address; //used to keep address for SD and S.D
} CompletedInstruction;

//data structure for each item in integer registers and work as well register status
typedef struct _INTReg {
	int data;
	int intResult;
} INTReg;

//data structure for each item in floating point registers and work as well register status
typedef struct _FPReg {
	double data;
        double fpResult;
} FPReg;

typedef struct _ROB{
	Instruction instruction;
	int intDestReg;
	int intValueReg;
	char isReady;
	char * state;
	char * Rename_name;
}ROB;

//main data structure representing CPU
typedef struct _cpu {
	int cycle; //counter for number of cycles

	int numberOfInstructionExecuted; //counter for total number of instructions committed

	int PC; //program counter

	INTReg **integerRegisters; //integer register
    FPReg **floatingPointRegisters; //FP registers

	ROB **reorderBuffer;
	
	int memoryAddress;	

	int intDestReg;
    int intResult;

    int fpDestReg;
    double fpResult;

    //New
    Dictionary *fetchBuffer;
    Dictionary *instructionQueue;
    Dictionary *branchTargetBuffer;
    //Pipelines
    CircularQueue *INTPipeline;
    CircularQueue *MULTPipeline;
    CircularQueue *LoadStorePipeline;
    CircularQueue *FPaddPipeline;
    CircularQueue *FPmultPipeline;
    CircularQueue *FPdivPipeline;
    int FPdivPipelineBusy;
    CircularQueue *BUPipeline;

} CPU;

#endif /* GLOBAL_DATATYPES_H_ */
