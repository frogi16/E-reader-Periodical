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
#include "StringUtils.h"

namespace fs = std::filesystem;

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
	catch (const std::exception & e)
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

		//last article can't have skip button linking to the next article and has to be handled separately
		if (&article == &articles.back())
			saveArticle(article, false);
		else
			saveArticle(article);									//saves each article as "article[number].xhtml" - article0.xhtml, article1.xhtml etc.

		++articleIndex;
		std::cout << "*";
	}

	std::cout << std::endl;

	addSeparationBetweenManifestAndSpine();
	addToSpine(articles);

	buildTableOfContent(articles);
}

void EbookCreator::saveArticle(ParsedArticle& article, bool addSkipButton)
{
	std::ofstream file;
	std::string articleName = "article" + std::to_string(articleIndex) + ".xhtml";
	file.open("book/OEBPS/Text/" + articleName, std::ios::out);											//create the article file

	auto head = article.xmlDocument->first_child().child("head");
	auto cssLink = head.append_child("link");
	cssLink.append_attribute("href").set_value("../Styles/stylesheet.css");
	cssLink.append_attribute("rel").set_value("stylesheet");
	cssLink.append_attribute("type").set_value("text/css");

	auto body = article.xmlDocument->first_child().child("body");

	auto header = body.prepend_child("div");
	header.append_attribute("class").set_value("article-header");

	//<p id="anchor"></p>	anchor to which "Skip" button can be tied
	auto link = header.append_child("p");
	link.text().set("");
	link.append_attribute("id").set_value("anchor");

	header.append_child("h1").text().set(article.title.c_str());			//article title

	header.append_child("i").text().set(article.domain.c_str());			//domain

	if (addSkipButton)
	{
		auto skip = header.append_child("i").append_child("a");											//"Skip" button is a reference to an anchor within the next article
		skip.text().set("Skip");
		skip.append_attribute("href").set_value(std::string("article" + std::to_string(articleIndex + 1) + ".xhtml#anchor").c_str());
		skip.append_attribute("class").set_value("reference");
		skip.append_attribute("id").set_value("reference1");
	}

	header.append_child("hr");																			//horizontal line separating header and content

	article.xmlDocument->save(file);																	//save xml content to file
	file.close();
}

void EbookCreator::addToManifest(ParsedArticle& article)
{
	std::ofstream file;

	file.open("book/OEBPS/content.opf", std::ios::app);											//beginning of this file is constant for all books so I just copy it from template directory and append new things

	std::string articleName = "article" + std::to_string(articleIndex) + ".xhtml";
	file << "<item id=" << '"' << articleName << '"' << " href=" << '"' << "Text/" << articleName << '"' << " media-type=" << '"' << "application/xhtml+xml" << '"' << "/>";	//adding the article itself

	auto images = EbookPeriodical::selectNodes<SelectNameTreeWalker>((*article.xmlDocument), std::string("img"));	//find all images the article links to, because they also have to be listed in manifest
	saveImages(images, article.domainFromRSS);

	file.close();
}

void EbookCreator::saveImages(std::vector<pugi::xml_node> images, const std::string& domain)
{
	std::ofstream file("book/OEBPS/content.opf", std::ios::app);

	for (auto& image : images)
	{
		std::string linkToImg;

		if (std::string srcSetString = image.attribute("srcset").as_string(); srcSetString.size())
		{
			if (SrcSet set; set.parseAndInsertLinks(srcSetString) > 0)				//parseAndInsertLinks returns number of inserted links
				linkToImg = *set.getLargestImageLink();
			else
				linkToImg = image.attribute("src").as_string();
		}
		else
		{
			linkToImg = image.attribute("src").as_string();							//extract link to image
		}

		fs::path path("book/OEBPS/Images/Image" + std::to_string(imageIndex++));	//path where image will be downloaded

		std::size_t found = linkToImg.find_first_not_of("/");

		if (found != 0 && found != std::string::npos)
			linkToImg.erase(0, found);												//eliminate all slashes at the beginning

		try
		{
			imageSaver.saveImage(linkToImg, path);									//WARNING! Image downloader detects extension of file and changes path sent to it

			//adding image to manifest. Template: <item id="sample.png" href="Images/sample.png" media-type="image/png"/>
			file << "<item id=" << '"' << path.filename() << '"' << " href=" << '"' << "Images/" << path.filename() << '"' << " media-type=" << '"' << "image/png" << '"' << "/>";
		}
		catch (const std::exception & e)												//if image couldn't be downloaded just use default filler. There is no need to 
		{
			try
			{
				imageSaver.saveImage(eprd::mergePathFragments(domain, linkToImg), path);	//WARNING! Image downloader detects extension of file and changes path sent to it

				//adding image to manifest. Template: <item id="sample.png" href="Images/sample.png" media-type="image/png"/>
				file << "<item id=" << '"' << path.filename() << '"' << " href=" << '"' << "Images/" << path.filename() << '"' << " media-type=" << '"' << "image/png" << '"' << "/>";
			}
			catch (const std::exception & e)
			{
				path.replace_filename("filler");
				path.replace_extension("png");
				//note that there is no need to add filler.jpeg to file, because each element in file must be unique and filler is already there
			}
		}

		fs::path relativePath("../Images/" + path.filename().string());				//path used in img src="PATH"
		image.attribute("src").set_value(relativePath.string().c_str());			//replacing link to web with path to file
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

void EbookCreator::addToSpine(std::vector<ParsedArticle>& articles)
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
