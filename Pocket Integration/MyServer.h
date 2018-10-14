#pragma once

#include "cpprest/json.h"
#include "cpprest/http_listener.h"
#include "cpprest/uri.h"
#include "cpprest/asyncrt_utils.h"

using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;

class Authenticator;

class MyServer
{
public:
	MyServer() noexcept : mOwner(nullptr) {}
	MyServer(utility::string_t url, Authenticator*  owner);

	pplx::task<void> open() { return m_listener.open(); }
	pplx::task<void> close() { return m_listener.close(); }
private:
	Authenticator* mOwner;

	void handle_get(http_request message);			//this "server" is used only to receive callback, so methods other than get are unnecessary. 
	http_listener m_listener;
};
