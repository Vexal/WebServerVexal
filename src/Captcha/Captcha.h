#pragma once

class Captcha
{
private:
	static const unsigned int CAPTCHA_LENGTH = 5;

	const unsigned long long createTime; //milliseconds since epoch
	const std::string answer;
	const std::string answerHash;

public:
	Captcha();
	std::string GenerateHtmlFormInput() const;

public:
	static bool ValidateAnswer(const std::string& answerHash, const std::string& answer, const unsigned long long createTime);
	//generate hash based on time and answer
	static std::string GenerateAnswerHash(const std::string& answer, const unsigned long long createTime);
	static std::string GenerateAnswerHash(const std::string& answer, const std::string& createTime);
	
private:
	static std::string generateAnswer();
};