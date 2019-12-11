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

	if (this->base < (this->base + slideWindowSize) % seqSize) //����һ��
		return seq >= this->base && seq < (this->base + slideWindowSize) % seqSize;
	else  //��Խ������β
		return seq >= this->base || seq < (this->base + slideWindowSize) % seqSize;
}

//�ϲ���÷�������
bool GBNSender::send(Message& message) {

	if (getWaitingState()) {  //���Ķ�������
		cout << "\n���ͷ�������������" << endl;
			return false;
	}

	//���챨��
	Packet& sendPkt = *new Packet;
	sendPkt.acknum = -1;
	sendPkt.seqnum = this->nextSeqNum;
	memcpy(sendPkt.payload, message.data, sizeof(message.data));  //���Ķ�����
	sendPkt.checksum = pUtils->calculateCheckSum(sendPkt);
	pUtils->printPacket("\n���ͷ����ͱ���:", sendPkt);
	queue.push_back(sendPkt);  //���ļ������

	//��ʾ��������
	cout << "\n���ͷ��Ļ������ڣ�";
	this->printSlideWindow();
	cout << "" << endl;

	if (base == nextSeqNum){ //��ǰ������û�з��ͻ�δȷ�ϵģ�baseλ��ֱ���ǿ��û�δ���͵�
		timerSeq = base;
		pns->startTimer(SENDER, Configuration::TIME_OUT, timerSeq);		//���ͷŶ�ʱ��
	}

	pns->sendToNetworkLayer(RECEIVER, sendPkt);  //���͵������
	nextSeqNum = (nextSeqNum + 1) % seqSize;

	//��ʾ��������
	cout << "\n���ͷ������걨�ĺ�Ļ������ڣ�";
	this->printSlideWindow();
	cout << "" << endl;
	return true;
}

//����ack����
void GBNSender::receive(Packet& ackPkt) {

	int checkSum = pUtils->calculateCheckSum(ackPkt);

	if (checkSum != ackPkt.checksum){
		pUtils->printPacket("\n���ͷ����յ��𻵵�ack��У��Ͳ����", ackPkt);
		return;
	}

	//��ȷ�յ�ack����
	pUtils->printPacket("\n���ͷ���ȷ�յ�ack���ģ�", ackPkt);
	for (int i = 0; i < (ackPkt.acknum - base + 1 +seqSize) % seqSize; i++){
		queue.erase(queue.begin(), queue.begin() + 1);
	}
	base = (ackPkt.acknum + 1) % seqSize;//�ƶ�base
	if (base == nextSeqNum){
		pns->stopTimer(SENDER, timerSeq);
		//��ʱû���ѷ��͵���ȷ�ϵı��ĶΣ��رն�ʱ��
	}
	else
	{//������ʱ��
		pns->stopTimer(SENDER, timerSeq);
		pns->startTimer(SENDER, Configuration::TIME_OUT, timerSeq);
	}
	//��ʾ��������
	cout << "\n���ͷ��Ļ������ڣ�";
	this->printSlideWindow();
	cout << "" << endl;
}

//��ʱ�¼�
void GBNSender::timeoutHandler(int seqNum) {

	pns->stopTimer(SENDER, seqNum);
	cout << "\n���ͳ�ʱ"<<endl;
	//����N��
	if (base != nextSeqNum){ //��������֮ǰ�ѷ��ͣ���ʱ��Ҫ�ط�������
		pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum); //������ʱ�������¼�ʱ
		for (auto i : queue) {
			pns->sendToNetworkLayer(RECEIVER, i);
			pUtils->printPacket("\n��ʱ�ش�����:", i);
		}
	}
	cout << "\n��ʱ�ش��������"<<endl;
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