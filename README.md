# Pocket integration
Program designed to work 24/7 in the background (I'm considering Raspberry Pi), load RSS feeds from .txt file, watch them and every few minutes check whether any new articles showed up. If so it connects with Pocket via OAuth 2.0 and sends articles using Pocket API. All downloaded articles are saved in .xml file, so there should not be any repeating articles send to Pocket. Maybe it is not the best database, but easy to use and doesn't require any additional libraries, as I was already using pugixml.  
It is my first contact with dealing with APIs and web generally, so modules connecting with internet may be horribly written.

![command_line_preview](https://user-images.githubusercontent.com/44038381/46904941-9e67b300-ceec-11e8-9c5b-0b9f0a5a2167.PNG)
