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
	if (this->getWaitingState()){//���ͷ����ڵȴ�ȷ��״̬
		cout << "\n���ͷ�������������" << endl;
		return false;
	}

	//����Ҫ���͵�����
	Packet& packet = *new Packet;
	packet.acknum = -1;
	packet.seqnum = this->nextSeq;
	memcpy(packet.payload, message.data, sizeof(message.data));
	packet.checksum = pUtils->calculateCheckSum(packet);
	pUtils->printPacket("\n���ͷ����ͱ���", packet);
	//queue[((packet.seqnum - base) + this->seqSize) % this->seqSize] = { packet, false };  //���봰�ڶ���
	queue[packet.seqnum] = { packet, false };

	//��ʾ��������
	cout << "\n���ͷ�����ǰ�Ļ�������: ";
	this->printSlideWindow();
	cout << "" << endl;

	//���ͱ���
	timerSeq = packet.seqnum;
	pns->startTimer(SENDER, Configuration::TIME_OUT, timerSeq);  //�������ͷ���ʱ��
	pns->sendToNetworkLayer(RECEIVER, packet);  //ͨ������㷢�ͱ���
	nextSeq = (nextSeq + 1) % seqSize;

	//��ʾ��������
	cout << "\n���ͷ����ͺ�Ļ�������: ";
	this->printSlideWindow();
	cout << "" << endl;
	return true;
}

void SRSender::receive(Packet& packet) {
	int checkSum = pUtils->calculateCheckSum(packet);
	if (checkSum != packet.checksum) {
		pUtils->printPacket("\n���ͷ����յ���ackУ��Ͳ���ȷ", packet);
		return;
	}
	else {
		pns->stopTimer(SENDER, packet.acknum);
		if (isInWindow(packet.acknum)) {
			//���´���
			queue[packet.acknum].second = true;
			while (queue[base].second) {
				//�ƶ������
				queue[base].second = false;
				base = (base + 1) % seqSize;
			}
			//��ʾ����
			cout << "\n���ͷ��յ�ack�������ƶ���";
			printSlideWindow();
			cout << "" << endl;
		}
	}
}

void SRSender::timeoutHandler(int seq) {
	//pns->stopTimer(SENDER, seq);
	cout << "\n���ͷ��������ݰ���ʱ��" << seq << endl;
	pns->sendToNetworkLayer(RECEIVER, queue[seq].first);  //�ط������
	pns->startTimer(SENDER, Configuration::TIME_OUT, seq);//������ʱ��
	pUtils->printPacket("\n���ͷ��ش����ݰ���ϣ�", queue[seq].first);
}

void SRSender::printSlideWindow() {
	for (int i = 0; i < this->seqSize; i++) {
		if (i == this->base)
			cout << "[";
		else if (i == (this->base + this->windowsize) % this->seqSize)
			cout << "] ";
		cout << i;
		if (isInWindow(i) && (i >= this->nextSeq))
			cout << "���� ";
		else if (isInWindow(i) && (queue[i].second))  //�������ұ�ȷ��
			cout << "��ȷ�� ";
		else if(isInWindow(i))
		cout << "�ѷ��� ";
		if (!isInWindow(i))
			cout << "������ ";
	}
	cout << "" << endl;
}