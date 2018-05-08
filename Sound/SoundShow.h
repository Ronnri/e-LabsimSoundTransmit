#pragma once
#include "Server.h"
#include "Client.h"
#include "afxcmn.h"
#include "afxwin.h"

#include   <afxpriv.h>


#include "mmeapi.h"
#define INP_BUFFER_SIZE 16384
#define BUFFER_SIZE 4096
// SoundShow 对话框
#define IDP_SOCKETS_INIT_FAILED            103

class SoundShow : public CDialog
{
	DECLARE_DYNAMIC(SoundShow)


public:
	SoundShow(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~SoundShow();

	//自定义函数
	void RunAlgrithm(const double * pdInput, double * pdOutput);


// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SOUNDSHOW };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

														//自定义变量
	BOOL         bRecording, bPlaying, bReverse, bPaused, bEnding, bTerminating;
	DWORD        dwDataLength, dwRepetitions;
	HWAVEIN      hWaveIn;
	HWAVEOUT     hWaveOut;
	PBYTE        pBuffer1, pBuffer2, pSaveBuffer, pNewBuffer;//采集声音的缓冲区

	PWAVEHDR     pWaveHdr1, pWaveHdr2;
	WAVEFORMATEX waveform;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedSave();
	afx_msg void OnBnClickedOpen();
	afx_msg void OnBnClickedRecordBeg();
	afx_msg void OnBnClickedRecordEnd();
	afx_msg void OnBnClickedPlayBeg();
	afx_msg void OnBnClickedPlayEnd();
	afx_msg void OnBnClickedQuit();
	afx_msg void OnBnClickedSs();
	afx_msg void OnBnClickedSc();
	afx_msg void OnBnClickedSreset();
	afx_msg void OnBnClickedSok();

	virtual BOOL OnInitDialog();
	
	afx_msg LRESULT OnMM_WIM_OPEN(UINT wParam, LONG lParam);
	afx_msg LRESULT OnMM_WIM_DATA(UINT wParam, LONG lParam);
	afx_msg LRESULT OnMM_WIM_CLOSE(UINT wParam, LONG lParam);
	afx_msg LRESULT OnMM_WOM_OPEN(UINT wParam, LONG lParam);
	afx_msg LRESULT OnMM_WOM_DONE(UINT wParam, LONG lParam);
	afx_msg LRESULT OnMM_WOM_CLOSE(UINT wParam, LONG lParam);
	afx_msg void OnSysCommand(UINT wParam, LPARAM lParam);
	afx_msg void OnBnClickedSend();

	enum IN_PortName { IN1 = 0, IN2, IN3, IN4, IN5, IN6, IN7, IN8, IN9, IN10 };
	enum OUT_PortName { OUT1 = 0, OUT2, OUT3, OUT4, OUT5, OUT6, OUT7, OUT8, OUT9, OUT10 };

private:
	int m_nSendBufferSize;
	int m_nFrameEndCount;
	double m_nClkState;
	int m_nBitCount;
	int m_nOutCount;
	//char m_nSendBuffer[4096];

	char *m_nSendBuffer;
	int m_nFrameHeaderCount;

	bool m_bFrameHeader;
	double m_nReClkState;
	int m_nFrameRevCount;
	int m_nRevBitCount;
	int m_nRevBitBuffer[8];
	int m_RevByteBuffer[4096];
	int m_nRevByteCount;
	bool m_bRevOK;
	char m_RevBuffer[4096];

public:

	CIPAddressCtrl m_IPAddress;
	CEdit			m_PortShow;

	Server Server;
	Client Client;

	BOOL m_IsServer;

	int m_TargetPort;
	CString m_TargetIp;
};


