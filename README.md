# Pocket integration

## Originally
This program was originally designed to work 24/7 in the background, monitor RSS feeds and send any new articles to Pocket API.

![command_line_preview](https://user-images.githubusercontent.com/44038381/46904941-9e67b300-ceec-11e8-9c5b-0b9f0a5a2167.PNG)

## Now
After a while the project has grown and new functionalities were added. Now it is possible to parse articles from RSS feeds and Pocket using Mercury Web Parser and save them in local directory. After downloading images embedded in articles, replacing paths with local ones and adding some additional content, ebook in EPUB format will be generated. Conversion to MOBI can be performed using kindlegen, which is an official tool released by Amazon. Generated ebooks are fully compatible with all e-readers and contain features like cover, table of content and hierarchical navigation menu.

![cover](https://user-images.githubusercontent.com/44038381/47454283-209a8600-d7cf-11e8-98f1-4f12f16a610f.PNG)

![title screen](https://user-images.githubusercontent.com/44038381/47454284-209a8600-d7cf-11e8-9aa0-db42a2446109.PNG)

![table of content](https://user-images.githubusercontent.com/44038381/59163603-7c6a1300-8b03-11e9-89fb-648a26c5c1bb.PNG)

![article preview](https://user-images.githubusercontent.com/44038381/59163869-8d685380-8b06-11e9-8f72-5d153fb5cca7.PNG)

![article preview 2](https://user-images.githubusercontent.com/44038381/47454286-21331c80-d7cf-11e8-849d-9e5824f4911c.PNG)

## Tools and frameworks
All network-related functionalities use libcurl via simple, self-made wrapper, so it could be used in object oriented style. Pugi xml has been used to easily read and modify .xml files and HTML tidy fixes internal problems with formatting and structure of downloaded articles.

