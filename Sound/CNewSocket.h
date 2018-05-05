#pragma once
#include <afxsock.h>
#define	MSG_HAS_NEW_MSG WM_USER + 101
#define	MSG_HAS_NEW_DATA WM_USER + 102
#define MSG_STOP_PLAYING WM_USER+103
class CNewSocket:public CAsyncSocket
{
public:
	CNewSocket();
	virtual ~CNewSocket();
	virtual void OnReceive(int nErrorCode);
	virtual void OnSend(int nErrorCode);
	virtual void OnClose(int nErrorCode);
public:
	char*	GetRevBuffer() { return m_szBuffer; }
	void	SetViewWnd(CWnd *pWnd) { m_pViewWnd = pWnd; }
public:
	//private:
	CWnd * m_pViewWnd;			//��ͼָ��
	BOOL	m_bConnected;		//�Ƿ�����
	UINT	m_nMsgLength;		//��Ϣ����
	char	m_szBuffer[4096];	//��Ϣ������
	CString	m_strIP;			//Զ������IP��ַ
	UINT	m_Port;				//Զ�����Ӷ˿�
	CString	m_strBuffer;		//�����ַ�ת��ΪCString;
};

