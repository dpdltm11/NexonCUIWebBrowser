#pragma once
#include <iostream>
#include<string>
#include <algorithm>
#include <sstream>
#include <vector>
#include<string.h>

using namespace std;

class Parser
{
private:
	string input;
	string statusNum;
	string status;
	string contentType;
	string html;

public:
	Parser();
	Parser(const string &);
	vector<char> imageParser(const string &);
	string getHtml();
	string getstatusNum();
	string getstatus();
};

