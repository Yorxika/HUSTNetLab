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
	//���У����Ƿ���ȷ
	int checkSum = pUtils->calculateCheckSum(packet);

	//���У�����ȷ��ͬʱ�յ����ĵ���ŵ��ڽ��շ��ڴ��յ��ı������һ��
	if (checkSum == packet.checksum && this->nextSeq == packet.seqnum) {
		pUtils->printPacket("\n���շ���ȷ�յ����ͷ��ı���", packet);

		//ȡ��Message�����ϵݽ���Ӧ�ò�
		Message msg;
		memcpy(msg.data, packet.payload, sizeof(packet.payload));
		pns->delivertoAppLayer(RECEIVER, msg);

		//lastAck.acknum = (packet.seqnum + 1) % this->seqSize; //��һ��ϣ���յ������
		lastAck.acknum = (packet.seqnum);
		lastAck.checksum = pUtils->calculateCheckSum(lastAck);
		pUtils->printPacket("\n���շ�����ȷ�ϱ���", lastAck);
		pns->sendToNetworkLayer(SENDER, lastAck);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�
		this->nextSeq = (packet.seqnum + 1) % this->seqSize; //�ڴ��յ���һ�����
	}
	else {
		if (checkSum != packet.checksum) {
			pUtils->printPacket("\n���շ�û����ȷ�յ����ͷ��ı���,����У�����", packet);
			pUtils->printPacket("\n���շ����·����ϴε�ȷ�ϱ���", lastAck);
			pns->sendToNetworkLayer(SENDER, lastAck);//����ģ�����绷����sendToNetworkLayer��ͨ������㷢���ϴε�ȷ�ϱ���
			return;
		}
		else {
			pUtils->printPacket("\n���շ�û����ȷ�յ����ͷ��ı���,������Ų���", packet);
			pUtils->printPacket("\n���շ����·����ϴε�ȷ�ϱ���", lastAck);
			pns->sendToNetworkLayer(SENDER, lastAck);//����ģ�����绷����sendToNetworkLayer��ͨ������㷢���ϴε�ȷ�ϱ���
			return;
		}
	}
}