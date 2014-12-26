#include "WebApp.h"

using namespace std;

WebApp::WebApp(const string& name, Server* server) :
	name(name),
	server(server)
{
}

WebApp::~WebApp()
{
}
