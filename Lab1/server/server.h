#include <winsock2.h>
#include <list>
#include <string>
#include <map>
#include<mutex>

using namespace std;

class Server
{
private:
	SOCKET ServerSocket;  //������socket
	sockaddr_in ServerAddr;		//��������IP��ַ
	char* RecvBuf;  //���ܻ�����
	char* SendBuf;  //���ͻ�����
	int SocketNums;//socket����

	fd_set rfds;				//���ڼ��socket�Ƿ������ݵ����ĵ��ļ�������
	fd_set wfds;				//���ڼ��socket�Ƿ���Է��͵��ļ�������

	mutex lockRecBuf;  //д���ջ������źŵ�
	mutex lockRecBuf2;  //�����ջ������źŵ�
	mutex sendHeader;  //���ͱ���ͷ���źŵ�
	mutex sendMessage;  //���ͱ��������źŵ�
	mutex ClosedList;  //ʧЧ�����źŵ�
	mutex ClosedList2;  //ʧЧ�����źŵ�

	list<SOCKET>* threadsessions; //�̶߳���
	list<SOCKET>* SocketList;  //��ǰ�ĻỰsocket����
	list<SOCKET>* ClosedSocketList;	 //������ʧЧ�ĻỰsocket����

public:

	Server(void);
	virtual ~Server(void);

	int closeServer(); //�رշ�����
	void RecvMessage(SOCKET socket);  //��SOCKET������Ϣ
	void SendMessage(SOCKET socket, string msg);  //��SOCKET������Ϣ
	bool endWith(const std::string& fullStr, const std::string& endStr); //�ж��Ƿ����ض���ʽ��β
	int WinsockStartup();  //��ʼ��Winsock
	int ServerStartup();  //��ʼ��Server����������SOCKET���󶨵�IP��PORT
	int ListenStartup();  //��ʼ�����ͻ�������
	int ServerWork();	  //�տͻ������� ���Ӳ����ɻỰsocket
	void AddSocket(SOCKET socket);  //���µĻỰSOCKET�������
	void AddClosedSocket(SOCKET socket); //��ʧЧ�ĻỰSOCKET����ʧЧ����
	void RemoveClosedSocket(SOCKET socket);  //��ʧЧ��SOCKET�ӻỰSOCKET����ɾ��
	void RemoveClosedSocket();  //������ʧЧ��SOCKET�ӻỰSOCKET����ɾ��
};
