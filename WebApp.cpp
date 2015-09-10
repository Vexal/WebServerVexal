#include "WebApp.h"

using namespace std;

WebApp::WebApp(const string& name, HttpServer* server) :
	name(name),
	server(server)
{
}

WebApp::~WebApp()
{
}

std::string WebApp::GetStringParameter(const std::string & input, const std::string & parameterName)
{
	const string builtParam = "&" + parameterName + "=";
	auto paramInd = input.find(builtParam);
	if (paramInd == input.npos)
	{
		paramInd = input.find("?" + parameterName + "=");
		if (paramInd == input.npos)
			return ""; //not found;
	}

	string result = input.substr(paramInd + builtParam.length());
	const auto nextAmpersand = result.find("&");
	if (nextAmpersand != input.npos) //more parameters in result
	{
		result = result.substr(0, nextAmpersand);
	}
	return result;
}
