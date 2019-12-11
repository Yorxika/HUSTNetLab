#include "stdafx.h"
#include "Global.h"
#include "TCPSender.h"

TCPSender::TCPSender() :
	windowsize(4), seqSize(8),base(0),nextSeq(0),timerSeq(0)
{
	//ackNum = new int[this->seqSize];
	queue = new Packet[this->seqSize];
	//for (int i = 0; i < this->seqSize; i++)
		//ackNum[i] = 0;
	ackNum = 0;
}


TCPSender::~TCPSender()
{
	delete[]queue;
}

bool TCPSender::isInWindow(int seq) {
	if (base < (base + windowsize) % seqSize)
		return seq >= base && seq < (base + windowsize) % seqSize;
	else
		return seq >= base || seq < (base + windowsize) % seqSize;
}

bool TCPSender::getWaitingState() {
	//return (nextSeq - base + this->seqSize) % this->seqSize >= this->windowsize;
	return (base + windowsize) % seqSize == (nextSeq % seqSize);
}

bool TCPSender::send(Message &message)
{
	if (this->getWaitingState()){  //发送方处于等待确认状态
		cout << "\n发送方滑动窗口已满" << endl;
		return false;
	}

	//构造要发送的包
	Packet& send = *new Packet;
	send.acknum = -1;
	send.seqnum = this->nextSeq;
	memcpy(send.payload, message.data, sizeof(message.data));
	send.checksum = pUtils->calculateCheckSum(send);
	pUtils->printPacket("\n发送方发送报文", send);

	//显示滑动窗口
	cout << "\n发送方滑动窗口： ";
	printSlideWindow();
	cout << "" << endl;

	queue[send.seqnum] = send;//放入缓存队列
	if (base == nextSeq){
		timerSeq = base;
		pns->startTimer(SENDER, Configuration::TIME_OUT, timerSeq);  //启动发送方定时器
	}
	pns->sendToNetworkLayer(RECEIVER,send);
	nextSeq = (nextSeq + 1) % seqSize;//不要忘了取模运算

	//显示滑动窗口
	cout << "\n发送方发送后的滑动窗口： ";
	printSlideWindow();
	cout << "" << endl;

	return true;
}

void TCPSender::timeoutHandler(int seq) {
	//快速重传最早的数据包
	cout << "\n发送数据包超时" << endl;
	if (base != nextSeq) {
		//窗口中有数据
		pns->stopTimer(SENDER, seq);
		pns->sendToNetworkLayer(RECEIVER, queue[base]);  //只重传最 早发送且没被确认的报文
		ackNum = 0;
		pns->startTimer(SENDER, Configuration::TIME_OUT, queue[base].seqnum);
	}
	else
		pns->stopTimer(SENDER, seq);
}

void TCPSender::receive(Packet& ack) {

	int checkSum = pUtils->calculateCheckSum(ack);
	if (checkSum != ack.checksum) {
		pUtils->printPacket("\n发送方接收到损坏的ack", ack);
		return;
	}

	if (isInWindow(ack.acknum)){
		base = (ack.acknum + 1) % seqSize;
		pns->stopTimer(SENDER, ack.seqnum);
		/*for (int i = 0; i < (ack.acknum - this->base + this->seqSize) % this->seqSize; i++)
			queue.pop_front();*/
		if (base == nextSeq) {
			pns->stopTimer(SENDER, timerSeq);
			//ackNum[ack.acknum] = 0;
			//ackNum = 0;
		}
		else {
			pns->stopTimer(SENDER, timerSeq);
			timerSeq = base;
			pns->startTimer(SENDER, Configuration::TIME_OUT, timerSeq);
		}
		ackNum = 0;
		cout << "\n发送方收到ack:" << ack.acknum<<endl;
		cout << "\n发送方移动窗口: ";
		printSlideWindow();
	}
	else{  //收到冗余的ack
		//ackNum[ack.acknum]++;
		ackNum++;
		//if (ackNum[ack.acknum] == 3){//3个冗余ack
		if(ackNum == 3){
			for (int i = 0; i < seqSize; i++) {
				if (queue[i].seqnum == ack.acknum) {
					pns->sendToNetworkLayer(RECEIVER, queue[i]);
					pUtils->printPacket("\n收到三个冗余ack，快速重传报文", queue[i]);
					break;
				}
			}
			//ackNum[ack.acknum] = 0;
			ackNum = 0;
		}
	}
}

void TCPSender::printSlideWindow()
{
	for (int i = 0; i < seqSize; i++)
	{
		if (i == base)
			std::cout << "[";
		std::cout << i;
		if (i == nextSeq)
			std::cout << "*";
		if (i == (base + windowsize - 1) % seqSize)
			std::cout << "]";
		std::cout << " ";
	}
	std::cout << std::endl;
}