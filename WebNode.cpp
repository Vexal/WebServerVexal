#include "WebNode.h"

using namespace std;

#ifdef WIN32
WebNode::WebNode(const string& fullPath, const string& name, NODE_TYPE nodeType) :
#else
WebNode::WebNode(const string& fullPath, const string& name, NODE_TYPE nodeType) :
#endif
	fullPath(fullPath),
	name(name),
	nodeType(nodeType)
{

}

WebNode::~WebNode()
{

}
