#include <string>
#include <winsock2.h>

using namespace std;

class Config
{
public:
	static int ServerPort;  //服务器端口号
	static const int ClientPort;   //客户端端口
	static string ServerAddr;   //服务器IP地址
	static const int MaxConnetion;  //最大连接数
	static const int BufferLength;   //缓冲区大小
    static string mainPath; //虚拟主路径
	static const string Ok;  //客户端请求成功
	static const string Error;   //客户端语法错误，服务端尚未实现
	static const string NotFound;   //文件不存在
	static const string HttpVersion;   //HTTP版本
	static const u_long uNonBlock;  //SOCKET阻塞模式
private:
	Config();
	~Config();
};

