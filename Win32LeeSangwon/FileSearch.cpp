#include "FileSearch.h"

FileSearch::FileSearch(HWND main, RECT rt){
	_hwd = main;
	_rt = rt;
}

FileSearch::~FileSearch() { }

// Dll의 Search 함수 호출
void FileSearch::search(string name, int n)
{
	ICalculatorPtr obj;
	CoInitialize(NULL);
	try {
		obj.CreateInstance(__uuidof(FileSearcher));
		HRESULT hr = obj->doSearch((int)_hwd, NULL, name.c_str(), n);			
	}
	catch (_com_error& x) {
		cout << x.ErrorMessage() << endl;
	}
	CoUninitialize();
}