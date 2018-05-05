#include "stdafx.h"
#include "CNewSocket.h"


CNewSocket::CNewSocket()
{
	m_nMsgLength = 0;
	memset(m_szBuffer, 0, sizeof(m_szBuffer));
}


CNewSocket::~CNewSocket()
{
}
// CNewSocket ��Ա����

void CNewSocket::OnReceive(int nErrorCode)
{
	m_nMsgLength = Receive(m_szBuffer, sizeof(m_szBuffer), 0);

		m_pViewWnd->SendMessage(MSG_HAS_NEW_DATA, (WPARAM)this, 0);

	//��ս��ջ���
	memset(m_szBuffer, 0, sizeof(m_szBuffer));
	CAsyncSocket::OnReceive(nErrorCode);
}
void CNewSocket::OnSend(int nErrorCode)
{
	//��������һ���������������¼�������Server��Ϣ
	AsyncSelect(FD_READ);
	CAsyncSocket::OnSend(nErrorCode);
}

void CNewSocket::OnClose(int nErrorCode)
{
	m_hSocket = INVALID_SOCKET;
	m_pViewWnd->SendMessage(MSG_STOP_PLAYING, 0, 0);
}
