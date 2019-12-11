#pragma once
#include "RdtReceiver.h"
#include <utility>

typedef std::pair<Packet, bool> recvPacket;  //接收报文二元组

class SRReceiver : public RdtReceiver
{
public:
	SRReceiver();
	~SRReceiver();
	void receive( Packet& packet);

private:
	int windowssize;  //滑动窗口大小
	int seqSize;  //队列大小
	int base;  //下一个基序号
	Packet laskAck;  //最后的ACK报文
	recvPacket* queue;  //报文缓存队列

	bool isInWindow(int seq);
	void printSlideWindow();
};
