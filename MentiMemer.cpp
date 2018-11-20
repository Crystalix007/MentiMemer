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
#include <thread>

using std::cout;
using std::endl;

std::string domainPre{ "https://www.menti.com" };
std::string gameIdUrl{ domainPre + "/f1759a04" };
std::string identiUrl{ domainPre + "/core/identifier" };
std::string voteUrl{   domainPre + "/core/vote" };

//std::string question{ "..." };
//std::string questionType{ "..." };
//std::string questionAns{ "..." };
std::string question{ "3b08cc04" };
std::string questionType{ "choices" };
std::string questionAns{ "1" };

size_t submissionCount = 100;
size_t threadCount = 100;

void doMultiSubmit();
bool doSubmit();

int main()
{
	std::vector<std::thread> threads{};

	for (size_t i  = 0; i < threadCount; i++)
		threads.push_back(std::thread{ doMultiSubmit });

	for (auto& thread : threads)
		thread.join();

	return 0;
}

void doMultiSubmit()
{
	for (size_t i = 0; i < submissionCount; i++)
		doSubmit();
}

bool doSubmit()
{
	//cout << "Starting identiRequest" << endl;
	Poco::URI identiURI{ identiUrl };
	std::string identiPath{ identiURI.getPathAndQuery() };

	Poco::Net::HTTPSClientSession identiSession{ identiURI.getHost(), identiURI.getPort() };
	Poco::Net::HTTPRequest identiRequest{ Poco::Net::HTTPRequest::HTTP_POST, identiPath, Poco::Net::HTTPMessage::HTTP_1_1 };
	
	identiRequest.add("Host",           "www.menti.com");
	identiRequest.add("User-Agent",     "Mozilla/5.0 (X11; Linux x86_64; rv:63.0) Gecko/20100101 Firefox/63.0");
	identiRequest.add("Accept",         "application/json");
	identiRequest.add("Referer",        gameIdUrl);
	identiRequest.add("Content-Type",   "application/json; charset=UTF-8");
	identiRequest.add("Content-Length", "0");

	identiSession.sendRequest(identiRequest);

	//cout << "Request data: " << endl;
	//identiRequest.write(cout);

	Poco::Net::HTTPResponse response{};
	std::istream& rs = identiSession.receiveResponse(response);

	//cout << "Got response: " << response.getStatus() << " - " << response.getReason() << endl;

	//cout << "Response stream: " << endl;
	std::string responseStr{};
	Poco::StreamCopier::copyToString(rs, responseStr);
	//cout << responseStr << endl << endl;
	std::string id;

	if (response.getStatus() == 200)
	{
		//Successfully obtained an id
		try
		{
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result = parser.parse(responseStr);
			Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
			Poco::Dynamic::Var identifier = object->get("identifier");
			id = identifier.convert<std::string>();
			//cout << "Got \'" << id << "\' as identifier\n" << endl;
			//identiSession.close();
			// Probably do want to close session somehow to reduce resource usage
		}
		catch (Poco::JSON::JSONException& jsonE)
		{
			cout << "JSON Error: " << jsonE.message() << endl;
			return false;
		}

		//Now submit a vote
		Poco::URI voteURI{ voteUrl };
		std::string votePath { voteURI.getPathAndQuery() };

		Poco::Net::HTTPSClientSession voteSession{ voteURI.getHost(), voteURI.getPort() };
		Poco::Net::HTTPRequest voteRequest{ Poco::Net::HTTPRequest::HTTP_POST, votePath, Poco::Net::HTTPMessage::HTTP_1_1 };

		voteRequest.add("Host",         "www.menti.com");
		voteRequest.add("User-Agent",   "Mozilla/5.0 (X11; Linux x86_64; rv:63.0) Gecko/20100101 Firefox/63.0");
		voteRequest.add("Accept",       "application/json");
		voteRequest.add("Referer",      gameIdUrl);
		voteRequest.add("Content-Type", "application/json; charset=UTF-8");
		voteRequest.add("Cookie",       "identifier1=" + id);
		voteRequest.add("X-Identifier", id);

		Poco::JSON::Object voteObj{};
		voteObj.set("question", question);
		voteObj.set("question_type", questionType);
		voteObj.set("vote", questionAns);

		std::stringstream voteObjSs{};
		std::string voteObjS{};
		voteObj.stringify(voteObjSs);
		Poco::StreamCopier::copyToString(voteObjSs, voteObjS);

		voteRequest.setContentLength(voteObjS.size());

		auto& bodyStream = voteSession.sendRequest(voteRequest);
		bodyStream << voteObjS;

		//cout << "Vote request: " << endl;
		//voteRequest.write(cout);
		//cout << endl;

		//cout << voteObjS << endl;

		Poco::Net::HTTPResponse voteResponse{};
		std::istream& rs2 = voteSession.receiveResponse(voteResponse);

		//cout << "Got response: " << voteResponse.getStatus() << " - " << voteResponse.getReason() << endl << endl;

		//cout << "Response stream:" << endl;
		//Poco::StreamCopier::copyStream(rs2, cout);

		return voteResponse.getStatus() == 200;
	}
}
