#include "config.h"
#include <winsock2.h>

Config::Config()
{

}
Config::~Config()
{

}

 int Config::ServerPort = 5050;
const int Config::ClientPort = 8080;
string Config::ServerAddr = "127.0.0.1";
const int Config::MaxConnetion = 100;
const int Config::BufferLength = 65535;
const string Config::Ok= "200 OK";
const string Config::Error = "501 Not Implemented";
const string Config::NotFound = "404 Not Found";
const string Config::HttpVersion = "HTTP/1.1 ";
const u_long Config::uNonBlock = 1;	  //SOCKET为非阻塞模式	
string Config::mainPath = "D:\\html\\";