#pragma once
#include <iostream>
#include<string>
#include <algorithm>

using namespace std;

class Uri
{
private:
	string QueryString, Path, Protocol, Host, Port;
public:
	Uri();
	string getPath();
	string getProtocol();
	string getHost();
	string getPort();
	Uri Parse(const string &);
};  // uri