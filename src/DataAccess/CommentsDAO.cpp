#include "CommentsDAO.h"

using namespace std;

string CommentsDAO::GetThreadKey(const string& input)
{
	//I promise to fix this later.
	if (input == "/")
	{
		return "/home.html";
	}

	return input;
}
