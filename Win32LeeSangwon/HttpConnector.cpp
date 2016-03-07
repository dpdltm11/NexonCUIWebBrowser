#include "HttpConnector.h"
#include <codecvt>
#include <atlstr.h>

HttpConnector::HttpConnector() :request(""), response("") {}

void HttpConnector::die_with_error(char *errorMessage)
{
	cout << errorMessage << endl;
	//exit(1);
}

void HttpConnector::die_with_wserror(char *errorMessage)
{
	cout << errorMessage << ": " << WSAGetLastError() << endl;
	//exit(1);
}

// DNS를 통한 ip주소 받아오기
char* HttpConnector::HostToIp(const string& host) {
	hostent* hostname = gethostbyname(host.c_str());
	// Init WinSock
	WSADATA wsa_Data;
	int wsa_ReturnCode = WSAStartup(0x101, &wsa_Data);
	// Get the local hostname
	struct hostent *host_entry;
	host_entry = gethostbyname(host.c_str());
	char * szLocalIP;
	szLocalIP = inet_ntoa(*(struct in_addr *)*host_entry->h_addr_list);

	WSACleanup();

	return szLocalIP;
}

// HTML 요청
string HttpConnector::httpConnect(const string& tempuri) {
	Uri *uri = new Uri;
	resultURL = uri->Parse(tempuri);
	resultURL.Parse(tempuri);

	// Protocol이 http 일 경우만 요청
	if (resultURL.getProtocol() == "http" || resultURL.getProtocol() == "")
	{
		// ip주소
		char *ipaddress = HostToIp(resultURL.getHost());
		int port = 0;
		// 포트번호가 지정되지 않은면 default로 80 설정
		if (resultURL.getPort() == "")
		{
			port = 80;
		}
		else
		{
			port = atoi(resultURL.getPort().c_str());
		}
		string path = resultURL.getPath();
		
		if (path == "" && resultURL.getHost() == "www.naver.com")
		{
			path = "/";
		}
		
		request = "GET " + path + " HTTP/1.1\r\n\r\n";

		//init winsock
		if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
			die_with_wserror("WSAStartup() failed");

		//open socket
		if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
			die_with_wserror("socket() failed");

		//connect
		memset(&serveraddr, 0, sizeof(serveraddr));
		serveraddr.sin_family = AF_INET;
		serveraddr.sin_addr.s_addr = inet_addr(ipaddress);
		serveraddr.sin_port = htons(port);
		if (connect(sock, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
		{
			die_with_wserror("connect() failed");
			return "not valid";
		}

		//send request
		if (send(sock, request.c_str(), request.length(), 0) != request.length())
		{
			die_with_wserror("send() sent a different number of bytes than expected");
			return "not valid";
		}

		//get response
		cout << "response" << endl;
		response = "";
		resp_leng = BUFFERSIZE;

		do
		{
			resp_leng = recv(sock, (char*)&buffer, BUFFERSIZE, 0);
			response += string(buffer).substr(0, resp_leng);
		} while (resp_leng != 0);

		//disconnect
		closesocket(sock);

		//cleanup
		WSACleanup();

		//UTF8 -> 유니코드
		wchar_t *strUnicode = new wchar_t[response.length()];
		int nLen = MultiByteToWideChar(CP_UTF8, 0, response.c_str(), response.length(), NULL, NULL);
		MultiByteToWideChar(CP_UTF8, 0, response.c_str(), response.length(), strUnicode, nLen);

		//유니코드 -> 멀티바이트
		int len = WideCharToMultiByte(CP_ACP, 0, strUnicode, -1, NULL, 0, NULL, NULL);
		string strMulti(len, 0);
		WideCharToMultiByte(CP_ACP, 0, strUnicode, -1, &strMulti[0], len, NULL, NULL);
		response = strMulti;
		//cout << response << endl;
	}
	else
		response = "not valid";
	return response;
}

// image 받아오기
// vector<char>로 이미지 content-length 없을 때 Size를 알기 위해 vector<char> 사용 but... copy 때 성능 문제!
/*
vector<char> HttpConnector::getImage(string tempuri) {
	image.clear();
	Uri *uri = new Uri;//::Parse(tempuri);
	Uri resultURL = uri->Parse(tempuri);
	resultURL.Parse(tempuri);
	tempuri = tempuri.erase(tempuri.length()-1, 1);
	if (resultURL.getProtocol() == "http" || resultURL.getProtocol() == "")
	{
		char *ipaddress = HostToIp(resultURL.getHost());
		int port = 0;
		if (resultURL.getPort() == "")
		{
			port = 80;
		}
		else
		{
			port = atoi(resultURL.getPort().c_str());
		}
		string path = resultURL.getPath();

		if (resultURL.getHost() == "sang12456.cafe24.com")
			request = "GET " + path + " HTTP/1.1\r\n\r\n";
		else
			request = "GET " + path + " HTTP/1.1\nHost: " + resultURL.getHost() + "\r\n\r\n";//"\nConnection : keep - alive\nCache - Control : max - age = 0\nAccept : text / html, application / xhtml + xml, application / xml; q = 0.9, image / webp, *;q=0.8\nUpgrade-Insecure-Requests: 1\nUser-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/48.0.2564.109 Safari/537.36\nAccept-Encoding: gzip, deflate, sdch\nAccept-Language: ko-KR,ko;q=0.8,en-US;q=0.6,en;q=0.4\r\n\r\n";

		//init winsock
		if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
			die_with_wserror("WSAStartup() failed");

		//open socket
		if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
			die_with_wserror("socket() failed");

		//connect
		memset(&serveraddr, 0, sizeof(serveraddr));
		serveraddr.sin_family = AF_INET;
		serveraddr.sin_addr.s_addr = inet_addr(ipaddress);
		serveraddr.sin_port = htons(port);
		if (connect(sock, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
			die_with_wserror("connect() failed");

		//send request
		if (send(sock, request.c_str(), request.length(), 0) != request.length())
			die_with_wserror("send() sent a different number of bytes than expected");

		//get response
		cout << "response" << endl;
		response = "";
		resp_leng = BUFFERSIZE;

		do
		{
			resp_leng = recv(sock, (char*)&buffer, BUFFERSIZE, 0);
			copy(buffer, buffer + resp_leng, back_inserter(image));
		} while (resp_leng != 0);

		//disconnect
		closesocket(sock);

		//cleanup
		WSACleanup();
	}
	else
		response = "not valid";

	return image;
}
*/