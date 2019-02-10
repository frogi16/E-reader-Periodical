#include "EbookCreator.h"

#include <fstream>
#include <string>
#include <filesystem>
#include <iostream>
#include <chrono>
#include <iomanip>

#include "pugixml.hpp"
#include "tidy.h"
#include "tidybuffio.h"
#include "EbookCreator.h"
#include "SrcSet.h"

namespace fs = std::experimental::filesystem;

EbookCreator::EbookCreator() : ebookPath("")
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
		size_t counter = 0;
		while (fs::exists(fs::path("book")))														//remove_all doesn't actually remove all things. Because of reasons. So I remove and remove, and remove...
		{
			fs::remove_all(fs::path("book"));

			if (++counter > 10)
				throw std::exception("After multiple tries couldn't remove /book directory. You are advised to check if application has all permissions it needs.");
		}
		
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
		saveArticle(article);																	//save each article as "article[number].xhtml" - article0.xhtml, article1.xhtml etc.
		++articleIndex;
		std::cout << "*";
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
	file.open("book/OEBPS/Text/" + articleName, std::ios::out);									//create the article file

	auto body = article.xmlDocument->first_child().child("body");
	body.prepend_child("hr");																	//add horizontal line separating header and content to xml
	body.prepend_child("i").append_child(pugi::node_pcdata).set_value(article.domain.c_str());	//add domain to xml
	body.prepend_child("h1").append_child(pugi::node_pcdata).set_value(article.title.c_str());	//add title	to xml		
	article.xmlDocument->save(file);															//save xml content to file
	file.close();
}

void EbookCreator::addToManifest(ParsedArticle & article)
{
	std::ofstream file;

	file.open("book/OEBPS/content.opf", std::ios::app);											//beginning of this file is constant for all books so I just copy it from template directory and append new things

	std::string articleName = "article" + std::to_string(articleIndex) + ".xhtml";
	file << "<item id=" << '"' << articleName << '"' << " href=" << '"' << "Text/" << articleName << '"' << " media-type=" << '"' << "application/xhtml+xml" << '"' << "/>";	//adding the article itself

	auto images = dataSelecter.selectNodesByName((*article.xmlDocument), std::string("img"));	//find all images the article links to, because they also have to be listed in manifest
	saveImages(images, article.domain);

	file.close();
}

void EbookCreator::saveImages(std::vector<pugi::xml_node> images, const std::string & domain)
{
	std::ofstream file("book/OEBPS/content.opf", std::ios::app);

	for (auto& image : images)
	{
		std::string srcSetString = image.attribute("srcset").as_string();			//extract links to images with different sizes
		std::string linkToImg;
		bool skipImage = false;

		if (srcSetString.size() > 0)												//if these links were found
		{
			try
			{
				SrcSet set(srcSetString);											//SrcSet automatically parse sent string - splits links and inserts them into map, where key is their width
				linkToImg = set.getLargestImageLink();
			}
			catch (const std::exception& e)
			{
				skipImage = true;
			}			
		}
		else
		{
			linkToImg = image.attribute("src").as_string();							//extract link to image
		}

		if (!skipImage)
		{
			if ((*linkToImg.begin()) == '/' || (*linkToImg.begin()) == '\\')			//convert relative path into the global one
				linkToImg = domain + linkToImg;

			fs::path path("book/OEBPS/Images/Image" + std::to_string(imageIndex++));	//path where image will be downloaded

			try
			{
				imageSaver.saveImage(linkToImg, path);									//WARNING! Image downloader detects extension of file and changes path sent to it

				//adding image to manifest. Template: <item id="sample.png" href="Images/sample.png" media-type="image/png"/>
				file << "<item id=" << '"' << path.filename() << '"' << " href=" << '"' << "Images/" << path.filename() << '"' << " media-type=" << '"' << "image/png" << '"' << "/>";
			}
			catch (const std::exception& e)												//if image couldn't be downloaded just use default filler. There is no need to 
			{
				path.replace_filename("filler");
				path.replace_extension("png");
				//note that there is no need to add filler.jpeg to file, because each element in file must be unique and filler is already there
			}

			fs::path relativePath("../Images/" + path.filename().string());				//path used in img src="PATH"
			image.attribute("src").set_value(relativePath.string().c_str());			//replacing link to web with path to file
		}
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

		file << "<navPoint id=" << '"' << "navPoint-" << articleIndex + 4 << '"' << " playOrder=" << '"' << articleIndex + 4 << '"' << "><navLabel><text>";		//begin each article's navPoint
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
		file << "<div class=" << '"' << "sgc-toc-level-1" << '"' << "><a href =" << '"' << "../Text/" << articleName << '"' << ">" << ++articleIndex << "." << article.title << "</a></div>" << std::endl;
	}
	
	file << "<div style='padding:0;border:0;text-indent:0;line-height:normal;margin:0 1cm 0.5cm 1cm;font-size:0pt;color:#FFFFFF;text-decoration:none;text-align:left;background:none;display:none;'>1c300054508041890ea8ce85</div></body></html>";

	file.close();
}

void EbookCreator::convertToMobi()
{
	auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	auto localTime = *std::localtime(&currentTime);

	std::ostringstream stringStream;
	stringStream << "RSS Periodical " << std::put_time(&localTime, "%d-%m-%Y");
	auto filename = stringStream.str() + ".mobi";
	ebookPath = filename;

	int retCode = system("kindlegen.exe book/OEBPS/content.opf -o book.mobi");		//-o is the output file name
	fs::copy(fs::path("book/OEBPS/book.mobi"), ebookPath);
}

void EbookCreator::removeEpub()
{
	fs::remove_all(fs::path("book"));
}
