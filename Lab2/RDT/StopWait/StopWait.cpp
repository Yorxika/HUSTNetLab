// StopWait.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Global.h"
#include "RdtSender.h"
#include "RdtReceiver.h"
#include "StopWaitRdtSender.h"
#include "StopWaitRdtReceiver.h"
#include "GBNReceiver.h"
#include "GBNSender.h"
#include "SRReceiver.h"
#include "SRSender.h"
#include "TCPSender.h"
#include "TCPReceiver.h"


int main(int argc, char* argv[])
{
	RdtSender *ps = new StopWaitRdtSender();
	RdtReceiver * pr = new StopWaitRdtReceiver();
	RdtSender *pGBNSender = new GBNSender();
	RdtReceiver *pGBNReceiver = new GBNReceiver();
	RdtSender *pSRSender = new SRSender();
	RdtReceiver *pSRReceiver = new SRReceiver();
	RdtSender *pTCPSender = new TCPSender();
	RdtReceiver *pTCPReceiver = new TCPReceiver();

    //pns->setRunMode(0);  //VERBOS模式
	//int i = 0;
	string outputFile = "..\\output";
	string inputFile = "..\\input";
	string txt = ".txt";
	string temp;
	RdtSender* Sender[3] = { pGBNSender ,pSRSender ,pTCPSender };
	RdtReceiver* Receiver[3] = { pGBNReceiver ,pSRReceiver,pTCPReceiver};

	/*for (int j = 0; j < 3; j++) {
		int i = 0;
		while (i < 11) {
			system("cls");
			pns->setRunMode(1);  //安静模式
			pns->init();
			//pns->setRtdSender(pTCPSender);
			//pns->setRtdReceiver(pTCPReceiver);
			pns->setRtdSender(Sender[j]);
			pns->setRtdReceiver(Receiver[j]);
			pns->setInputFile("..\\input.txt");
			temp = outputFile + (char)(i + '0') + txt;
			pns->setOutputFile(temp.c_str());
			pns->start();
			i++;
			//system("pause");
		}
		//system("pause");
	}*/

	pns->setRunMode(1);  //安静模式
	pns->init();
	//pns->setRtdSender(pTCPSender);
	//pns->setRtdReceiver(pTCPReceiver);
	pns->setRtdSender(Sender[2]);
	pns->setRtdReceiver(Receiver[2]);
	pns->setInputFile("..\\input.txt");
	//temp = outputFile + (char)(i + '0') + txt;
	//pns->setOutputFile(temp.c_str());
	pns->setOutputFile("..\\output.txt");
	pns->start();


	delete ps;
	delete pr;
	delete pGBNSender;
	delete pGBNReceiver;
	delete pSRSender;
	delete pSRReceiver;
	delete pTCPSender;
	delete pTCPReceiver;
	delete pUtils;									//指向唯一的工具类实例，只在main函数结束前delete
	delete pns;										//指向唯一的模拟网络环境类实例，只在main函数结束前delete
	
	return 0;
}

