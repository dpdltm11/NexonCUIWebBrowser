#include "Uri.h"

Uri::Uri()
{
	QueryString = Path = Protocol = Host = Port = "";
}

string Uri::getPath()
{
	return Path;
}

string Uri::getProtocol()
{
	return Protocol;
}

string Uri::getHost()
{
	return Host;
}

string Uri::getPort()
{
	return Port;
}

Uri Uri::Parse(const string &uri)
{
	Uri result;

	typedef string::const_iterator iterator_t;

	if (uri.length() == 0)
		return result;

	iterator_t uriEnd = uri.end();

	// get query start
	iterator_t queryStart = find(uri.begin(), uriEnd, '?');

	// protocol
	iterator_t protocolStart = uri.begin();
	iterator_t protocolEnd = find(protocolStart, uriEnd, ':');            //"://");

	if (protocolEnd != uriEnd)
	{
		string prot = &*(protocolEnd);
		if ((prot.length() > 3) && (prot.substr(0, 3) == "://"))
		{
			result.Protocol = string(protocolStart, protocolEnd);
			protocolEnd += 3;   //      ://
		}
		else
			protocolEnd = uri.begin();  // no protocol
	}
	else
		protocolEnd = uri.begin();  // no protocol

									// host
	iterator_t hostStart = protocolEnd;
	iterator_t pathStart = find(hostStart, uriEnd, '/');  // get pathStart

	iterator_t hostEnd = find(protocolEnd,
		(pathStart != uriEnd) ? pathStart : queryStart, ':');  // check for port

	result.Host = string(hostStart, hostEnd);

	// port
	if ((hostEnd != uriEnd) && ((&*(hostEnd))[0] == ':'))  // we have a port
	{
		hostEnd++;
		iterator_t portEnd = (pathStart != uriEnd) ? pathStart : queryStart;
		result.Port = string(hostEnd, portEnd);
	}

	// path
	if (pathStart != uriEnd)
		result.Path = string(pathStart, queryStart);

	// query
	if (queryStart != uriEnd)
		result.QueryString = string(queryStart, uri.end());

	return result;
}   // Parse