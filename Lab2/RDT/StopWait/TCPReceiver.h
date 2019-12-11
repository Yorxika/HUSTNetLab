#pragma once
#include "RdtReceiver.h"

class TCPReceiver : public RdtReceiver {
private:
	int windowsize;
	int seqSize;
	int nextSeq;
	Packet lastAck;
	void printSlideWindow();

public:
	TCPReceiver();
	~TCPReceiver();
	void receive(Packet& packet);
};

