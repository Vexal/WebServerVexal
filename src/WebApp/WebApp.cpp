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
