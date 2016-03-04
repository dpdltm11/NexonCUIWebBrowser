#include "Uri.h"
#include "HttpConnector.h"
#include "Paser.h"
#include "HTMLParser.h"
#include "InputController.h"
#include <istream>
#include <streambuf>
#include <assert.h>
#include <atlimage.h>
#include <gdiplus.h>
#include <ole2.h>
#include <Shlwapi.h>
#include <iterator>
#include <stdio.h>
#include <fstream>
#include <thread>
#include <future>
#include <mutex> 
using namespace Gdiplus;

#pragma comment (lib, "gdiplus")
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#pragma comment(lib,"ws2_32.lib")
#define WIDTHBYTES(bits) (((bits)+31)/32*4)

typedef unsigned char BYTE;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void getDataFromServer(string);
BOOL LoadBitmapFromBMPFile(LPTSTR szFileName, HBITMAP *phBitmap, HPALETTE *phPalette, int flag);

HINSTANCE g_hInst;
LPCTSTR lpszClass = TEXT("CUIWebBrowser");

// http response
string result;
vector<string> ret;
unordered_map< int, char* > images;
unordered_map< int, int > imageSize;
unordered_map< string, string > hyperLinkMap;
unordered_map< string, char* > imagecache;
unordered_map< string, int > imagecacheSize;
vector<thread> threads;
std::mutex mtx;

// url 입력(input)
static char str[256];
int scroll;
InputController userInterface; 

HWND hwndMain;
HWND hEdit;
HWND hWndVScrollBar;
HWND hWndHScrollBar;
WNDPROC origLVEditWndProc = NULL;
RECT rt = { 10,60,400,300 };
int maxHeight;

// string 한개만 replace 하는 함수
void replace(vector<string>& my_vector_2, string old, string replacement) {
	vector<string>::iterator it;
	for (it = my_vector_2.begin(); it != my_vector_2.end(); ++it) {

		if (*it == old) {
			it = my_vector_2.erase(it);
			it = my_vector_2.insert(it, replacement);
			break;
		}
	}
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst = hInstance;

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = NULL;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);

	while (GetMessage(&Message, nullptr, 0, 0))
	{
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

	return (int)Message.wParam;
}

LRESULT CALLBACK LVEditWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		//focus가 edit일 때 스크롤
		case VK_PRIOR:
			scroll += 50;
			InvalidateRect(hwndMain, &rt, TRUE);
			break;
		case VK_NEXT:
			scroll -= 50;
			InvalidateRect(hwndMain, &rt, TRUE);
			break;

		case VK_RETURN:
		{
			scroll = 0;
			ret.clear();
			//ret.shrink_to_fit();
			images.clear();
			imageSize.clear();
			threads.clear();
			// get current text
			int textLength = (int)SendMessage(hWnd, WM_GETTEXTLENGTH, 0, 0) + 1;
			SendMessage(hWnd, WM_GETTEXT, (WPARAM)textLength, (LPARAM)str);
			printf("enter!\n");
			
			// Input Control
			string usrInput(str);
			STATE curState = userInterface.getState(usrInput);
			if (curState == HELPSTATE)
			{
				ret.push_back(userInterface.getHelpText());
			}
			else if (curState == GOSTATE || curState == BACKSTATE || curState == FORWARDSTATE || curState == HOMESTATE)
			{
				hyperLinkMap.clear();
				string uri = userInterface.getURI();
				getDataFromServer(uri);
			}
			else if (curState == REFRESHSTATE)
			{
				string uri = userInterface.getURI();
				getDataFromServer(uri);
			}
			else if (curState == LSSTATE)
			{
				vector<string> urilist = userInterface.getURIList();				
				for (int p = 0; p < urilist.size(); p++)
				{
					ret.push_back(urilist[p]);
				}
			}
			else if (curState == HGOSTATE)
			{
				string getKey = userInterface.getUserHyperLinkText();
				string hyperURI = "";
				hyperURI = hyperLinkMap.find(getKey)->second;
				if (hyperURI != "")
				{
					userInterface.pushHyperLinkURI(hyperURI);
				}
				getDataFromServer(hyperURI);
			}
			else if (curState == NOTVALIDSTATE)
			{
				ret.push_back("명령어를 잘못 입력하셨습니다\n");
			}
			InvalidateRect(hwndMain, &rt, TRUE);
			break;
		}
		case VK_BACK:
		{
			break;
		}
		}
		break;
	}
	}
	return CallWindowProc(origLVEditWndProc, hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	int len = 0;

	hwndMain = hWnd;
	HBITMAP hBMP, hOldBmp;

	HBITMAP       hBitmap, hOldBitmap;
	HPALETTE      hPalette, hOldPalette;
	HDC           hMemDC;
	BITMAP        bm;
	HDC          hdcBuffer;
	HFONT font, oldfont;
	RECT rect;
	int width;
	int height;

	if (GetWindowRect(hWnd, &rect))
	{
		rect.left = 10;
		rect.top = 45;
		width = rect.right - rect.left;
		height = rect.bottom - rect.top;
	}
	rt = rect;
	rect.top += 20;

	switch (iMessage)
	{
	case WM_CREATE:
		// edit control
		hEdit = CreateWindow("edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER |
			ES_AUTOHSCROLL, 250, 10, 300, 25, hWnd, (HMENU)NULL, g_hInst, NULL);
		origLVEditWndProc = (WNDPROC)SetWindowLong(hEdit, GWL_WNDPROC, (DWORD)LVEditWndProc);

	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_PRIOR:
			scroll += 50;
			InvalidateRect(hwndMain, &rt, TRUE);
			break;
		case VK_NEXT:
			scroll -= 50;
			InvalidateRect(hwndMain, &rt, TRUE);
			break;
		}
	}

	case WM_PAINT:
	{
		int imageCount = 0;
		hdc = BeginPaint(hWnd, &ps);
		//FillRect(hdc, &rt, (HBRUSH)GetStockObject(WHITE_BRUSH));
		rect.top += scroll;
		// bmp local image load with double buffering
		if (LoadBitmapFromBMPFile("nexon.bmp", &hBMP, &hPalette, 0))
		{
			GetObject(hBMP, sizeof(BITMAP), &bm);
			hMemDC = CreateCompatibleDC(hdc);
			hOldBmp = (HBITMAP)SelectObject(hMemDC, hBMP);
			hOldPalette = SelectPalette(hdc, hPalette, FALSE);
			RealizePalette(hdc);
			BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight,
				hMemDC, 0, 0, SRCCOPY);
			SelectObject(hMemDC, hOldBmp);
			DeleteObject(hBMP);
			SelectPalette(hdc, hOldPalette, FALSE);
			DeleteObject(hPalette);
		}
		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		ULONG_PTR gdiplusToken;
		Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
		Graphics graphics(hdc);

		for (int i = 0; i < ret.size(); i++)
		{
			// 텍스트 tag일 경우
			if (ret[i] != "image")
			{
				if (ret[i].substr(ret[i].length() - 5, 5) == "image")
				{
					imageCount++;
					//continue;
				}
				int fontSize = 15;
				if (ret[i].substr(ret[i].length() - 5, 5) == "title")
				{
					fontSize = 35;
					font = CreateFont(fontSize, 0, 0, 0, FW_HEAVY, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0, 0, "돋움");
					oldfont = (HFONT)SelectObject(hdc, font);
					DrawText(hdc, (ret[i].substr(0, ret[i].length() - 5)).c_str(), -1, &rect, DT_LEFT | DT_WORDBREAK | DT_NOCLIP);
				}
				else if (ret[i].substr(ret[i].length() - 2, 2) == "h1")
				{
					fontSize = 30;
					font = CreateFont(fontSize, 0, 0, 0, FW_ULTRABOLD, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0, 0, "돋움");
					oldfont = (HFONT)SelectObject(hdc, font);
					DrawText(hdc, (ret[i].substr(0, ret[i].length() - 2)).c_str(), -1, &rect, DT_LEFT | DT_WORDBREAK | DT_NOCLIP);
				}
				else if (ret[i].substr(ret[i].length() - 2, 2) == "h2")
				{
					fontSize = 25;
					font = CreateFont(fontSize, 0, 0, 0, FW_BOLD, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0, 0, "돋움");
					oldfont = (HFONT)SelectObject(hdc, font);
					DrawText(hdc, (ret[i].substr(0, ret[i].length() - 2)).c_str(), -1, &rect, DT_LEFT | DT_WORDBREAK | DT_NOCLIP);
				}
				else if (ret[i].substr(ret[i].length() - 2, 2) == "h3")
				{
					fontSize = 20;
					font = CreateFont(fontSize, 0, 0, 0, FW_BOLD, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0, 0, "돋움");
					oldfont = (HFONT)SelectObject(hdc, font);
					DrawText(hdc, (ret[i].substr(0, ret[i].length() - 2)).c_str(), -1, &rect, DT_LEFT | DT_WORDBREAK | DT_NOCLIP);
				}
				else if (ret[i].substr(ret[i].length() - 2, 2) == "h4")
				{
					fontSize = 15;
					font = CreateFont(fontSize, 0, 0, 0, FW_BOLD, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0, 0, "돋움");
					oldfont = (HFONT)SelectObject(hdc, font);
					DrawText(hdc, (ret[i].substr(0, ret[i].length() - 2)).c_str(), -1, &rect, DT_LEFT | DT_WORDBREAK | DT_NOCLIP);
				}
				else if (ret[i].substr(ret[i].length() - 2, 2) == "h5")
				{
					fontSize = 10;
					font = CreateFont(fontSize, 0, 0, 0, FW_BOLD, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0, 0, "돋움");
					oldfont = (HFONT)SelectObject(hdc, font);
					DrawText(hdc, (ret[i].substr(0, ret[i].length() - 2)).c_str(), -1, &rect, DT_LEFT | DT_WORDBREAK | DT_NOCLIP);
				}
				else if (ret[i].substr(ret[i].length() - 2, 2) == "h6")
				{
					fontSize = 5;
					font = CreateFont(fontSize, 0, 0, 0, FW_BOLD, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0, 0, "돋움");
					oldfont = (HFONT)SelectObject(hdc, font);
					DrawText(hdc, (ret[i].substr(0, ret[i].length() - 2)).c_str(), -1, &rect, DT_LEFT | DT_WORDBREAK | DT_NOCLIP);
				}
				else if (ret[i].substr(ret[i].length() - 9, 9) == "hyperText")
				{
					fontSize = 15;
					font = CreateFont(fontSize, 0, 0, 0, FW_BOLD, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0, 0, "돋움");
					SetTextColor(hdc, RGB(0, 0, 255));
					oldfont = (HFONT)SelectObject(hdc, font);
					DrawText(hdc, (ret[i].substr(0, ret[i].length() - 9)).c_str(), -1, &rect, DT_LEFT | DT_WORDBREAK | DT_NOCLIP);
					SetTextColor(hdc, RGB(0, 0, 0));
				}
				else
				{
					fontSize = 15;
					font = CreateFont(fontSize, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0, 0, "돋움");
					oldfont = (HFONT)SelectObject(hdc, font);
					if (ret[i].substr(ret[i].length() - 6, 6) == "center")
					{
						DrawText(hdc, (ret[i].substr(0, ret[i].length() - 2)).c_str(), -1, &rect, DT_CENTER | DT_WORDBREAK | DT_NOCLIP);
					}
					else if (ret[i].substr(ret[i].length() - 1, 1) == "p")
					{
						DrawText(hdc, (ret[i].substr(0, ret[i].length() - 2)).c_str(), -1, &rect, DT_LEFT | DT_WORDBREAK | DT_NOCLIP);
					}
					else
					{
						DrawText(hdc, ret[i].c_str(), -1, &rect, DT_LEFT | DT_WORDBREAK | DT_NOCLIP);
					}
				}
				SelectObject(hdc, oldfont);
				DeleteObject(font);
				rect.top += fontSize + 30;
			}
			else if (ret[i] == "image")
			{
				// image tag일 경우
				unordered_map<int, int>::iterator FindImageSize = imageSize.find(imageCount);
				// 찾았다면
				if (FindImageSize != imageSize.end())
				{
					DWORD dwImageSize = FindImageSize->second;
					BYTE* pImageBuffer = NULL;
					unordered_map<int, char*>::iterator FindImage = images.find(imageCount);
					// 찾았다면
					if (FindImage != images.end())
					{
						pImageBuffer = (BYTE*)(FindImage->second);
						HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, dwImageSize);
						void* pData = GlobalLock(hGlobal);
						memcpy(pData, pImageBuffer, dwImageSize);
						GlobalUnlock(hGlobal);
						IStream* pStream = NULL;
						if (CreateStreamOnHGlobal(hGlobal, TRUE, &pStream) == S_OK)
						{
							//Image *testImage = Image::FromStream(pStream);
							//graphics.DrawImage(testImage, rect.left, rect.top);	
							Bitmap *pImage = Bitmap::FromStream(pStream);
							pImage->GetHBITMAP(Color::White, &hBitmap);
							if (LoadBitmapFromBMPFile("", &hBitmap, &hPalette, 1))
							{
								GetObject(hBitmap, sizeof(BITMAP), &bm);
								hMemDC = CreateCompatibleDC(hdc);
								hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
								hOldPalette = SelectPalette(hdc, hPalette, FALSE);
								RealizePalette(hdc);
								BitBlt(hdc, rect.left, rect.top, bm.bmWidth, bm.bmHeight,
									hMemDC, 0, 0, SRCCOPY);
								SelectObject(hMemDC, hOldBitmap);
								DeleteObject(hBitmap);
								SelectPalette(hdc, hOldPalette, FALSE);
								DeleteObject(hPalette);
							}
							rect.top += pImage->GetHeight();
						}
					}				
				}			
				imageCount++;
			}
		}
		maxHeight = rect.top;
		EndPaint(hWnd, &ps);
		return 0;
	}
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	}

	return DefWindowProc(hWnd, iMessage, wParam, lParam);
}

void die_with_error(char *errorMessage)
{
	cout << errorMessage << endl;
	//exit(1);
}

void die_with_wserror(char *errorMessage)
{
	cout << errorMessage << ": " << WSAGetLastError() << endl;
	//exit(1);
}

// DNS를 통한 ip주소 받아오기
char* HostToIp(const string& host) {
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

// 멀티스레드를 위한 imageRequset
void imageRequset(string tempuri, int index)
{
	int resp_leng;
	string request, response;
	char buffer[BUFFERSIZE];
	struct sockaddr_in serveraddr;
	int sock = 0;
	WSADATA wsaData;
	char *ipaddress;
	vector<char> image;

	Uri *uri = new Uri;
	Uri resultURL = uri->Parse(tempuri);
	resultURL.Parse(tempuri);
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
			request = "GET " + path + " HTTP/1.1\nHost: " + resultURL.getHost() + "\r\n\r\n";//"\nConnection : keep - alive\nCache - Control : max - age = 0\nAccept : text / html, application / xhtml + xml, application / xml; q = 0.9, image / webp, */*;q=0.8\nUpgrade-Insecure-Requests: 1\nUser-Agent: Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/48.0.2564.109 Safari/537.36\nAccept-Encoding: gzip, deflate, sdch\nAccept-Language: ko-KR,ko;q=0.8,en-US;q=0.6,en;q=0.4\r\n\r\n";

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
		//cout << "response" << endl;
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

	if (image[9] == '2' && image[10] == '0' && image[11] == '0')
	{
		int imageLen = image.size();
		char *binary = new char[imageLen];
		memset(binary, 0, imageLen);
		int j = 0;
		for (int k = 0; k < image.size(); k++)
		{
			binary[j] = image[k];
			j++;
		}
		mtx.lock();
		char *temp = strstr(binary, "\n\n");
		if (temp == NULL)
		{
			temp = strstr(binary, "\r\n\r\n");
			if (temp != NULL)
			{
				images.insert(pair<int, char*>(index, &temp[4]));
				imagecache.insert(pair<string, char*>(tempuri, &temp[4]));
			}
		}
		else
		{
			images.insert(pair<int, char*>(index, &temp[2]));
			imagecache.insert(pair<string, char*>(tempuri, &temp[2]));
		}
		imagecacheSize.insert(pair<string, int>(tempuri, image.size()));
		replace(ret, tempuri+"\"image", "image");
		imageSize.insert(pair<int, int>(index, image.size()));
		mtx.unlock();
		InvalidateRect(hwndMain, &rt, TRUE);
	}
}

void getDataFromServer(string uri)
{
	int count=0;
	// http get request
	HttpConnector *getrequest = new HttpConnector;
	//result = getrequest->httpConnect(usrInput);
	result = getrequest->httpConnect(uri);
	if (result != "not valid")
	{
		Parser *htmlParse = new Parser(result);
		// 성공하지 못했을 때 error코드 출력
		if (htmlParse->getstatusNum() != "200")
		{
			ret.push_back("\n페이지 요청 결과 : " + htmlParse->getstatusNum() + "  " + htmlParse->getstatus());
		}
		else
		{
			// html
			result = htmlParse->getHtml();
			HTMLParser *htmlInfo = new HTMLParser(result);
			ret = htmlInfo->getResult();
			hyperLinkMap = htmlInfo->getHyperLink();
			for (int i = 0; i < ret.size(); i++)
			{
				if (ret[i].substr(ret[i].length() - 5, 5) == "image")
				{
					vector<char> tempimage;
					unordered_map<string, char*>::iterator FindIter = imagecache.find(ret[i].substr(0, ret[i].length() - 6));
					// cache image 찾았다면
					if(FindIter != imagecache.end())
					{
						images.insert(pair<int, char*>(count, FindIter->second));
						imageSize.insert(pair<int, int>(count, imagecacheSize.find(ret[i].substr(0, ret[i].length() - 6))->second));
						replace(ret, ret[i], "image");
						count++;
					}
					else
					{
						threads.push_back(thread(imageRequset, ret[i].substr(0, ret[i].length() - 6), count));
						count++;
					}
				}
			}
		}
	}
	else
	{
		ret.push_back("not valid");
	}
	for (auto& th : threads) th.detach();
}

BOOL LoadBitmapFromBMPFile(LPTSTR szFileName, HBITMAP *phBitmap, HPALETTE *phPalette, int flag)
{
	BITMAP  bm;
	if (flag == 0)
		*phBitmap = NULL;
	*phPalette = NULL;

	// Use LoadImage() to get the image loaded into a DIBSection
	if (flag == 0)
		*phBitmap = (HBITMAP)LoadImage(NULL, szFileName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE);

	if (*phBitmap == NULL)
		return FALSE;

	// Get the color depth of the DIBSection
	GetObject(*phBitmap, sizeof(BITMAP), &bm);
	// If the DIBSection is 256 color or less, it has a color table
	if ((bm.bmBitsPixel * bm.bmPlanes) <= 8)
	{
		HDC           hMemDC;
		HBITMAP       hOldBitmap;
		RGBQUAD       rgb[256];
		LPLOGPALETTE  pLogPal;
		WORD          i;

		// Create a memory DC and select the DIBSection into it
		hMemDC = CreateCompatibleDC(NULL);
		hOldBitmap = (HBITMAP)SelectObject(hMemDC, *phBitmap);
		// Get the DIBSection's color table
		GetDIBColorTable(hMemDC, 0, 256, rgb);
		// Create a palette from the color tabl
		pLogPal = (LOGPALETTE *)malloc(sizeof(LOGPALETTE) + (256 * sizeof(PALETTEENTRY)));
		pLogPal->palVersion = 0x300;
		pLogPal->palNumEntries = 256;
		for (i = 0; i<256; i++)
		{
			pLogPal->palPalEntry[i].peRed = rgb[i].rgbRed;
			pLogPal->palPalEntry[i].peGreen = rgb[i].rgbGreen;
			pLogPal->palPalEntry[i].peBlue = rgb[i].rgbBlue;
			pLogPal->palPalEntry[i].peFlags = 0;
		}
		*phPalette = CreatePalette(pLogPal);
		// Clean up
		free(pLogPal);
		SelectObject(hMemDC, hOldBitmap);
		DeleteDC(hMemDC);
	}
	else   // It has no color table, so use a halftone palette
	{
		HDC    hRefDC;

		hRefDC = GetDC(NULL);
		*phPalette = CreateHalftonePalette(hRefDC);
		ReleaseDC(NULL, hRefDC);
	}
	return TRUE;
}