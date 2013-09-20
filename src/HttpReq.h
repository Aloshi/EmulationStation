#pragma once

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

//Based on: http://www.boost.org/doc/libs/1_51_0/doc/html/boost_asio/example/http/client/async_client.cpp

/* Usage:
 * HttpReq myRequest("www.google.com", "/index.html");
 * //for blocking behavior: while(myRequest.status() == HttpReq::REQ_IN_PROGRESS);
 * //for non-blocking behavior: check if(myRequest.status() != HttpReq::REQ_IN_PROGRESS) in some sort of update method
 * 
 * //once one of those completes, the request is ready
 * if(myRequest.status() != REQ_SUCCESS)
 * {
 *    //an error occured
 *    LOG(LogError) << "HTTP request error - " << myRequest.getErrorMessage();
 *    return;
 * }
 *
 * std::string content = myRequest.getContent();
 * //process contents...
*/

class HttpReq
{
public:
	HttpReq(const std::string& server, const std::string& path);
	HttpReq(const std::string& url);

	~HttpReq();

	enum Status
	{
		REQ_IN_PROGRESS,		//request is in progress
		REQ_SUCCESS,			//request completed successfully, get it with getContent()

		REQ_IO_ERROR,			//some boost::asio error happened, get it with getErrorMsg()
		REQ_BAD_STATUS_CODE,	//some invalid HTTP response status code happened (non-200)
		REQ_INVALID_RESPONSE	//the HTTP response was invalid
	};

	Status status(); //process any received data and return the status afterwards

	std::string getErrorMsg();

	std::string getContent();

private:
	static boost::asio::io_service io_service;

	void start(const std::string& server, const std::string& path);
	void handleResolve(const boost::system::error_code& err, tcp::resolver::iterator endpoint_iterator);
	void handleConnect(const boost::system::error_code& err);
	void handleWriteRequest(const boost::system::error_code& err);
	void handleReadStatusLine(const boost::system::error_code& err);
	void handleReadHeaders(const boost::system::error_code& err);
	void handleReadContent(const boost::system::error_code& err);

	void onError(const boost::system::error_code& error);

	tcp::resolver mResolver;
	tcp::socket mSocket;
	boost::asio::streambuf mRequest;
	boost::asio::streambuf mResponse;

	Status mStatus;
	std::stringstream mContent;
	unsigned int mResponseStatusCode;
	boost::system::error_code mError;
};
