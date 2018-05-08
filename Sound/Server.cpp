#include "stdafx.h"
#include "Server.h"
#include "fileSource.h"
#include "resource.h"
#include "SoundShow.h"
Server::Server()
{
}
Server::~Server()
{
}
SOCKET Server::BindListen(int PORT)  //绑定套接字
{
	SOCKET sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd == INVALID_SOCKET)
	{
		cout << "创建套接字失败" << WSAGetLastError() << endl;
		return INVALID_SOCKET;
	}
	sockaddr_in sListen;
	sListen.sin_family = AF_INET;
	sListen.sin_addr.s_addr = htonl(INADDR_ANY);
	sListen.sin_port = htons(PORT);
	int nSize;
	nSize = sizeof(sockaddr_in);
	if (bind(sd, (sockaddr*)&sListen, nSize) == SOCKET_ERROR)
	{
		closesocket(sd);
		cout << "绑定失败" << WSAGetLastError() << endl;
		return INVALID_SOCKET;
	}
	if (listen(sd, 10) == SOCKET_ERROR)
	{
		closesocket(sd);
		cout << "监听失败" << WSAGetLastError() << endl;
		return INVALID_SOCKET;
	}
	return sd;

}
SOCKET Server::AcceptConnection(SOCKET sd)    //接收客户端
{
	sockaddr_in saRemote;
	int nSize = sizeof(sockaddr_in);
	SOCKET sdListen = accept(sd, (sockaddr*)&saRemote, &nSize);
	if (sdListen == INVALID_SOCKET)
	{
		//cout << "接收客户端失败" << WSAGetLastError() << endl;
		AfxMessageBox(TEXT("接收客户端失败"));
		return INVALID_SOCKET;
	}
	return sdListen;
}
bool Server::ReceiveFile(SOCKET sd)
{
	char buff[MAX_PACK_SIZE];
	FILE *pFile;
	const char *str = "C:\\";
	const char *str2 = fileName;
	const size_t len = strlen(str) + strlen(str2);
	char *n_str = new char[len + 1];
	strcpy(n_str, str);
	strcat(n_str, str2);
	//cout << n_str << endl;
	

	pFile = fopen(n_str, "ab+");
	delete[] n_str;
	_int64 i = 0;
	while (i + 1<fileLength)
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
	AfxMessageBox(TEXT("接收完成"));
	return true;
}
void Server::CloseSocket()
{
	closesocket(sd);
	WSACleanup();
}
bool Server::ProcessConnection(SOCKET sd)
{
	//----------------------------------------------
	//可以将下面代码看做设置系统缓冲区
	int nRecvBuf = 1024000;//设置为1000K
	setsockopt(sd, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int));
	//发送缓冲区
	int nSendBuf = 1024000;//设置为1000K
	setsockopt(sd, SOL_SOCKET, SO_SNDBUF, (const char*)&nSendBuf, sizeof(int));
	//-------------------------------------------------------------
	char buff[MAX_PACK_SIZE];
	Message::MsgHead *msgHead;
	if (recv(sd, buff, MAX_PACK_SIZE, 0) == SOCKET_ERROR)
	{

		cout << "接收失败" << WSAGetLastError() << endl;
		return false;
	}
	msgHead = (Message::MsgHead *)&buff;
	switch (msgHead->msgId)
	{
	case MSG_SEND_FILE:         //客户端向服务器发送文件
		cout << "客户端请求向服务器发送文件" << endl;
		//AfxMessageBox(TEXT("客户端请求向服务器发送文件"));
		break;
	case MSG_DOWNLOAD_FILE:      //客户端从服务器下载文件
	{
		cout << "客户端请求从服务器下载文件" << endl;
		fileSource source;      //用来获取指定文件加下的后缀为jpg文件
		string sPath = "E:\\图片";
		string sExt = ".jpg";
		source.GetFiles(sPath, sExt, source.catalogInfo);
		int nSize;
		nSize = source.catalogInfo.size();
		cout << "搜集到" << nSize << "个文件" << endl;
		char buff[MAX_PACK_SIZE];
		for (int i = 0; i<nSize; i++)   //将目录信息发送到客户端
		{
			strcpy(buff, source.catalogInfo[i].c_str());
			//cout<<source.catalogInfo[i]<<endl;
			if (send(sd, buff, MAX_PACK_SIZE, 0) == SOCKET_ERROR)
			{
				cout << "发送目录信息失败" << WSAGetLastError() << endl;
				return false;
			}
			Sleep(10);   //睡眠10ms让接收端将信息取走
		}
		Message::MsgComplete msgComplete;
		if (send(sd, (char *)&msgComplete, sizeof(Message::MsgComplete), 0) == SOCKET_ERROR)
		{
			cout << "发送完成信息失败" << WSAGetLastError() << endl;
			return false;
		}
		Sleep(10);
		if (!RecvFileName(sd))
		{
			return false;
		}
		Sleep(10);
		if (!GetAndSendFileLength(sd))
		{
			return false;
		}
		Sleep(10);
		if (!SendFileName(sd))
		{
			return false;
		}
		Sleep(10);
		if (!SendFile(sd))
		{
			return false;
		}
	}
	break;
	case MSG_FILE_NAME:      //发送的文件名
	{
		Message::MsgFileName *msgFileName;
		msgFileName = (Message::MsgFileName*)msgHead;
		strcpy(fileName, msgFileName->fileName);
		cout << "收到发送来的文件名" << fileName << endl;
	//	AfxMessageBox(TEXT("收到发送来的文件名"));
	}
	break;
	case MSG_FILE_LENGTH:    //发送的文件长度
	{
		Message::MsgFileLength *msgFileLength;
		msgFileLength = (Message::MsgFileLength *)msgHead;
		fileLength = msgFileLength->fileLength;
		cout << "接收到文件的长度为" << fileLength << endl;
	//	AfxMessageBox(TEXT("接收到文件的长度为"));
	}
	break;
	case MSG_FILE:     //发送的文件内容
	{
		cout << "开始接收文件" << endl;
	//	AfxMessageBox(TEXT("开始接收文件"));
		if (!ReceiveFile(sd))
		{
			//cout << "接收文件失败" << endl;
			AfxMessageBox(TEXT("接收文件失败"));
			return false;
		}
	}
	break;

	}
	return true;
}
bool Server::RecvFileName(SOCKET sd)
{
	//memset(fileName,0,sizeof(char)*MAX_FILE_NAME_LENGTH);  //清空
	char buff[MAX_PACK_SIZE];
	Message::MsgFileName *msgFileName;
	if (recv(sd, buff, MAX_PACK_SIZE, 0) == SOCKET_ERROR)
	{
		cout << "接收文件名失败" << WSAGetLastError() << endl;
		return false;
	}
	msgFileName = (Message::MsgFileName *)buff;
	strcpy(fileName, msgFileName->fileName);
	cout << "接收的文件名为" << fileName << endl;
	return true;
}
bool Server::GetAndSendFileLength(SOCKET sd)       //获取客户端要下载的文件长度
{
	Message::MsgFileLength msgFileLength;
	FILE *pFile;
	pFile = fopen(fileName, "r+b");
	if (pFile == NULL)
	{
		cout << "打开文件失败" << endl;
		return false;
	}
	fseek(pFile, 0, SEEK_END);
	fileLength = _ftelli64(pFile);
	fclose(pFile);
	msgFileLength.fileLength = fileLength;
	if (send(sd, (char*)&msgFileLength, sizeof(Message::MsgFileLength), 0) == SOCKET_ERROR)
	{
		cout << "发送文件长度失败" << WSAGetLastError() << endl;
		return false;
	}
	return true;
}
bool Server::SendFileName(SOCKET sd)       //向客户端发送文件名
{
	Message::MsgFileName msgFileName;
	char fileDrive[_MAX_DRIVE];
	char fileDir[_MAX_DIR];
	char Name[_MAX_FNAME];
	char fileExt[_MAX_EXT];
	_splitpath(fileName, fileDrive, fileDir, Name, fileExt);
	strcat(Name, fileExt);
	strcpy(msgFileName.fileName, Name);
	cout << "要发送的文件名为" << Name << endl;
	if (send(sd, (char *)&msgFileName, sizeof(Message::MsgFileName), 0) == SOCKET_ERROR)
	{
		cout << "发送文件名出错" << WSAGetLastError() << endl;
		return false;
	}
	return true;
}
bool Server::SendFile(SOCKET sd)  //向客户端发送文件
{
	cout << "进入到发送文件内容" << endl;
	cout << "要发送的文件为" << fileName << endl;
	FILE *pFile;
	pFile = fopen(fileName, "r+b");
	fseek(pFile, 0, SEEK_SET);   //定位到文件首位置
	_int64 i = 0;
	char buff[MAX_PACK_SIZE];
	cout << "要发送的文件长度为" << fileLength << endl;
	while (i<fileLength)
	{
		int nSize;
		if (i + MAX_PACK_SIZE>fileLength)
		{
			nSize = (int)(fileLength - i);
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
			cout << "发送失败" << WSAGetLastError() << endl;
			return false;
		}
		i += nSend;
		fseek(pFile, -(nSize - nSend), SEEK_CUR);  //定位到实际已发送到的位置
		memset(buff, 0, sizeof(char)*MAX_PACK_SIZE); //将buff清空
	}
	fclose(pFile);
	return true;
}
bool Server::InitSock()   //初始化winsocket
{
	WSADATA wsData;
	WORD wr = MAKEWORD(2, 2);
	if (WSAStartup(wr, &wsData) == 0)
	{
		return true;
	}
	return false;
}