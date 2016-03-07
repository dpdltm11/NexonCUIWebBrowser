#pragma once
#include <iostream>
#include<string>
#include <algorithm>

using namespace std;

// URI Parse Class
class Uri
{
private:
	string QueryString, Path, Protocol, Host, Port;
public:
	Uri();
	inline string getPath() { return Path; };
	inline string getProtocol() { return Protocol; };
	inline string getHost() { return Host; };
	inline string getPort() { return Port; };
	Uri Parse(const string &);
};