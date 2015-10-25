#include "WebNode.h"
#include <iostream>

using namespace std;

WebNode::WebNode(const string& fullPath, const string& name, NODE_TYPE nodeType) :
	fullPath(fullPath),
	name(name),
	nodeType(nodeType)
{
	//cout << "loaded " << fullPath << endl;
}

WebNode::~WebNode()
{

}
