#pragma once
#include <Windows.h>
#include <vector>
#include"Uri.h"
#include <iterator>     // std::back_insert_iterator

//#define BUFFERSIZE 1024
//#define BUFFERSIZE 100000
const int BUFFERSIZE = 8192;
class HttpConnector
{
private:
	int resp_leng;
	string request, response;
	char buffer[BUFFERSIZE];
	struct sockaddr_in serveraddr;
	int sock;
	WSADATA wsaData;
	string tempuri;
	Uri *uri;
	Uri resultURL;
	vector<char> image;
	char *ipaddress;

public:
	HttpConnector();
	string httpConnect(const string&);
	inline string HttpConnector::getResponse() { return response; }
	//vector<char> getImage(string);
	char* HostToIp(const string& host);
	void die_with_error(char *errorMessage);
	void die_with_wserror(char *errorMessage);
};