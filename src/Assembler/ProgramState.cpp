#include "ProgramState.h"

using namespace std;

namespace CS350
{
	ProgramState::ProgramState(const int programCounter, const int executionCount, const RegisterFile& registerFile, const vector<unsigned short>& memory, const unsigned short instruction, const unsigned short printedProgramCounter) :
		programCounter(programCounter), executionCount(executionCount), registerFile(registerFile), memory(memory), instruction(instruction), printedProgramCounter(printedProgramCounter)
	{

	}

	ProgramState::ProgramState(const vector<unsigned short>& memory) :
		memory(memory), instruction(memory[0])
	{

	}

	bool ProgramState::operator==(const ProgramState& rhs) const
	{
		return this->registerFile == rhs.registerFile &&
			this->programCounter == rhs.programCounter;
	}
}