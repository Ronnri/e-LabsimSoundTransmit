#pragma once
#include <afxsock.h>
#include "CNewSocket.h"
// CLtSocket 命令目标

class CLtSocket : public CAsyncSocket
{
public:
	CLtSocket();
	virtual ~CLtSocket();
	virtual void OnConnect(int nErrorCode);		//连接网络
	virtual void OnAccept(int nErrorCode);		//接收网络响应
	virtual void OnReceive(int nErrorCode);		//接收数据
public:
	void	SetViewWnd(CWnd *pWnd) { m_pViewWnd = pWnd; }
	int		SendData(char *szData, int nLen);		//手动发送数据
	void	CloseSocket();		//关闭通信SOCKET
	char*	GetRevBuffer() { return m_szBuffer; }
	//void	SetDataChannel(int nChannel) { m_nChannel = nChannel; }
private:
	CWnd * m_pViewWnd;
	BOOL	m_bConnected;		//是否连接
	UINT	m_nMsgLength;		//消息长度
	char	m_szBuffer[4096];	//消息缓冲区
	int		m_nChannel;
	//	CBuffer	m_RevBuffer;

public:
	CNewSocket * m_pSocket;		//通信SOCKET
};

