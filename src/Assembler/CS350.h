#pragma once

namespace CS350
{
	enum OPCODES
	{
		ADD,
		SUB,
		SLT,
		SLTU,
		AND,
		OR,
		XOR,
		SWI,
		SHL,
		SHR,
		LD,
		ST,
		LEA,
		CALL,
		BRNZ,
		BRZ
	};

	enum ENDIANESS
	{
		CS_ORIGINAL,
		CS_LITTLE_ENDIAN,
		CS_BIG_ENDIAN
	};
}

enum DataType
{
	CODE,
	DATA
};

enum InstructionType
{
	A,
	B,
	C
};

enum SpecialOpcodes
{
	callFunc,
	ret,
	push,
	pop,
};

#define MEMORY_DEPTH 1024
