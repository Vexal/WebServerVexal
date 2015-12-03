#include "../Util.h"
#include "Captcha.h"

using namespace std;

Captcha::Captcha() :
	createTime(Util::EpochTimeMillis()),
	answer(generateAnswer()),
	answerHash(GenerateAnswerHash(this->answer, this->createTime))
{

}

bool Captcha::ValidateAnswer(const string& answerHash, const string& answer, const unsigned long long createTime)
{
	return answerHash == GenerateAnswerHash(answer, createTime);
}

string Captcha::generateAnswer()
{
	string result;
	result.resize(CAPTCHA_LENGTH);

	for (int a = 0; a < CAPTCHA_LENGTH; ++a)
	{
		result[a] = (rand() % 26) + 'a';
	}

	return result;
}

string Captcha::GenerateAnswerHash(const string& answer, const unsigned long long createTime)
{
	return GenerateAnswerHash(answer, to_string(createTime));
}

string Captcha::GenerateAnswerHash(const string& answer, const string& createTime)
{
	return to_string(hash<string>()(answer + createTime));
}

string Captcha::GenerateHtmlFormInput() const
{
	string html = "";
	html += "Please enter this captcha: " + this->answer;
	html += "<input type='text' size=7 name='captchaanswer'/>";
	html += "<input type='hidden' name='captchatime' value='"; html += to_string(this->createTime); html += "'/>";	
	html += "<input type='hidden' name='captchahash' value='"; html += this->answerHash; html += "'/>";
	return html;
}
