#pragma once
#include "RdtSender.h"
#include <vector>
#include <utility>

typedef std::pair<Packet, bool> recvPacket;  //���ͱ��Ķ�Ԫ��

class SRSender : public RdtSender
{

public:
	SRSender();
	virtual ~SRSender();

	bool getWaitingState();
	bool send( Message& message);  //����Ӧ�ò�������Message����NetworkServiceSimulator����,������ͷ��ɹ��ؽ�Message���͵�����㣬����true;�����Ϊ���ͷ����ڵȴ���ȷȷ��״̬���ܾ�����Message���򷵻�false
	void timeoutHandler(int seq);  //Timeout handler������NetworkServiceSimulator����
	void receive( Packet& ackPacket);  //����ȷ��Ack������NetworkServiceSimulator����	

private:
	int base;  //���ڻ����
	int nextSeq;  //��һ���������
	int windowsize;  //���ڴ�С
	int seqSize;  //���д�С
	int timerSeq;  //timer���
	recvPacket* queue;  //���Ļ������

	bool isInWindow(int seq);
	void printSlideWindow();
};
