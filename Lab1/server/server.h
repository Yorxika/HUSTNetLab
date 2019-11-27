#include <winsock2.h>
#include <list>
#include <string>
#include <map>
#include<mutex>

using namespace std;

class Server
{
private:
	SOCKET ServerSocket;  //服务器socket
	sockaddr_in ServerAddr;		//服务器端IP地址
	char* RecvBuf;  //接受缓冲区
	char* SendBuf;  //发送缓冲区
	int SocketNums;//socket总数

	fd_set rfds;				//用于检查socket是否有数据到来的的文件描述符
	fd_set wfds;				//用于检查socket是否可以发送的文件描述符

	mutex lockRecBuf;  //写接收缓冲区信号灯
	mutex lockRecBuf2;  //读接收缓冲区信号灯
	mutex sendHeader;  //发送报文头部信号灯
	mutex sendMessage;  //发送报文主体信号灯
	mutex ClosedList;  //失效队列信号灯
	mutex ClosedList2;  //失效队列信号灯

	list<SOCKET>* threadsessions; //线程队列
	list<SOCKET>* SocketList;  //当前的会话socket队列
	list<SOCKET>* ClosedSocketList;	 //所有已失效的会话socket队列

public:

	Server(void);
	virtual ~Server(void);

	int closeServer(); //关闭服务器
	void RecvMessage(SOCKET socket);  //从SOCKET接受消息
	void SendMessage(SOCKET socket, string msg);  //向SOCKET发送消息
	bool endWith(const std::string& fullStr, const std::string& endStr); //判断是否以特定格式结尾
	int WinsockStartup();  //初始化Winsock
	int ServerStartup();  //初始化Server，包括创建SOCKET，绑定到IP和PORT
	int ListenStartup();  //开始监听客户端请求
	int ServerWork();	  //收客户机请求 连接并生成会话socket
	void AddSocket(SOCKET socket);  //将新的会话SOCKET加入队列
	void AddClosedSocket(SOCKET socket); //将失效的会话SOCKET加入失效队列
	void RemoveClosedSocket(SOCKET socket);  //将失效的SOCKET从会话SOCKET队列删除
	void RemoveClosedSocket();  //将所有失效的SOCKET从会话SOCKET队列删除
};
