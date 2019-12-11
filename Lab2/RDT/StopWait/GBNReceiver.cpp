#include "stdafx.h"
#include "GBNReceiver.h"
#include "Global.h"

GBNReceiver::GBNReceiver() :
	seqSize(8){
	exSeqNum = 0;
	lastAck.acknum = -1;
	lastAck.checksum = 0;
	lastAck.seqnum = -1;
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
		lastAck.payload[i] = '.';
	}
	lastAck.checksum = pUtils->calculateCheckSum(lastAck);
}

GBNReceiver::~GBNReceiver(){
}


void GBNReceiver::receive(Packet &aPacket){

	//���У���
	int checkSum = pUtils->calculateCheckSum(aPacket);

	//���У�����ȷ���յ����ĵ���ź��ڴ��յ��ı��ĺ�һ��
	if (aPacket.checksum == checkSum && this->exSeqNum == aPacket.seqnum) {
		//ȡ��Message�����ϵݽ���Ӧ�ò�
		Message msg;
		memcpy(msg.data, aPacket.payload, sizeof(aPacket.payload));
		pns->delivertoAppLayer(RECEIVER, msg);
		pUtils->printPacket("\n���շ�������ȷ�����ݸ�Ӧ�ò���飺",aPacket);

		lastAck.acknum = (aPacket.seqnum); //ȷ����ŵ����յ��ı������
		lastAck.checksum = pUtils->calculateCheckSum(lastAck);
		pUtils->printPacket("\n���շ�����ȷ�ϱ���", lastAck);
		pns->sendToNetworkLayer(SENDER, lastAck);	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢��ȷ�ϱ��ĵ��Է�
		this->exSeqNum = (this->exSeqNum + 1) % seqSize;
		return;
	}
	else if (aPacket.checksum != checkSum){
		//�յ�������
		pUtils->printPacket("\n���շ�û����ȷ�յ����ͷ��ı���,����У�����", aPacket);
		pUtils->printPacket("\n���շ����·����ϴε�ȷ�ϱ���", lastAck);
		pns->sendToNetworkLayer(SENDER, lastAck);//����ģ�����绷����sendToNetworkLayer��ͨ������㷢���ϴε�ȷ�ϱ���
		return;
	}
	else if (aPacket.seqnum != exSeqNum){
		//�յ�ʧ����
		//cout << "�ڴ��ı�����ţ�" << this->exSeqNum << endl;
		pUtils->printPacket("\n���շ�û����ȷ�յ����ͷ��ı���,������Ų���", aPacket);
		pUtils->printPacket("\n���շ����·����ϴε�ȷ�ϱ���", lastAck);
		pns->sendToNetworkLayer(SENDER, lastAck);//����ģ�����绷����sendToNetworkLayer��ͨ������㷢���ϴε�ȷ�ϱ���
		return;
	}
}

