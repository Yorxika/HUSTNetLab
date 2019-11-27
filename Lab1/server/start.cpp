#include "server.h"
#include <iostream>

//�رշ������߳�
int closeServer(Server *server) {
	cout << "If you want to close server, please input exit" << endl;
	std::string Exit;
	cin >> Exit;
	while (1) {
		if (Exit == "exit")
			break;
	}
	//��������ζ��Ҫ�رշ�����
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