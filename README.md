# E-reader Periodical

## Overview

Popularity of e-readers constantly increases and despite their usefulness in reading books, their ability to browse web is still limited. This project aims to connect these two different worlds by providing an automated way to download articles from your favourite sites, process them and create an e-book ready to send to your Kindle or other devices.

![title screen](https://user-images.githubusercontent.com/44038381/47454284-209a8600-d7cf-11e8-9aa0-db42a2446109.PNG)

Not only can you specify sites' RSS feeds and let the computer convert new articles to an e-book, you can also add any article from any site from every place on the globe using [Pocket](https://getpocket.com/), which is a great service available on PCs and smartphones that allows you to easily save content for later consumption. E-reader Periodical synchronizes with Pocket every time it creates an e-book for you and adds articles from your Pocket account to the e-book.

![table of content](https://user-images.githubusercontent.com/44038381/59163603-7c6a1300-8b03-11e9-89fb-648a26c5c1bb.PNG)

Generated content is formatted as close to traditional books as possible, but E-reader Periodical also adds some useful features like fully functional Table of Contents and skip buttons placed for user convenience in the header of every article.

![article header preview](https://user-images.githubusercontent.com/44038381/59977444-58adcf00-95d1-11e9-831d-559b58cab793.PNG)

During the processing phase E-reader Periodical searches for every image linked in the article, downloads the best resolution possible and embedds it into the e-book. All references requiring connection to the Internet are replaced.

![article preview](https://user-images.githubusercontent.com/44038381/59163869-8d685380-8b06-11e9-8f72-5d153fb5cca7.PNG)

Articles are filtered based on rules provided by the user and may be removed completely or partially from the e-book because of their length, found keywords or HTML tags. This simple system is enough to fix small mistakes done by web parser and remove annoying and repetitive texts (e.q. requests for support and donations).

![command_line_preview](https://user-images.githubusercontent.com/44038381/46904941-9e67b300-ceec-11e8-9c5b-0b9f0a5a2167.PNG)

E-books are constructed in EPUB format and conversion to MOBI is performed using kindlegen, which is an official tool released by Amazon. E-reader Periodical ensures that all changes and features are fully compatible with both formats and can be converted properly. 

![article preview 2](https://user-images.githubusercontent.com/44038381/47454286-21331c80-d7cf-11e8-849d-9e5824f4911c.PNG)

## Tools and frameworks

Extracting important content from the whole website is handled by Mercury Web Parser, which recently went open-source. 
All network-related functionalities use libcurl via simple, self-made wrapper, so it can be used in object oriented style. Pugi xml has been used to easily read and modify .xml files and HTML tidy fixes internal problems with formatting and structure of downloaded articles.

## Future development plans

- adding proper GUI. A few prototypes using Qt framework have been already made and further works are in progress.
- fully automated generation and delivery should be implemented as an alternative process.
- storing passwords, usernames, and API keys has to be done more securely and reliably.
- rewriting parts of the program responsible for generation EPUB file. 
