#pragma once

#include <vector>

#include "ParsedArticle.h"
#include "ImageSaver.h"
#include "Scraper.h"

class EbookCreator
{
public:
	EbookCreator();
	~EbookCreator();
	void createEpub(std::vector<ParsedArticle> & articles);
	void convertToMobi();												//convert ready .epub to .mobi using kindlegen
	void removeEpub();
	std::experimental::filesystem::path getEbookPath() { return ebookPath; }
private:
	void prepareDirectory();
	void appendData(std::vector<ParsedArticle> & articles);				//whole process of saving articles to files, constructing table of content, manifest etc.
	void saveArticle(ParsedArticle & article);							//save article to .xhtml file
	void addToManifest(ParsedArticle & article);						//add metadata about article to content.opf manifest
	void saveImages(std::vector<pugi::xml_node> images);				//takes vector of <img> nodes, finds images, download them and list in manifest file
	void addSeparationBetweenManifestAndSpine();						//add markups ending manifest and beginning spine
	void addToSpine(std::vector<ParsedArticle> & articles);				//add metadata about article to content.opf spine
	void buildTableOfContent(std::vector<ParsedArticle> & articles);
	void tidyAndConvertToXhtml(ParsedArticle article);

	ImageSaver imageSaver;
	Scraper scraper;

	std::experimental::filesystem::path ebookPath;
	pugi::xml_document currentDocument;
	size_t articleIndex = 0;											//index of avtually processed article
	size_t imageIndex = 0;												//index of avtually processed image
};

