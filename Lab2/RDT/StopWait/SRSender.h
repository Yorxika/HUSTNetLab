#pragma once
#include "RdtSender.h"
#include <vector>
#include <utility>

typedef std::pair<Packet, bool> recvPacket;  //发送报文二元组

class SRSender : public RdtSender
{

public:
	SRSender();
	virtual ~SRSender();

	bool getWaitingState();
	bool send( Message& message);  //发送应用层下来的Message，由NetworkServiceSimulator调用,如果发送方成功地将Message发送到网络层，返回true;如果因为发送方处于等待正确确认状态而拒绝发送Message，则返回false
	void timeoutHandler(int seq);  //Timeout handler，将被NetworkServiceSimulator调用
	void receive( Packet& ackPacket);  //接受确认Ack，将被NetworkServiceSimulator调用	

private:
	int base;  //窗口基序号
	int nextSeq;  //下一个发送序号
	int windowsize;  //窗口大小
	int seqSize;  //队列大小
	int timerSeq;  //timer序号
	recvPacket* queue;  //报文缓存队列

	bool isInWindow(int seq);
	void printSlideWindow();
};
