#pragma once
#include "WebNode.h"
#include <map>
#include <vector>

enum PageType
{
	HTML,
	PNG,
	JPG,
	GIF,
	ICO,
	CSS,
	WAV,
	MP3
};

enum ContentType
{
	TEXT,
	IMAGE,
	SOUND,
	ERROR400
};

struct PageCompare2
{
	bool operator() (const std::string& lhs, const std::string& rhs) const
	{
		return lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
	}
};

class Page : public WebNode
{
private:
	char* content;
	size_t contentLength;
	bool valid = false;
	PageType pageType;
	ContentType contentType;
	int viewCount = 0;
	std::map<std::string, int, PageCompare2> pages;

public:
	Page(const std::string& filePath, const std::string& name);
	Page(const std::string& filePath, const std::string& name, const std::string& text);
	~Page();

	char* GetContent() const {return this->content;}
	size_t GetContentLength() const {return this->contentLength;}
	bool IsValid() const {return this->valid;}
	int GetViewCount() const {return this->viewCount;}
	void IncrementViewCount() {++this->viewCount;}
	void IncrementViewCount(const std::string& referer);
	ContentType GetContentType() const {return this->contentType;}
	PageType GetPageType() const {return this->pageType;}

	//clone page with new text inserted somewhere!
	Page* ClonePage(const std::string& replaceToken, const std::string& replaceText, const std::string& replaceToken2, const std::string& replaceText2) const;
	Page* ClonePage(const std::string& replaceToken, const std::string& replaceText) const;
	Page* Page::ClonePage(const std::vector<std::string>& replaceTokens, const std::vector<std::string>& replaceTexts) const;

private:
	void loadContent();
};