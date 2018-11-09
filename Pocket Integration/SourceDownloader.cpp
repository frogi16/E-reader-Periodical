#include "SourceDownloader.h"
#include <ostream>


SourceDownloader::SourceDownloader() noexcept
{
}

void SourceDownloader::downloadSource(std::string url)
{
	CURL *curlHandle = curl_easy_init();											//handle to easy mode session
	CURLcode code;
	char errorBuffer[CURL_ERROR_SIZE];

	if (curlHandle)
	{
		code = curl_easy_setopt(curlHandle, CURLOPT_URL, url.c_str());				//setopt is used to set properties and options (in this case URL of scrapped website)
		if (code != CURLE_OK)														//error checks, similar are repeated below
		{
			fprintf(stderr, "Failed to set URL [%s]\n", errorBuffer);
			throw std::exception((std::string("Failed to set URL ") + errorBuffer).c_str());
		}

		//CURLOPT_FOLLOWLOCATION - flag used to tell CURL whether it should follow locations. 1 means follow it everywhere
		curl_easy_setopt(curlHandle, CURLOPT_FOLLOWLOCATION, 1L);
		if (code != CURLE_OK)
		{
			fprintf(stderr, "Failed to set redirect option [%s]\n", errorBuffer);
			throw std::exception((std::string("Failed to set redirect option ") + errorBuffer).c_str());
		}

		//CURLOPT_WRITEFUNCTION - "Pass a pointer to your callback function, which gets called by libcurl as soon as there is data received that needs to be saved."
		//as callback I assigned special curlWriteToString function, which matches strictly specified prototype.
		curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, curlWriteToString);
		if (code != CURLE_OK)
		{
			fprintf(stderr, "Failed to set write function [%s]\n", errorBuffer);
			throw std::exception((std::string("Failed to set write function ") + errorBuffer).c_str());
		}

		//CURLOPT_WRITEDATA - "A data pointer to pass to the write callback. Fourth argument in write funtion (in our case std::string * s)"	All downloaded data will be saved into sourceCode variable
		curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &sourceCode);
		if (code != CURLE_OK)
		{
			fprintf(stderr, "Failed to set write variable [%s]\n", errorBuffer);
			throw std::exception((std::string("Failed to set write variable ") + errorBuffer).c_str());
		}

		//connect to the remote site, do the necessary commands and receive the transfer. 
		curl_easy_perform(curlHandle);
		if (code != CURLE_OK)
		{
			fprintf(stderr, "Failed to easy perform [%s]\n", errorBuffer);
			throw std::exception((std::string("Failed to easy perform ") + errorBuffer).c_str());
		}

		curl_easy_cleanup(curlHandle);
	}
}

pugi::xml_node SourceDownloader::getData()
{
	document.load_string(sourceCode.c_str());
	sourceCode.clear();
	return document;
}

std::vector<pugi::xml_node> SourceDownloader::selectData(std::string attributeName, std::string attributeValue)
{
	std::vector<pugi::xml_node> results;
	document.load_string(sourceCode.c_str());																//loading data into pugi document

	std::queue<pugi::xml_node> childs;
	childs.push(document);

	while (!childs.empty())
	{
		pugi::xml_node currentNode = childs.front();

		for (pugi::xml_node i = currentNode.first_child(); i; i = i.next_sibling())							//iterate through all children
		{
			childs.push(i);
			for (pugi::xml_attribute attr = i.first_attribute(); attr; attr = attr.next_attribute())		//and all of their atributes
			{
				if (!attributeName.empty() &&																//if any of atributes matches searched name
					std::string(attr.name()) == attributeName)					
				{
					if (!attributeValue.empty())
					{
						if(std::string(attr.as_string()) == attributeValue)									//and value, we found appropriate node
							results.push_back(i);
					}
				}
			}
		}

		childs.pop();
	}

	return std::move(results);
}


SourceDownloader::~SourceDownloader()
{
}

size_t SourceDownloader::curlWriteToString
   (void * contents,							//pointer to place where recieved data is
	size_t size,								//size of one data element
	size_t nmemb,								//number of elements which was recieved from site and passed to write function
	std::string * s)							//destination where recieved data will be stored
{
	size_t newLength = size*nmemb;				//space necessary to write new data
	size_t oldLength = s->size();				//currently used space

	try
	{
		//try resizing
		s->resize(oldLength + newLength);
	}
	catch (std::bad_alloc &e)
	{
		//TODO: handle memory problem
		return 0;
	}

	std::copy((char*)contents, (char*)contents + newLength, s->begin() + oldLength);		//arguments: 
																							//1) source beginning
																							//2) source end
																							//3) destination beginning
	
	return size*nmemb;							//return number of bytes which function could save (everything it recieved if try block succeed, less if something went wrong. In that case further transfer will be stopped)
}
