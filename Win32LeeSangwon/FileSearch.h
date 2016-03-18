#pragma once
#include <iostream>
#include <Windows.h>
#include <list>
using namespace std;

//#import "C:\Users\leesangwon0114\Documents\Visual Studio 2015\Projects\filesearcher\filesearcher\bin\Debug\filesearcher.tlb" no_namespace
#import "filesearcher.tlb" no_namespace

class FileSearch
{
	HWND _hwd;
	RECT _rt;
public:
	FileSearch(HWND main, RECT rt);
	~FileSearch();
	void search(string, int);
};