#pragma once
#include <afxsock.h>
#include "CNewSocket.h"
// CLtSocket ����Ŀ��

class CLtSocket : public CAsyncSocket
{
public:
	CLtSocket();
	virtual ~CLtSocket();
	virtual void OnConnect(int nErrorCode);		//��������
	virtual void OnAccept(int nErrorCode);		//����������Ӧ
	virtual void OnReceive(int nErrorCode);		//��������
public:
	void	SetViewWnd(CWnd *pWnd) { m_pViewWnd = pWnd; }
	int		SendData(char *szData, int nLen);		//�ֶ���������
	void	CloseSocket();		//�ر�ͨ��SOCKET
	char*	GetRevBuffer() { return m_szBuffer; }
	//void	SetDataChannel(int nChannel) { m_nChannel = nChannel; }
private:
	CWnd * m_pViewWnd;
	BOOL	m_bConnected;		//�Ƿ�����
	UINT	m_nMsgLength;		//��Ϣ����
	char	m_szBuffer[4096];	//��Ϣ������
	int		m_nChannel;
	//	CBuffer	m_RevBuffer;

public:
	CNewSocket * m_pSocket;		//ͨ��SOCKET
};

