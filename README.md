# Pocket integration

## Overview
This program was originally designed to work 24/7 in the background, monitor RSS feeds and send any new articles to Pocket API.

![command_line_preview](https://user-images.githubusercontent.com/44038381/46904941-9e67b300-ceec-11e8-9c5b-0b9f0a5a2167.PNG)

[Pocket](https://getpocket.com/) is a great service available on PCs and smartphones which allows to easily save content for later.
Although it is very handy, later consumption is limited to monitors and screens. Why not to create a way to read your favourite articles as a periodical delivered daily to your ebook reader? 

![title screen](https://user-images.githubusercontent.com/44038381/47454284-209a8600-d7cf-11e8-9aa0-db42a2446109.PNG)

Now it is possible to parse articles from RSS feeds and Pocket using Mercury Web Parser and turn them into books in EPUB and MOBI formats.

![article preview](https://user-images.githubusercontent.com/44038381/59163869-8d685380-8b06-11e9-8f72-5d153fb5cca7.PNG)

Application embedds cover, table of content, hierarchical navigation menu and of course all downloaded articles into the book. The articles are properly formatted and prepared, images are downloaded and embedded, titles, sources and "Skip" buttons are also added for user comfort.

![table of content](https://user-images.githubusercontent.com/44038381/59163603-7c6a1300-8b03-11e9-89fb-648a26c5c1bb.PNG)

Ebook are constructed in EPUB format end conversion to MOBI can be performed using kindlegen, which is an official tool released by Amazon. 

![article preview 2](https://user-images.githubusercontent.com/44038381/47454286-21331c80-d7cf-11e8-849d-9e5824f4911c.PNG)

## Tools and frameworks
All network-related functionalities use libcurl via simple, self-made wrapper, so it could be used in object oriented style. Pugi xml has been used to easily read and modify .xml files and HTML tidy fixes internal problems with formatting and structure of downloaded articles.

