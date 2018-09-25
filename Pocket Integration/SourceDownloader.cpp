#include "SourceDownloader.h"
#include <ostream>


SourceDownloader::SourceDownloader()
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
		}

		//CURLOPT_FOLLOWLOCATION - flag used to tell CURL whether it should follow locations. 1 means follow it everywhere
		curl_easy_setopt(curlHandle, CURLOPT_FOLLOWLOCATION, 1L);
		if (code != CURLE_OK)
		{
			fprintf(stderr, "Failed to set redirect option [%s]\n", errorBuffer);
		}

		//CURLOPT_WRITEFUNCTION - "Pass a pointer to your callback function, which gets called by libcurl as soon as there is data received that needs to be saved."
		//as callback I assigned special curlWriteToString function, which matches strictly specified prototype.
		curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, curlWriteToString);
		if (code != CURLE_OK)
		{
			fprintf(stderr, "Failed to set write function [%s]\n", errorBuffer);
		}

		//CURLOPT_WRITEDATA - "A data pointer to pass to the write callback. Fourth argument in write funtion (in our case std::string * s)"	All downloaded data will be saved into sourceCode variable
		curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &sourceCode);
		if (code != CURLE_OK)
		{
			fprintf(stderr, "Failed to set write variable [%s]\n", errorBuffer);
		}

		//connect to the remote site, do the necessary commands and receive the transfer. 
		curl_easy_perform(curlHandle);
		if (code != CURLE_OK)
		{
			fprintf(stderr, "Failed to easy perform [%s]\n", errorBuffer);
		}


		curl_easy_cleanup(curlHandle);
	}
}

void SourceDownloader::convertToXML()
{
	//tidy documentation is horrible (in my opinion...) and I found only one really usefull text - http://api.html-tidy.org/tidy/tidylib_api_5.6.0/libtidy_04.html which is more or less repeated below
	//tidy uses primitive system based on returning integers to indicate if function caused any errors. For simplicity I paste it below:
	/*	0 == Success, good to go.
		1 == Warnings, but no errors. Check the error buffer or track error messages for details.
		2 == Errors (and maybe warnings). By default, Tidy will not produce output. You can force output with the TidyForceOutput option. As with warnings, check the error buffer or track error messages for details.
		<0 == Severe error. Usually the value equals -errno. See errno.h.*/

	TidyBuffer errbuf = { 0 };
	int rc = -1;				//return code
	Bool ok;

	TidyDoc tidyDoc = tidyCreate();							//initialize "document"

	ok = tidyOptSetBool(tidyDoc, TidyXhtmlOut, yes);		//convert to XHTML
	
	if (ok)
		rc = tidySetErrorBuffer(tidyDoc, &errbuf);			//link tidy document to error buffer
	if (rc >= 0)
		rc = tidyParseString(tidyDoc, sourceCode.c_str());	//load source code and parse it
	if (rc >= 0)
		rc = tidyCleanAndRepair(tidyDoc);					//tidy it up!
	if (rc >= 0)
		rc = tidyRunDiagnostics(tidyDoc);					//reports the document type and diagnostic statistics on parsed and repaired markup. 

	if (rc > 1)												//if error, force output.
		rc = (tidyOptSetBool(tidyDoc, TidyForceOutput, yes) ? rc : -1);
	if (rc >= 0)
		rc = tidySaveBuffer(tidyDoc, &XMLdata);				//save output into XMLdata variable

	if (rc >= 0)
	{
		if (rc > 0)
		{
			/*std::ofstream myfile;
			myfile.open("output.txt");
			myfile << XMLdata.bp << "\n\n";*/
		}
		else
		{
			std::ofstream myfile;
			myfile.open("output.txt");
			myfile << "\nDiagnostics:\n\n%s" << errbuf.bp << "\n\n";
		}
	}
	else
		printf("A severe error (%d) occurred.\n", rc);

	tidyBufFree(&errbuf);
	tidyRelease(tidyDoc);
	sourceCode.clear();			//source code has been tidyied and converted into xml, so raw data can be removed
}

pugi::xml_node SourceDownloader::getData()
{
	document.load_buffer(XMLdata.bp, XMLdata.size);			//loading data into pugi document
	tidyBufFree(&XMLdata);									//removing xml data in tidy buffer
	return document;
}

std::vector<pugi::xml_node> SourceDownloader::selectData(std::string attributeName, std::string attributeValue)
{
	std::vector<pugi::xml_node> results;
	document.load_buffer(XMLdata.bp, XMLdata.size);			//loading data into pugi document
	tidyBufFree(&XMLdata);									//removing xml data in tidy buffer

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
   (void * contents,					//pointer to place where recieved data is
	size_t size,						//size of one data element
	size_t nmemb,						//number of elements which was recieved from site and passed to write function
	std::string * s)					//destination where recieved data will be stored
{
	size_t newLength = size*nmemb;		//space necessary to write new data
	size_t oldLength = s->size();		//currently used space

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
	
	return size*nmemb;					//return number of bytes which function could save (everything it recieved if try block succeed, less if something went wrong. In that case further transfer will be stopped)
}
