#include "InputController.h"

InputController::InputController() : curIndex(0), helpText("help/HELP:   help/HELP\n\n   ����� �� �ִ� ��ɾ �����ݴϴ�.\n\n\ngo/GO:   go/GO [URI]\n\n   URI�� �̵��մϴ�.\n\n\nhgo/HGO:   hgo/HGO [�������ؽ�Ʈ]\n\n   �������ؽ�Ʈ ��ũ�� �̵��մϴ�.\n\n\nrefresh/REFRESH:   refresh/REFRESH\n\n   ���ΰ�ħ\n\n\nb/B:   b/B\n\n   �ڷΰ���\n\n\nf/F:   f/F\n\n   ������ ����\n\n\nhome/HOME:   home/HOME\n\n   �� ó��ȭ������ ����\n\n\nls/LS:   ls/LS\n\n   URI �α� ����ϱ�"), fileName(""), threadNum(0) {}

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
	else if (input.substr(0, 4) == "file" || input.substr(0, 4) == "FILE")
	{
		typedef string::const_iterator iterator_t;

		iterator_t userInputEnd = input.end();

		// get query start
		iterator_t fileNameStart = find(input.begin(), userInputEnd, ' ');
		//uriList.push_back(string(uriStart + 1, userInputEnd));
		iterator_t threadNumStart = find(fileNameStart + 1, userInputEnd, ' ');
		fileName = string(fileNameStart + 1, threadNumStart);	
		threadNum = stoi(string(threadNumStart + 1, userInputEnd));		
		curState = FILESTATE;
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