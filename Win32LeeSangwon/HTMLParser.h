#pragma once

#include <iostream>
#include<string>
#include<string.h>
#include <algorithm>
#include <sstream>
#include <vector>
#include <unordered_map>

using namespace std;
using namespace stdext;

enum PARSESTATE {
	TEXTPARSE,
	IMAGEPARSE,
	NONPARSE
};

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
	PARSESTATE validtag(string);
	string getTag(string tag);
	string getTitle();
	string geth1();
	vector<string> getResult();
	vector<string> getkeyHyperLink();
	unordered_map< string, string > getHyperLink();
};  // uri