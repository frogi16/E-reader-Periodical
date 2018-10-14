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
	/*m_listener.support(methods::PUT, std::bind(&MyServer::handle_put, this, std::placeholders::_1));
	m_listener.support(methods::POST, std::bind(&MyServer::handle_post, this, std::placeholders::_1));
	m_listener.support(methods::DEL, std::bind(&MyServer::handle_delete, this, std::placeholders::_1));*/
}

void MyServer::handle_get(http_request message)
{
	message.reply(status_codes::OK);
	mOwner->receivedCallback(m_listener.uri());
};

/*void MyServer::handle_post(http_request message)
{
	message.reply(status_codes::OK);
	mOwner->receivedCallback(m_listener.uri());

};

void MyServer::handle_delete(http_request message)
{
	message.reply(status_codes::OK);
	mOwner->receivedCallback(m_listener.uri());
}

void MyServer::handle_put(http_request message)
{
	ucout << message.to_string() << endl;

	auto paths = uri::split_path(uri::decode(message.relative_uri().path()));
	auto query = uri::split_query(uri::decode(message.relative_uri().query()));
	auto queryItr = query.find(U("request"));
	utility::string_t request = queryItr->second;
	ucout << request << endl;

	if (request == U("leave"))
	{
		Data data;
		data.job = U("Devs");
		People p1;
		p1.age = 10;
		p1.name = U("Franck");
		data.peoples.push_back(p1);
		People p2;
		p2.age = 20;
		p2.name = U("Joe");
		data.peoples.push_back(p2);

		utility::string_t response = data.AsJSON().serialize();
		ucout << response << endl;

		message.reply(status_codes::OK, data.AsJSON());
		return;
	}

	message.reply(status_codes::OK);
};*/
