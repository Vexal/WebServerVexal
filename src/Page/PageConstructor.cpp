#include "PageConstructor.h"
#include <string>
#include <algorithm>
#include "../WebApp/WebPageApp/WebPageApp.h"
#include "../HttpServer/HttpUtils.h"
#include "Page.h"
#include "Folder.h"

using namespace std;

Page const* PageConstructor::ConstructPage(const Page* const page,
	const Folder* const dir,
	const HttpRequest& request,
	const unordered_map<string, string>& params,
	const unordered_map<string, function<string(string const&)> > paramFuncs)
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
				const Page* const constructedChild = ConstructPage(static_cast<Page*>(dir->GetPage(operationTarget)), dir, request, params, paramFuncs);
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
			else if (operation == "OPT_REQUEST_PARAM_ENCODE" ||
				operation == "OPT_REQUEST_PARAM" ||
				operation == "REQUEST_PARAM_ENCODE" ||
				operation == "REQUEST_PARAM")
			{
				const auto param = request.paramMap.find(operationTarget);
				if (param != request.paramMap.end())
				{
					if (operation == "REQUEST_PARAM_ENCODE" || operation == "OPT_REQUEST_PARAM_ENCODE")
					{
						finalPage += HttpUtils::urlDecode(param->second);
					}
					else
					{
						finalPage += param->second;
					}
				}
				else if (operation != "OPT_REQUEST_PARAM" && operation != "OPT_REQUEST_PARAM_ENCODE")
				{
					finalPage += "{ERROR}";
				}
			}
			else if (operation == "PARAM" ||
				operation == "OPT_PARAM" ||
				operation == "PARAM_ENCODE" ||
				operation == "OPT_PARAM_ENCODE")
			{
				const auto param = params.find(operationTarget);
				if (param != params.end())
				{
					if (operation == "PARAM_ENCODE" || operation == "OPT_PARAM_ENCODE")
					{
						finalPage += param->second;//HttpUtils::urlEncode(param->second);
					}
					else
					{
						finalPage += param->second;
					}
				}
				else
				{
					const auto funcParam = paramFuncs.find(operationTarget);
					if (funcParam != paramFuncs.end())
					{
						finalPage += funcParam->second(operationTarget);
					}
					else if (operation != "OPT_PARAM" && operation != "OPT_PARAM_ENCODE")
					{
						finalPage += "{ERROR}";
					}
				}
			}
			currentPosition += 1; // }
		}
	}

	//caller is responsible for freeing memory used by constructed page.
	return new const Page(page->GetFullPath(), page->GetName(), finalPage);
}
