#include "NullCommentsDAO.h"
#include "../../Util.h"

using namespace std;

UserCommentsThread NullCommentsDAO::GetThread(const unsigned int threadId) const
{
	return
	{
		threadId,
		{
			{
				1,
				threadId,
				1,
				"vexal",
				"127.0.0.1",
				Util::CurrentDateTime(),
				"user comments test 1"
			},
			{
				2,
				threadId,
				4,
				"linda",
				"127.0.0.2",
				Util::CurrentDateTime(),
				"This is a cool site!</div>"
			},
			{
				3,
				threadId,
				5,
				"jtsense",
				"127.0.0.3",
				Util::CurrentDateTime(),
				"This is a cool site!  Checkout mine!"
			}
		}
	};
}

UserCommentsThread NullCommentsDAO::GetThread(const string& threadKey) const
{
	return this->GetThread(5U);
}

NullCommentsDAO* NullCommentsDAO::Create()
{
	static NullCommentsDAO singletonDAO = NullCommentsDAO();
	return &singletonDAO;
}
