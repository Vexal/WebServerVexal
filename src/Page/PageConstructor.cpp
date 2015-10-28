#include "PageConstructor.h"
#include <string>
#include <algorithm>
#include "Page.h"
#include "../WebApp/WebPageApp/WebPageApp.h"
#include "Folder.h"
using namespace std;

Page const* PageConstructor::ConstructPage(const Page* const page, const Folder* const dir)
{
	string pageContent(page->GetContent());

	auto dollarSignPosition = pageContent.find('$', 0);

	while (dollarSignPosition != string::npos)
	{
		const auto foundPosition = dollarSignPosition;
		const auto delimeterPosition = pageContent.find("=", dollarSignPosition + 1);
		const string operation = pageContent.substr(dollarSignPosition + 2, delimeterPosition - dollarSignPosition - 2);
		const auto eraseEnd = pageContent.find("}", foundPosition);
		const auto quotePosition = pageContent.find("\"", delimeterPosition + 2);
		const string operationTarget = pageContent.substr(delimeterPosition + 2, quotePosition - delimeterPosition - 2);

		if (operation == "LOAD")
		{
			const string data = static_cast<Page*>(dir->GetPage(operationTarget))->GetContent();
			pageContent.insert(foundPosition + eraseEnd - foundPosition + 1, data);
		}
		else if (operation == "THIS")
		{
			string pa = page->GetFullPath();
			transform(pa.begin(), pa.end(), pa.begin(), ::tolower);
			const auto vc = WebPageApp::viewCounts.find(pa);
			const int viewCount = vc != WebPageApp::viewCounts.end() ? vc->second : 0;
			pageContent.insert(foundPosition + eraseEnd - foundPosition + 1, to_string(viewCount));
		}

		pageContent.erase(foundPosition, eraseEnd - foundPosition + 1);
		dollarSignPosition = pageContent.find('$', delimeterPosition + 1);
	}

	return new const Page(page->GetFullPath(), page->GetName(), pageContent);
}
