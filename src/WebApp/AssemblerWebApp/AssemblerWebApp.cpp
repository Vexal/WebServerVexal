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
	const string firstHalf = "<textarea spellcheck='false' rows = '20' cols = '20' name = 'assembledcode'>";
	const string secondHalf = "</textarea>";
	const string firstHalf2 = "<textarea spellcheck='false' rows = '20' cols = '40' name = 'comment'>";
	const string secondHalf2 = "</textarea>";
	string errorText = "";
	string assembledCode = "";

	const bool createStack = httpRequest.GetParameter("stack") == "createstack";
	const bool useSimulator = !httpRequest.GetParameter("stepcount").empty();
	
	string code = httpRequest.GetParameter("comment");
	const string username = httpRequest.GetParameter("username");

	CS350::ENDIANESS endianType = CS350::CS_ORIGINAL;

	const string endianness = httpRequest.GetParameter("endian");
	if (endianness == "littleendian")
	{
		endianType = CS350::CS_LITTLE_ENDIAN;
	}
	else if (endianness == "bigendian")
	{
		endianType = CS350::CS_BIG_ENDIAN;
	}
	else
	{

	}

	if (code.empty() && !createStack)
	{
		errorText += "missing code";
	}

	if (username.empty())
	{
		errorText += "missing user name\n";
	}


	unordered_map<string, string> pageParams;
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

		Assembler ourAssembler(code, endianType, createStack);
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

		pageParams["stepcount"] = stepCount;
		simulator.Run(stepCt);
		ProgramState endState = simulator.GetCurrentProgramState();


		const string firstHalf = "<pre cols='130' style='border-style:solid; height:500px; overflow-y:scroll; background-color:white;' name='simval'>";
		const string secondHalf = "</pre>";

		pageParams["simulator"] = firstHalf + simulator.GetOutput() + secondHalf;

		//create another textbox if there is expanded code
		if (ourAssembler.HasExpandedCode() && errorText == "")
		{
			const string firstHalf3 = "<textarea spellcheck='false' rows = '20' cols = '40' name = 'expandedcode' readonly>";
			const string secondHalf3 = "</textarea>";

			pageParams["expansion"] = firstHalf3 + "//Expanded code generated\n//Uses r5 for scratch arithmetic\n\n" + ourAssembler.GetAssemblyCode() + secondHalf3;
		}
	}

	pageParams["username"] = username;
	pageParams["code"] = code;

	if (errorText == "")
	{
		pageParams["results"] = firstHalf + assembledCode + secondHalf;
	}
	else
	{
		pageParams["results"] = firstHalf + errorText + secondHalf;
	}

	const Page* const constructedPage = PageConstructor::ConstructPage(assemblerPage, rootDirectory, httpRequest, pageParams);

	this->server->SendPage(constructedPage, clientSocket);
	delete constructedPage;
}