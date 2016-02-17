#include "../Util.h"
#include "Captcha.h"
#include <fstream>
#include <iostream>
#ifndef _WIN32
#include <cstring>
#endif

using namespace std;
extern bool printEverything;

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

	//generate a random number between 1 and 6.
	for (int a = 0; a < CAPTCHA_LENGTH; ++a)
	{
		result[a] = (rand() % 6) + '1';
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
	static const int CAPTCHA_IMAGE_SIZE = 702;
	static char CAPTCHA_TEMPLATE[CAPTCHA_IMAGE_SIZE];
	static const bool hasLoadedCaptchaImageTemplate = []() {
		//load a plain white .BMP image file as captcha template so that
		//I do not have to learn how the BMP header works.
		//captchas are generated by copying this template and modifying the bytes to change the colors
		std::ifstream myFile("captchatemplate.bmp", std::ios::in | std::ios::binary | std::ios::ate);

		const size_t len = myFile.tellg();
		myFile.seekg(0, std::ios::beg);
		myFile.read(CAPTCHA_TEMPLATE, len);
		myFile.close();
		return true;
	}();

	char captchaImage[CAPTCHA_IMAGE_SIZE];
	memcpy(captchaImage, CAPTCHA_TEMPLATE, CAPTCHA_IMAGE_SIZE);
	const unsigned long long startTime = Util::EpochTimeMillis();

	//current captcha generator is simple -- generate columns of lines
	//which the user specifies the number of lines per column as the captcha.
	for (int a = 0; a < CAPTCHA_LENGTH; ++a)
	{
		const int offset = 64 + a * 32 / 8;
		const char height = this->answer[a] - '1' + 1;
		for (int b = 0; b < height; ++b)
		{
			captchaImage[offset + 160 * (b * 5) / 8] = 0;
			captchaImage[offset + 160 * (b * 5) / 8 + 1] = 0;
			captchaImage[offset + 160 * (b * 5 + 1) / 8] = 0;
			captchaImage[offset + 160 * (b * 5 + 1) / 8 + 1] = 0;
		}
	}

	//convert captcha image to base 64 string to display inline in html
	const string base64EncodedImage = Util::GetBase64String((const unsigned char* const)captchaImage, CAPTCHA_IMAGE_SIZE);

	string html = "";
	html.reserve(500);
	html += "<div class='captchaarea'>";
	html += "Captcha: ";
	html += "<input class='captchaanswer' type='text' size=7 name='captchaanswer' placeholder='captcha'/>";
	html += "Please enter the digits corresponding to the number of lines in each of the 5 columns in the image(ex: 31735) ";
	html += "<br/>";
	html += "<img class='captchaimage' src='data:image/bmp;base64,"; html += base64EncodedImage; html += "' width='200'/>";
	html += "</div>";
	html += "<input type='hidden' name='captchatime' value='"; html += to_string(this->createTime); html += "'/>";	
	html += "<input type='hidden' name='captchahash' value='"; html += this->answerHash; html += "'/>";

	const unsigned long long tt = Util::EpochTimeMillis() - startTime;
	if (printEverything)
		cout << "Captcha generation time " << tt << endl;
	return html;
}
