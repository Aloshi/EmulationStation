#include <iostream>
#include "HttpReq.h"
#include <boost/bind.hpp>
#include "Log.h"

boost::asio::io_service HttpReq::io_service;

std::string HttpReq::urlEncode(const std::string &s)
{
    const std::string unreserved = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";

    std::string escaped="";
    for(size_t i=0; i<s.length(); i++)
    {
        if (unreserved.find_first_of(s[i]) != std::string::npos)
        {
            escaped.push_back(s[i]);
        }
        else
        {
            escaped.append("%");
            char buf[3];
            sprintf(buf, "%.2X", s[i]);
            escaped.append(buf);
        }
    }
    return escaped;
}

HttpReq::HttpReq(const std::string& server, const std::string& path)
	: mResolver(io_service), mSocket(io_service), mStatus(REQ_IN_PROGRESS)
{
	start(server, path);
}

HttpReq::HttpReq(const std::string& url)
	: mResolver(io_service), mSocket(io_service), mStatus(REQ_IN_PROGRESS)
{
	size_t startpos = 0;

	if(url.substr(startpos, 7) == "http://")
		startpos = 7;
	else if(url.substr(0, 8) == "https://")
		startpos = 8;

	if(url.substr(startpos, 4) == "www.")
		startpos += 4;

	size_t pathStart = url.find('/', startpos);
	std::string server = url.substr(startpos, pathStart - startpos);
	std::string path = url.substr(pathStart, std::string::npos);
	
	start(server, path);
}

HttpReq::~HttpReq()
{
	mResolver.cancel();
	mSocket.close();
	while(status() == REQ_IN_PROGRESS); //otherwise you get really weird heap-allocation-related crashes
}

void HttpReq::start(const std::string& server, const std::string& path)
{
	std::ostream req_str(&mRequest);
	req_str << "GET " << path << " HTTP/1.0\r\n";
	req_str << "Host: " << server << "\r\n";
	req_str << "Accept: */*\r\n";
	req_str << "Connection: close\r\n\r\n";

	tcp::resolver::query query(server, "http");
	mResolver.async_resolve(query,
		boost::bind(&HttpReq::handleResolve, this, 
			boost::asio::placeholders::error,
			boost::asio::placeholders::iterator));
}

void HttpReq::handleResolve(const boost::system::error_code& err, tcp::resolver::iterator endpoint_iterator)
{
	if (!err)
	{
		// Attempt a connection to each endpoint in the list until we
		// successfully establish a connection.
		boost::asio::async_connect(mSocket, endpoint_iterator,
			boost::bind(&HttpReq::handleConnect, this,
				boost::asio::placeholders::error));
	}
	else
	{
		onError(err);
	}
}

void HttpReq::handleConnect(const boost::system::error_code& err)
{
	if (!err)
	{
		// The connection was successful. Send the request.
		boost::asio::async_write(mSocket, mRequest,
			boost::bind(&HttpReq::handleWriteRequest, this,
			boost::asio::placeholders::error));
	}
	else
	{
		onError(err);
	}
}

void HttpReq::handleWriteRequest(const boost::system::error_code& err)
{
	if (!err)
	{
		// Read the response status line. The response_ streambuf will
		// automatically grow to accommodate the entire line. The growth may be
		// limited by passing a maximum size to the streambuf constructor.
		boost::asio::async_read_until(mSocket, mResponse, "\r\n",
			boost::bind(&HttpReq::handleReadStatusLine, this,
			boost::asio::placeholders::error));
	}
	else
	{
		onError(err);
	}
}

void HttpReq::handleReadStatusLine(const boost::system::error_code& err)
{
	if (!err)
	{
		// Check that response is OK.
		std::istream response_stream(&mResponse);
		std::string http_version;
		response_stream >> http_version;
		response_stream >> mResponseStatusCode;
		std::string status_message;
		std::getline(response_stream, status_message);
		if(!response_stream || http_version.substr(0, 5) != "HTTP/")
		{
			mStatus = REQ_INVALID_RESPONSE;
			return;
		}
		if(mResponseStatusCode != 200)
		{
			mStatus = REQ_BAD_STATUS_CODE;
			return;
		}

		// Read the response headers, which are terminated by a blank line.
		boost::asio::async_read_until(mSocket, mResponse, "\r\n\r\n",
			boost::bind(&HttpReq::handleReadHeaders, this,
				boost::asio::placeholders::error));
	}
	else
	{
		onError(err);
	}
}

void HttpReq::handleReadHeaders(const boost::system::error_code& err)
{
	if (!err)
	{
		// Process the response headers.
		std::istream response_stream(&mResponse);
		std::string header;
		while (std::getline(response_stream, header) && header != "\r"); //and by process we mean ignore

		// Write whatever content we already have to output.
		if (mResponse.size() > 0)
			mContent << &mResponse;
		
		// Start reading remaining data until EOF.
		boost::asio::async_read(mSocket, mResponse,
			boost::asio::transfer_at_least(1),
			boost::bind(&HttpReq::handleReadContent, this,
				boost::asio::placeholders::error));
	}
	else
	{
		onError(err);
	}
}

void HttpReq::handleReadContent(const boost::system::error_code& err)
{
	if (!err)
	{
		// Write all of the data that has been read so far.
		mContent << &mResponse;

		// Continue reading remaining data until EOF.
		boost::asio::async_read(mSocket, mResponse,
			boost::asio::transfer_at_least(1),
			boost::bind(&HttpReq::handleReadContent, this,
				boost::asio::placeholders::error));
	}else{
		if (err != boost::asio::error::eof)
		{
			onError(err);
		}else{
			mStatus = REQ_SUCCESS;
		}
	}
}

HttpReq::Status HttpReq::status()
{
	io_service.poll();
	return mStatus;
}

std::string HttpReq::getContent()
{
	if(mStatus != REQ_SUCCESS)
	{
		LOG(LogError) << "Called getContent() on an unsuccessful HttpReq!";
		return "";
	}

	return mContent.str();
}

//only called for boost-level errors (REQ_IO_ERROR)
void HttpReq::onError(const boost::system::error_code& err)
{
	mError = err;
	mStatus = REQ_IO_ERROR;
}

std::string HttpReq::getErrorMsg()
{
	switch(mStatus)
	{
	case REQ_BAD_STATUS_CODE:
		return "Bad status code";
	case REQ_INVALID_RESPONSE:
		return "Invalid response from server";
	case REQ_IO_ERROR:
		return mError.message();
	case REQ_IN_PROGRESS:
		return "Not done yet";
	case REQ_SUCCESS:
		return "No error";
	default:
		return "???";
	}
}
