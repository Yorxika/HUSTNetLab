#pragma once
#include "RdtReceiver.h"

class GBNReceiver : public RdtReceiver
{
private:
	int exSeqNum;  //�ڴ��յ��Ķ������
	int seqSize;  //���д�С
	Packet lastAck;  //���һ��������ACK

public:
	GBNReceiver();
	virtual ~GBNReceiver();
	void receive( Packet& aPacket);
};