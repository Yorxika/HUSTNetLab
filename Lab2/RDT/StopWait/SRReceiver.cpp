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
	if (checksum != packet.checksum){  //���ݰ���
		pUtils->printPacket("\n���շ����յ��ı�����", packet);
		return;
	}
	else
	{
		if (!isInWindow(packet.seqnum)){//���Ǵ����ڵı���
			pUtils->printPacket("\n�����ڴ������ݷ���", packet);
			laskAck.acknum = packet.seqnum;
			laskAck.seqnum = -1;
			memset(laskAck.payload, '.', Configuration::PAYLOAD_SIZE);
			laskAck.checksum = pUtils->calculateCheckSum(laskAck);
			pns->sendToNetworkLayer(SENDER, laskAck);
			return;
		}
		else{  //�Ǵ����ڵı���
			pUtils->printPacket("\n���շ���ȷ�յ����ͷ��ı���", packet);
			queue[packet.seqnum] = { packet , true};
			laskAck.acknum = packet.seqnum;  //ȷ�����
			laskAck.seqnum = 0;
			memset(laskAck.payload, '.', sizeof(laskAck.payload));
			laskAck.checksum = pUtils->calculateCheckSum(laskAck);
			pUtils->printPacket("\n���շ�����ack", laskAck);
			pns->sendToNetworkLayer(SENDER, laskAck);
			while(queue[base].second) {
				//�ƶ�����
				Message msg;
				memcpy(msg.data, queue[base].first.payload, sizeof(queue[base].first.payload));
				pns->delivertoAppLayer(RECEIVER, msg);
				pUtils->printPacket("\n���շ��ݽ���Ӧ�ò㱨��: ", queue[base].first);
				queue[base].second = false;
				base = (base + 1) % seqSize;
			}
			cout << "\n���շ����յ����ģ��ƶ�����: ";
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
			cout << "�ѻ��� ";
		else if (isInWindow(i) && !(queue[i].second))  //�������ұ�ȷ��
			cout << "�ȴ� ";
		if (!isInWindow(i))
			cout << "������ ";
	}
	cout << "" << endl;
}