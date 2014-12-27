#include "AssemblerWebApp.h"
#include "WebPageApp.h"
#include "Server.h"
#include "Assembler/Assembler.h"
#include "Assembler/Simulator.h"
#include "Folder.h"
#include "Page.h"
#include <iostream>

using namespace std;
using namespace CS350;

AssemblerWebApp::AssemblerWebApp(Server* server, const WebPageApp* const serverWebPageApp) :
	WebApp("compile", server),
	serverWebPageApp(serverWebPageApp)
{
}

void AssemblerWebApp::HandleRequest(const string & request, ContentHost* contentHost, int clientSocket)
{
	string file = request;
	string rest = file;

	const Folder* const dir = contentHost->GetRootDirectory();
	const Page* const page = static_cast<Page*>(dir->GetPage("/Projects/Assembler/Assembler.html"));

	vector<string> replaceTokens;
	vector<string> replaceTexts;

	const string firstHalf = "<textarea spellcheck='false' rows = '20' cols = '20' name = 'assembledcode'>";
	const string secondHalf = "</textarea>";
	const string firstHalf2 = "<textarea spellcheck='false' rows = '20' cols = '40' name = 'comment'>";
	const string secondHalf2 = "</textarea>";
	string errorText = "";
	string userName = "";
	string assembledCode = "";
	string finalCode = "";

	const auto finalCodeInd = file.find("&comment=");
	const auto stepCountInd = file.find("&stepcount=");
	const auto originalInd = file.find("=16bit");

	const bool useHProtection = file.find("&hprot=on") != string::npos;
	const bool createStack = file.find("stack=createstack") != string::npos;
	const bool useSimulator = stepCountInd != string::npos;

	CS350::ENDIANESS endianType = CS350::CS_ORIGINAL;
	if (file.find("=littleendian") != string::npos)
	{
		endianType = CS350::CS_LITTLE_ENDIAN;
		replaceTokens.push_back("~little");
		replaceTexts.push_back(" checked");
	}
	else if (file.find("=bigendian") != string::npos)
	{
		endianType = CS350::CS_BIG_ENDIAN;
		replaceTokens.push_back("~big");
		replaceTexts.push_back(" checked");
	}
	else if (file.find("=indian") != string::npos)
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
	if (file.find("username=") == string::npos)
	{
		errorText += "missing user name\n";
	}
	else
	{
		const int nextAmpersand = rest.find("&");
		userName = rest.substr(18, nextAmpersand - 18);
		cout << "Initiating a compile for user " << userName << endl;
	}

	if (errorText == "")
	{
		if (finalCodeInd < 0)
		{
			errorText += "Error: code not found\n";
		}

		if (createStack)
		{
			finalCode = Assembler::GenerateStackCode();
		}
		else
		{
			rest = rest.substr(finalCodeInd + 9);
			finalCode = Server::cleanAssemblyString(rest);
		}

		Assembler ourAssembler(finalCode, endianType, useHProtection, createStack);
		assembledCode = ourAssembler.Assemble();
		errorText += ourAssembler.GetErrorText();
		Simulator simulator(ourAssembler.GetMachineCode(), ourAssembler.GetLineDataType());
		int stepCt = ourAssembler.GetMachineCode().size();
		string stepCount = to_string(stepCt);

		if (useSimulator)
		{
			const int nextAmpersand = file.substr(stepCountInd + 1).find("&");
			const string stepCount2 = file.substr(stepCountInd + strlen("&stepcount="), (nextAmpersand)-strlen("&stepcount=") + 1);
			stringstream sp(stepCount2);
			int sentStep = 0;
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


		/*const string firstHalf = "<textarea spellcheck='false' rows = '30' cols = '130' name = 'simval' readonly>";
		const string secondHalf = "</textarea>";
		*/
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
	replaceTexts.push_back(userName + "'");

	replaceTokens.push_back("form='usrform'>");
	replaceTexts.push_back(finalCode);

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

	const Page* const newPage = page->ClonePage(replaceTokens, replaceTexts);

	const Page* const constructedPage = this->serverWebPageApp->ConstructPage(newPage, dir);
	delete newPage;

	this->server->SendPage(constructedPage, clientSocket);
	delete constructedPage;
}