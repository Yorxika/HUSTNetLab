#pragma once
#include "RdtReceiver.h"
#include <utility>

typedef std::pair<Packet, bool> recvPacket;  //���ձ��Ķ�Ԫ��

class SRReceiver : public RdtReceiver
{
public:
	SRReceiver();
	~SRReceiver();
	void receive( Packet& packet);

private:
	int windowssize;  //�������ڴ�С
	int seqSize;  //���д�С
	int base;  //��һ�������
	Packet laskAck;  //����ACK����
	recvPacket* queue;  //���Ļ������

	bool isInWindow(int seq);
	void printSlideWindow();
};
