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
		std::cout << std::endl << "Encountered critical error, application will be closed." << std::endl << e.what();
	}

	return 0;
}