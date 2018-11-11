#include "SourceDownloader.h"
#include <ostream>


SourceDownloader::SourceDownloader() noexcept
{
	curlWrapper.setFollowLocation(true);
	curlWrapper.setWritingToString();
}

void SourceDownloader::downloadSource(std::string url)
{
	curlWrapper.setURL(url);
	curlWrapper.perform();
}

pugi::xml_node SourceDownloader::getData()
{
	document.load_string(curlWrapper.getResponseString().c_str());
	return document;
}

SourceDownloader::~SourceDownloader()
{
}