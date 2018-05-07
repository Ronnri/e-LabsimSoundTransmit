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

bool Client::InitSock()    //初始socket
{
	WSADATA wsData;
	WORD wr = MAKEWORD(2, 2);
	if (WSAStartup(wr, &wsData) == 0)
	{
		return true;
	}
	return false;
}
u_long Client::ResolveAdress(char *serverIp)   //解析IP地址
{
	u_long nAddr = inet_addr(serverIp);
	if (nAddr == INADDR_NONE)  //表明serverIp使用的是主机名形式
	{
		hostent *ent = gethostbyname(serverIp);
		if (ent == NULL)
		{
			//cout << "获取主机名出错" << WSAGetLastError() << endl;
			AfxMessageBox(TEXT("获取主机名出错"));
		}
		else
		{
			nAddr = *((u_long *)ent->h_addr_list[0]);
		}
	}
	if (nAddr == INADDR_NONE)
	{
		//cout << "解析主机地址失败" << endl;
		AfxMessageBox(TEXT("解析主机地址失败"));
	}
	return nAddr;
}
SOCKET Client::ConnectServer(u_long serverIp, int port)   //连接服务器
{
	sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd == INVALID_SOCKET)
	{
		AfxMessageBox(TEXT("创建套接字失败"));
		//cout << "创建套接字失败" << endl;
		return INVALID_SOCKET;
	}
	sockaddr_in saServer;
	saServer.sin_family = AF_INET;
	saServer.sin_addr.S_un.S_addr = serverIp;
	saServer.sin_port = htons(port);
	if (connect(sd, (sockaddr*)&saServer, sizeof(sockaddr_in)) == SOCKET_ERROR)
	{
		AfxMessageBox(TEXT("连接服务器失败"));
		//cout << "连接服务器失败" << WSAGetLastError() << endl;
		closesocket(sd);
		return INVALID_SOCKET;
	}
	return sd;
}
bool Client::ProcessConnection(SOCKET sd)      //进行通信
{
	//-------------------------------------------------
	//可以将下面代码看做设置系统缓冲区
	int nRecvBuf = 1024000;//设置为1000K
	setsockopt(sd, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int));
	//发送缓冲区
	int nSendBuf = 1024000;//设置为1000K
	setsockopt(sd, SOL_SOCKET, SO_SNDBUF, (const char*)&nSendBuf, sizeof(int));
	//---------------------------------------------------------
	//向服务器发送传送文件消息
	Message::MsgSendFile msgSendFile;
	if (send(sd, (char *)&msgSendFile, sizeof(Message::MsgSendFile), 0) == SOCKET_ERROR)
	{
		AfxMessageBox(TEXT("发送消息失败"));
		return false;
	}
	Sleep(10);     //睡眠10ms保证对方将发送的消息取走
	char filePath[MAX_FILE_NAME_LENGTH] = "C:\\test.wave";
	char fileDrive[_MAX_DRIVE];
	char fileDir[_MAX_DIR];
	char fileName[_MAX_FNAME];
	char fileExt[_MAX_EXT];
	_splitpath(filePath, fileDrive, fileDir, fileName, fileExt);  //将文件路径解析
	Message::MsgFileName msgFileName;
	strcat(fileName, fileExt);
	strcpy(msgFileName.fileName, fileName);
	if (send(sd, (char *)&msgFileName, sizeof(Message::MsgFileName), 0) == SOCKET_ERROR)  //发送文件名
	{
		//cout << "发送文件名出错" << WSAGetLastError() << endl;
		AfxMessageBox(TEXT("发送文件名出错"));
	}
	Sleep(10);
	if (!SendFileLength(sd, filePath))  //发送文件长度
	{

		AfxMessageBox(TEXT("发送文件长度出错"));
		return false;
	}
	Sleep(10);
	if (!SendFile(sd, filePath))  //发送文件
	{
		AfxMessageBox(TEXT("发送文件出错"));

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
bool Client::SendFile(SOCKET sd, char *filePath)   //发送文件
{
	cout << "进入到发送文件内容" << endl;
	Message::MsgFile msgFile;
	if (send(sd, (char *)&msgFile, sizeof(Message::MsgFile), 0) == SOCKET_ERROR)
	{
		cout << "发送文件消息出错" << WSAGetLastError() << endl;
		return false;
	}
	Sleep(10);
	FILE *pFile;
	pFile = fopen(filePath, "r+b");
	fseek(pFile, 0, SEEK_SET);   //定位到文件首位置
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
			cout << "发送失败" << endl;
			return false;
		}
		i += nSend;
		fseek(pFile, -(nSize - nSend), SEEK_CUR);  //定位到实际已发送到的位置
		memset(buff, 0, sizeof(char)*MAX_PACK_SIZE); //将buff清空
	}
	fclose(pFile);
	return true;
}
bool Client::RecvCatalogInfo(SOCKET sd)   //接收目录信息
{
	int flag = 1;     //接收目录信息成功标志
	char buff[MAX_PACK_SIZE];
	Message::MsgHead *msgHead;
	while (true)
	{
		if (recv(sd, buff, MAX_PACK_SIZE, 0) == SOCKET_ERROR)
		{
			cout << "接收目录信息失败" << WSAGetLastError() << endl;
			flag = 0;
			break;
		}
		msgHead = (Message::MsgHead *)buff;
		if (msgHead->msgId == MSG_COMPLETE)      //判断消息是否是标准消息
		{
			cout << "目录信息发送完成" << endl;
			break;
		}
		else
		{
			cout << buff << endl;     //发送来的是目录信息，即文件名
		}
	}
	if (flag == 0)
	{
		return false;
	}
	return true;
}
bool Client::SendDownLoadFileName(SOCKET sd)      //发送下载的文件名
{
	cout << "请输入你要下载的文件名" << endl;
	char fileName[_MAX_FNAME + _MAX_EXT];
	cin >> fileName;
	Message::MsgFileName msgFileName;
	strcpy(msgFileName.fileName, fileName);
	if (send(sd, (char *)&msgFileName, MAX_PACK_SIZE, 0) == SOCKET_ERROR)
	{
		cout << "发送下载文件名出错" << WSAGetLastError() << endl;
		return false;
	}
	return true;
}
bool Client::ReceiveFileLength(SOCKET sd)     //接收下载的文件长度
{
	char buff[MAX_PACK_SIZE];
	Message::MsgFileLength *msgFileLength;
	if (recv(sd, buff, MAX_PACK_SIZE, 0) == SOCKET_ERROR)
	{
		cout << "接收文件长度失败" << WSAGetLastError() << endl;
		return false;
	}
	msgFileLength = (Message::MsgFileLength *)buff;
	nFileLength = msgFileLength->fileLength;
	cout << "接收到文件长度" << nFileLength << endl;
	return true;
}
bool Client::ReceiveFileName(SOCKET sd)   //接收下载的文件名
{
	char buff[MAX_PACK_SIZE];
	memset(fileName, 0, sizeof(char)*(_MAX_FNAME + _MAX_EXT));
	Message::MsgFileName *msgFileName;
	if (recv(sd, buff, MAX_PACK_SIZE, 0) == SOCKET_ERROR)
	{
		cout << "接收文件名出错" << endl;
		return false;
	}
	msgFileName = (Message::MsgFileName *)buff;
	strcpy(fileName, msgFileName->fileName);
	cout << "接收到文件名" << fileName << endl;
	return true;
}
bool Client::ReceiveFile(SOCKET sd)      //接收文件内容
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
void Client::CloseSocket()   //关闭套接字
{
	closesocket(sd);
	WSACleanup();
} 