#include "MyServer.h"
#include "Authenticator.h"

using namespace std;
using namespace web;
using namespace utility;
using namespace http;
using namespace web::http::experimental::listener;

MyServer::MyServer(utility::string_t url, Authenticator* owner) : m_listener(url), mOwner(owner)
{
	m_listener.support(methods::GET, std::bind(&MyServer::handle_get, this, std::placeholders::_1));
}

void MyServer::handle_get(http_request message)
{
	message.reply(status_codes::OK);
	mOwner->receivedCallback(m_listener.uri());
};