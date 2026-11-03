# Introduction
This project is a all in one place for my GUI applications, every project will be living in its own path exh. "/proj_name".
To not write everything in js/ts this project is shipped with http server that will process requests and call API in charge of 
modifying requests to display intended objects. My own mini lib will convert objects to html tags and plug them into .html file
so web can display them properlly.
Because of this schema every project have to have thier lib and web part
- Library
  This is the "heart" of the project that should contain core objects, functions and concepts that project should operate on
  
- Web
  Are API that http server will call to process, convert to displayable format for web
 
# Before building
- asio - any version that supports corutines, I used 1.30.2
- while building this project i used g++ compiler, not sure how others will react to this code
- c++ version minimum 20 because of gorutines, in makefile it is set to 23 just in case

# Build
To build this project you need to use make, might need to change flags or compiler but most of makefile is correct according to this project structure
Commands:
- "make" - compile
- "make run" - compile and run

# Reverse-Proxy
While working on this project i used Caddy (easy to set up) and processed forwarded requests, but it isn't nessecary, still works without proxy.

# Features
- TODO List
  * Adding new projects, objectives, tasks
  * Edditing objectives, tasks
