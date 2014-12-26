#pragma once
#include <string>
#include <sstream>
#include <iomanip>

namespace CS350
{
	struct RegisterFile
	{
		short registers[8];
		bool changed[8];
		RegisterFile()
		{
			for (int a = 0; a < 8; ++a)
			{
				this->registers[a] = 0;
				this->changed[a] = false;
			}
		}

		RegisterFile(const RegisterFile& rhs)
		{
			for (int a = 0; a < 8; ++a)
			{
				this->registers[a] = rhs.registers[a];
				this->changed[a] = rhs.changed[a];
			}
		}

		void operator=(const RegisterFile& rhs)
		{
			this->registers[0] = rhs.registers[0];
			this->registers[1] = rhs.registers[1];
			this->registers[2] = rhs.registers[2];
			this->registers[3] = rhs.registers[3];
			this->registers[4] = rhs.registers[4];
			this->registers[5] = rhs.registers[5];
			this->registers[6] = rhs.registers[6];
			this->registers[7] = rhs.registers[7];

			for (int a = 0; a < 8; ++a)
			{
			//	this->changed[a] = false;
			}
		}

		bool operator==(const RegisterFile& rhs) const
		{
			return this->registers[0] == rhs.registers[0] &&
				this->registers[1] == rhs.registers[1] &&
				this->registers[2] == rhs.registers[2] &&
				this->registers[3] == rhs.registers[3] &&
				this->registers[4] == rhs.registers[4] &&
				this->registers[5] == rhs.registers[5] &&
				this->registers[6] == rhs.registers[6] &&
				this->registers[7] == rhs.registers[7];
		}

		void ResetChanges()
		{
			for (int a = 0; a < 8; ++a)
			{
				this->changed[a] = false;
			}
		}

		std::string to_string() const
		{
			std::stringstream outstream;
			const std::string beginRed = "<span style='color:red; border-style:dotted; border-width:1px; border-color:red;'><b>";
			const std::string beginNothing = "<span style='border-style:solid; border-width:1px; border-color:white'>";
			const std::string endRed = "</b></span>";
			const std::string endNothing = "</span>";
			if (this->changed[0])
			{
				int breakHere = 0;
			}
			outstream << (this->changed[0] ? beginRed : beginNothing) << "[<b>r0</b>: " << "0x" << std::hex << std::setfill('0') << std::setw(4) << this->registers[0] << (this->changed[0] ? endRed : endNothing) << ", " <<
				(this->changed[1] ? beginRed : beginNothing) << "<b>r1</b>: " << "0x" << std::setfill('0') << std::setw(4) << this->registers[1] << (this->changed[1] ? endRed : endNothing) << ", " <<
				(this->changed[2] ? beginRed : beginNothing) << "<b>r2</b>: " << "0x" << std::setfill('0') << std::setw(4) << this->registers[2] << (this->changed[2] ? endRed : endNothing) << ", " <<
				(this->changed[3] ? beginRed : beginNothing) << "<b>r3</b>: " << "0x" << std::setfill('0') << std::setw(4) << this->registers[3] << (this->changed[3] ? endRed : endNothing) << ", " <<
				(this->changed[4] ? beginRed : beginNothing) << "<b>r4</b>: " << "0x" << std::setfill('0') << std::setw(4) << this->registers[4] << (this->changed[4] ? endRed : endNothing) << ", " <<
				(this->changed[5] ? beginRed : beginNothing) << "<b>r5</b>: " << "0x" << std::setfill('0') << std::setw(4) << this->registers[5] << (this->changed[5] ? endRed : endNothing) << ", " <<
				(this->changed[6] ? beginRed : beginNothing) << "<b>r6</b>: " << "0x" << std::setfill('0') << std::setw(4) << this->registers[6] << (this->changed[6] ? endRed : endNothing) << ", " <<
				(this->changed[7] ? beginRed : beginNothing) << "<b>r7</b>: " << "0x" << std::setfill('0') << std::setw(4) << this->registers[7] << (this->changed[7] ? endRed : endNothing) << "]";

			return outstream.str();
		}
	};
}