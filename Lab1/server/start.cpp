#include "server.h"
#include <iostream>

//关闭服务器线程
int closeServer(Server *server) {
	cout << "If you want to close server, please input exit" << endl;
	std::string Exit;
	cin >> Exit;
	while (1) {
		if (Exit == "exit")
			break;
	}
	//到这里意味着要关闭服务器
	(*server).~Server();
	exit(0);
}

int main() {
	Server server;
	if (server.WinsockStartup() == -1) return -1;
	if (server.ServerStartup() == -1 ) return -2;
	if (server.ListenStartup() == -1) return -3;
	thread closeThread(closeServer, &server);
	closeThread.detach();
	if (server.ServerWork() == -1) return -4;
}