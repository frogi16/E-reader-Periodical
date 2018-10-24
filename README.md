# Pocket integration

## Originally
Program originally designed to work 24/7 in the background, load RSS feeds from .txt file, watch them and every few minutes check whether any new articles showed up. If so it connects with Pocket via OAuth 2.0 and send links to articles using Pocket API. Info about all delivered articles is saved in .xml file, so no articles should be sent to Pocket twice. XML file maybe is not the best database, but easy to use and doesn't require any additional libraries, as I was already using pugixml.  

![command_line_preview](https://user-images.githubusercontent.com/44038381/46904941-9e67b300-ceec-11e8-9c5b-0b9f0a5a2167.PNG)

## Now
After a while the project has grown and new functionalities were added. Now it is possible to completely skip Pocket and instead parse articles using Mercury Web Parser and save them in local directory. After downloading images embedded in articles, replacing paths by local ones and adding some additional content, it is possible to create ebook using EPUB format. Conversion to MOBI is done using kindlegen, which is an official tool released by Amazon. Generated ebooks are fully compatible with all e-readers and contain features like cover, table of content and hierarchical navigation menu.

![cover](https://user-images.githubusercontent.com/44038381/47454283-209a8600-d7cf-11e8-98f1-4f12f16a610f.PNG)

![title screen](https://user-images.githubusercontent.com/44038381/47454284-209a8600-d7cf-11e8-9aa0-db42a2446109.PNG)

![article preview](https://user-images.githubusercontent.com/44038381/47454287-21331c80-d7cf-11e8-803e-1230028a10a6.PNG)

![article preview 2](https://user-images.githubusercontent.com/44038381/47454286-21331c80-d7cf-11e8-849d-9e5824f4911c.PNG)

It is my first contact with dealing with APIs and web generally, so modules connecting with internet may be horribly written.
