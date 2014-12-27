#pragma once
#include <vector>
#include "ProgramState.h"
#include <string>
#include "CS350.h"

namespace CS350
{
	class Simulator
	{
	private:
		std::vector<ProgramState> programStates;
		std::vector<unsigned short> memory;

	public:
		Simulator(const std::vector<unsigned short>& programMemory, const std::vector<DataType>& isDataOrCodeLines);
		ProgramState StepProgram(const ProgramState& previousState) const;
		void Run(int stepCount = 1);

		std::string GetOutput() const;
		unsigned int GetExecutionCount() const { return this->programStates.size(); }
		const ProgramState GetCurrentProgramState() const { return this->GetExecutionCount() > 0 ? this->programStates[this->GetExecutionCount() - 1] : ProgramState(this->memory); }
	private:
	};
}