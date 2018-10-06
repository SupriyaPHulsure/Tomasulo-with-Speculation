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

typedef enum _unit {
    INT,
    MULT,
    LS,
    FPadd,
    FPmult,
    FPdiv,
    BU
} Unit;

static inline char *getOpcodeString (int opcode) {
	char *opcodeString[23] = {"AND", "ANDI", "OR", "ORI", "SLT", "SLTI", "DADD", "DADDI", "DSUB", "DMUL", "LD", "SD", "L_D", "S_D", "ADD_D", "SUB_D", "MUL_D", "DIV_D", "BEQZ", "BNEZ", "BEQ", "BNE", "NOOP"};

	return opcodeString[opcode];
}

//data structure for decoded instruction
typedef struct _instruction {
	OpCode op;
	int address; //Instruction address (PC)

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
    int ROB_number;
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
    int ROB_number;
	Instruction *instruction;
	int DestReg;
	int DestValueInt; //Value to be written
	float DestValueFP;
	int isReady;
	int isINT;
	int isStore;
	int DestAddr; // optional, memory address to write
}ROB;

//Data structure for reservation stations
typedef struct _RSint{
	Instruction *instruction;
	int Vj; //value of input register j
	int Vk; //value of input register k
	int Qj; //ROB number of input register j
	int Qk;  //ROB number of input register k
	int Dest; //ROB number of destination register
	int isReady;
}RSint;

typedef struct _RSfloat{
	Instruction *instruction;
	float Vj; //value of input register j
	float Vk; //value of input register k
	int Qj; //ROB number of input register j
	int Qk;  //ROB number of input register k
	int Dest; //ROB number of destination register
	int isReady;
}RSfloat;

//Data Structure for renaming register
typedef struct _RenameReg{
    int reorderNum;
    int regNum;
    int busy;
}RenameReg;

//main data structure representing CPU
typedef struct _cpu {
	int cycle; //counter for number of cycles

	int numberOfInstructionExecuted; //counter for total number of instructions committed

	int PC; //program counter

	INTReg **integerRegisters; //integer register
    FPReg **floatingPointRegisters; //FP registers

	int memoryAddress;	

	int intDestReg;
    int intResult;

    int fpDestReg;
    double fpResult;

    //Fetch and decode
    Dictionary *fetchBuffer;
    Dictionary *fetchBufferResult;
    CircularQueue *instructionQueue;
    CircularQueue *instructionQueueResult;
    Dictionary *branchTargetBuffer;
    //Reorder buffer
    CircularQueue *reorderBuffer;
    //Reservation station
    Dictionary *resStaInt;
    Dictionary *resStaMult;
    Dictionary *resStaLoad;
    Dictionary *resStaStore;
    Dictionary *resStaFPadd;
    Dictionary *resStaFPmult;
    Dictionary *resStaFPdiv;
    Dictionary *resStaBU;
    //Renaming registers
    CircularQueue *renameRegInt;
    CircularQueue *renameRegFP;
    //Pipelines
    CircularQueue *INTPipeline;
    CircularQueue *MULTPipeline;
    CircularQueue *LoadStorePipeline;
    CircularQueue *FPaddPipeline;
    CircularQueue *FPmultPipeline;
    CircularQueue *FPdivPipeline;
    int FPdivPipelineBusy;
    CircularQueue *BUPipeline;
    //Install flag
    int stallNextFetch;
    //Stall counter
    stallFullROB;

} CPU;

#endif /* GLOBAL_DATATYPES_H_ */
