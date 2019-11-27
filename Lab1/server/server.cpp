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
	this->RecvBuf = new char[Config::BufferLength]; //��ʼ�����ܻ�����
	memset(this->RecvBuf, '\0', Config::BufferLength); // ������Ϊ0
	this->SendBuf = new char[Config::BufferLength]; //��ʼ�����ͻ�����
	memset(this->SendBuf, '\0', Config::BufferLength); // ������Ϊ0

	this->SocketList = new list<SOCKET>();  //��ʼ���й�List��Map
	this->ClosedSocketList = new list<SOCKET>();
}

Server::~Server(void)
{
	if (this->RecvBuf != NULL) {
		//�ͷŽ��ջ�����
		free(this->RecvBuf);
		this->RecvBuf = NULL;
		cout <<"RecvBuf has been free!" << endl;
	}

	if (this->SendBuf != NULL) {
		//�ͷŽ��ջ�����
		free(this->SendBuf);
		this->SendBuf = NULL;
		cout << "SendBuf has been free!" << endl;
	}

	//�رշ�������socket
	if (this->ServerSocket != NULL) {
		closesocket(this->ServerSocket);
		this->ServerSocket = NULL;
		cout << "ServerSocket has been closed!" << endl;
	}

	//�ر����лỰsocket���ͷŻỰ����
	if (this->SocketList != NULL) {
		for (list<SOCKET>::iterator it = this->SocketList->begin(); it != this->SocketList->end(); it++)
			closesocket(*it); //�رջỰ
		delete this->SocketList;  //�ͷŶ���
		this->SocketList = NULL;
		cout << "SocketList has been deleted!" << endl;
	}

	//�ͷ�ʧЧ�Ự����
	if (this->ClosedSocketList != NULL) {
		for (list<SOCKET>::iterator it = this->ClosedSocketList->begin(); it != this->ClosedSocketList->end(); it++)
			closesocket(*it);
		delete this->ClosedSocketList;
		this->ClosedSocketList = NULL;
		cout << "ClosedSocketList has been deleted!" << endl;
	}

	WSACleanup(); //����winsock ���л���
}

//�ж��Ƿ���ĳһ�ض���׺����β
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

//��ʼ��Winsock
int Server::WinsockStartup() {

	WSADATA wsaData;//���ڷ���Winsock�Ļ�����Ϣ   
	int nRc = WSAStartup(0x0202, &wsaData);

	if (nRc) {
		//Winsock��ʼ��ʧ��
		cout << "WSAstartup error!\n";
		return -1;
	}

	//�жϷ��ص�Winsock�汾��
	if (wsaData.wVersion != 0x0202) { //����汾����
		cout << "Server's winsock version error!";
		WSACleanup();  //���Winsock
		return -1;
	}

	cout << "Server's winsock startup success!\n";
	return 0;
}

//��ʼ��Server
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

	//���� TCP socket
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

///�رշ������߳�
int Server::closeServer() {
	cout << "If you want to close server, please input exit" <<endl ;
	std::string Exit;
	cin >> Exit;
	while (1) {
		if (Exit == "exit")
			break;
	}
	//��������ζ��Ҫ�رշ�����
	this->~Server();
	exit(0);
}

//��ʼ�������̣��ȴ��ͻ�������
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

//���µĻỰsocket�������
void Server::AddSocket(SOCKET socket) {
	if (socket != INVALID_SOCKET) {
		this->SocketList->insert(this->SocketList->end(), socket);
	}
}

//��ʧЧ�ĻỰsocket����ʧЧ����
void Server::AddClosedSocket(SOCKET socket) {
	if (socket != INVALID_SOCKET) {
		this->ClosedSocketList->insert(this->ClosedSocketList->end(), socket);
	}
}

//��ʧЧ��socket�ӻỰsocket����ɾ��
void Server::RemoveClosedSocket(SOCKET socket) {
	if (socket != INVALID_SOCKET) {
		list<SOCKET>::iterator it = find(this->SocketList->begin(), this->SocketList->end(), socket);
		if (it != this->SocketList->end()) {
			this->SocketList->erase(it);
			//closesocket(*it);
		}
	}
}

//������ʧЧ��socket�ӻỰsocket����ɾ��
void Server::RemoveClosedSocket() {
	for (list<SOCKET>::iterator it = this->ClosedSocketList->begin(); it != this->ClosedSocketList->end(); it++) {
		/*if ((*it) != INVALID_SOCKET) {
			if (it != this->SocketList->end())
				this->SocketList->erase(it);
		}*/
		this->RemoveClosedSocket(*it);
	}
}

//��socket������Ϣ
void Server::SendMessage(SOCKET socket, string msg) {
	int rtn = send(socket, msg.c_str(), msg.length(), 0);
	unique_lock<mutex>closedList(this->ClosedList2, defer_lock);
	if (rtn == SOCKET_ERROR) {					//�������ݴ���
		closedList.lock();
		this->AddClosedSocket(socket);
		closedList.unlock();
	}
}

//��socket������Ϣ
void Server::RecvMessage(SOCKET socket) {

	unique_lock<mutex>writeRecBuf(this->lockRecBuf, defer_lock);  //������Դд�źŵ�
	unique_lock<mutex>readRecBuf(this->lockRecBuf2, defer_lock);  //������Դ���źŵ�
	unique_lock<mutex>sendHeader(this->sendHeader, defer_lock);  //���ͱ���ͷ���źŵ�
	unique_lock<mutex>sendMessage(this->sendMessage, defer_lock);  //���ͱ��������źŵ�
	unique_lock<mutex>closedList(this->ClosedList, defer_lock);  //���ͱ��������źŵ�

	//writeRecBuf.lock();   //дRecBufʱ��Ҫ����
	char* RecvBufn = new char[Config::BufferLength];
	memset(RecvBufn, '\0', sizeof(RecvBufn)); //��������
	int receivedBytes = recv(socket, RecvBufn, Config::BufferLength, 0);
	//writeRecBuf.unlock();

	//����ỰSOCKET�����ݵ���������ܿͻ�������
	if (receivedBytes == SOCKET_ERROR) {    //�������ݴ���
		/*receivedBytes = WSAGetLastError();
		cout << "Thread falied!" << std::endl;
		return;*/
		//cout << "ReceiveBytes == -1,Receive Error!" << endl;
		closedList.lock();
		this->AddClosedSocket(socket);
		closedList.unlock();
	}
	else if (receivedBytes == 0) { //�Է��Ͽ�����
		//cout << "ReceiveBytes ==0,Server disconnected!" << std::endl;
		closedList.lock();
		this->AddClosedSocket(socket);
		closedList.unlock();
		//return;
	}
	else {
		//cout << "\r\n" << RecvBuf;
		//�ҵ�����ͷ��
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

		//�ҵ�HTTP����ͷ��
		//readRecBuf.lock();  //��RecBufʱ��Ҫ����
		strRecv.assign(RecvBufn);
		size_t headerEnd = strRecv.find("\r\n\r\n");
		clientHttpHeader = strRecv.substr(0, headerEnd);
		
		//������ʽ��ȡ�ؼ���method(GET,POST),url
		if (std::regex_search(clientHttpHeader, matchRst, regRequest)) {
			method = matchRst[1].str();
			url = matchRst[2].str();
		}

		for (int i = 0; i < url.size(); i++) {
			//�滻��windows·��
			if (url[i] == '/')
				url[i] = '\\';
		}

		fullPath = Config::mainPath + url; //����·��
		//writeRecBuf.unlock();
		//readRecBuf.unlock();

		cout << "Client http header:\r\n" << clientHttpHeader.c_str() << endl;
		cout << "\r\nmethod:" << method.c_str() << endl;
		cout << "url:" << url.c_str() << endl;

		//sendHeader.lock();  //���ͱ���ͷ���źŵ�

		DWORD ftyp;
		ftyp = GetFileAttributesA(fullPath.c_str());
		if ((ftyp & FILE_ATTRIBUTE_DIRECTORY) && (!INVALID_FILE_ATTRIBUTES)) {
			//�����һ��Ŀ¼���򿪸�Ŀ¼�µ�����index.html
			fullPath = fullPath + "\\index.html";
		}

		FILE* infile = fopen(fullPath.c_str(), "rb");

		if (!infile) {
			//�ļ�������
			//TODO:�����Զ���404ҳ��
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
			if (infile != NULL)  //�ļ����ڣ����ǲ�֧�ֽ���
				fclose(infile);
			//TODO:�����Զ���501ҳ��
			infile = fopen((Config::mainPath + "\\501.html").c_str(), "rb");
			statusCode = Config::Error;
			firstHeader = "HTTP/1.1 501 Not Implemented\r\n";
			typeStr = "Content-Type: text/html\r\n";
		}

		//��ȡ�ļ���С
		fseek(infile, 0, SEEK_SET);
		fseek(infile, 0, SEEK_END);
		int fileLength = ftell(infile);
		//�ļ�ָ���λ
		fseek(infile, 0, SEEK_SET);

		respondHttpHeader = firstHeader + typeStr + "Content-Length: " + std::to_string(fileLength) + "\r\n" + "Server: CServer_HTTP1.1\r\n" + "Connection: close\r\n" + "\r\n";
		//���ͱ���ͷ��
	   //send(socket, respondHttpHeader.c_str, respondHttpHeader.length(), 0);
		this->SendMessage(socket, respondHttpHeader);
		//sendHeader.unlock();

		cout << "respond http header:" << respondHttpHeader.c_str();

		//����������ļ�
		//sendMessage.lock();
		int bufReadNum;
		while (true) {
			//��������
			memset(RecvBufn, 0, sizeof(RecvBufn));
			bufReadNum = fread(RecvBufn, 1, Config::BufferLength, infile);
			if (SOCKET_ERROR == (send(socket, RecvBufn, bufReadNum, 0)))
			{//����ʧ��
				//rtn = SOCKET_ERROR;							
				cout << "File: " << url << " transfer error!" << endl;
				break;
			}
			if (feof(infile))
				break;
		}
		fclose(infile);
		//memset(RecvBufn, '\0', Config::BufferLength);//������ܻ�����
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

//���ܿͻ��˷������������Ӳ����ɻỰsocket
int Server::ServerWork() {

	u_long blockMode = Config::uNonBlock;
	int rtn;
	//nThread.MemberProc = &Server::RecvMessage;

	//��ServerSocket��Ϊ������ģʽ�Լ����ͻ���������
	if ((rtn = ioctlsocket(this->ServerSocket, FIONBIO, &blockMode) == SOCKET_ERROR)) { //FIONBIO��������ֹ�׽ӿ�s�ķ�����ģʽ��
		cout << "Set Server unblockMode error!"<<endl;
		return -1;
	}

	while (1) {
		//�ȵ��ͻ��˵���������

		//����socket��
		//FD_ZERO(fd_set *fdset);��ָ�����ļ�����������գ��ڶ��ļ����������Ͻ�������ǰ�����������г�ʼ����
		//�������գ�������ϵͳ�����ڴ�ռ��ͨ����������մ������Խ���ǲ���֪�ġ�
		FD_ZERO(&this->rfds);  
		FD_ZERO(&this->wfds);
		
		//FD_SET(fd_set *fdset);�������ļ�����������������һ���µ��ļ���������
		//��serverSocket���뵽rfds���ȴ��û���������
		FD_SET(this->ServerSocket, &this->rfds);

		//�ѵ�ǰ�ĻỰsocket���뵽rfds,�ȴ��û����ݵĵ���;�ӵ�wfds���ȴ�socket�ɷ�������
		for (list<SOCKET>::iterator it = this->SocketList->begin(); it != this->SocketList->end(); it++) {
			FD_SET(*it, &rfds);
			FD_SET(*it, &wfds);
		}

		//�ȴ��û�����������û����ݵ�����Ựsocke�ɷ�������
		//�����пɶ����д��socket������
		if ((this->SocketNums = select(0, &this->rfds, &this->wfds, NULL, NULL)) == SOCKET_ERROR)
		{ 
			cout << "Select socket nums error!" << endl;
			return -1;
		}

		//��ʱ�пͻ���������
		sockaddr_in ClientAddr;  //�ͻ���IP��ַ
		int AddrLen = sizeof(ClientAddr);

		//���srvSocket�Ƿ��յ��û���������
		if (this->SocketNums > 0) {

			if (FD_ISSET(this->ServerSocket, &rfds)) {  //�����socket����
				this->SocketNums--;

				//�����Ựsocket
				SOCKET sockket = accept(this->ServerSocket, (LPSOCKADDR) & (ClientAddr), &AddrLen);
				if (sockket == INVALID_SOCKET) {
					cout << "Server accept error!\n";
					//TODO:
					/*closesocket(this->ServerSocket);
					WSACleanup();
					return -1;*/
				}

				//��socket��Ϊ������ģʽ�Լ����ͻ���������
				if (ioctlsocket(sockket, FIONBIO, &blockMode) == SOCKET_ERROR) {
					cout << "Set accept socket unbolockmode error!\n";
					return -1;
				}

				//�������ĻỰSOCKET������SocketList��
				getpeername(sockket, (struct sockaddr*) & ClientAddr, &AddrLen);
				cout << "Receive Ip:" << inet_ntoa(ClientAddr.sin_addr) << " from Port:" << ntohs(ClientAddr.sin_port) << " request";
				cout << "" << endl;
				this->AddSocket(sockket);
				cout << "Server accept success!" << endl;
			}
		}

		//�����û�����
		if (this->SocketNums > 0) {
			//����socket���У�����Ƿ������ݵ���
			for (list<SOCKET>::iterator it = this->SocketList->begin(); it != this->SocketList->end(); it++) {
				if (*it != INVALID_SOCKET) {
					//FD_ISSET(int fd,fd_set *fdset);���ڲ���ָ�����ļ��������Ƿ��ڸü����С�
					if (FD_ISSET(*it, &rfds)) {  //ĳ�Ựsocket�����ݵ���
						SOCKADDR_IN RequestSocket;
						int socketAddrLen = sizeof(RequestSocket);
						getpeername(*it, (struct sockaddr*) & RequestSocket, &socketAddrLen);
						/*cout << "Receive Ip:" << inet_ntoa(RequestSocket.sin_addr) << " from Port:" << ntohs(RequestSocket.sin_port) << " request";
						cout << "" << endl;*/
						thread talkThread(&Server::RecvMessage, std::ref(*this), *it);  //����һ���µ��߳���������һ���ͻ�������
						talkThread.detach();
					}
				}
			}
		}
		
		//��socketList��ɾ���Ѿ��رյ�socket
		this->RemoveClosedSocket();
	}

	return 0;
}

