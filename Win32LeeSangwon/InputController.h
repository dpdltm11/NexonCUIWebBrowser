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
public:
	InputController();
	STATE getState(string);
	string getHelpText();
	string getURI(); 
	vector<string> getURIList();
	string getUserHyperLinkText();
	void pushHyperLinkURI(string);
};