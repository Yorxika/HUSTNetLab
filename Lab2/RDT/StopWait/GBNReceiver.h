#pragma once
#include "RdtReceiver.h"

class GBNReceiver : public RdtReceiver
{
private:
	int exSeqNum;  //期待收到的队列序号
	int seqSize;  //队列大小
	Packet lastAck;  //最后一个包裹的ACK

public:
	GBNReceiver();
	virtual ~GBNReceiver();
	void receive( Packet& aPacket);
};