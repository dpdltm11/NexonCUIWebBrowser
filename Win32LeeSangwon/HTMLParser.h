#pragma once

#include <iostream>
#include<string>
#include<string.h>
#include <algorithm>
#include <sstream>
#include <vector>
#include <unordered_map>

using namespace std;

// html parsing을 위한 클래스
class HTMLParser
{
private:
	string html, title, h1;
	vector<string> result;
	vector<string> keyHyperLink;
	unordered_map<string, string> hyperLink;
public:
	HTMLParser(string);
	string removeTag(string,string, int);
	void moreParse(const string &);
	string getTag(string tag);
	inline vector<string> HTMLParser::getResult() { return result; }
	inline vector<string> HTMLParser::getkeyHyperLink() { return keyHyperLink; }
	inline unordered_map<string, string> HTMLParser::getHyperLink() { return hyperLink; }
};