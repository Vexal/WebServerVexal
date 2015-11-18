#include "PageConstructor.h"
#include <string>
#include <algorithm>
#include "Page.h"
#include "../WebApp/WebPageApp/WebPageApp.h"
#include "Folder.h"
using namespace std;

Page const* PageConstructor::ConstructPage(const Page* const page, const Folder* const dir, const unordered_map<string, string> params)
{
	const char* const pageContent = page->GetContent();
	string finalPage;
	finalPage.reserve(page->GetContentLength());

	for (int currentPosition = 0; currentPosition < page->GetContentLength(); ++currentPosition)
	{
		if (pageContent[currentPosition] != '$')
		{
			finalPage += pageContent[currentPosition];
		}
		else
		{
			currentPosition += 2; //skip $ and {
			string operation;
			while (pageContent[currentPosition] != '=')
			{
				operation += pageContent[currentPosition];
				++currentPosition;
			}

			currentPosition += 2; //skip " and =

			string operationTarget;
			while (pageContent[currentPosition] != '\"')
			{
				operationTarget += pageContent[currentPosition];
				++currentPosition;
			}

			if (operation == "LOAD")
			{
				const Page* const constructedChild = ConstructPage(static_cast<Page*>(dir->GetPage(operationTarget)), dir, params);
				finalPage += string(constructedChild->GetContent()); // is it okay to do this recursively.
				delete constructedChild;
			}
			else if (operation == "THIS")
			{
				string pa = page->GetFullPath();
				transform(pa.begin(), pa.end(), pa.begin(), ::tolower);
				const auto vc = WebPageApp::viewCounts.find(pa);
				const int viewCount = vc != WebPageApp::viewCounts.end() ? vc->second : 0;
				finalPage += to_string(viewCount);
			}
			else if (operation == "PARAM")
			{
				const auto param = params.find(operationTarget);
				if (param != params.end())
				{
					finalPage += param->second;
				}
				else
				{
					finalPage += "{ERROR}";
				}
			}

			currentPosition += 2; //skip " and }
		}
	}

	//caller is responsible for freeing memory used by constructed page.
	return new const Page(page->GetFullPath(), page->GetName(), finalPage);
}
