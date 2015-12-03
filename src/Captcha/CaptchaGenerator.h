#pragma once
#include <unordered_set>
#include <mutex>
#include "Captcha.h"

class CaptchaGenerator
{
private:
	static const unsigned long long MAX_CAPTCHA_AGE = 8 * 60 * 60 * 1000; //8 hours
	//keep track of recently accepted captcha hashes to prevent user from reusing correct captcha
	std::unordered_set<std::string> invalidatedAnswerHashes;
	std::mutex captchaHashMutex;

private:
	CaptchaGenerator();

public:
	Captcha GenerateCaptcha() const;
	//has side effect of invalidating for future use the hash, if accepted as correct answer
	bool ValidateAnswer(const std::string& answerHash, const std::string& answer, const std::string& createTime);
	//invalidate the hash of this captcha so that it cannot be used again
	void InvalidateCaptcha(const Captcha& captcha);

public:
	//return singleton
	static CaptchaGenerator* Create();
};