#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/URI.h>
#include <Poco/StreamCopier.h>
#include <Poco/JSON/JSON.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>
#include <iostream>
#include <string>

using std::cout;
using std::endl;

std::string domainPre{ "https://www.menti.com" };
std::string gameIdUrl{ domainPre + "/f1759a04" };
std::string identiUrl{ domainPre + "/core/identifier" };

int main()
{
	cout << "Starting request" << endl;
	Poco::URI uri{ identiUrl };
	std::string path{ uri.getPathAndQuery() };

	Poco::Net::HTTPSClientSession session{ uri.getHost(), uri.getPort() };
	Poco::Net::HTTPRequest request{ Poco::Net::HTTPRequest::HTTP_POST, path, Poco::Net::HTTPMessage::HTTP_1_1 };
	
	request.add("Host",           "www.menti.com");
	request.add("User-Agent",     "Mozilla/5.0 (X11; Linux x86_64; rv:63.0) Gecko/20100101 Firefox/63.0");
	request.add("Accept",         "application/json");
	request.add("Referer",        gameIdUrl);
	request.add("Content-Type",   "application/json; charset=UTF-8");
	request.add("Content-Length", "0");

	session.sendRequest(request);

	cout << "Request data: " << endl;
	request.write(cout);

	Poco::Net::HTTPResponse response{};
	std::istream& rs = session.receiveResponse(response);

	cout << "Got response: " << response.getStatus() << " - " << response.getReason() << endl;

	cout << "Response stream: " << endl;
	std::string responseStr{};
	Poco::StreamCopier::copyToString(rs, responseStr);
	cout << responseStr << endl << endl;

	if (response.getStatus() == 200)
	{
		//Successfully obtained an id
		try
		{
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result = parser.parse(responseStr);
			Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
			Poco::Dynamic::Var identifier = object->get("identifier");
			cout << "Got \'" << identifier.convert<std::string>() << "\' as identifier" << endl;
		}
		catch (Poco::JSON::JSONException jsonE)
		{
			cout << "JSON Error: " << jsonE.message() << endl;
			return -1;
		}
	}

	return 0;
}
