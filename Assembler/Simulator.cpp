#include "Simulator.h"
#include "CS350.h"

using namespace std;

namespace CS350
{
	Simulator::Simulator(const vector<unsigned short>& programMemory, const vector<DataType>& isDataOrCodeLines) :
		memory(programMemory)
	{
		this->memory.resize(MEMORY_DEPTH);
		this->programStates.push_back(ProgramState(this->memory));
	}

	void Simulator::Run(int stepCount)
	{
		for (int a = 0; a < stepCount; ++a)
		{
			this->programStates.push_back(this->StepProgram(this->GetCurrentProgramState()));
		}
	}

	ProgramState Simulator::StepProgram(const ProgramState& previousState) const
	{
		if (previousState.GetProgramCounter() >= MEMORY_DEPTH)
		{
			return previousState;
		}

		const vector<unsigned short>& previousMemory = previousState.GetMemory();
		const unsigned short currentPC = previousState.GetProgramCounter();
		short nextPC = previousState.GetProgramCounter() + 1;

		const unsigned short nextInstruction = previousMemory[previousState.GetProgramCounter()];

		const unsigned char opcode = (nextInstruction & 0xF000) >> 12;
		const unsigned int format = (nextInstruction & 0x0800) >> 11;
		const unsigned int rdIndex = (nextInstruction & 0x0700) >> 8;
		const unsigned int raIndex = (nextInstruction & 0x00E0) >> 5;
		const unsigned int rbIndex = (nextInstruction & 0x0007);
		const short s1 = (const short) nextInstruction & 0x00FF;
		const short s2 = s1 << 8;
		const short imm8 = s2 >> 8; // maybe gets signed value.

		const short s51 = (const short) nextInstruction & 0x001F;
		const short s52 = s51 << 11;
		const short imm5 = s52 >> 11; // maybe gets signed value.

		//const short imm5 = (((const short)nextInstruction & 0x001F) << 11) >> 11;  // why doesn't this work.
		//const short imm8 = (((const short)nextInstruction & 0x00FF) << 8) >> 8;

		const short va = previousState.GetRegisterFile().registers[raIndex];
		const short vb = previousState.GetRegisterFile().registers[rbIndex];
		const short vd = previousState.GetRegisterFile().registers[rdIndex];

		short result = 0;
		const unsigned int nextExecutionCount = previousState.GetExecutionCount() + 1;
		vector<unsigned short> newMemory = previousMemory;

		RegisterFile newRegisterState = previousState.GetRegisterFile();
		newRegisterState.ResetChanges();

		switch (opcode)
		{
		case ADD:
			if (rdIndex == 7) break;
			newRegisterState.registers[rdIndex] = (format == 1) ? va + vb : va + imm5;
			newRegisterState.changed[rdIndex] = true;
			break;
		case SUB:
			if (rdIndex == 7) break;
			newRegisterState.registers[rdIndex] = (format == 1) ? va - vb : va - imm5;
			newRegisterState.changed[rdIndex] = true;
			break;
		case SLT:
			if (rdIndex == 7) break;
			newRegisterState.registers[rdIndex] = (format == 0) ? (va < imm5 ? 1 : 0) : (va < vb ? 1 : 0);
			newRegisterState.changed[rdIndex] = true;
			break;
		case SHL:
			if (rdIndex == 7) break;
			newRegisterState.registers[rdIndex] = (format == 0) ? va << imm5 : 0;
			newRegisterState.changed[rdIndex] = true;
			break;
		case LD:
		{
			if (rdIndex == 7) break;
			const unsigned short addr = format == 0 ? (va + imm5) : (nextPC + imm8);
			newRegisterState.registers[rdIndex] = addr < previousMemory.size() ? previousMemory[addr] : 0;
			newRegisterState.changed[rdIndex] = true;
		}
			break;
		case ST:
		{
			const unsigned short addr = format == 0 ? (va + imm5) : (nextPC + imm8);
			if (addr < MEMORY_DEPTH)
			{
				newMemory[addr] = vd;
			}
		}
			break;
		case LEA:
			if (rdIndex == 7) break;
			newRegisterState.registers[rdIndex] = (format == 0) ? va + imm5 : nextPC + imm8;
			newRegisterState.changed[rdIndex] = true;
			break;
		case CALL:
			if (rdIndex != 7)
			{
				newRegisterState.registers[rdIndex] = nextPC;
				newRegisterState.changed[rdIndex] = true;
			}
			nextPC = (format == 0) ? va + imm5 : nextPC + imm8;
			break;
		case BRZ:
			nextPC = vd == 0 ? ((format == 0) ? va + imm5 : nextPC + imm8) : nextPC;
			break;
		default:
			break;
		}

		return ProgramState(nextPC, nextExecutionCount, newRegisterState, newMemory, nextInstruction, currentPC);		
	}

	string Simulator::GetOutput() const
	{
		stringstream outstream;
		for (int a = 0; a < this->GetExecutionCount(); ++a)
		{
			outstream << std::setfill(' ') << std::setw(4) << (to_string((int)this->programStates[a].GetExecutionCount() - 1) + ")") <<
				"  <b>PC</b>:" << std::setfill(' ') << std::setw(4) << to_string(this->programStates[a].GetPrintedProgramCounter()) <<
				" <b>Inst</b>: 0x" << std::setfill('0') << std::setw(4) << std::hex << this->programStates[a].GetInstruction() <<
				" " << this->programStates[a].GetRegisterFile().to_string() << "\n";
		}

		return outstream.str();
	}
}