#include "stdafx.h"
#include "CLtSocket.h"



// CLtSocket

CLtSocket::CLtSocket()
{
	m_pSocket = NULL;	//将SOCKET置为空
}

CLtSocket::~CLtSocket()
{	//关闭套接字
	if (m_pSocket != NULL)
		m_pSocket->Close();
	if (m_hSocket != INVALID_SOCKET)
	{
		Close();	//关闭本地socket服务,停止监听端口
	}
}

// CLtSocket 成员函数
//返回值：0-发送失败，1-发送成功
int CLtSocket::SendData(char *szData, int nLen)
{
	int nRev = 0;
	if (m_pSocket != NULL)
	{
		m_pSocket->Send(szData, nLen, 0);
		//m_pSocket->AsyncSelect( FD_WRITE );	//发送完毕，进入接收
		nRev = 1;
	}
	return nRev;
}

//断开通信SOCKET
void CLtSocket::CloseSocket()
{
	if (m_pSocket != NULL)
	{
		m_pSocket->Close();
		m_pSocket = NULL;	//将SOCKET置为空
	}
}

void CLtSocket::OnConnect(int nErrorCode)
{

}

void CLtSocket::OnReceive(int nErrorCode)
{
	m_nMsgLength = Receive(m_szBuffer, sizeof(m_szBuffer), 0);
	//这里将数据存起来
	m_pViewWnd->SendMessage(MSG_HAS_NEW_DATA, (WPARAM)this, 0);		//告诉视图有新的数据到来
																	//清空接收缓存
	memset(m_szBuffer, 0, sizeof(m_szBuffer));
	CAsyncSocket::OnReceive(nErrorCode);
}

void CLtSocket::OnAccept(int nErrorCode)
{
	//侦听到连接请求，调用Accept函数
	//创建一个新的SOCKET对象，用来保存新连接的终端信息，还可以将GPS、温度等信息也保存到这里，当对象销毁时，相应的信息也会被销毁
	CNewSocket *pSocket = new CNewSocket();
	if (Accept(*pSocket))
	{
		CString strIP;
		UINT SocketPort;
		pSocket->GetPeerName(strIP, SocketPort);
		pSocket->AsyncSelect(FD_READ | FD_CLOSE);
		pSocket->SetViewWnd(m_pViewWnd);
		if (m_pSocket != NULL)
		{
			m_pSocket->Close();
			m_pSocket = NULL;
		}
		m_pSocket = pSocket;
	
	}
	else
		delete pSocket;
}