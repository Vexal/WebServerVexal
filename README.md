WebServerVexal
==============

main() is in WebServerVexal.cpp

The Server class (Server.cpp / Server.h) handles accepting any client tcp connections.

HttpServer/HttpServer.cpp handles HTTP requests.

Incoming requests are passed off to webapps, which is determined by the request target; if request target matches no web app controller names, it is assumed the request is for a webpage itself, and is served through WebPageApp


WebPageApp serves web pages.

AssemblerWebApp does the assembling web app for that assembler and simulator I wrote.

VimWebApp allows people to like and share code from VIM to facebook.

HomeAutomationWebApp handles requests to interface with my house

AccountCreateApp handles requests to update user accounts

trunk is an open source encryption library used solely for the websockets connection handshake.

The SerialHandler and WebSockets are for interfacing with the Arduino board connected to my server.


The other files do things.
