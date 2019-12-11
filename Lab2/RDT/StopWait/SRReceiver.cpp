#include "stdafx.h"
#include "SRReceiver.h"
#include "Global.h"

SRReceiver::SRReceiver() :
	seqSize(8), windowssize(4),base(0)
{
	laskAck.acknum = -1;
	laskAck.seqnum = -1;
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++)
		laskAck.payload[i] = '.';
	laskAck.checksum = 0;
	laskAck.checksum = pUtils->calculateCheckSum(laskAck);
	queue = new recvPacket[seqSize];
	for (int i = 0; i < this->seqSize; i++)
		queue[i] = { laskAck, false};
}


SRReceiver::~SRReceiver()
{
	delete []queue;
}

void SRReceiver::receive(Packet& packet)
{

	int checksum = pUtils->calculateCheckSum(packet);
	if (checksum != packet.checksum){  //数据包损坏
		pUtils->printPacket("\n接收方接收到的报文损坏", packet);
		return;
	}
	else
	{
		if (!isInWindow(packet.seqnum)){//不是窗口内的报文
			pUtils->printPacket("\n不是期待的数据分组", packet);
			laskAck.acknum = packet.seqnum;
			laskAck.seqnum = -1;
			memset(laskAck.payload, '.', Configuration::PAYLOAD_SIZE);
			laskAck.checksum = pUtils->calculateCheckSum(laskAck);
			pns->sendToNetworkLayer(SENDER, laskAck);
			return;
		}
		else{  //是窗口内的报文
			pUtils->printPacket("\n接收方正确收到发送方的报文", packet);
			queue[packet.seqnum] = { packet , true};
			laskAck.acknum = packet.seqnum;  //确认序号
			laskAck.seqnum = 0;
			memset(laskAck.payload, '.', sizeof(laskAck.payload));
			laskAck.checksum = pUtils->calculateCheckSum(laskAck);
			pUtils->printPacket("\n接收方发送ack", laskAck);
			pns->sendToNetworkLayer(SENDER, laskAck);
			while(queue[base].second) {
				//移动窗口
				Message msg;
				memcpy(msg.data, queue[base].first.payload, sizeof(queue[base].first.payload));
				pns->delivertoAppLayer(RECEIVER, msg);
				pUtils->printPacket("\n接收方递交给应用层报文: ", queue[base].first);
				queue[base].second = false;
				base = (base + 1) % seqSize;
			}
			cout << "\n接收方接收到报文，移动窗口: ";
			printSlideWindow();
			cout << "" << endl;
		}
	}
}

bool SRReceiver::isInWindow(int seq) {
	if (base < (base + windowssize) % seqSize)
		return seq >= base && seq < (base + windowssize) % seqSize;
	else
		return seq >= base || seq < (base + windowssize) % seqSize;
}

void SRReceiver::printSlideWindow() {
	for (int i = 0; i < this->seqSize; i++) {
		if (i == this->base)
			cout << "[";
		else if (i == (this->base + this->windowssize) % this->seqSize)
			cout << "] ";
		cout << i;
		if (isInWindow(i) && (queue[i].second))
			cout << "已缓存 ";
		else if (isInWindow(i) && !(queue[i].second))  //发送了且被确认
			cout << "等待 ";
		if (!isInWindow(i))
			cout << "不可用 ";
	}
	cout << "" << endl;
}