#pragma once
#include "RdtSender.h"
#include "DataStructure.h"
#include <vector>

class GBNSender :public RdtSender
{
public:
	GBNSender();
	virtual ~GBNSender();

	bool getWaitingState();
	bool send( Message &message);  //����Ӧ�ò�������Message����NetworkServiceSimulator����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;�����Ϊ���ͷ����ڵȴ���ȷȷ��״̬���ܾ�����Message���򷵻�false
	void timeoutHandler(int seq);  //Timeout handler������NetworkServiceSimulator����
	void receive( Packet& ackPacket);  //����ȷ��Ack������NetworkServiceSimulator����	

private:
	int base;  //�����δȷ�Ϸ�������
	int nextSeqNum;  //��һ���ȴ����ͷ�������
	int timerSeq;  //��ʱ�����
	int slideWindowSize;  //�������ڴ�С
	int seqSize;  //������Ŵ�С
	std::vector<Packet> queue;  //�����еı��Ķ���

	bool isInWindow(int seq);
	void printSlideWindow();  //��ӡ��������
};
