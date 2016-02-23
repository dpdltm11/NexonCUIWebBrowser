#pragma once
#include <iostream>
#include<string>
#include <algorithm>
#include <sstream>
#include <vector>
#include<string.h>

using namespace std;

// html 결과를 위한 class
// 200일 경우만 html parses
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

