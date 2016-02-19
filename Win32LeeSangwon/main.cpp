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
#include <ole2.h> // 아래2줄은 IStream쓰려면필요함
#include <Shlwapi.h>
#include <iterator>
#include <stdio.h>
#include <fstream>

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
vector<char*> images;
vector<char*> jpgimages;
vector<int> jpgSize;
unordered_map< string, string > hyperLinkMap;
// url 입력(input)
static char str[256];
int scroll;
InputController userInterface;

HWND hwndMain;
HWND hEdit;
WNDPROC origLVEditWndProc = NULL;
RECT rt = { 10,60,400,300 };

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

HWND CreateAHorizontalScrollBar(HWND hwndParent, int sbHeight)
{
	RECT rect;

	// Get the dimensions of the parent window's client area;
	if (!GetClientRect(hwndParent, &rect))
		return NULL;

	// Create the scroll bar.
	return (CreateWindowEx(
		0,                      // no extended styles 
		"SCROLLBAR",           // scroll bar control class 
		(PTSTR)NULL,           // no window text 
		WS_CHILD | WS_VISIBLE   // window styles  
		| SBS_VERT,         // horizontal scroll bar style 
		rect.right - 30,              // horizontal position 
		150 - sbHeight, // vertical position 
		20,             // width of the scroll bar 
		rect.bottom - 50,               // height of the scroll bar
		hwndParent,             // handle to main window 
		(HMENU)NULL,           // no menu 
		g_hInst,                // instance owning this window 
		(PVOID)NULL            // pointer not needed 
		));
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst = hInstance;

	printf("hello\n");

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
		case VK_PRIOR:
			cout << "pgup" << endl;
			scroll += 10;
			InvalidateRect(hwndMain, &rt, FALSE);
			break;
		case VK_NEXT:
			cout << "pgdn" << endl;
			scroll -= 10;
			InvalidateRect(hwndMain, &rt, FALSE);
			break;

		case VK_RETURN:
		{
			scroll = 0;
			ret.clear();
			images.clear();
			jpgimages.clear();
			jpgSize.clear();
			// get current text
			int textLength = (int)SendMessage(hWnd, WM_GETTEXTLENGTH, 0, 0) + 1;
			SendMessage(hWnd, WM_GETTEXT, (WPARAM)textLength, (LPARAM)str);
			printf("enter!\n");
			
			string usrInput(str);

			STATE curState = userInterface.getState(usrInput);

			if (curState == HELPSTATE)
			{
				ret.push_back(userInterface.getHelpText());
			}
			else if (curState == GOSTATE)
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
			else if (curState == BACKSTATE)
			{
				hyperLinkMap.clear();
				string uri = userInterface.getURI();
				getDataFromServer(uri);
			}
			else if (curState == FORWARDSTATE)
			{
				hyperLinkMap.clear();
				string uri = userInterface.getURI();
				getDataFromServer(uri);
			}
			else if (curState == HOMESTATE)
			{
				hyperLinkMap.clear();
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
			else if (curState == HLSSTATE)
			{
				for (auto it = hyperLinkMap.begin(); it != hyperLinkMap.end(); ++it)
					ret.push_back(it->first);// << ":" << it->second;
			}
			else if (curState == HGOSTATE)
			{
				string getKey = userInterface.getUserHyperLinkText();
				string hyperURI = "";
				for (auto it = hyperLinkMap.begin(); it != hyperLinkMap.end(); it++) {
					//cout << " " << it->first << ":" << it->second;
					if ((it->first) == getKey)
					{
						hyperURI = it->second;
						break;
					}
				}
				if (hyperURI != "")
				{
					userInterface.pushHyperLinkURI(hyperURI);
				}
				getDataFromServer(hyperURI);
			}
			InvalidateRect(hwndMain, NULL, FALSE);
			break;
		}
		case VK_BACK:
		{
			//scroll = 0;
			printf("backspace!\n");
			result = "";
			/*
			result = "";
			ret.clear();
			images.clear();
			jpgimages.clear();
			jpgSize.clear();
			hyperLinkMap.clear();
			*/
			InvalidateRect(hwndMain, &rt, FALSE);
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
	int imageCount = 0;
	int jpgimageCount = 0;
	int h = 0;

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
		rect.top = 40;
		width = rect.right - rect.left;
		height = rect.bottom - rect.top;
	}
	rt = rect;
	rect.top += 20;
	switch (iMessage)
	{
	case WM_CREATE:
		hEdit = CreateWindow("edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER |
			ES_AUTOHSCROLL, 250, 10, 300, 25, hWnd, (HMENU)NULL, g_hInst, NULL);
		origLVEditWndProc = (WNDPROC)SetWindowLong(hEdit, GWL_WNDPROC, (DWORD)LVEditWndProc);
		return 0;

	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_PRIOR:
			cout << "pgup" << endl;
			scroll += 10;
			InvalidateRect(hwndMain, &rt, FALSE);
			//InvalidateRect(hwndMain, NULL, FALSE);
			break;
		case VK_NEXT:
			cout << "pgdn" << endl;
			scroll -= 10;
			InvalidateRect(hwndMain, &rt, FALSE);
			//InvalidateRect(hwndMain, NULL, FALSE);
			break;
		}
	}

	case WM_PAINT:
	{
		hdc = BeginPaint(hWnd, &ps);
		FillRect(hdc, &rt, (HBRUSH)GetStockObject(WHITE_BRUSH));
		rect.top += scroll;
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

		for (int i = 0; i < ret.size(); i++)
		{
			if (ret[i] != "bmpimage" && ret[i] != "jpgimage"&& ret[i] != "pngimage")
			{
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
			else
			{
				if (ret[i] == "bmpimage")
				{
					BITMAPFILEHEADER* pBmfh = (BITMAPFILEHEADER*)images[imageCount];
					BITMAPINFOHEADER* pBmih = (BITMAPINFOHEADER*)(images[imageCount] + sizeof(BITMAPFILEHEADER));
					BITMAPINFO* pBmi = (BITMAPINFO*)pBmih;
					void* pBMPdata = (void*)(images[imageCount] + pBmfh->bfOffBits);
					void* pToFill = 0;
					hBitmap = CreateDIBSection(NULL, pBmi, DIB_RGB_COLORS, &pToFill, NULL, NULL);

					memcpy(pToFill, pBMPdata, pBmfh->bfSize - pBmfh->bfOffBits);                                         // this line should be added!

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
					cout << "bmpimage" << endl;
					imageCount++;
					rect.top += bm.bmHeight;
				}
				else if (ret[i] == "jpgimage" || ret[i] == "pngimage")
				{
					cout << ret[i] << endl;
					Gdiplus::GdiplusStartupInput gdiplusStartupInput;
					ULONG_PTR gdiplusToken;
					Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
					Graphics graphics(hdc);

					DWORD dwImageSize = jpgSize[jpgimageCount];
					BYTE* pImageBuffer = NULL;
					pImageBuffer = (BYTE*)(jpgimages[jpgimageCount]);
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
						//rect.top += testImage->GetHeight();
					}
					jpgimageCount++;
				}
			}
		}
		//RECT rt = { 0,0,400,300 };
		//DrawText(hdc, "Simple Web Browser", -1, &rect, DT_LEFT | DT_WORDBREAK | DT_NOCLIP);
		//DrawText(hdc, "주소 입력 후 Enter를 입력하세요!", -1, &rect, DT_LEFT | DT_WORDBREAK | DT_NOCLIP);

		//TextOut(hdc, 110, 10, , strlen("Simple Web Browser"));
		//TextOut(hdc, 110, 30, , strlen("주소 입력 후 Enter를 입력하세요!"));
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

void getDataFromServer(string uri)
{
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
					if (jpgimages.size() < 10)
						tempimage = getrequest->getImage(ret[i].substr(0, ret[i].length() - 6));
					else
						continue;

					if (tempimage.size() != 0)
					{
						if (tempimage[9] == '2' && tempimage[10] == '0' && tempimage[11] == '0')
						{
							if (ret[i].substr(ret[i].length() - 9, 3) == "bmp")
							{
								int imageLen = tempimage.size();
								char *binary = new char[imageLen];
								memset(binary, 0, imageLen);
								int j = 0;
								for (int k = 0; k < tempimage.size(); k++)
								{
									binary[j] = tempimage[k];
									j++;
								}
								char *temp = strstr(binary, "\n\n");
								cout << "bmp" << endl;
								images.push_back(&temp[2]);
								replace(ret, ret[i], "bmpimage");
							}
							else if ((ret[i].substr(ret[i].length() - 9, 3) == "jpg" || ret[i].substr(ret[i].length() - 9, 3) == "png"))
							{
								int imageLen = tempimage.size();
								char *binary = new char[imageLen];
								memset(binary, 0, imageLen);
								int j = 0;
								for (int k = 0; k < tempimage.size(); k++)
								{
									binary[j] = tempimage[k];
									j++;
								}
								if (ret[i].substr(ret[i].length() - 9, 3) == "png")
								{
									char *temp = strstr(binary, "PNG");
									temp = temp - 1;
									jpgimages.push_back(&temp[0]);
									replace(ret, ret[i], "pngimage");
								}
								else
								{
									char *temp;
									for (int k = 0; k < imageLen - 4; k++)
									{
										//4a 46 49 46 00 -> JFIF
										if (binary[k] == 0x4a && binary[k + 1] == 0x46 && binary[k + 2] == 0x49 && binary[k + 3] == 0x46)
										{
											temp = &binary[k];
											break;
										}
										//45 78 69 66 00 -> EFIF
										else if (binary[k] == 0x45 && binary[k + 1] == 0x78 && binary[k + 2] == 0x69 && binary[k + 3] == 0x66)
										{
											temp = &binary[k];
											break;
										}
									}
									//char *temp2 = strstr(binary, "JFIF");
									if (temp != NULL)
									{
										//char *temp3 = strstr(binary, "\n\n");
										temp = temp - 6;
										jpgimages.push_back(&temp[0]);
										//jpgimages.push_back(&temp3[2]);
										replace(ret, ret[i], "jpgimage");
									}
								}
								cout << "otherimg" << endl;
								jpgSize.push_back(imageLen);
							}
						}
					}
					else
					{
						ret.push_back("image failed");
					}
				}
			}
		}
	}
	else
	{
		ret.push_back("not valid");
	}
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