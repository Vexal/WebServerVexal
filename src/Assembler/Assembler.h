#pragma once
#include <string>
#include <map>
#include <vector>
#include "CS350.h"

struct Instruction
{
	int opcode;
	InstructionType type;
	int format;
	int rd;
	int ra;
	int rb;
	int imm5;
	int imm8;
	bool invalid;
	short data;
	int skipAmount;
};

class Assembler
{
private:
	static std::map<std::string, int> opcodes;
	static std::map<std::string, SpecialOpcodes> specialInstructions;
	static std::map<std::string, int> registers;
	std::map<std::string, int> labels;
	bool isParsingData;
	const bool createStack;
	std::string errorText;
	std::string assemblyCode;
	std::vector<unsigned short> machineCode;
	std::vector<DataType> dataType;

	bool hasExpandedCode;
	CS350::ENDIANESS endianess;

public:
	Assembler(const std::string& assemblyCode, CS350::ENDIANESS endianess, bool createStack = false) :
		isParsingData(false), createStack(createStack), assemblyCode(assemblyCode), hasExpandedCode(false), endianess(endianess) {}

	std::string Assemble();
	static std::string GenerateStackCode();

	std::string GetErrorText() const { return this->errorText; }
	std::string GetAssemblyCode() const { return this->assemblyCode; }
	std::vector<unsigned short> GetMachineCode() const { return this->machineCode; }
	std::vector<DataType> GetLineDataType() const { return this->dataType; }
	bool HasExpandedCode() const { return this->hasExpandedCode; }

private:
	void prepareInput();
	bool findLabels(std::string input);
	void expandSpecialInstructions();
	std::string expandSpecialInstruction(const std::string& instruction);
	int readImmediate(const std::string& token, int lineNumber) const;
	Instruction parseLine(const std::string& line, int currentLine);

	static std::string formatOutput(std::string s);
	static std::string createAssembledLine(const Instruction& instruction);
	static bool isCommented(const std::string& s);
	//returns -1 if the token is not a register
	static int readRegister(const std::string& token);
};