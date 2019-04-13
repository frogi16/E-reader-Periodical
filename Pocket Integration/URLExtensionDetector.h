#pragma once

#include <string>
#include <map>

#include "CurlWrapper.h"

class URLExtensionDetector
{
public:
	URLExtensionDetector();
	~URLExtensionDetector();

	std::string get(const std::string &link);
private:
	void configureCurlToHeaderDownloading(const std::string & url);

	std::string detectExtension(const std::string& header);
	std::string mimeFromJsonHeader(const std::string & header);
	std::string mimeFromHTMLHeader(const std::string & header);

	CurlWrapper curlWrapper;
	static const std::map<std::string, std::string> extensionsFromMIME;
};

