#include <string>
#include <winsock2.h>

using namespace std;

class Config
{
public:
	static int ServerPort;  //�������˿ں�
	static const int ClientPort;   //�ͻ��˶˿�
	static string ServerAddr;   //������IP��ַ
	static const int MaxConnetion;  //���������
	static const int BufferLength;   //��������С
    static string mainPath; //������·��
	static const string Ok;  //�ͻ�������ɹ�
	static const string Error;   //�ͻ����﷨���󣬷������δʵ��
	static const string NotFound;   //�ļ�������
	static const string HttpVersion;   //HTTP�汾
	static const u_long uNonBlock;  //SOCKET����ģʽ
private:
	Config();
	~Config();
};

