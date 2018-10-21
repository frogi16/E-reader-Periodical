#include "EbookCreator.h"

#include <fstream>
#include <string>
#include <filesystem>

#include "pugixml.hpp"
#include "tidy.h"
#include "tidybuffio.h"
#include "EbookCreator.h"

namespace fs = std::experimental::filesystem;

EbookCreator::EbookCreator()
{
}


EbookCreator::~EbookCreator()
{
}

void EbookCreator::createEpub(std::vector<ParsedArticle>& articles)
{
	prepareDirectory();
	appendData(articles);
}

void EbookCreator::prepareDirectory()
{
	fs::remove_all(fs::path("book"));
	fs::remove(fs::path("book.mobi"));
	fs::create_directory(fs::path("book"));
	fs::copy(fs::path("template"), fs::path("book"), fs::copy_options::recursive);
}

void EbookCreator::appendData(std::vector<ParsedArticle>& articles)
{
	articleIndex = 0;
	imageIndex = 0;

	//Building table of content. Unfortunately each article must be listed in the manifest and in the spine.
	//Additionally, it is very important to set references to images in the articles and in the manifest exactly the same, so it will be much easier and cleaner to do article after article
	for (auto& article : articles)
	{
		addToManifest(article);
		saveArticle(article);														//save each article as "article[number].xhtml" - article0.xhtml, article1.xhtml etc.
		++articleIndex;
	}

	addSeparationBetweenManifestAndSpine();
	addToSpine(articles);

	buildTableOfContent(articles);
}

void EbookCreator::saveArticle(ParsedArticle & article)
{
	std::ofstream file;
	std::string articleName = "article" + std::to_string(articleIndex) + ".xhtml";
	file.open("book/OEBPS/Text/" + articleName, std::ios::app);									//create the article file

	auto body = currentDocument.first_child().child("body");
	body.prepend_child("hr");																	//add horizontal line separating header and content to xml
	body.prepend_child("i").append_child(pugi::node_pcdata).set_value(article.domain.c_str());	//add domain to xml
	body.prepend_child("h1").append_child(pugi::node_pcdata).set_value(article.title.c_str());	//add title	to xml		
	currentDocument.save(file);																	//save xml content to file
	file.close();
}

void EbookCreator::addToManifest(ParsedArticle & article)
{
	std::ofstream file;

	file.open("book/OEBPS/content.opf", std::ios::app);								//beginning of this file is constant for all books so I just copy it from template directory and append new things

	std::string articleName = "article" + std::to_string(articleIndex) + ".xhtml";
	file << "<item id=" << '"' << articleName << '"' << " href=" << '"' << "Text/" << articleName << '"' << " media-type=" << '"' << "application/xhtml+xml" << '"' << "/>";	//adding the article itself

	tidyAndConvertToXhtml(article);													//most sites has multiple errors and markups don't match, so tidying is necessary. It also loads tidied string to pugi xml object
	auto images = scraper.selectDataByName(currentDocument, std::string("img"));	//find all images the article links to, because they also have to be listed in manifest

	for (auto& image : images)
	{
		std::string link = image.attribute("src").as_string();						//extract link to image
		std::string extension = ".jpg";												//extension of image, by default is set to ".jpg"

		size_t positionOfDot = link.find_last_of('.');								//position of last dot in link. After this the only thing is an extension
		if (positionOfDot != std::string::npos)										//if something was found
		{
			extension = link.substr(positionOfDot, 4);								//copy characters from dot to the end of string
		}

		auto httpPosition = link.rfind("http");
		std::string linkToImage = link.substr(httpPosition, positionOfDot + 4 - httpPosition);	//quick, dirty HACK! Found out that nodes like this:
																								//<img class="alignnone size-large wp-image-35164" src="https://zaufanatrzeciastrona.pl/wp-content/uploads/2018/10/indonesia_0-580x384.jpg"
																								//alt="" width="580" height="384" srcset="https://zaufanatrzeciastrona.pl/wp-content/uploads/2018/10/indonesia_0-580x384.jpg 580w, 
																								//https://zaufanatrzeciastrona.pl/wp-content/uploads/2018/10/indonesia_0-300x199.jpg 300w,
																								//https://zaufanatrzeciastrona.pl/wp-content/uploads/2018/10/indonesia_0-768x509.jpg 768w,
																								//https://zaufanatrzeciastrona.pl/wp-content/uploads/2018/10/indonesia_0.jpg 1260w"
																								//sizes="(max-width: 580px) 100vw, 580px">
																								//
																								//are interpreted as if attribute src had all this links as a value.
																								//I have no idea why this is the case, but for now I just detect last link and use it
																								//unfortunately sometimes this is the best, sometimes the worst resolution, so it has to be changed
																								//TODO: come up with better way to separate links and choose the best image

		fs::path path("book/OEBPS/Images/Image" + std::to_string(imageIndex++) + extension);	//path where image will be downloaded
		imageSaver.saveImage(linkToImage, path);

		std::string relativePath = "../Images/" + path.filename().string();			//path used in img src="PATH"
		image.attribute("src").set_value(relativePath.c_str());						//replacing link to web by path to file

		//<item id="sample.png" href="Images/sample.png" media-type="image/png"/>
		file << "<item id=" << '"' << path.filename() << '"' << " href=" << '"' << "Images/" << path.filename() << '"' << " media-type=" << '"' << "image/png" << '"' << "/>";	//adding image to manifest
	}

	file.close();
}

void EbookCreator::addSeparationBetweenManifestAndSpine()
{
	std::ofstream file("book/OEBPS/content.opf", std::ios::app);
	file << "</manifest>";															//end manifest
	file << "<spine toc=" << '"' << "ncx" << '"' << ">";							//begin spine
	file << "<itemref idref=" << '"' << "cover" << '"' << "/>";						//list cover
	file << "<itemref idref=" << '"' << "title_page.xhtml" << '"' << "/>";			//list title_page
}

void EbookCreator::addToSpine(std::vector<ParsedArticle> & articles)
{
	std::ofstream file("book/OEBPS/content.opf", std::ios::app);

	articleIndex = 0;
	for (auto& article : articles)
	{
		std::string articleName = "article" + std::to_string(articleIndex++) + ".xhtml";
		file << "<itemref idref=" << '"' << articleName << '"' << "/>";				//list articles in spine like this: <itemref idref="article0.xhtml"/>
	}

	file << "</spine></package>";													//close last markup in this file
	file.close();
}

void EbookCreator::buildTableOfContent(std::vector<ParsedArticle>& articles)
{
	std::ofstream file("book/OEBPS/toc.ncx", std::ios::app);

	articleIndex = 0;
	for (auto& article : articles)
	{
		std::string articleName = "article" + std::to_string(articleIndex) + ".xhtml";

		file << "<navPoint id=" << '"' << "navPoint-" << articleIndex + 2 << '"' << " playOrder=" << '"' << articleIndex + 2 << '"' << "><navLabel><text>";		//begin each article's navPoint
		file << article.title << "</text></navLabel><content src=" << '"' << "Text/";																			//add title of each article
		file << articleName << '"' << "/></navPoint>";																											//close navPoint
		++articleIndex;
	}

	file << "</navPoint></navMap></ncx>";																														//end whole table of content
	file.close();
}

void EbookCreator::tidyAndConvertToXhtml(ParsedArticle article)
{
	TidyBuffer XMLdata = { 0 };
	TidyBuffer errbuf = { 0 };
	int rc = -1;				//return code
	Bool ok;

	TidyDoc tidyDoc = tidyCreate();													//initialize "document"

	ok = tidyOptSetBool(tidyDoc, TidyXhtmlOut, yes);								//convert to XHTML
	ok = tidyOptSetBool(tidyDoc, TidyPreserveEntities, yes);						//leaving it on false causes text to show many &nbsp strings

	if (ok)
		rc = tidySetErrorBuffer(tidyDoc, &errbuf);									//link tidy document to error buffer
	if (rc >= 0)
		rc = tidyParseString(tidyDoc, article.content.c_str());						//load source code and parse it
	if (rc >= 0)
		rc = tidyCleanAndRepair(tidyDoc);											//tidy it up!
	if (rc >= 0)
		rc = tidyRunDiagnostics(tidyDoc);											//reports the document type and diagnostic statistics on parsed and repaired markup. 

	if (rc > 1)																		//if error, force output.
		rc = (tidyOptSetBool(tidyDoc, TidyForceOutput, yes) ? rc : -1);
	if (rc >= 0)
		rc = tidySaveBuffer(tidyDoc, &XMLdata);										//save output into XMLdata variable

	if (rc >= 0)
	{
	}
	else
		printf("A severe error (%d) occurred.\n", rc);

	tidyBufFree(&errbuf);
	tidyRelease(tidyDoc);

	currentDocument.load_buffer(XMLdata.bp, XMLdata.size);							//loading data into pugi document
	tidyBufFree(&XMLdata);															//removing xml data from tidy buffer
}

void EbookCreator::convertToMobi()
{
	int retCode = system("kindlegen.exe book/OEBPS/content.opf -o book.mobi");		//-o is the output file name
	fs::copy(fs::path("book/OEBPS/book.mobi"), fs::path("book.mobi"));
}

void EbookCreator::removeEpub()
{
	fs::remove_all(fs::path("book"));
}
