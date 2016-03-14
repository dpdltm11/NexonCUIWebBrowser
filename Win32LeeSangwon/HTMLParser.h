#pragma once

#include <iostream>
#include<string>
#include<string.h>
#include <algorithm>
#include <sstream>
#include <vector>
#include <unordered_map>

using namespace std;

class Node {
public:
	Node(string name) : tagName(name), tagContent("") {};
	inline string& getTagName() { return tagName; }
	inline string& getTagContent() { return tagContent; }
	inline vector<Node*>& getFriendNode() { return friendNode; }
	void insertNode(Node* cur) { friendNode.push_back(cur); }
	void setTagContent(string s) { tagContent = s; }
private:
	string tagName;
	string tagContent;
	vector<Node*> friendNode;
};

// html parsing을 위한 클래스
class HTMLParser
{
private:
	string html, title, h1;
	vector<string> result;
	vector<string> keyHyperLink;
	unordered_map<string, string> hyperLink;
	Node *head;
public:
	HTMLParser(string);
	string removeTag(string,string, int);
	void testParse(const string &);
	void recursion(const string &, const string &, Node*);
	void moreParse(const string &);
	string getTag(string tag);
	inline vector<string> HTMLParser::getResult() { return result; }
	inline vector<string> HTMLParser::getkeyHyperLink() { return keyHyperLink; }
	inline unordered_map<string, string> HTMLParser::getHyperLink() { return hyperLink; }
};