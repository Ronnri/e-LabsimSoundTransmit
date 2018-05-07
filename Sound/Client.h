#pragma once
#include<iostream>
#include<fstream>
#include<vector>
#include<WinSock2.h>
#pragma comment(lib,"Ws2_32.lib")
using namespace std;


class Client
{
public:
	Client();
	virtual ~Client();

	_int64 nFileLength;
	char fileName[_MAX_FNAME + _MAX_EXT];
	SOCKET sd;
	bool InitSock();   //��ʼ��winsock
	u_long ResolveAdress(char *serverIp);    //������������ַ
	SOCKET ConnectServer(u_long serverIp, int port);//���ӷ�����
	bool ProcessConnection(SOCKET sd);    //�ͻ��˷���������
	void CloseSocket();         //�ͷ��׽���
	bool SendFileLength(SOCKET sd, char *filePath);  //�����ļ�����
	bool SendFile(SOCKET sd, char *filePath);    //�����ļ�
	bool RecvCatalogInfo(SOCKET sd);     //����Ŀ¼��Ϣ
	bool SendDownLoadFileName(SOCKET sd);  //����Ҫ���ص��ļ���
	bool ReceiveFileLength(SOCKET sd);    //�����ļ�����
	bool ReceiveFileName(SOCKET sd);   //�����ļ���
	bool ReceiveFile(SOCKET sd);      //�����ļ�
};

