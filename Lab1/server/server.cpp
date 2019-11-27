#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>
#include "server.h"
#include "config.h"
#include <winsock2.h>
#include <thread>
#include <regex>
#include<mutex>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

Server::Server(void)
{
	this->RecvBuf = new char[Config::BufferLength]; //初始化接受缓冲区
	memset(this->RecvBuf, '\0', Config::BufferLength); // 并设置为0
	this->SendBuf = new char[Config::BufferLength]; //初始化发送缓冲区
	memset(this->SendBuf, '\0', Config::BufferLength); // 并设置为0

	this->SocketList = new list<SOCKET>();  //初始化有关List和Map
	this->ClosedSocketList = new list<SOCKET>();
}

Server::~Server(void)
{
	if (this->RecvBuf != NULL) {
		//释放接收缓冲区
		free(this->RecvBuf);
		this->RecvBuf = NULL;
		cout <<"RecvBuf has been free!" << endl;
	}

	if (this->SendBuf != NULL) {
		//释放接收缓冲区
		free(this->SendBuf);
		this->SendBuf = NULL;
		cout << "SendBuf has been free!" << endl;
	}

	//关闭服务器端socket
	if (this->ServerSocket != NULL) {
		closesocket(this->ServerSocket);
		this->ServerSocket = NULL;
		cout << "ServerSocket has been closed!" << endl;
	}

	//关闭所有会话socket并释放会话队列
	if (this->SocketList != NULL) {
		for (list<SOCKET>::iterator it = this->SocketList->begin(); it != this->SocketList->end(); it++)
			closesocket(*it); //关闭会话
		delete this->SocketList;  //释放队列
		this->SocketList = NULL;
		cout << "SocketList has been deleted!" << endl;
	}

	//释放失效会话队列
	if (this->ClosedSocketList != NULL) {
		for (list<SOCKET>::iterator it = this->ClosedSocketList->begin(); it != this->ClosedSocketList->end(); it++)
			closesocket(*it);
		delete this->ClosedSocketList;
		this->ClosedSocketList = NULL;
		cout << "ClosedSocketList has been deleted!" << endl;
	}

	WSACleanup(); //清理winsock 运行环境
}

//判断是否以某一特定后缀名结尾
bool Server::endWith(const std::string& fullStr, const std::string& endStr) {
	if (endStr.size() > fullStr.size())
		return false;
	int indexFull = fullStr.size() - 1;
	int indexEnd = endStr.size() - 1;
	while (indexEnd >= 0)
	{
		if (fullStr[indexFull] != endStr[indexEnd])
		{
			return false;
		}
		indexFull--;
		indexEnd--;
	}
	return true;
}

//初始化Winsock
int Server::WinsockStartup() {

	WSADATA wsaData;//用于返回Winsock的环境信息   
	int nRc = WSAStartup(0x0202, &wsaData);

	if (nRc) {
		//Winsock初始化失败
		cout << "WSAstartup error!\n";
		return -1;
	}

	//判断返回的Winsock版本号
	if (wsaData.wVersion != 0x0202) { //如果版本不对
		cout << "Server's winsock version error!";
		WSACleanup();  //清除Winsock
		return -1;
	}

	cout << "Server's winsock startup success!\n";
	return 0;
}

//初始化Server
int Server::ServerStartup() {

	cout << "Please input the ServerAddr: ";
	cin >> Config::ServerAddr;
	cout << "\rPlease input the ServerPort: ";
	cin >> Config::ServerPort;
	cout << "Server mainpath is:" <<Config::mainPath << " now" << endl;
	cout << "Do you want to change it: y/n ";
	char changePath;
	cin >> changePath;

	if (changePath == 'Y' || changePath == 'y')
	{
		cout << "Please input the new server mainpath:";
		cin >> Config::mainPath;
	}

	//创建 TCP socket
	this->ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (this->ServerSocket == INVALID_SOCKET) {
		cout << "Server create socket error!!"<<endl;
		WSACleanup();
		return -1;
	}

	cout << "Server TCP socket create OK!"<<endl;

	//Bind socket to Server's IP and port 5050
	this->ServerAddr.sin_family = AF_INET;
	this->ServerAddr.sin_port = htons(Config::ServerPort);
	this->ServerAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	int nRC = bind(this->ServerSocket, (LPSOCKADDR) & (this->ServerAddr), sizeof(this->ServerAddr));
	if (nRC == SOCKET_ERROR) {
		cout << "Server socket bind error!\n";
		closesocket(this->ServerSocket);
		WSACleanup();
		return -1;
	}
	cout << "Server socket bind ok!\n";
	return 0;
}

///关闭服务器线程
int Server::closeServer() {
	cout << "If you want to close server, please input exit" <<endl ;
	std::string Exit;
	cin >> Exit;
	while (1) {
		if (Exit == "exit")
			break;
	}
	//到这里意味着要关闭服务器
	this->~Server();
	exit(0);
}

//开始监听过程，等待客户的连接
int Server::ListenStartup() {
	int nRC = listen(this->ServerSocket, Config::MaxConnetion);
	if (nRC == SOCKET_ERROR) {
		cout << "Server socket listen error!"<<endl;
		closesocket(this->ServerSocket);
		WSACleanup();
		return -1;
	}
	cout << "Server socket listen ok!"<<endl;
	return 0;
}

//将新的会话socket加入队列
void Server::AddSocket(SOCKET socket) {
	if (socket != INVALID_SOCKET) {
		this->SocketList->insert(this->SocketList->end(), socket);
	}
}

//将失效的会话socket加入失效队列
void Server::AddClosedSocket(SOCKET socket) {
	if (socket != INVALID_SOCKET) {
		this->ClosedSocketList->insert(this->ClosedSocketList->end(), socket);
	}
}

//将失效的socket从会话socket队列删除
void Server::RemoveClosedSocket(SOCKET socket) {
	if (socket != INVALID_SOCKET) {
		list<SOCKET>::iterator it = find(this->SocketList->begin(), this->SocketList->end(), socket);
		if (it != this->SocketList->end()) {
			this->SocketList->erase(it);
			//closesocket(*it);
		}
	}
}

//将所有失效的socket从会话socket队列删除
void Server::RemoveClosedSocket() {
	for (list<SOCKET>::iterator it = this->ClosedSocketList->begin(); it != this->ClosedSocketList->end(); it++) {
		/*if ((*it) != INVALID_SOCKET) {
			if (it != this->SocketList->end())
				this->SocketList->erase(it);
		}*/
		this->RemoveClosedSocket(*it);
	}
}

//向socket发送消息
void Server::SendMessage(SOCKET socket, string msg) {
	int rtn = send(socket, msg.c_str(), msg.length(), 0);
	unique_lock<mutex>closedList(this->ClosedList2, defer_lock);
	if (rtn == SOCKET_ERROR) {					//发送数据错误
		closedList.lock();
		this->AddClosedSocket(socket);
		closedList.unlock();
	}
}

//从socket接受消息
void Server::RecvMessage(SOCKET socket) {

	unique_lock<mutex>writeRecBuf(this->lockRecBuf, defer_lock);  //接收资源写信号灯
	unique_lock<mutex>readRecBuf(this->lockRecBuf2, defer_lock);  //接收资源读信号灯
	unique_lock<mutex>sendHeader(this->sendHeader, defer_lock);  //发送报文头部信号灯
	unique_lock<mutex>sendMessage(this->sendMessage, defer_lock);  //发送报文主体信号灯
	unique_lock<mutex>closedList(this->ClosedList, defer_lock);  //发送报文主体信号灯

	//writeRecBuf.lock();   //写RecBuf时需要上锁
	char* RecvBufn = new char[Config::BufferLength];
	memset(RecvBufn, '\0', sizeof(RecvBufn)); //缓存清零
	int receivedBytes = recv(socket, RecvBufn, Config::BufferLength, 0);
	//writeRecBuf.unlock();

	//如果会话SOCKET有数据到来，则接受客户的数据
	if (receivedBytes == SOCKET_ERROR) {    //接受数据错误
		/*receivedBytes = WSAGetLastError();
		cout << "Thread falied!" << std::endl;
		return;*/
		//cout << "ReceiveBytes == -1,Receive Error!" << endl;
		closedList.lock();
		this->AddClosedSocket(socket);
		closedList.unlock();
	}
	else if (receivedBytes == 0) { //对方断开连接
		//cout << "ReceiveBytes ==0,Server disconnected!" << std::endl;
		closedList.lock();
		this->AddClosedSocket(socket);
		closedList.unlock();
		//return;
	}
	else {
		//cout << "\r\n" << RecvBuf;
		//找到报文头部
		std::string fullPath;
		std::string respondHttpHeader, clientHttpHeader;
		std::string statusCode, firstHeader, typeStr, lengthStr;
		std::regex regRequest(R"(([A-Z]+) (.*?) HTTP/\d\.\d)");
		std::smatch matchRst;
		std::string strRecv, strSend;
		std::string method;
	    std:string url;

		respondHttpHeader = "";
		statusCode = Config::Ok;
		firstHeader = "HTTP/1.1 200 OK\r\n";

		//找到HTTP报文头部
		//readRecBuf.lock();  //读RecBuf时需要上锁
		strRecv.assign(RecvBufn);
		size_t headerEnd = strRecv.find("\r\n\r\n");
		clientHttpHeader = strRecv.substr(0, headerEnd);
		
		//正则表达式提取关键字method(GET,POST),url
		if (std::regex_search(clientHttpHeader, matchRst, regRequest)) {
			method = matchRst[1].str();
			url = matchRst[2].str();
		}

		for (int i = 0; i < url.size(); i++) {
			//替换成windows路径
			if (url[i] == '/')
				url[i] = '\\';
		}

		fullPath = Config::mainPath + url; //完整路径
		//writeRecBuf.unlock();
		//readRecBuf.unlock();

		cout << "Client http header:\r\n" << clientHttpHeader.c_str() << endl;
		cout << "\r\nmethod:" << method.c_str() << endl;
		cout << "url:" << url.c_str() << endl;

		//sendHeader.lock();  //发送报文头部信号灯

		DWORD ftyp;
		ftyp = GetFileAttributesA(fullPath.c_str());
		if ((ftyp & FILE_ATTRIBUTE_DIRECTORY) && (!INVALID_FILE_ATTRIBUTES)) {
			//如果是一个目录，打开该目录下的索引index.html
			fullPath = fullPath + "\\index.html";
		}

		FILE* infile = fopen(fullPath.c_str(), "rb");

		if (!infile) {
			//文件不存在
			//TODO:发送自定义404页面
			infile = fopen((Config::mainPath + "\\404.html").c_str(), "rb");
			statusCode = Config::Error;
			firstHeader = "HTTP/1.1 404 Not Found\r\n";
			typeStr = "Content-Type: text/html\r\n";
		}
		else if (endWith(url, ".html") || endWith(url, "htm"))
			typeStr = "Content-Type: text/html\r\n";
		else if (endWith(url, ".txt"))
			typeStr = "Content-Type: text /plain\r\n";
		else if (endWith(url, ".jpg"))
			typeStr = "Content-Type: image/jpeg\r\n";
		else if (endWith(url, ".jpeg") || endWith(url, ".png"))
			typeStr = "Content-Type: image/" + (url.substr(url.rfind('.') + 1)) + "\r\n";
		else if (endWith(url, ".ico"))
			typeStr = "Content-Type: image/x-icon\r\n";
		else if (endWith(url, ".css"))
			typeStr = "Content-Type: text/css\r\n";
		else if (endWith(url, ".gif"))
			typeStr = "Content-Type: image/gif\r\n";
		else if (endWith(url, ".png"))
			typeStr = "Content-Type: image/png\r\n";
		else if (endWith(url, ".js"))
			typeStr = "Content-Type: application/javascript\r\n";
		else if (endWith(url, ".eot"))
			typeStr = "Content-Type: application/vnd.ms-fontobject\r\n";
		else if(endWith(url,".mp3"))
			typeStr = "Content-Type: audio/mpeg\r\n";
		else {
			if (infile != NULL)  //文件存在，但是不支持解析
				fclose(infile);
			//TODO:发送自定义501页面
			infile = fopen((Config::mainPath + "\\501.html").c_str(), "rb");
			statusCode = Config::Error;
			firstHeader = "HTTP/1.1 501 Not Implemented\r\n";
			typeStr = "Content-Type: text/html\r\n";
		}

		//获取文件大小
		fseek(infile, 0, SEEK_SET);
		fseek(infile, 0, SEEK_END);
		int fileLength = ftell(infile);
		//文件指针归位
		fseek(infile, 0, SEEK_SET);

		respondHttpHeader = firstHeader + typeStr + "Content-Length: " + std::to_string(fileLength) + "\r\n" + "Server: CServer_HTTP1.1\r\n" + "Connection: close\r\n" + "\r\n";
		//发送报文头部
	   //send(socket, respondHttpHeader.c_str, respondHttpHeader.length(), 0);
		this->SendMessage(socket, respondHttpHeader);
		//sendHeader.unlock();

		cout << "respond http header:" << respondHttpHeader.c_str();

		//发送请求的文件
		//sendMessage.lock();
		int bufReadNum;
		while (true) {
			//缓存清零
			memset(RecvBufn, 0, sizeof(RecvBufn));
			bufReadNum = fread(RecvBufn, 1, Config::BufferLength, infile);
			if (SOCKET_ERROR == (send(socket, RecvBufn, bufReadNum, 0)))
			{//发送失败
				//rtn = SOCKET_ERROR;							
				cout << "File: " << url << " transfer error!" << endl;
				break;
			}
			if (feof(infile))
				break;
		}
		fclose(infile);
		//memset(RecvBufn, '\0', Config::BufferLength);//清除接受缓冲区
		//sendMessage.unlock();
		if (RecvBufn != NULL)
			delete(RecvBufn);
		cout << "File: " << fullPath << " transfer successful!" << endl;
	}
	//writeRecBuf.unlock();
	//closesocket(socket);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	return;
}

//接受客户端发来的请求连接并生成会话socket
int Server::ServerWork() {

	u_long blockMode = Config::uNonBlock;
	int rtn;
	//nThread.MemberProc = &Server::RecvMessage;

	//将ServerSocket设为非阻塞模式以监听客户连接请求
	if ((rtn = ioctlsocket(this->ServerSocket, FIONBIO, &blockMode) == SOCKET_ERROR)) { //FIONBIO：允许或禁止套接口s的非阻塞模式。
		cout << "Set Server unblockMode error!"<<endl;
		return -1;
	}

	while (1) {
		//等到客户端的连接请求

		//设置socket集
		//FD_ZERO(fd_set *fdset);将指定的文件描述符集清空，在对文件描述符集合进行设置前，必须对其进行初始化，
		//如果不清空，由于在系统分配内存空间后，通常并不作清空处理，所以结果是不可知的。
		FD_ZERO(&this->rfds);  
		FD_ZERO(&this->wfds);
		
		//FD_SET(fd_set *fdset);用于在文件描述符集合中增加一个新的文件描述符。
		//把serverSocket加入到rfds，等待用户连接请求
		FD_SET(this->ServerSocket, &this->rfds);

		//把当前的会话socket加入到rfds,等待用户数据的到来;加到wfds，等待socket可发送数据
		for (list<SOCKET>::iterator it = this->SocketList->begin(); it != this->SocketList->end(); it++) {
			FD_SET(*it, &rfds);
			FD_SET(*it, &wfds);
		}

		//等待用户连接请求或用户数据到来或会话socke可发送数据
		//返回有可读或可写的socket的总数
		if ((this->SocketNums = select(0, &this->rfds, &this->wfds, NULL, NULL)) == SOCKET_ERROR)
		{ 
			cout << "Select socket nums error!" << endl;
			return -1;
		}

		//此时有客户端请求到来
		sockaddr_in ClientAddr;  //客户端IP地址
		int AddrLen = sizeof(ClientAddr);

		//检查srvSocket是否收到用户连接请求
		if (this->SocketNums > 0) {

			if (FD_ISSET(this->ServerSocket, &rfds)) {  //检查有socket就绪
				this->SocketNums--;

				//产生会话socket
				SOCKET sockket = accept(this->ServerSocket, (LPSOCKADDR) & (ClientAddr), &AddrLen);
				if (sockket == INVALID_SOCKET) {
					cout << "Server accept error!\n";
					//TODO:
					/*closesocket(this->ServerSocket);
					WSACleanup();
					return -1;*/
				}

				//将socket设为非阻塞模式以监听客户连接请求
				if (ioctlsocket(sockket, FIONBIO, &blockMode) == SOCKET_ERROR) {
					cout << "Set accept socket unbolockmode error!\n";
					return -1;
				}

				//将产生的会话SOCKET保存在SocketList中
				getpeername(sockket, (struct sockaddr*) & ClientAddr, &AddrLen);
				cout << "Receive Ip:" << inet_ntoa(ClientAddr.sin_addr) << " from Port:" << ntohs(ClientAddr.sin_port) << " request";
				cout << "" << endl;
				this->AddSocket(sockket);
				cout << "Server accept success!" << endl;
			}
		}

		//接收用户请求
		if (this->SocketNums > 0) {
			//遍历socket队列，检查是否有数据到来
			for (list<SOCKET>::iterator it = this->SocketList->begin(); it != this->SocketList->end(); it++) {
				if (*it != INVALID_SOCKET) {
					//FD_ISSET(int fd,fd_set *fdset);用于测试指定的文件描述符是否在该集合中。
					if (FD_ISSET(*it, &rfds)) {  //某会话socket有数据到来
						SOCKADDR_IN RequestSocket;
						int socketAddrLen = sizeof(RequestSocket);
						getpeername(*it, (struct sockaddr*) & RequestSocket, &socketAddrLen);
						/*cout << "Receive Ip:" << inet_ntoa(RequestSocket.sin_addr) << " from Port:" << ntohs(RequestSocket.sin_port) << " request";
						cout << "" << endl;*/
						thread talkThread(&Server::RecvMessage, std::ref(*this), *it);  //开启一个新的线程来处理这一个客户的请求
						talkThread.detach();
					}
				}
			}
		}
		
		//从socketList里删掉已经关闭的socket
		this->RemoveClosedSocket();
	}

	return 0;
}

