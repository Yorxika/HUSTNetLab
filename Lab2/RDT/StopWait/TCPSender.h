#pragma once
#include "RdtSender.h"

using namespace std;

class TCPSender : public RdtSender
{
private:
	int base;  //基序号
	int nextSeq;
	int windowsize;  //窗口大小
	int seqSize;
	//list<Packet> queue;
	Packet* queue;
	//int *ackNum;  //记录3个重复的ack
	int ackNum;
	int timerSeq;  //timer的序号

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