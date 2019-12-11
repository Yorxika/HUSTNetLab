#include "stdafx.h"
#include "SRSender.h"
#include "Tool.h"
#include "Global.h"

SRSender::SRSender() :
	seqSize(8), windowsize(4), base(0), nextSeq(0),timerSeq(0)
{
	Packet ackPacket;
	ackPacket.acknum = -1;
	ackPacket.seqnum = -1;
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++)
		ackPacket.payload[i] = '.';
	ackPacket.checksum = 0;
	ackPacket.checksum = pUtils->calculateCheckSum(ackPacket);
	queue = new recvPacket[this->seqSize];
	for (int i = 0; i < this->seqSize; i++)
		queue[i] = { ackPacket, false};
}

SRSender::~SRSender() 
{
	delete[]queue;
}

bool SRSender::isInWindow(int seq) {
	if (base < (base + windowsize) % seqSize)
		return seq >= base && seq < (base + windowsize) % seqSize;
	else
		return seq >= base || seq < (base + windowsize) % seqSize;
}

bool SRSender::getWaitingState() {
	//return (nextSeq - base + seqSize) % seqSize >= windowsize;
	return (base + windowsize) % seqSize == (nextSeq) % seqSize;
}

bool SRSender::send(Message& message) {
	if (this->getWaitingState()){//发送方处于等待确认状态
		cout << "\n发送方滑动窗口已满" << endl;
		return false;
	}

	//构造要发送的数据
	Packet& packet = *new Packet;
	packet.acknum = -1;
	packet.seqnum = this->nextSeq;
	memcpy(packet.payload, message.data, sizeof(message.data));
	packet.checksum = pUtils->calculateCheckSum(packet);
	pUtils->printPacket("\n发送方发送报文", packet);
	//queue[((packet.seqnum - base) + this->seqSize) % this->seqSize] = { packet, false };  //放入窗口队列
	queue[packet.seqnum] = { packet, false };

	//显示滑动窗口
	cout << "\n发送方发送前的滑动窗口: ";
	this->printSlideWindow();
	cout << "" << endl;

	//发送报文
	timerSeq = packet.seqnum;
	pns->startTimer(SENDER, Configuration::TIME_OUT, timerSeq);  //启动发送方定时器
	pns->sendToNetworkLayer(RECEIVER, packet);  //通过网络层发送报文
	nextSeq = (nextSeq + 1) % seqSize;

	//显示滑动窗口
	cout << "\n发送方发送后的滑动窗口: ";
	this->printSlideWindow();
	cout << "" << endl;
	return true;
}

void SRSender::receive(Packet& packet) {
	int checkSum = pUtils->calculateCheckSum(packet);
	if (checkSum != packet.checksum) {
		pUtils->printPacket("\n发送方接收到的ack校验和不正确", packet);
		return;
	}
	else {
		pns->stopTimer(SENDER, packet.acknum);
		if (isInWindow(packet.acknum)) {
			//更新窗口
			queue[packet.acknum].second = true;
			while (queue[base].second) {
				//移动基序号
				queue[base].second = false;
				base = (base + 1) % seqSize;
			}
			//显示窗口
			cout << "\n发送方收到ack，窗口移动：";
			printSlideWindow();
			cout << "" << endl;
		}
	}
}

void SRSender::timeoutHandler(int seq) {
	//pns->stopTimer(SENDER, seq);
	cout << "\n发送方发送数据包超时：" << seq << endl;
	pns->sendToNetworkLayer(RECEIVER, queue[seq].first);  //重发这个包
	pns->startTimer(SENDER, Configuration::TIME_OUT, seq);//重启计时器
	pUtils->printPacket("\n发送方重传数据包完毕：", queue[seq].first);
}

void SRSender::printSlideWindow() {
	for (int i = 0; i < this->seqSize; i++) {
		if (i == this->base)
			cout << "[";
		else if (i == (this->base + this->windowsize) % this->seqSize)
			cout << "] ";
		cout << i;
		if (isInWindow(i) && (i >= this->nextSeq))
			cout << "可用 ";
		else if (isInWindow(i) && (queue[i].second))  //发送了且被确认
			cout << "已确认 ";
		else if(isInWindow(i))
		cout << "已发送 ";
		if (!isInWindow(i))
			cout << "不可用 ";
	}
	cout << "" << endl;
}