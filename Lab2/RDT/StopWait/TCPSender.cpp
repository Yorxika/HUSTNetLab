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
	if (this->getWaitingState()){  //���ͷ����ڵȴ�ȷ��״̬
		cout << "\n���ͷ�������������" << endl;
		return false;
	}

	//����Ҫ���͵İ�
	Packet& send = *new Packet;
	send.acknum = -1;
	send.seqnum = this->nextSeq;
	memcpy(send.payload, message.data, sizeof(message.data));
	send.checksum = pUtils->calculateCheckSum(send);
	pUtils->printPacket("\n���ͷ����ͱ���", send);

	//��ʾ��������
	cout << "\n���ͷ��������ڣ� ";
	printSlideWindow();
	cout << "" << endl;

	queue[send.seqnum] = send;//���뻺�����
	if (base == nextSeq){
		timerSeq = base;
		pns->startTimer(SENDER, Configuration::TIME_OUT, timerSeq);  //�������ͷ���ʱ��
	}
	pns->sendToNetworkLayer(RECEIVER,send);
	nextSeq = (nextSeq + 1) % seqSize;//��Ҫ����ȡģ����

	//��ʾ��������
	cout << "\n���ͷ����ͺ�Ļ������ڣ� ";
	printSlideWindow();
	cout << "" << endl;

	return true;
}

void TCPSender::timeoutHandler(int seq) {
	//�����ش���������ݰ�
	cout << "\n�������ݰ���ʱ" << endl;
	if (base != nextSeq) {
		//������������
		pns->stopTimer(SENDER, seq);
		pns->sendToNetworkLayer(RECEIVER, queue[base]);  //ֻ�ش��� �緢����û��ȷ�ϵı���
		ackNum = 0;
		pns->startTimer(SENDER, Configuration::TIME_OUT, queue[base].seqnum);
	}
	else
		pns->stopTimer(SENDER, seq);
}

void TCPSender::receive(Packet& ack) {

	int checkSum = pUtils->calculateCheckSum(ack);
	if (checkSum != ack.checksum) {
		pUtils->printPacket("\n���ͷ����յ��𻵵�ack", ack);
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
		cout << "\n���ͷ��յ�ack:" << ack.acknum<<endl;
		cout << "\n���ͷ��ƶ�����: ";
		printSlideWindow();
	}
	else{  //�յ������ack
		//ackNum[ack.acknum]++;
		ackNum++;
		//if (ackNum[ack.acknum] == 3){//3������ack
		if(ackNum == 3){
			for (int i = 0; i < seqSize; i++) {
				if (queue[i].seqnum == ack.acknum) {
					pns->sendToNetworkLayer(RECEIVER, queue[i]);
					pUtils->printPacket("\n�յ���������ack�������ش�����", queue[i]);
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