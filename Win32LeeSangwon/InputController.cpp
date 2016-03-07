#include "InputController.h"

InputController::InputController() : curIndex(0), helpText("help/HELP:   help/HELP\n\n   사용할 수 있는 명령어를 보여줍니다.\n\ngo/GO:   go/GO [URI]\n\n   URI로 이동합니다.\n\nhgo/HGO:   hgo/HGO 하이퍼텍스트\n\n   하이퍼텍스트 링크로 이동합니다.\n\nrefresh/REFRESH:   refresh/REFRESH\n\n   새로고침\n\nb/B:   b/B\n\n   뒤로가기\n\nf/F:   f/F\n\n   앞으로 가기\n\nhome/HOME:   home/HOME\n\n   맨 처음화면으로 가기\n\nls/LS:   ls/LS\n\n   URI 로그 출력하기") {}

STATE InputController::getState(const string& usrSendString)
{	
	STATE curState = NOTVALIDSTATE;
	
	if (usrSendString.length() == 0)
	{
		return curState;
	}
	const string &input = usrSendString;
	if (input == "help" || input == "HELP")
	{
		curState = HELPSTATE;
	}
	else if (input.substr(0, 2) == "go" || input.substr(0, 4) == "GO")
	{
		typedef string::const_iterator iterator_t;

		iterator_t userInputEnd = input.end();

		// get query start
		iterator_t uriStart = find(input.begin(), userInputEnd, ' ');
		uriList.push_back(string(uriStart+1, userInputEnd));
		curIndex = uriList.size() - 1;
		curState = GOSTATE;
	}
	else if (input == "refresh" || input == "REFRESH")
	{
		curState = REFRESHSTATE;
	}
	else if (input == "b" || input == "B")
	{
		if (curIndex - 1 >= 0)
		{
			curIndex--;
		}
		curState = BACKSTATE;
	}
	else if (input == "f" || input == "F")
	{
		if (curIndex + 1 <= uriList.size() - 1)
		{
			curIndex++;
		}
		curState = FORWARDSTATE;
	}
	else if (input == "home" || input == "HOME")
	{
		curIndex = 0;
		curState = HOMESTATE;
	}
	else if (input == "ls" || input == "LS")
	{
		curState = LSSTATE;
	}
	else if (input == "hls" || input == "HLS")
	{
		curState = HLSSTATE;
	}
	else if (input.substr(0, 3) == "hgo" || input.substr(0, 3) == "HGO")
	{
		typedef string::const_iterator iterator_t;

		iterator_t userInputEnd = input.end();

		// get query start
		iterator_t uriStart = find(input.begin(), userInputEnd, ' ');
		//uriList.push_back(string(uriStart + 1, userInputEnd));
		curUserHyperLinkText = string(uriStart + 1, userInputEnd);
		curIndex = uriList.size() - 1;
		curState = HGOSTATE;
	}
	else
	{
		curState = NOTVALIDSTATE;
	}
	return curState;
}


void InputController::pushHyperLinkURI(const string& uri)
{
	uriList.push_back(uri);
	curIndex = uriList.size() - 1;
}