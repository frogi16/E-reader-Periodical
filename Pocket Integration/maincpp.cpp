#include "Application.h"

int main()
{
	try
	{
		Application app;
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cout << std::endl << "CRITICAL ERROR. APPLICATION WILL BE CLOSED!" << std::endl << e.what();
	}

	return 0;
}