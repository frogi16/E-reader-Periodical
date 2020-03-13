#include "CurlWrapper.h"

#include <iostream>

CurlWrapper::CurlWrapper() : code(CURLE_OK), responseType(ResponseType::String)
{
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
}

void CurlWrapper::reset()
{
	resetSlist();
	curl_easy_reset(curl);
}

void CurlWrapper::setURL(const std::string& url)
{
	code = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	if (code != CURLE_OK)
	{
		throw std::exception((std::string("Failed to set URL ") + errorBuffer).c_str());
	}
}

void CurlWrapper::setPostFields(const std::string& parameters)
{
	mParameters = parameters;					//I spent lovely time trying to figure out why I were getting bad requests. Isolating bugged place wasn't easy, but
												//turns out that CURL doesn't copy parameters and they have to be in the same place in memory when performing connection.
												//Kids, read documentation.

	code = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, mParameters.c_str());
	if (code != CURLE_OK)
	{
		throw std::exception((std::string("Failed to set post fields ") + errorBuffer).c_str());
	}
}

void CurlWrapper::setFollowLocation(bool value)
{
	//CURLOPT_FOLLOWLOCATION - flag used to tell CURL whether it should follow locations. 1 means follow it everywhere
	code = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, value);
	if (code != CURLE_OK)
	{
		throw std::exception((std::string("Failed to set follow location ") + errorBuffer).c_str());
	}
}

void CurlWrapper::setWritingToString()
{
	//CURLOPT_WRITEFUNCTION - "Pass a pointer to your callback function, which gets called by libcurl as soon as there is data received that needs to be saved."
	//as callback I assigned special curlWriteToString function, which matches strictly specified prototype.
	code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteToString);
	if (code != CURLE_OK)
	{
		throw std::exception((std::string("Failed to set write function ") + errorBuffer).c_str());
	}

	setResponseString();
}

void CurlWrapper::setWritingToFile(std::filesystem::path& pathToImage)
{
	errno_t err = fopen_s(&fp, pathToImage.generic_string().c_str(), "wb");

	if (fp == NULL)
	{
		throw std::exception((std::string("File cannot be opened ") + errorBuffer).c_str());
	}
	else
	{
		code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
		if (code != CURLE_OK)
		{
			throw std::exception((std::string("Failed to unset write function ") + errorBuffer).c_str());
		}

		code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		if (code != CURLE_OK)
		{
			throw std::exception((std::string("Failed to set file as write location ") + errorBuffer).c_str());
		}

		responseType = ResponseType::File;
	}
}

void CurlWrapper::setNoBody(bool value)
{
	code = curl_easy_setopt(curl, CURLOPT_NOBODY, value);
	if (code != CURLE_OK)
	{
		throw std::exception((std::string("Failed to set nobody ") + errorBuffer).c_str());
	}
}

void CurlWrapper::setHeaderOnly(bool value)
{
	code = curl_easy_setopt(curl, CURLOPT_HEADER, value);
	if (code != CURLE_OK)
	{
		throw std::exception((std::string("Failed to set header mode ") + errorBuffer).c_str());
	}
}

void CurlWrapper::setPassword(const std::string& password)
{
	code = curl_easy_setopt(curl, CURLOPT_HEADER, password);
	if (code != CURLE_OK)
	{
		throw std::exception((std::string("Failed to set password ") + errorBuffer).c_str());
	}
}

void CurlWrapper::resetSlist()
{
	curl_slist_free_all(slist);
	slist = NULL;
}

void CurlWrapper::addToSlist(std::string value)
{
	slist = curl_slist_append(slist, value.c_str());
}

void CurlWrapper::perform()
{
	if (slist != NULL)
		setHeaderList();

	if (responseType == ResponseType::String)
		responseString.clear();

	//connect to the remote site, do the necessary commands and receive the transfer. 
	code = curl_easy_perform(curl);
	if (code != CURLE_OK)
	{
		throw std::exception((std::string("Failed to easy perform ") + errorBuffer).c_str());
	}

	if (responseType == ResponseType::File)
		fclose(fp);
}

size_t CurlWrapper::curlWriteToString
(void* contents,								//pointer to place where recieved data is located
	size_t size,								//size of one data element
	size_t nmemb,								//number of elements which were recieved from site and passed to write function
	std::string* s)							//destination where recieved data will be stored
{
	size_t newLength = size * nmemb;			//space necessary to write new data
	size_t oldLength = s->size();				//currently used space


	//so, here should be code responsible for resizing string and handling all errors, but there is really nothing we can do with memory
	//allocation errors beside repeating resize few more times.

	int count = 0;
	int maxTries = 3;							//max number of tries to resize buffer string. TODO: consider exposing maxTries variable via interface.

	while (true)
	{
		try
		{
			s->resize(oldLength + newLength);
			break;
		}
		catch (std::bad_alloc & e)
		{
			if (++count >= maxTries) throw e;	//if repeating doesn't help, just rethrow the error
		}
	}

	std::copy((char*)contents, (char*)contents + newLength, s->begin() + oldLength);		//arguments: 
																							//1) source beginning
																							//2) source end
																							//3) destination beginning

	return size * nmemb;						//return number of bytes which function could save (everything it recieved if try block succeed, less if something went wrong. In that case further transfer will be stopped)
}

void CurlWrapper::setResponseString()
{
	//CURLOPT_WRITEDATA - "A data pointer to pass to the write callback. Fourth argument in write funtion (in our case std::string * s)"
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);
	if (code != CURLE_OK)
	{
		throw std::exception((std::string("Failed to set writing to string ") + errorBuffer).c_str());
	}
	responseType = ResponseType::String;
}

void CurlWrapper::setHeaderList()
{
	code = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
	if (code != CURLE_OK)
	{
		throw std::exception((std::string("Failed to set http header list ") + errorBuffer).c_str());
	}
}

CurlWrapper::~CurlWrapper()
{
	if (fp != NULL)
		fclose(fp);

	curl_easy_cleanup(curl);
}