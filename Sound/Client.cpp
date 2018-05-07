#include "stdafx.h"
#include "Client.h"



#define _CRT_SECURE_NO_WARNINGS
#pragma once
#include<iostream>
#include<vector>
#include<WinSock2.h>
#include"client.h"
#include"message.h"
using namespace std;
Client::Client()
{
}
Client::~Client()
{
}

bool Client::InitSock()    //��ʼsocket
{
	WSADATA wsData;
	WORD wr = MAKEWORD(2, 2);
	if (WSAStartup(wr, &wsData) == 0)
	{
		return true;
	}
	return false;
}
u_long Client::ResolveAdress(char *serverIp)   //����IP��ַ
{
	u_long nAddr = inet_addr(serverIp);
	if (nAddr == INADDR_NONE)  //����serverIpʹ�õ�����������ʽ
	{
		hostent *ent = gethostbyname(serverIp);
		if (ent == NULL)
		{
			//cout << "��ȡ����������" << WSAGetLastError() << endl;
			AfxMessageBox(TEXT("��ȡ����������"));
		}
		else
		{
			nAddr = *((u_long *)ent->h_addr_list[0]);
		}
	}
	if (nAddr == INADDR_NONE)
	{
		//cout << "����������ַʧ��" << endl;
		AfxMessageBox(TEXT("����������ַʧ��"));
	}
	return nAddr;
}
SOCKET Client::ConnectServer(u_long serverIp, int port)   //���ӷ�����
{
	sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd == INVALID_SOCKET)
	{
		AfxMessageBox(TEXT("�����׽���ʧ��"));
		//cout << "�����׽���ʧ��" << endl;
		return INVALID_SOCKET;
	}
	sockaddr_in saServer;
	saServer.sin_family = AF_INET;
	saServer.sin_addr.S_un.S_addr = serverIp;
	saServer.sin_port = htons(port);
	if (connect(sd, (sockaddr*)&saServer, sizeof(sockaddr_in)) == SOCKET_ERROR)
	{
		AfxMessageBox(TEXT("���ӷ�����ʧ��"));
		//cout << "���ӷ�����ʧ��" << WSAGetLastError() << endl;
		closesocket(sd);
		return INVALID_SOCKET;
	}
	return sd;
}
bool Client::ProcessConnection(SOCKET sd)      //����ͨ��
{
	//-------------------------------------------------
	//���Խ�������뿴������ϵͳ������
	int nRecvBuf = 1024000;//����Ϊ1000K
	setsockopt(sd, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int));
	//���ͻ�����
	int nSendBuf = 1024000;//����Ϊ1000K
	setsockopt(sd, SOL_SOCKET, SO_SNDBUF, (const char*)&nSendBuf, sizeof(int));
	//---------------------------------------------------------
	//����������ʹ����ļ���Ϣ
	Message::MsgSendFile msgSendFile;
	if (send(sd, (char *)&msgSendFile, sizeof(Message::MsgSendFile), 0) == SOCKET_ERROR)
	{
		AfxMessageBox(TEXT("������Ϣʧ��"));
		return false;
	}
	Sleep(10);     //˯��10ms��֤�Է������͵���Ϣȡ��
	char filePath[MAX_FILE_NAME_LENGTH] = "C:\\test.wave";
	char fileDrive[_MAX_DRIVE];
	char fileDir[_MAX_DIR];
	char fileName[_MAX_FNAME];
	char fileExt[_MAX_EXT];
	_splitpath(filePath, fileDrive, fileDir, fileName, fileExt);  //���ļ�·������
	Message::MsgFileName msgFileName;
	strcat(fileName, fileExt);
	strcpy(msgFileName.fileName, fileName);
	if (send(sd, (char *)&msgFileName, sizeof(Message::MsgFileName), 0) == SOCKET_ERROR)  //�����ļ���
	{
		//cout << "�����ļ�������" << WSAGetLastError() << endl;
		AfxMessageBox(TEXT("�����ļ�������"));
	}
	Sleep(10);
	if (!SendFileLength(sd, filePath))  //�����ļ�����
	{

		AfxMessageBox(TEXT("�����ļ����ȳ���"));
		return false;
	}
	Sleep(10);
	if (!SendFile(sd, filePath))  //�����ļ�
	{
		AfxMessageBox(TEXT("�����ļ�����"));

		return false;
	}
	return true;
}
bool Client::SendFileLength(SOCKET sd, char *filePath)
{

	FILE *pFile;
	pFile = fopen(filePath, "r+b");
	fseek(pFile, 0, SEEK_END);
	nFileLength = _ftelli64(pFile);
	Message::MsgFileLength msgFileLength;
	msgFileLength.fileLength = nFileLength;
	fclose(pFile);
	if (send(sd, (char *)&msgFileLength, sizeof(Message::MsgFileLength), 0) == SOCKET_ERROR)
	{
		return false;
	}
	return true;
}
bool Client::SendFile(SOCKET sd, char *filePath)   //�����ļ�
{
	cout << "���뵽�����ļ�����" << endl;
	Message::MsgFile msgFile;
	if (send(sd, (char *)&msgFile, sizeof(Message::MsgFile), 0) == SOCKET_ERROR)
	{
		cout << "�����ļ���Ϣ����" << WSAGetLastError() << endl;
		return false;
	}
	Sleep(10);
	FILE *pFile;
	pFile = fopen(filePath, "r+b");
	fseek(pFile, 0, SEEK_SET);   //��λ���ļ���λ��
	_int64 i = 0;
	char buff[MAX_PACK_SIZE];
	while (i<nFileLength)
	{
		int nSize;
		if (i + MAX_PACK_SIZE>nFileLength)
		{
			nSize = (int)(nFileLength - i);
		}
		else
		{
			nSize = MAX_PACK_SIZE - 1;
		}
		fread(buff, sizeof(char), nSize, pFile);
		int nSend;
		nSend = send(sd, buff, nSize, 0);
		if (nSend == SOCKET_ERROR)
		{
			cout << "����ʧ��" << endl;
			return false;
		}
		i += nSend;
		fseek(pFile, -(nSize - nSend), SEEK_CUR);  //��λ��ʵ���ѷ��͵���λ��
		memset(buff, 0, sizeof(char)*MAX_PACK_SIZE); //��buff���
	}
	fclose(pFile);
	return true;
}
bool Client::RecvCatalogInfo(SOCKET sd)   //����Ŀ¼��Ϣ
{
	int flag = 1;     //����Ŀ¼��Ϣ�ɹ���־
	char buff[MAX_PACK_SIZE];
	Message::MsgHead *msgHead;
	while (true)
	{
		if (recv(sd, buff, MAX_PACK_SIZE, 0) == SOCKET_ERROR)
		{
			cout << "����Ŀ¼��Ϣʧ��" << WSAGetLastError() << endl;
			flag = 0;
			break;
		}
		msgHead = (Message::MsgHead *)buff;
		if (msgHead->msgId == MSG_COMPLETE)      //�ж���Ϣ�Ƿ��Ǳ�׼��Ϣ
		{
			cout << "Ŀ¼��Ϣ�������" << endl;
			break;
		}
		else
		{
			cout << buff << endl;     //����������Ŀ¼��Ϣ�����ļ���
		}
	}
	if (flag == 0)
	{
		return false;
	}
	return true;
}
bool Client::SendDownLoadFileName(SOCKET sd)      //�������ص��ļ���
{
	cout << "��������Ҫ���ص��ļ���" << endl;
	char fileName[_MAX_FNAME + _MAX_EXT];
	cin >> fileName;
	Message::MsgFileName msgFileName;
	strcpy(msgFileName.fileName, fileName);
	if (send(sd, (char *)&msgFileName, MAX_PACK_SIZE, 0) == SOCKET_ERROR)
	{
		cout << "���������ļ�������" << WSAGetLastError() << endl;
		return false;
	}
	return true;
}
bool Client::ReceiveFileLength(SOCKET sd)     //�������ص��ļ�����
{
	char buff[MAX_PACK_SIZE];
	Message::MsgFileLength *msgFileLength;
	if (recv(sd, buff, MAX_PACK_SIZE, 0) == SOCKET_ERROR)
	{
		cout << "�����ļ�����ʧ��" << WSAGetLastError() << endl;
		return false;
	}
	msgFileLength = (Message::MsgFileLength *)buff;
	nFileLength = msgFileLength->fileLength;
	cout << "���յ��ļ�����" << nFileLength << endl;
	return true;
}
bool Client::ReceiveFileName(SOCKET sd)   //�������ص��ļ���
{
	char buff[MAX_PACK_SIZE];
	memset(fileName, 0, sizeof(char)*(_MAX_FNAME + _MAX_EXT));
	Message::MsgFileName *msgFileName;
	if (recv(sd, buff, MAX_PACK_SIZE, 0) == SOCKET_ERROR)
	{
		cout << "�����ļ�������" << endl;
		return false;
	}
	msgFileName = (Message::MsgFileName *)buff;
	strcpy(fileName, msgFileName->fileName);
	cout << "���յ��ļ���" << fileName << endl;
	return true;
}
bool Client::ReceiveFile(SOCKET sd)      //�����ļ�����
{
	char buff[MAX_PACK_SIZE];
	FILE *pFile;
	pFile = fopen(fileName, "a+b");
	_int64 i = 0;
	while (i + 1<nFileLength)
	{
		int nRecv = recv(sd, buff, MAX_PACK_SIZE, 0);
		if (nRecv == SOCKET_ERROR)
		{
			return false;
		}
		fwrite(buff, sizeof(char), nRecv, pFile);
		i += nRecv;
		memset(buff, 0, sizeof(char)*MAX_PACK_SIZE);
	}
	fclose(pFile);
	return true;
}
void Client::CloseSocket()   //�ر��׽���
{
	closesocket(sd);
	WSACleanup();
} 