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
	CWnd * m_pViewWnd;			//视图指针
	BOOL	m_bConnected;		//是否连接
	UINT	m_nMsgLength;		//消息长度
	char	m_szBuffer[4096];	//消息缓冲区
	CString	m_strIP;			//远程连接IP地址
	UINT	m_Port;				//远程连接端口
	CString	m_strBuffer;		//接收字符转换为CString;
};

