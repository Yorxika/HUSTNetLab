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
	bool send( Message &message);  //发送应用层下来的Message，由NetworkServiceSimulator调用,如果发送方成功地将Message发送到网络层，返回true;如果因为发送方处于等待正确确认状态而拒绝发送Message，则返回false
	void timeoutHandler(int seq);  //Timeout handler，将被NetworkServiceSimulator调用
	void receive( Packet& ackPacket);  //接受确认Ack，将被NetworkServiceSimulator调用	

private:
	int base;  //最早的未确认分组的序号
	int nextSeqNum;  //下一个等待发送分组的序号
	int timerSeq;  //定时器序号
	int slideWindowSize;  //滑动窗口大小
	int seqSize;  //报文序号大小
	std::vector<Packet> queue;  //窗口中的报文队列

	bool isInWindow(int seq);
	void printSlideWindow();  //打印滑动窗口
};
