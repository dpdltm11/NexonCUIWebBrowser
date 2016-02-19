#include "Paser.h"

vector<string> &split(const string &s, char delim, vector<string> &elems) {
	stringstream ss(s);
	string item;
	while (getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

vector<string> split(const string &s, char delim) {
	vector<string> elems;
	split(s, delim, elems);
	return elems;
}

Parser::Parser()
{
}

Parser::Parser(const string &result)
{
	typedef string::const_iterator iterator_t;

	if (result.length() == 0)
		return;

	// iterator_t resultEnd = result.end();

	// get query start
	// iterator_t htmlStart = find(result.begin(), resultEnd, );

	//html = string(htmlStart, resultEnd);
	
	vector<string> init = split(result, '\n\n');
	vector<string> statusArray = split(init[0], ' ');

	statusNum = statusArray[1];
	status = statusArray[2];

	if (statusNum == "200")
	{
		// html�� ���
		if (result.find("<html") != string::npos) 
		{
			html = string(strstr(result.c_str(), "<html"));
		}
		else if (result.find("<HTML") != string::npos)
		{
			html = string(strstr(result.c_str(), "<HTML"));
		}
		
	}

	/*
	else if (statusArray[0].substr(0, 9) == "<!DOCTYPE")
	{
		statusNum = "200";
		// html�� ���
		if (result.find("<html") != string::npos)
		{
			html = string(strstr(result.c_str(), "<html"));
		}
		else if (result.find("<HTML") != string::npos)
		{
			html = string(strstr(result.c_str(), "<HTML"));
		}
	}
	*/
}

vector<char> Parser::imageParser(const string &imageresult)
{
	typedef string::const_iterator iterator_t;
	//vector<char> result;
	
	vector<string> init = split(imageresult, '\n');
	vector<string> statusArray = split(init[0], ' ');

	statusNum = statusArray[1];
	status = statusArray[2];

	string imageBinary = "false";

	if (statusNum == "200" || statusNum =="304")
	{
		int i = 0;
		imageBinary = "";
		for (i = 0; i < init.size(); i++)
		{
			if (init[i] == "")
				break;
		}

		for (int j = i + 1; j < init.size(); j++)
		{
			imageBinary += init[j];
		}
	}

	vector<char> data(imageBinary.c_str(), imageBinary.c_str() + imageBinary.length());	
	return data;
}

string Parser::getHtml()

{
	return html;
}

string Parser::getstatusNum()
{
	return statusNum;
}

string Parser::getstatus()
{
	return status;
}