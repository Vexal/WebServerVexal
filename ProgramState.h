#pragma once
#define REGISTER_COUNT 8
#include "RegisterFile.h"
#include <vector>

namespace CS350
{
	class ProgramState
	{
	private:
		const RegisterFile registerFile;
		const unsigned short programCounter;
		const unsigned short printedProgramCounter; // not purely functional, but it's necessary to print the output correctly
		const unsigned int executionCount; //how many instructions have been executed before this state
		const std::vector<unsigned short> memory;
		const unsigned short instruction;

	public:
		ProgramState(const int programCounter, const int executionCount, const RegisterFile& registerFile, const std::vector<unsigned short>& memory, const unsigned short instruction, const unsigned short printedProgramCounter);
		ProgramState(const std::vector<unsigned short>& memory);

		bool operator==(const ProgramState& rhs) const;
		const RegisterFile& GetRegisterFile() const { return this->registerFile; }
		unsigned short GetProgramCounter() const { return this->programCounter; }
		unsigned int GetExecutionCount() const { return this->executionCount; }
		const std::vector<unsigned short>& GetMemory() const { return this->memory; }
		const unsigned short GetInstruction() const { return this->instruction; }
		const unsigned short GetPrintedProgramCounter() const { return this->printedProgramCounter; }
	};
}