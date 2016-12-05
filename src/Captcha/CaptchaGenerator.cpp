#include "CaptchaGenerator.h"
#include "../Util.h"

using namespace std;

CaptchaGenerator::CaptchaGenerator()
{
}

bool CaptchaGenerator::ValidateAnswer(const string& answerHash, const string& answer, const string& createTime)
{
	if (stoull(createTime) <  Util::EpochTimeMillis() - MAX_CAPTCHA_AGE)
	{
		return false;
	}

	const string givenAnswerHash = Captcha::GenerateAnswerHash(answer, createTime);
	if (answerHash != givenAnswerHash)
	{
		return false;
	}

	bool isValid = false;
	this->captchaHashMutex.lock();
	if (this->invalidatedAnswerHashes.find(answerHash) == this->invalidatedAnswerHashes.end())
	{
		isValid = true;
		this->invalidatedAnswerHashes.insert(answerHash);
	}

	this->captchaHashMutex.unlock();
	return isValid;
}

Captcha CaptchaGenerator::GenerateCaptcha() const
{
	return Captcha();
}

void CaptchaGenerator::InvalidateCaptcha(const Captcha & captcha)
{
}

CaptchaGenerator* CaptchaGenerator::Create()
{
	static CaptchaGenerator singletonCaptchaGenerator;
	return &singletonCaptchaGenerator;
}
