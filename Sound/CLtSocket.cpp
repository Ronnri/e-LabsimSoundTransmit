#include "stdafx.h"
#include "CLtSocket.h"



// CLtSocket

CLtSocket::CLtSocket()
{
	m_pSocket = NULL;	//��SOCKET��Ϊ��
}

CLtSocket::~CLtSocket()
{	//�ر��׽���
	if (m_pSocket != NULL)
		m_pSocket->Close();
	if (m_hSocket != INVALID_SOCKET)
	{
		Close();	//�رձ���socket����,ֹͣ�����˿�
	}
}

// CLtSocket ��Ա����
//����ֵ��0-����ʧ�ܣ�1-���ͳɹ�
int CLtSocket::SendData(char *szData, int nLen)
{
	int nRev = 0;
	if (m_pSocket != NULL)
	{
		m_pSocket->Send(szData, nLen, 0);
		//m_pSocket->AsyncSelect( FD_WRITE );	//������ϣ��������
		nRev = 1;
	}
	return nRev;
}

//�Ͽ�ͨ��SOCKET
void CLtSocket::CloseSocket()
{
	if (m_pSocket != NULL)
	{
		m_pSocket->Close();
		m_pSocket = NULL;	//��SOCKET��Ϊ��
	}
}

void CLtSocket::OnConnect(int nErrorCode)
{

}

void CLtSocket::OnReceive(int nErrorCode)
{
	m_nMsgLength = Receive(m_szBuffer, sizeof(m_szBuffer), 0);
	//���ｫ���ݴ�����
	m_pViewWnd->SendMessage(MSG_HAS_NEW_DATA, (WPARAM)this, 0);		//������ͼ���µ����ݵ���
																	//��ս��ջ���
	memset(m_szBuffer, 0, sizeof(m_szBuffer));
	CAsyncSocket::OnReceive(nErrorCode);
}

void CLtSocket::OnAccept(int nErrorCode)
{
	//�������������󣬵���Accept����
	//����һ���µ�SOCKET�����������������ӵ��ն���Ϣ�������Խ�GPS���¶ȵ���ϢҲ���浽�������������ʱ����Ӧ����ϢҲ�ᱻ����
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