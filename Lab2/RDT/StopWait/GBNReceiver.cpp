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

	//检查校验和
	int checkSum = pUtils->calculateCheckSum(aPacket);

	//如果校验和正确且收到报文的序号和期待收到的报文号一致
	if (aPacket.checksum == checkSum && this->exSeqNum == aPacket.seqnum) {
		//取出Message，向上递交给应用层
		Message msg;
		memcpy(msg.data, aPacket.payload, sizeof(aPacket.payload));
		pns->delivertoAppLayer(RECEIVER, msg);
		pUtils->printPacket("\n接收方接收正确并传递给应用层分组：",aPacket);

		lastAck.acknum = (aPacket.seqnum); //确认序号等于收到的报文序号
		lastAck.checksum = pUtils->calculateCheckSum(lastAck);
		pUtils->printPacket("\n接收方发送确认报文", lastAck);
		pns->sendToNetworkLayer(SENDER, lastAck);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方
		this->exSeqNum = (this->exSeqNum + 1) % seqSize;
		return;
	}
	else if (aPacket.checksum != checkSum){
		//收到受损报文
		pUtils->printPacket("\n接收方没有正确收到发送方的报文,数据校验错误", aPacket);
		pUtils->printPacket("\n接收方重新发送上次的确认报文", lastAck);
		pns->sendToNetworkLayer(SENDER, lastAck);//调用模拟网络环境的sendToNetworkLayer，通过网络层发送上次的确认报文
		return;
	}
	else if (aPacket.seqnum != exSeqNum){
		//收到失序报文
		//cout << "期待的报文序号：" << this->exSeqNum << endl;
		pUtils->printPacket("\n接收方没有正确收到发送方的报文,报文序号不对", aPacket);
		pUtils->printPacket("\n接收方重新发送上次的确认报文", lastAck);
		pns->sendToNetworkLayer(SENDER, lastAck);//调用模拟网络环境的sendToNetworkLayer，通过网络层发送上次的确认报文
		return;
	}
}

