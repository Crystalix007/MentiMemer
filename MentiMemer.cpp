#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/DNS.h>
#include <Poco/URI.h>
#include <Poco/StreamCopier.h>
#include <Poco/JSON/JSON.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>
#include <iostream>
//#include <fstream>
#include <string>
#include <thread>

using std::cout;
using std::endl;

void doMultiSubmit();
bool doSubmit();
std::string getMentiIP();

std::string domainPre{ "https://www.menti.com" };
std::string gameIdUrl{ domainPre + "/" };
std::string identiUrl{ domainPre + "/core/identifier" };
std::string voteUrl{   domainPre + "/core/vote" };

std::string question;
std::string questionType;
std::string questionAns;

size_t submissionCount = 100;
size_t threadCount = 100;

int main(int argc, char** argv)
{
	//std::ofstream arguments{ "Arguments.txt" };
	//for (size_t i = 0; i <= argc; i++)
	//	arguments << argv[i] << endl;
	//arguments.close();

	if (argc != 5 && argc != 6)
	{
		cout << "Usage: " << argv[0] << " \"gameCode\" \"questionId\" \"questionType\" \"answer\" [threadCount]" << endl;
		return -1;
	}

	gameIdUrl += argv[1];
	question = argv[2];
	questionType = argv[3];
	questionAns = argv[4];

	if (argc == 6)
		threadCount = std::stoul(argv[5]);

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
	Poco::URI identiURI{ identiUrl };
	std::string identiPath{ identiURI.getPathAndQuery() };

	Poco::Net::HTTPSClientSession session{ identiURI.getHost(), identiURI.getPort() };

	try
	{
		Poco::Net::HTTPRequest identiRequest{ Poco::Net::HTTPRequest::HTTP_POST, identiPath, Poco::Net::HTTPMessage::HTTP_1_1 };
		
		identiRequest.add("Host",           "www.menti.com");
		identiRequest.add("Accept",         "application/json");
		identiRequest.add("Referer",        gameIdUrl);
		identiRequest.add("Content-Type",   "application/json; charset=UTF-8");
		identiRequest.setContentLength(0);

		session.sendRequest(identiRequest);

		Poco::Net::HTTPResponse response{};
		std::istream& rs = session.receiveResponse(response);

		std::string responseStr{};
		Poco::StreamCopier::copyToString(rs, responseStr);
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
			}
			catch (Poco::JSON::JSONException& jsonE)
			{
				cout << "JSON Error: " << jsonE.message() << endl;
				return false;
			}

			//Now submit a vote
			Poco::URI voteURI{ voteUrl };
			std::string votePath { voteURI.getPathAndQuery() };

			Poco::Net::HTTPRequest voteRequest{ Poco::Net::HTTPRequest::HTTP_POST, votePath, Poco::Net::HTTPMessage::HTTP_1_1 };

			voteRequest.add("Host",         "www.menti.com");
			voteRequest.add("Accept",       "application/json");
			voteRequest.add("Referer",      gameIdUrl);
			voteRequest.add("Content-Type", "application/json; charset=UTF-8");
			voteRequest.add("Cookie",       "identifier1=" + id);
			voteRequest.add("X-Identifier", id);

			std::string voteObjS{ "{\"question\":\"" + question + "\",\"question_type\":\"" + questionType + "\",\"vote\":\"" + questionAns + "\"}"};

			voteRequest.setContentLength(voteObjS.size());

			auto& bodyStream = session.sendRequest(voteRequest);
			bodyStream << voteObjS;

			Poco::Net::HTTPResponse voteResponse{};
			session.receiveResponse(voteResponse);
			return voteResponse.getStatus() == 200;
		}
		return false;
	}
	catch (Poco::Exception& e)
	{
		session.reset();
		return false;
	}
}
