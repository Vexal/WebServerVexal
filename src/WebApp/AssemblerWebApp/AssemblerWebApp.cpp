#include <iostream>
#include "AssemblerWebApp.h"
#include "../../HttpServer/HttpServer.h"
#include "../../Assembler/Assembler.h"
#include "../../Assembler/Simulator.h"
#include "../../Page/Folder.h"
#include "../../Page/Page.h"
#include "../../HttpServer/HttpUtils.h"
#include "../../HttpServer/HttpRequest.h"
#include "../../Page/PageConstructor.h"

using namespace std;
using namespace CS350;

AssemblerWebApp::AssemblerWebApp(HttpServer* server, const Folder* const rootDirectory) :
	WebApp("compile", server),
	rootDirectory(rootDirectory),
	assemblerPage(static_cast<const Page* const>(rootDirectory->GetPage("/Projects/Assembler/Assembler.html")))
{
}

void AssemblerWebApp::HandleRequest(SOCKET clientSocket, const HttpRequest& httpRequest)
{
	vector<string> replaceTokens;
	vector<string> replaceTexts;

	const string firstHalf = "<textarea spellcheck='false' rows = '20' cols = '20' name = 'assembledcode'>";
	const string secondHalf = "</textarea>";
	const string firstHalf2 = "<textarea spellcheck='false' rows = '20' cols = '40' name = 'comment'>";
	const string secondHalf2 = "</textarea>";
	string errorText = "";
	string assembledCode = "";

	const bool useHProtection = httpRequest.GetParameter("hprot") == "on";
	const bool createStack = httpRequest.GetParameter("stack") == "createstack";
	const bool useSimulator = !httpRequest.GetParameter("stepcount").empty();
	
	string code = httpRequest.GetParameter("comment");
	const string username = httpRequest.GetParameter("username");

	CS350::ENDIANESS endianType = CS350::CS_ORIGINAL;

	const string endianness = httpRequest.GetParameter("endian");
	if (endianness == "littleendian")
	{
		endianType = CS350::CS_LITTLE_ENDIAN;
		replaceTokens.push_back("~little");
		replaceTexts.push_back(" checked");
	}
	else if (endianness == "bigendian")
	{
		endianType = CS350::CS_BIG_ENDIAN;
		replaceTokens.push_back("~big");
		replaceTexts.push_back(" checked");
	}
	else if (endianness == "indian")
	{
		endianType = CS350::CS_INDIAN;
		replaceTokens.push_back("~indian");
		replaceTexts.push_back(" checked");
	}
	else
	{
		replaceTokens.push_back("~16b");
		replaceTexts.push_back(" checked");
	}

	if (code.empty() && !createStack)
	{
		errorText += "missing code";
	}

	if (username.empty())
	{
		errorText += "missing user name\n";
	}

	if (errorText.empty())
	{
		if (createStack)
		{
			code = Assembler::GenerateStackCode();
		}
		else
		{
			code = HttpUtils::urlDecode(code);
		}

		Assembler ourAssembler(code, endianType, useHProtection, createStack);
		assembledCode = ourAssembler.Assemble();
		errorText += ourAssembler.GetErrorText();
		Simulator simulator(ourAssembler.GetMachineCode(), ourAssembler.GetLineDataType());
		size_t stepCt = ourAssembler.GetMachineCode().size();
		string stepCount = to_string(stepCt);

		if (useSimulator)
		{
			const string stepCount2 = httpRequest.GetParameter("stepcount");
			stringstream sp(stepCount2);
			unsigned int sentStep = 0;
			sp >> sentStep;
			if (sentStep > stepCt && sentStep < 600)
			{
				stepCt = sentStep;
				stepCount = stepCount2;
			}
		}
	
		simulator.Run(stepCt);
		ProgramState endState = simulator.GetCurrentProgramState();
		replaceTokens.push_back("~sim");


		const string firstHalf = "<pre cols='130' style='border-style:solid; height:500px; overflow-y:scroll; background-color:white;' name='simval'>";
		const string secondHalf = "</pre>";

		replaceTexts.push_back(firstHalf + simulator.GetOutput() + secondHalf);

		replaceTokens.push_back("name='stepcount' value='");
		replaceTexts.push_back(stepCount);

		//create another textbox if there is expanded code
		if (ourAssembler.HasExpandedCode() && errorText == "")
		{
			const string firstHalf3 = "<textarea spellcheck='false' rows = '20' cols = '40' name = 'expandedcode' readonly>";
			const string secondHalf3 = "</textarea>";


			replaceTokens.push_back("~exp");
			replaceTexts.push_back(firstHalf3 + "//Expanded code generated\n//Uses r5 for scratch arithmetic\n\n" + ourAssembler.GetAssemblyCode() + secondHalf3);
		}
	}

	replaceTokens.push_back("name='username' value='");
	replaceTexts.push_back(username + "'");

	replaceTokens.push_back("form='usrform'>");
	replaceTexts.push_back(code);

	replaceTokens.push_back("~ins");

	if (errorText == "")
	{
		replaceTexts.push_back(firstHalf + assembledCode + secondHalf);
	}
	else
	{
		replaceTexts.push_back(firstHalf + errorText + secondHalf);
	}

	if (useHProtection)
	{
		replaceTokens.push_back("hprot'");
		replaceTexts.push_back(" checked");
	}

	const Page* const newPage = assemblerPage->ClonePage(replaceTokens, replaceTexts);
	const Page* const constructedPage = PageConstructor::ConstructPage(newPage, rootDirectory);

	delete newPage;

	this->server->SendPage(constructedPage, clientSocket);
	delete constructedPage;
}