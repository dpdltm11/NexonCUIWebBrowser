#pragma once
#include "Uri.h"
#include <vector>

enum STATE {
	HELPSTATE,
	GOSTATE,
	REFRESHSTATE,
	BACKSTATE,
	FORWARDSTATE,
	HOMESTATE,
	LSSTATE,
	HLSSTATE,
	HGOSTATE,
	FILESTATE,
	NOTVALIDSTATE
};

class InputController
{
private:
	string userInput;
	string helpText;
	vector<string> uriList;
	int curIndex;
	string curUserHyperLinkText;
	string fileName;
	int threadNum;
public:
	InputController();
	STATE getState(const string&);
	inline vector<string> InputController::getURIList()	{ return uriList; }
	inline string InputController::getHelpText() { return helpText; }
	inline string InputController::getURI()	{ return uriList[curIndex];	}
	inline string InputController::getUserHyperLinkText() { return curUserHyperLinkText; }
	inline string InputController::getFileName() { return fileName; }
	inline int InputController::getthreadNum() { return threadNum; }
	void pushHyperLinkURI(const string&);
};