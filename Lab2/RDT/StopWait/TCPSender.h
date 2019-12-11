#pragma once
#include "RdtSender.h"

using namespace std;

class TCPSender : public RdtSender
{
private:
	int base;  //�����
	int nextSeq;
	int windowsize;  //���ڴ�С
	int seqSize;
	//list<Packet> queue;
	Packet* queue;
	//int *ackNum;  //��¼3���ظ���ack
	int ackNum;
	int timerSeq;  //timer�����

	bool isInWindow(int seq);
	void printSlideWindow();

public:
	TCPSender();
	virtual ~TCPSender();
	bool getWaitingState();
	bool send( Message &message);
	void timeoutHandler(int seqNum);
	void receive( Packet &ackPkt);
};