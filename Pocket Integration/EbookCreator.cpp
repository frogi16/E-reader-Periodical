#include "EbookCreator.h"

#include <fstream>
#include <string>
#include <filesystem>
#include <iostream>

#include "pugixml.hpp"
#include "tidy.h"
#include "tidybuffio.h"
#include "EbookCreator.h"
#include "SrcSet.h"

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
	try
	{
		fs::remove_all(fs::path("book"));											//encountered problems with remove_all - sometimes it removes everything except two files or except book directory.
		fs::remove_all(fs::path("book"));											//The weirdest thing is that after repeating this process some more files are removed. Three times are enough for now.
		fs::remove_all(fs::path("book"));
		fs::remove(fs::path("book.mobi"));
		fs::create_directory(fs::path("book"));
		fs::copy(fs::path("template"), fs::path("book"), fs::copy_options::recursive);
	}
	catch (const std::exception& e)
	{
		std::cout << "Unexpected error: " << e.what() << std::endl;
	}
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
		std::cout << ".";
	}

	std::cout << std::endl;

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
	saveImages(images);

	file.close();
}

void EbookCreator::saveImages(std::vector<pugi::xml_node> images)
{
	std::ofstream file("book/OEBPS/content.opf", std::ios::app);

	for (auto& image : images)
	{
		std::string srcSetString = image.attribute("srcset").as_string();			//extract links to images with different sizes
		std::string linkToImg;

		if (srcSetString.size() > 0)												//if these links were found
		{
			SrcSet set(srcSetString);												//SrcSet automatically parse sent string - splits links and inserts them into map, where key is their width
			linkToImg = set.links.rbegin()->second;									//keys are sorted from the least to the biggest, so last one means the biggest image
		}
		else
		{
			linkToImg = image.attribute("src").as_string();							//extract link to image
		}

		std::string extension = ".jpg";												//extension of image, by default is set to ".jpg"

		size_t positionOfDot = linkToImg.find_last_of('.');							//position of last dot in link. After this the only thing should be an extension

		if (positionOfDot != std::string::npos)
		{
			extension = linkToImg.substr(positionOfDot, 4);							//copy characters from dot to the end of string
		}

		fs::path path("book/OEBPS/Images/Image" + std::to_string(imageIndex++) + extension);	//path where image will be downloaded
		imageSaver.saveImage(linkToImg, path);

		std::string relativePath = "../Images/" + path.filename().string();			//path used in img src="PATH"
		image.attribute("src").set_value(relativePath.c_str());						//replacing link to web by path to file

																					//<item id="sample.png" href="Images/sample.png" media-type="image/png"/>
		file << "<item id=" << '"' << path.filename() << '"' << " href=" << '"' << "Images/" << path.filename() << '"' << " media-type=" << '"' << "image/png" << '"' << "/>";	//adding image to manifest
	}
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

	file << "<itemref idref=" << '"' << "toc-file" << '"' << "/>";					//reference to table of contents in file

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
	//table of content built here is a metadata object which can be used to navigate between pages from anywhere in the ebook
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

	//now we will create real table of content embedded as visible page. It is very useful to quickly estimate what today's periodical consists of and if there is something interesting
	file.open("book/OEBPS/Text/toc.xhtml", std::ios::app);
	articleIndex = 0;
	for (auto& article : articles)
	{
		std::string articleName = "article" + std::to_string(articleIndex) + ".xhtml";
		
		//entry for every article
		file << "<div class=" << '"' << "sgc-toc-level-1" << '"' << "><a href =" << '"' << "../Text/" << articleName << '"' << ">" << articleIndex << "." << article.title << "</a></div>" << std::endl;
		++articleIndex;
	}
	
	file << "<div style='padding:0;border:0;text-indent:0;line-height:normal;margin:0 1cm 0.5cm 1cm;font-size:0pt;color:#FFFFFF;text-decoration:none;text-align:left;background:none;display:none;'>1c300054508041890ea8ce85</div></body></html>";

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
