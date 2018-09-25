#include "Application.h"


#include "cpprest/json.h"
#include "cpprest/http_listener.h"
#include "cpprest/uri.h"
#include "cpprest/asyncrt_utils.h"
#include "cpprest/http_client.h"

using namespace http;
using namespace http::experimental::listener;



int main()
{
	Application app;
	app.run();

	return 0;
}