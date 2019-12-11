#include "stdafx.h"
#include "Global.h"
#include "TCPReceiver.h"

TCPReceiver::TCPReceiver() :
	windowsize(4), seqSize(8), nextSeq(0)
{
	lastAck.acknum = 1;
	lastAck.acknum = -1;
	lastAck.seqnum = -1;
	memset(lastAck.payload, '.', Configuration::PAYLOAD_SIZE);
	lastAck.checksum = pUtils->calculateCheckSum(lastAck);
}

TCPReceiver::~TCPReceiver()
{
}

void TCPReceiver::receive(Packet& packet) {
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(packet);

	//如果校验和正确，同时收到报文的序号等于接收方期待收到的报文序号一致
	if (checkSum == packet.checksum && this->nextSeq == packet.seqnum) {
		pUtils->printPacket("\n接收方正确收到发送方的报文", packet);

		//取出Message，向上递交给应用层
		Message msg;
		memcpy(msg.data, packet.payload, sizeof(packet.payload));
		pns->delivertoAppLayer(RECEIVER, msg);

		//lastAck.acknum = (packet.seqnum + 1) % this->seqSize; //下一个希望收到的序号
		lastAck.acknum = (packet.seqnum);
		lastAck.checksum = pUtils->calculateCheckSum(lastAck);
		pUtils->printPacket("\n接收方发送确认报文", lastAck);
		pns->sendToNetworkLayer(SENDER, lastAck);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方
		this->nextSeq = (packet.seqnum + 1) % this->seqSize; //期待收到下一个序号
	}
	else {
		if (checkSum != packet.checksum) {
			pUtils->printPacket("\n接收方没有正确收到发送方的报文,数据校验错误", packet);
			pUtils->printPacket("\n接收方重新发送上次的确认报文", lastAck);
			pns->sendToNetworkLayer(SENDER, lastAck);//调用模拟网络环境的sendToNetworkLayer，通过网络层发送上次的确认报文
			return;
		}
		else {
			pUtils->printPacket("\n接收方没有正确收到发送方的报文,报文序号不对", packet);
			pUtils->printPacket("\n接收方重新发送上次的确认报文", lastAck);
			pns->sendToNetworkLayer(SENDER, lastAck);//调用模拟网络环境的sendToNetworkLayer，通过网络层发送上次的确认报文
			return;
		}
	}
}