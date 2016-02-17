#include "../HttpServer/HttpUtils.h"
#include "UserCommentsThread.h"

using namespace std;

string UserCommentsThread::GenerateFullHtml()
{
	string commentsHtml;

	for (const UserComment& comment : this->threadComments)
	{
		commentsHtml += GenerateCommentHtml(comment);
	}

	return commentsHtml;
}

string UserCommentsThread::GenerateCommentHtml(const UserComment& comment)
{
	string html = "<div class=\"comment\">";
	html += "<span class=\"commentheader\">";
	html += "<span class=\"commentusername\">";
	html += HttpUtils::entityEncode(comment.username);
	html += " (id: ";
	html += comment.userId;
	html += ")";
	html += "</span>";
	html += "<span class=\"commentdate\">";
	html += comment.timestamp;
	html += "</span>";
	html += "<span class=\"commentip\">";
	html += comment.userIpAddress;
	html += "</span>";
	html += "</span>";
	html += "</br>";
	html += HttpUtils::entityEncode(comment.text);
	html += "</div>";
	html += "</br>";
	return html;
}
