#include "stdafx.h"
#include "GBNSender.h"
#include "Global.h"
#include <cstring>

GBNSender::GBNSender():
	slideWindowSize(4),seqSize(8),base(0),nextSeqNum(0),timerSeq(0)
{
}

GBNSender::~GBNSender()
{
}


bool GBNSender::isInWindow(int seq) {

	if (this->base < (this->base + slideWindowSize) % seqSize) //连续一段
		return seq >= this->base && seq < (this->base + slideWindowSize) % seqSize;
	else  //跨越窗口首尾
		return seq >= this->base || seq < (this->base + slideWindowSize) % seqSize;
}

//上层调用发送数据
bool GBNSender::send(Message& message) {

	if (getWaitingState()) {  //报文队列已满
		cout << "\n发送方滑动窗口已满" << endl;
			return false;
	}

	//构造报文
	Packet& sendPkt = *new Packet;
	sendPkt.acknum = -1;
	sendPkt.seqnum = this->nextSeqNum;
	memcpy(sendPkt.payload, message.data, sizeof(message.data));  //报文段数据
	sendPkt.checksum = pUtils->calculateCheckSum(sendPkt);
	pUtils->printPacket("\n发送方发送报文:", sendPkt);
	queue.push_back(sendPkt);  //报文加入队列

	//显示滑动窗口
	cout << "\n发送方的滑动窗口：";
	this->printSlideWindow();
	cout << "" << endl;

	if (base == nextSeqNum){ //当前队列中没有发送还未确认的，base位置直接是可用还未发送的
		timerSeq = base;
		pns->startTimer(SENDER, Configuration::TIME_OUT, timerSeq);		//发送放定时器
	}

	pns->sendToNetworkLayer(RECEIVER, sendPkt);  //发送到网络层
	nextSeqNum = (nextSeqNum + 1) % seqSize;

	//显示滑动窗口
	cout << "\n发送方发送完报文后的滑动窗口：";
	this->printSlideWindow();
	cout << "" << endl;
	return true;
}

//接收ack报文
void GBNSender::receive(Packet& ackPkt) {

	int checkSum = pUtils->calculateCheckSum(ackPkt);

	if (checkSum != ackPkt.checksum){
		pUtils->printPacket("\n发送方接收到损坏的ack，校验和不相等", ackPkt);
		return;
	}

	//正确收到ack报文
	pUtils->printPacket("\n发送方正确收到ack报文：", ackPkt);
	for (int i = 0; i < (ackPkt.acknum - base + 1 +seqSize) % seqSize; i++){
		queue.erase(queue.begin(), queue.begin() + 1);
	}
	base = (ackPkt.acknum + 1) % seqSize;//移动base
	if (base == nextSeqNum){
		pns->stopTimer(SENDER, timerSeq);
		//此时没有已发送但待确认的报文段，关闭定时器
	}
	else
	{//重启计时器
		pns->stopTimer(SENDER, timerSeq);
		pns->startTimer(SENDER, Configuration::TIME_OUT, timerSeq);
	}
	//显示滑动窗口
	cout << "\n发送方的滑动窗口：";
	this->printSlideWindow();
	cout << "" << endl;
}

//超时事件
void GBNSender::timeoutHandler(int seqNum) {

	pns->stopTimer(SENDER, seqNum);
	cout << "\n发送超时"<<endl;
	//回退N步
	if (base != nextSeqNum){ //窗口中有之前已发送，此时需要重发的数据
		pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum); //重启计时器，重新计时
		for (auto i : queue) {
			pns->sendToNetworkLayer(RECEIVER, i);
			pUtils->printPacket("\n超时重传报文:", i);
		}
	}
	cout << "\n超时重传报文完毕"<<endl;
}

bool GBNSender::getWaitingState() {
	return (nextSeqNum - base + seqSize) % seqSize >= slideWindowSize;
}

void GBNSender::printSlideWindow() {
	int i;
	for (i = 0; i < this->seqSize; i++) {
		if (i == this->base)
			cout << "[";
		cout << i;
		if (i == this->nextSeqNum)
			cout << "*";
		if (i == (this->base + this->slideWindowSize - 1) % this->seqSize)
			cout << "]";
		cout << " ";
	}
	cout << "" << endl;
}