#include "HTMLParser.h"

string replaceAll(const string &str, const string &pattern, const string &replace)
{
	string result = str;
	string::size_type pos = 0;
	string::size_type offset = 0;

	while ((pos = result.find(pattern, offset)) != string::npos)
	{
		result.replace(result.begin() + pos, result.begin() + pos + pattern.size(), replace);
		offset = pos + replace.size();
	}

	return result;
}

vector<string> &htmlsplit(const string &s, char delim, vector<string> &elems) {
	stringstream ss(s);
	string item;
	while (getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

vector<string> htmlsplit(const string &s, char delim) {
	vector<string> elems;
	htmlsplit(s, delim, elems);
	return elems;
}

HTMLParser::HTMLParser(string text)
{
	//test1
	//text = "<html><head><title>response</title></head><body><h1>Hello Web Server!</h1></body></html>";
	//test2
	//string url = "\"http://www.codeproject.com/KB/GDI-plus/ImageProcessing2/img.jpg\"" ;

	//string url = " \"http://htmlfive.co.kr/html5/xe/layouts/sketchbook5/img/chrome.png \" ";
	//text = "<html><head><title>HTML5 ¿ÀÇÂ Ä¿¹Â´ÏÆ¼</title></head><body><h1>HTML5 ¿ÀÇÂ Ä¿¹Â´ÏÆ¼</h1><p>ÀÌ ÀÚ·á´Â HTML5 ¿ÀÇÂ Ä¿¹Â´ÏÆ¼ ¿¡¼­ ¹èÆ÷ÇÕ´Ï´Ù.</p> <img src =" + url + "></body></html>";

	html = text;
	//title = removeTag("title");
	//h1 = removeTag("h1");
	moreParse(text);
}

void HTMLParser::moreParse(const string &resource)
{
	typedef string::const_iterator iterator_t;

	if (resource.length() == 0)
		return;

	iterator_t htmlEnd = resource.end();

	iterator_t parseStart = find(resource.begin(), htmlEnd, '<');

	iterator_t current = parseStart;

	while (current != htmlEnd)
	{
		iterator_t tagStart = find(current, htmlEnd, '<');
		iterator_t tagEnd = find(tagStart, htmlEnd, '>');            //"://");
		string tag(tagStart, tagEnd);
		tag = tag.erase(0, 1);
		//PARSESTATE curState = validtag(tag);
		string tagname = getTag(tag);
		//int len = tag.length() - tagname.length() - 1;
		if (tagname == "img")
		{
			string src(strstr(tag.c_str(), "src"));
			vector<string> tempSplit = htmlsplit(src, ' ');
			src = tempSplit[0].substr(5, tempSplit[0].size() - 5) + "image";
			result.push_back(src);
			current = tagEnd;
		}
		else if(tagname == "a")
		{ 
			const char *v = strstr(tag.c_str(), "href");
			if (v != NULL)
			{
				string link(strstr(tag.c_str(), "href"));
				vector<string> tempSplit = htmlsplit(link, ' ');
				link = tempSplit[0].substr(6, tempSplit[0].size() - 6);
				link = link.erase(link.length() - 1, 1);
				string temp(current, htmlEnd);
				string hyperText = removeTag(temp, tagname, tag.length());// + "hyperText";
				hyperText = replaceAll(hyperText, "<span>", "");
				hyperText = replaceAll(hyperText, "</span>", "");
				pair<string, string> p(hyperText, link);
				hyperLink.insert(p);
				result.push_back(hyperText + "hyperText");
				current = tagEnd;
			}
			else
				current = tagStart + 1;
		}
		else
		{
			if (tagname != "none")
			{
				string temp(current, htmlEnd);
				temp = removeTag(temp, tagname, tag.length()) + tagname;
				temp = replaceAll(temp, "<br>", "\n");
				temp = replaceAll(temp, "</br>", "\n");
				temp = replaceAll(temp, "<span>", "");
				temp = replaceAll(temp, "</span>", "");
				result.push_back(temp);
			}
			current = tagEnd;
		}
	}

	cout << "here" << endl;
}

PARSESTATE HTMLParser::validtag(string tag)
{
	if (tag.substr(0, 5) == "title" || tag.substr(0, 2) == "h1" || tag.substr(0, 2) == "h2" || tag.substr(0, 2) == "h3" || tag.substr(0, 2) == "h4" || tag.substr(0, 2) == "h5" || tag.substr(0, 2) == "h6" || tag.substr(0, 1) == "p" || tag.substr(0, 6) == "center")
	{
		return TEXTPARSE;
	}
	else if (tag.substr(0, 3) == "img")
	{
		return IMAGEPARSE;
	}
	return NONPARSE;
}

string HTMLParser::getTag(string tag)
{
	string htag = tag.substr(0, 2);
	if (htag == "h1" || htag == "h2" || htag == "h3" || htag == "h4" || htag == "h5" || htag == "h6")
	{
		return htag;
	}
	else if (tag.substr(0, 5) == "title")
	{
		return "title";
	}
	else if (tag.substr(0, 1) == "p")
	{
		return "p";
	}
	else if (tag.substr(0, 2) == "a ")
	{
		return "a";
	}
	else if (tag.substr(0, 6) == "center")
	{
		return "center";
	}
	else if (tag.substr(0, 3) == "img")
	{
		return "img";
	}
	else
		return "none";
}

string HTMLParser::removeTag(string tag, string tagname, int len)
{
	string temp = "";
	//temp = html.substr((html.find("<" + tag) + tag.length() + 1 ), (html.find("</" + tag + ">")) - (html.find("<" + tag) + tag.length() + 1));
	temp = tag.substr((tag.find("<" + tagname) + len + 1), (tag.find("</" + tagname + ">")) - (tag.find("<" + tagname) + len + 1));
	//temp = html.substr(html.find(">"), (html.find("</" + tag + ">")) - (html.find(">")));
	temp = strstr(temp.c_str(), ">");
	temp = temp.erase(0, 1);
	return temp;
}

string HTMLParser::getTitle()
{
	return title;
}

string HTMLParser::geth1()
{
	return h1;
}

vector<string> HTMLParser::getResult()
{
	return result;
}

vector<string> HTMLParser::getkeyHyperLink()
{
	return keyHyperLink;
}

unordered_map<string, string> HTMLParser::getHyperLink()
{
	return hyperLink;
}