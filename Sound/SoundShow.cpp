// SoundShow.cpp : 实现文件
//

#include "stdafx.h"
#include "Sound.h"
#include "SoundShow.h"
#include "afxdialogex.h"
#include "g711.h"
#include "utility.h"

// SoundShow 对话框

IMPLEMENT_DYNAMIC(SoundShow, CDialog)

SoundShow::SoundShow(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_SOUNDSHOW, pParent)
{
	m_nClkState = 0;
	m_nSendBufferSize = 0;
	m_nFrameHeaderCount = 0;
	m_nBitCount = 0;
	m_nOutCount = 0;
	m_nFrameEndCount = 0;
	m_bRevOK = FALSE;
	m_bFrameHeader = FALSE;
	m_nReClkState = 0;
	m_nFrameRevCount = 0;
	m_nRevBitCount = 0;
	memset(m_nRevBitBuffer, 0, sizeof(m_nRevBitBuffer));
	memset(m_RevByteBuffer, 0, sizeof(m_RevByteBuffer));
	m_nRevByteCount = 0;
	m_bRevOK = false;
	memset(m_RevBuffer, 0, sizeof(m_RevBuffer));


}

SoundShow::~SoundShow()
{
}

void SoundShow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IPADDRESS1, m_IPAddress);
	DDX_Control(pDX, IDC_EDIT1, m_PortShow);

}

BEGIN_MESSAGE_MAP(SoundShow, CDialog)
	ON_WM_SYSCOMMAND()
	ON_MESSAGE(MM_WIM_OPEN, OnMM_WIM_OPEN)
	ON_MESSAGE(MM_WIM_DATA, OnMM_WIM_DATA)
	ON_MESSAGE(MM_WIM_CLOSE, OnMM_WIM_CLOSE)
	ON_MESSAGE(MM_WOM_OPEN, OnMM_WOM_OPEN)
	ON_MESSAGE(MM_WOM_DONE, OnMM_WOM_DONE)
	ON_MESSAGE(MM_WOM_CLOSE, OnMM_WOM_CLOSE)
	ON_MESSAGE(MSG_HAS_NEW_DATA, &SoundShow::OnNewData)
	ON_BN_CLICKED(IDC_SAVE, &SoundShow::OnBnClickedSave)
	ON_BN_CLICKED(IDC_OPEN, &SoundShow::OnBnClickedOpen)
	ON_BN_CLICKED(IDC_RECORD_BEG, &SoundShow::OnBnClickedRecordBeg)
	ON_BN_CLICKED(IDC_RECORD_END, &SoundShow::OnBnClickedRecordEnd)
	ON_BN_CLICKED(IDC_PLAY_BEG, &SoundShow::OnBnClickedPlayBeg)
	ON_BN_CLICKED(IDC_PLAY_END, &SoundShow::OnBnClickedPlayEnd)
	ON_BN_CLICKED(IDC_QUIT, &SoundShow::OnBnClickedQuit)
	ON_BN_CLICKED(IDC_SEND, &SoundShow::OnBnClickedSend)
	ON_BN_CLICKED(IDC_SS, &SoundShow::OnBnClickedSs)
	ON_BN_CLICKED(IDC_SC, &SoundShow::OnBnClickedSc)
	ON_BN_CLICKED(IDC_SRESET, &SoundShow::OnBnClickedSreset)
	ON_BN_CLICKED(IDC_SOK, &SoundShow::OnBnClickedSok)
END_MESSAGE_MAP()

// SoundShow 消息处理程序

//打开设备时消息，在此期间我们可以进行一些初始化工作
LRESULT SoundShow::OnMM_WIM_OPEN(UINT wParam, LONG lParam)
{
	// TODO: Add your message handler code here and/or call default     
	// Shrink down the save buffer

	GetDlgItem(IDC_STATIC1)->SetWindowTextW(TEXT("OnMM_WIM_OPEN"));

	((CWnd *)(this->GetDlgItem(IDC_RECORD_BEG)))->EnableWindow(FALSE);
	((CWnd *)(this->GetDlgItem(IDC_RECORD_END)))->EnableWindow(TRUE);
	((CWnd *)(this->GetDlgItem(IDC_PLAY_BEG)))->EnableWindow(FALSE);
	((CWnd *)(this->GetDlgItem(IDC_PLAY_END)))->EnableWindow(FALSE);
	((CWnd *)(this->GetDlgItem(IDC_SAVE)))->EnableWindow(TRUE);
	((CWnd *)(this->GetDlgItem(IDC_OPEN)))->EnableWindow(TRUE);


	// Shrink down the save buffer
	pSaveBuffer = (PBYTE)realloc(pSaveBuffer, 1);
	// Add the buffers
	waveInAddBuffer(hWaveIn, pWaveHdr1, sizeof(WAVEHDR));
	waveInAddBuffer(hWaveIn, pWaveHdr2, sizeof(WAVEHDR));
	//begin sampling
	bRecording = TRUE;
	bEnding = FALSE;
	dwDataLength = 0;
	waveInStart(hWaveIn);

	
	return NULL;
}

LRESULT SoundShow::OnMM_WIM_DATA(UINT wParam, LONG lParam)
{
	//当开始录音后 当buffer已满后，将收到MM_WIN_DATA消息，处理该消息可以保存已录好的数据

	GetDlgItem(IDC_STATIC1)->SetWindowTextW(TEXT("OnMM_WIM_DATA"));

	pNewBuffer = (PBYTE)realloc(pSaveBuffer, dwDataLength +((PWAVEHDR)lParam)->dwBytesRecorded);

	if (pNewBuffer == NULL)
	{
		waveInClose(hWaveIn);
		MessageBeep(MB_ICONEXCLAMATION);
		return NULL;
	}

	pSaveBuffer = pNewBuffer;
	CopyMemory(pSaveBuffer + dwDataLength, ((PWAVEHDR)lParam)->lpData,((PWAVEHDR)lParam)->dwBytesRecorded);

	dwDataLength += ((PWAVEHDR)lParam)->dwBytesRecorded;

	if (bEnding)
	{
		waveInClose(hWaveIn);
		return TRUE;
	}

	waveInAddBuffer(hWaveIn, (PWAVEHDR)lParam, sizeof(WAVEHDR));
	return NULL;
}

LRESULT SoundShow::OnMM_WIM_CLOSE(UINT wParam, LONG lParam)
{
	// Free the buffer memory

	waveInUnprepareHeader(hWaveIn, pWaveHdr1, sizeof(WAVEHDR));
	waveInUnprepareHeader(hWaveIn, pWaveHdr2, sizeof(WAVEHDR));

	free(pBuffer1);
	free(pBuffer2);

	// Enable and disable buttons

	((CWnd *)(this->GetDlgItem(IDC_RECORD_BEG)))->EnableWindow(TRUE);
	((CWnd *)(this->GetDlgItem(IDC_RECORD_END)))->EnableWindow(FALSE);


	if (dwDataLength > 0)
	{

		((CWnd *)(this->GetDlgItem(IDC_PLAY_BEG)))->EnableWindow(TRUE);
		((CWnd *)(this->GetDlgItem(IDC_PLAY_END)))->EnableWindow(FALSE);
	}

	if (bTerminating)
		SendMessage((DWORD)m_hWnd, WM_SYSCOMMAND, SC_CLOSE);

	return NULL;
}

LRESULT SoundShow::OnMM_WOM_OPEN(UINT wParam, LONG lParam)
{
	//打开音频文件
	// Set up header

	pWaveHdr1->lpData = (LPSTR)pSaveBuffer;
	pWaveHdr1->dwBufferLength = dwDataLength;
	pWaveHdr1->dwBytesRecorded = 0;
	pWaveHdr1->dwUser = 0;
	pWaveHdr1->dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;
	pWaveHdr1->dwLoops = dwRepetitions;
	pWaveHdr1->lpNext = NULL;
	pWaveHdr1->reserved = 0;

	// Prepare and write

	waveOutPrepareHeader(hWaveOut, pWaveHdr1, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, pWaveHdr1, sizeof(WAVEHDR));

	((CWnd *)(this->GetDlgItem(IDC_RECORD_BEG)))->EnableWindow(FALSE);
	((CWnd *)(this->GetDlgItem(IDC_RECORD_END)))->EnableWindow(FALSE);
	((CWnd *)(this->GetDlgItem(IDC_PLAY_BEG)))->EnableWindow(FALSE);
	((CWnd *)(this->GetDlgItem(IDC_PLAY_END)))->EnableWindow(TRUE);

	bEnding = FALSE;
	bPlaying = TRUE;

	return NULL;
}

LRESULT SoundShow::OnMM_WOM_DONE(UINT wParam, LONG lParam)
{
	waveOutUnprepareHeader(hWaveOut, pWaveHdr1, sizeof(WAVEHDR));
	waveOutClose(hWaveOut);
	return NULL;
}

LRESULT SoundShow::OnMM_WOM_CLOSE(UINT wParam, LONG lParam)
{
	((CWnd *)(this->GetDlgItem(IDC_RECORD_BEG)))->EnableWindow(TRUE);
	((CWnd *)(this->GetDlgItem(IDC_RECORD_END)))->EnableWindow(TRUE);
	((CWnd *)(this->GetDlgItem(IDC_PLAY_BEG)))->EnableWindow(TRUE);
	((CWnd *)(this->GetDlgItem(IDC_PLAY_END)))->EnableWindow(TRUE);
	dwRepetitions = 1;

	if (bTerminating)
	{
		SendMessage((DWORD)m_hWnd, WM_SYSCOMMAND, SC_CLOSE);
	}
		
	return NULL;
}

void SoundShow::OnSysCommand(UINT wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
	case SC_CLOSE: {
		if (bRecording)
		{
			bTerminating = TRUE;
			bEnding = TRUE;
			waveInReset(hWaveIn);
		//	return;
		}

		if (bPlaying)
		{
			bTerminating = TRUE;
			bEnding = TRUE;
			waveOutReset(hWaveOut);
			return;
		}

		free(pWaveHdr1);
		free(pWaveHdr2);
		free(pSaveBuffer);
		EndDialog((DWORD)m_hWnd);
		AfxGetMainWnd()->SendMessage(WM_CLOSE);    /* exit the pragma*/
	//	return;
	}
	default:CDialog::OnSysCommand(wParam, lParam); break;
	}
	return;
}

void SoundShow::OnBnClickedQuit()
{
	AfxGetMainWnd()->SendMessage(WM_CLOSE);    /* exit the pragma*/

}

//保存文件
void SoundShow::OnBnClickedSave()
{
	// TODO: 在此添加控件通知处理程序代码
	//保存文件
	FILE *pfile = fopen("c:\\wave.pcm", "wb");
	if (pfile == NULL) return ;
	fwrite(pSaveBuffer, sizeof(BYTE), dwDataLength, pfile);
	fclose(pfile);
	g711encode();
}

//打开文件
void SoundShow::OnBnClickedOpen()
{
	// TODO: 在此添加控件通知处理程序代码
	//打开文件
	FILE *pfile = fopen("c:\\wave.pcm", "rb");
	if (pfile == NULL) return  ;
	dwDataLength = 1024 * 1024 * 30;
	//  fread(&dwDataLength,sizeof(DWORD),1,pfile);
	pSaveBuffer = (PBYTE)realloc(pSaveBuffer, dwDataLength);
	dwDataLength = fread(pSaveBuffer, sizeof(BYTE), dwDataLength, pfile);
	fclose(pfile);
	//文件存在(数据长度不为零) 使能部分按键
	if (dwDataLength > 0)
	{
		((CWnd *)(this->GetDlgItem(IDC_RECORD_BEG)))->EnableWindow(TRUE);
		((CWnd *)(this->GetDlgItem(IDC_RECORD_END)))->EnableWindow(FALSE);
		((CWnd *)(this->GetDlgItem(IDC_PLAY_BEG)))->EnableWindow(TRUE);
		((CWnd *)(this->GetDlgItem(IDC_PLAY_END)))->EnableWindow(FALSE);
	}
	bRecording = FALSE;
}

//开始录音
void SoundShow::OnBnClickedRecordBeg()
{
	// TODO: 在此添加控件通知处理程序代码

	//开始录音 分配内存  双通道
	pBuffer1 = (PBYTE)malloc(INP_BUFFER_SIZE);
	pBuffer2 = (PBYTE)malloc(INP_BUFFER_SIZE);

	if (!pBuffer1 || !pBuffer2)
	{
		if (pBuffer1) free(pBuffer1);
		if (pBuffer2) free(pBuffer2);

		MessageBeep(MB_ICONEXCLAMATION);
		AfxMessageBox(TEXT("Memory error!"));
		return ;
	}
	waveform.wFormatTag = WAVE_FORMAT_PCM;//设置波形声音的格式
	waveform.nChannels = 1;//设置音频文件的通道数量
	waveform.nSamplesPerSec = 8000;//设置每个声道播放和记录时的样本频率
	waveform.wBitsPerSample = 16;
	waveform.nBlockAlign = waveform.nChannels * (waveform.wBitsPerSample / 8);//以字节为单位设置块对齐
	waveform.nAvgBytesPerSec = waveform.nSamplesPerSec * waveform.nBlockAlign; //设置请求的平均数据传输率,单位byte/s。这个值对于创建缓冲大小是很有用的
	waveform.cbSize = 0;//额外信息的大小
	/*
	waveInOpen函数的
		第1个参数表示：一个特定的录音设备指针，如果设备启动成功，该参数的值将会被赋值为启动的设备
		第2参数表示：需要启动的设备ID。一般不会手动的指定某个设备，而是通过设置WAVE_MAPPER，通过系统查找可用的设备
		第3个参数表示：音频流信息对象的指针。这个参数就是我们第一步设置的对象
		第4个参数表示：录音消息的处理程序，可以设置为一个函数、或者事件句柄、窗口句柄、一个特定的线程。也就是说录音消息产生后，由这个参数对应的值来处理该消息。包括关闭录音、缓冲区已满、开启设备。
		第5个参数表示：第四个参数的参数列表
		第6个参数表示：打开设备的标示符。对应第四个参数，如果第四个参数设置为函数，则第6个参数的值为CALLBACK_FUNCTION；如果是事件，则为CALLBACK_EVENT；
													如果为窗体句柄（第5个参数设置为0），则为CALLBACK_WINDOW；
													如过设置为0，则为CALLBACK_NULL；如果为线程，则为CALLBACK_THREAD
		表示的意思就是当录音产生消息后，由谁来处理相应的消息。
		注意：要想该函数成功执行，必须在开始之前，有录音设备的存在（台式电脑一定要插入麦克风才可以被检测到）*/
	MMRESULT mRet = waveInOpen(&hWaveIn, WAVE_MAPPER, &waveform, (DWORD)m_hWnd, NULL, CALLBACK_WINDOW);
	if (mRet != MMSYSERR_NOERROR)
	{
		free(pBuffer1);
		free(pBuffer2);
		MessageBeep(MB_ICONEXCLAMATION);
		AfxMessageBox(TEXT("Message error!"));
		return;
	}

	//设置头部值
	pWaveHdr1->lpData = (LPSTR)pBuffer1;
	pWaveHdr1->dwBufferLength = INP_BUFFER_SIZE;
	pWaveHdr1->dwBytesRecorded = 0;
	pWaveHdr1->dwUser = 0;
	pWaveHdr1->dwFlags = 0;
	pWaveHdr1->dwLoops = 1;
	pWaveHdr1->lpNext = NULL;
	pWaveHdr1->reserved = 0;

	waveInPrepareHeader(hWaveIn, pWaveHdr1, sizeof(WAVEHDR));

	pWaveHdr2->lpData = (LPSTR)pBuffer2;
	pWaveHdr2->dwBufferLength = INP_BUFFER_SIZE;
	pWaveHdr2->dwBytesRecorded = 0;
	pWaveHdr2->dwUser = 0;
	pWaveHdr2->dwFlags = 0;
	pWaveHdr2->dwLoops = 1;
	pWaveHdr2->lpNext = NULL;
	pWaveHdr2->reserved = 0;

	waveInPrepareHeader(hWaveIn, pWaveHdr2, sizeof(WAVEHDR));
	
	((CWnd *)(this->GetDlgItem(IDC_RECORD_BEG)))->EnableWindow(FALSE);
	((CWnd *)(this->GetDlgItem(IDC_RECORD_END)))->EnableWindow(TRUE);
	((CWnd *)(this->GetDlgItem(IDC_PLAY_BEG)))->EnableWindow(FALSE);
	((CWnd *)(this->GetDlgItem(IDC_PLAY_END)))->EnableWindow(FALSE);

	/*
	pSaveBuffer = (PBYTE)realloc(pSaveBuffer, 16384);
	//add the bufer
	waveInAddBuffer(hWaveIn, pWaveHdr1, sizeof(WAVEHDR));
	waveInAddBuffer(hWaveIn, pWaveHdr2, sizeof(WAVEHDR));

	waveInStart(hWaveIn);*/

	GetDlgItem(IDC_STATIC1)->SetWindowTextW(TEXT("start recording!"));
}

//结束录音
void SoundShow::OnBnClickedRecordEnd()
{
	// TODO: 在此添加控件通知处理程序代码
	((CWnd *)(this->GetDlgItem(IDC_RECORD_BEG)))->EnableWindow(TRUE);
	((CWnd *)(this->GetDlgItem(IDC_RECORD_END)))->EnableWindow(FALSE);
	((CWnd *)(this->GetDlgItem(IDC_PLAY_BEG)))->EnableWindow(TRUE);
	((CWnd *)(this->GetDlgItem(IDC_PLAY_END)))->EnableWindow(FALSE);

	bEnding = TRUE;
	
	// Reset input to return last buffer
	waveInReset(hWaveIn);
	//waveInClose(hWaveIn);
	GetDlgItem(IDC_STATIC1)->SetWindowTextW(TEXT("stop recording!"));
}

//开始播放
void SoundShow::OnBnClickedPlayBeg()
{
	// TODO: 在此添加控件通知处理程序代码
	waveform.wFormatTag = WAVE_FORMAT_PCM;//设置波形声音的格式
	waveform.nChannels = 1;//设置音频文件的通道数量
	waveform.nSamplesPerSec = 8000;//设置每个声道播放和记录时的样本频率
	waveform.wBitsPerSample = 16;
	waveform.nBlockAlign = waveform.nChannels * (waveform.wBitsPerSample / 8);//以字节为单位设置块对齐
	waveform.nAvgBytesPerSec = waveform.nSamplesPerSec * waveform.nBlockAlign; //设置请求的平均数据传输率,单位byte/s。这个值对于创建缓冲大小是很有用的
	waveform.cbSize = 0;//额外信息的大小

	//MMRESULT mRet = waveOutOpen(&hWaveOut, WAVE_MAPPER, &waveform, (DWORD)MYCALLBACK_FUN, (DWORD)this, CALLBACK_FUNCTION);
	if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &waveform, (DWORD)m_hWnd, NULL, CALLBACK_WINDOW))
	{
		MessageBeep(MB_ICONEXCLAMATION);
		AfxMessageBox(TEXT("Audio output error"));
	}
}

//结束播放
void SoundShow::OnBnClickedPlayEnd()
{
	// TODO: 在此添加控件通知处理程序代码
	bEnding = TRUE;
	waveOutReset(hWaveOut);
	//waveOutClose(hWaveOut);
}

//窗口初始化
BOOL SoundShow::OnInitDialog()
{
	CDialog::OnInitDialog();
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}
	
	// TODO:  在此添加额外的初始化
	try {
		waveform.wFormatTag = WAVE_FORMAT_PCM;//设置波形声音的格式
		waveform.nChannels = 1; //设置音频文件的通道数量
		waveform.nSamplesPerSec = 8000; // 设置每个声道播放和记录时的样本频率
		waveform.wBitsPerSample = 16;	//设置每个样本的位深（即每次采样样本的大小，以bit为单位）
		waveform.nBlockAlign = (waveform.wBitsPerSample * waveform.nChannels) >> 3;//以字节为单位设置块对齐
		waveform.nAvgBytesPerSec = waveform.nBlockAlign * waveform.nSamplesPerSec;//设置请求的平均数据传输率,单位byte/s。这个值对于创建缓冲大小是很有用的
		waveform.cbSize = 0; //额外信息的大小
		
		((CWnd *)(this->GetDlgItem(IDC_RECORD_BEG)))->EnableWindow(TRUE);
		((CWnd *)(this->GetDlgItem(IDC_RECORD_END)))->EnableWindow(FALSE);
		((CWnd *)(this->GetDlgItem(IDC_PLAY_BEG)))->EnableWindow(TRUE);
		((CWnd *)(this->GetDlgItem(IDC_PLAY_END)))->EnableWindow(FALSE);


		//为头部分配空间
		pWaveHdr1 = (PWAVEHDR)malloc(sizeof(WAVEHDR));
		pWaveHdr2 = (PWAVEHDR)malloc(sizeof(WAVEHDR));
		//存储buff分配内存
		pSaveBuffer = (PBYTE)malloc(1);
		dwDataLength = 0;

		//socket设置
		m_TargetPort = 8000;
		m_IPAddress.SetAddress(127, 0, 0, 1);
		m_PortShow.SetWindowText(_T("9000"));

		//默认为发送方
		m_bIsSend = TRUE;	
		m_IsServer = TRUE;
		m_IsRemoteOK = TRUE;
		m_bHasNewData = FALSE;
		m_IsServerCreatedSucceed = FALSE;
		m_IsClientCreatedSucceed = FALSE;
		m_nSendBufferIndex = 0;
		m_nReceiveBufferIndex = 0;

		return TRUE;
		
	}
	catch(WAVEFORMATEX){
		return FALSE;// 异常: OCX 属性页应返回 FALSE
	}	 
}

//程序主要算法
//pdInput[IN1]		发送端时钟
//pdInput[IN2]		接收端时钟
//pdInput[IN3]		帧数据
//pdOutput[OUT1]	发送端转化的bit stream
//pdOutput[OUT2]	接收端还原的bit stream
void SoundShow::RunAlgrithm(const double * pdInput, double * pdOutput) {
	
}


void SoundShow::OnBnClickedSend()
{
	// TODO: 在此添加控件通知处理程序代码
	//socket发送,写入文件，然后通过另一个模块来读取
	OnBnClickedSave();

	FILE *pfile = fopen("c:\\test.wave", "rb");
	if (pfile == NULL) return;
	dwDataLength = 1024 * 1024 * 30;
	pSaveBuffer = (PBYTE)realloc(pSaveBuffer, dwDataLength);
	m_nSendBufferSize = fread(pSaveBuffer, sizeof(BYTE), dwDataLength, pfile);
	fclose(pfile);

	m_nSendBuffer = new char[m_nSendBufferSize];//获取 需要发送的数据的大小
	m_TargetSocket.Send(m_nSendBuffer, m_nSendBufferSize);

}

LRESULT SoundShow::OnNewData(WPARAM wParam, LPARAM lParam)
{
	CNewSocket *pSocket = reinterpret_cast<CNewSocket*>(wParam);
	ASSERT(pSocket != NULL && !::IsBadReadPtr((void*)pSocket, sizeof(CNewSocket)));

	if (m_IsServer) {//服务器端，作为接受数据的一端
		char* a = pSocket->GetRevBuffer();
		int bs = pSocket->m_nMsgLength;
		char *buff = new char[bs];
		memcpy(buff, a, bs * sizeof(char));
		bs =bs;
		pSocket->Send(buff, bs*sizeof(char));
	}
	else {//客户端，作为数据的发送端
		//操作代码在OnSend中
	}
	return LRESULT();
}

#if 0
void SoundShow::RunAlgrithm(const double * pdInput, double * pdOutput) {
	//输入时钟
	//数据发送
	double nClk = pdInput[IN1];

#if 1//进行bit流转换
	//数据帧格式： 2B帧头 + 数据段大小可变 + 2B帧尾
	if (nClk > 1.0 && m_nClkState < 1.0)	//时钟上升沿输出数据
	{
		if (m_nOutCount == 0 || m_nOutCount == 1)	//重新开始传输,首先开始传输帧头,帧头为0xffff，占2字节
		{
			pdOutput[OUT1] = 3.3;
			m_nFrameHeaderCount++;
			if (m_nFrameHeaderCount > 15)
				m_nOutCount = 2;

		}
		else if (m_nOutCount < m_nSendBufferSize + 2)	//发送数据，由于2B帧头，故存在加减2修复误差
		{
			int nBitData[8] = { 0 };
			int nData = m_nSendBuffer[m_nOutCount - 2];//读取1B
			for (int i = 0; i < 8; ++i)//逐bit赋值
			{
				if ((nData & 0x80) > 0)
					nBitData[i] = 1;
				else
					nBitData[i] = 0;
				nData = nData << 1;
			}
			pdOutput[OUT1] = nBitData[m_nBitCount] * 3.3;
			m_nBitCount++;
			if (m_nBitCount > 7)
			{
				m_nBitCount = 0;
				m_nOutCount++;
			}
		}
		else if (m_nOutCount > m_nSendBufferSize + 1)//也即m_nOutCount >= m_nSendBufferSize + 2
		{
			//发送完毕后，发送帧尾,帧尾为0xfffe,占2字节
			if (m_nFrameEndCount < 15)
			{
				pdOutput[OUT1] = 3.3;
				m_nFrameEndCount++;
			}
			else
			{
				pdOutput[OUT1] = 0;
				m_nFrameEndCount = 0;
				m_nOutCount = 0;
				m_nBitCount = 0;
				m_nFrameHeaderCount = 0;
				//delete m_nSendBuffer;
			}
		}
		/*
		m_RevTestBuffer[m_nTestCount] = pdOutput[OUT1];
		m_nTestCount++;
		if (m_nTestCount > 63)
		m_nTestCount = 0;
		*/
	}
	m_nClkState = nClk;

	//数据接收
	//pdInput[IN2]为时钟信号
	//pdInput[IN3]为数据流
	//pdOutput[OUT2]为恢复的数据流
	double nReClk = pdInput[IN2];
	if (nReClk < 1.0 && m_nReClkState > 1.0)	//时钟下降沿处理数据
	{
		//接收缓冲
		//首先else接收帧头0xffff,当接收完帧头后，数据以8bit为单位组成一个字节放入char数组缓冲中
		//帧尾为0xfffe
		if (m_bFrameHeader == TRUE)
		{
			if (pdInput[IN3] > 1.0)
				m_nRevBitBuffer[m_nRevBitCount] = 1;
			else
				m_nRevBitBuffer[m_nRevBitCount] = 0;
			pdOutput[OUT2] = m_nRevBitBuffer[m_nRevBitCount] * 3.3;
			m_nRevBitCount++;
			if (m_nRevBitCount > 7)	//接收完一字节
			{
				m_RevByteBuffer[m_nRevByteCount] = m_nRevBitBuffer[0] * 128 + m_nRevBitBuffer[1] * 64 + m_nRevBitBuffer[2] * 32 + m_nRevBitBuffer[3] * 16 + m_nRevBitBuffer[4] * 8 + m_nRevBitBuffer[5] * 4 + m_nRevBitBuffer[6] * 2 + m_nRevBitBuffer[7];
				m_nRevBitCount = 0;
				if (m_RevByteBuffer[m_nRevByteCount] == 0xfe && m_RevByteBuffer[m_nRevByteCount - 1] == 0xff)	//接收到帧尾,一帧数据接收完毕
				{
					//将数据赋值给字符串
					for (int i = 0; i < m_nRevByteCount - 1; ++i)
					{
						m_RevBuffer[i] = m_RevByteBuffer[i];
					}
					dwDataLength = m_nRevByteCount - 1;
					pSaveBuffer = (PBYTE)malloc(sizeof(PBYTE)*dwDataLength);
					memcpy(pSaveBuffer, m_RevBuffer, dwDataLength);
					/*
					std::string strRevText = m_RevBuffer;
					std::wstring wstrRevText;
					utility::Ansi2Unicode(strRevText, wstrRevText);
					m_TextShowEdit.SetWindowText(wstrRevText.c_str());
					*/
					m_bRevOK = TRUE;
					m_bFrameHeader = FALSE;
				}
				m_nRevByteCount++;
				if (m_nRevByteCount > 10 * 1024 * 1024)//max = 10M
				{
					m_nRevByteCount = 0;
					m_bFrameHeader = FALSE;
				}
			}
		}
		// 先接收帧头
		else
		{
			if (pdInput[IN3] > 1.0)
				m_nFrameRevCount++;
			else
				m_nFrameRevCount = 0;
			if (m_nFrameRevCount < 0)
				m_nFrameRevCount = 0;
			if (m_nFrameRevCount == 16)//帧头接收完成，初始化开始接受数据
			{
				m_bFrameHeader = TRUE;
				m_nRevBitCount = 0;
				m_nRevByteCount = 0;
				m_nFrameRevCount = 0;
				memset(m_RevByteBuffer, 0, sizeof(m_RevByteBuffer));
				memset(m_nRevBitBuffer, 0, sizeof(m_nRevBitBuffer));
				memset(m_RevBuffer, 0, sizeof(m_RevBuffer));
			}
		}
	}
	m_nReClkState = nReClk;
#endif
}
#endif

void SoundShow::OnBnClickedSs()
{
	// TODO: 在此添加控件通知处理程序代码
	((CWnd *)(this->GetDlgItem(IDC_SC)))->EnableWindow(FALSE);
	((CWnd *)(this->GetDlgItem(IDC_IPADDRESS1)))->EnableWindow(FALSE);
	m_IsServer = TRUE;

}


void SoundShow::OnBnClickedSc()
{
	// TODO: 在此添加控件通知处理程序代码
	((CWnd *)(this->GetDlgItem(IDC_SS)))->EnableWindow(FALSE);
	((CWnd *)(this->GetDlgItem(IDC_IPADDRESS1)))->EnableWindow(TRUE);
	m_IsServer = FALSE;
}


void SoundShow::OnBnClickedSreset()
{
	// TODO: 在此添加控件通知处理程序代码
	((CWnd *)(this->GetDlgItem(IDC_SC)))->EnableWindow(TRUE);
	((CWnd *)(this->GetDlgItem(IDC_SS)))->EnableWindow(TRUE);
	((CWnd *)(this->GetDlgItem(IDC_IPADDRESS1)))->EnableWindow(TRUE);
	m_IsServer = TRUE;
	if (m_IsServerCreatedSucceed) {

		m_IsServerCreatedSucceed = FALSE;
		m_Server.Close();
		
	}
	if (m_IsClientCreatedSucceed) {

		m_IsClientCreatedSucceed = FALSE;
		m_TargetSocket.Close();
	}

}


void SoundShow::OnBnClickedSok()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strPortNum;
	m_PortShow.GetWindowText(strPortNum);//获取端口号
	m_TargetPort = _wtoi(strPortNum);

	//server
	/*
	首先，讨论Create函数，分析socket句柄如何被创建并和CAsyncSocket对象关联。Create的实现如下：
	BOOL CAsyncSocket::Create(UINT nSocketPort, int nSocketType,
	long lEvent, LPCTSTR lpszSocketAddress)
	{
	if (Socket(nSocketType, lEvent))
	{
	if (Bind(nSocketPort,lpszSocketAddress))
	return TRUE;
	int nResult = GetLastError();
	Close();
	WSASetLastError(nResult);
	}
	return FALSE;
	}
	其中：
	参数1表示本socket的端口，缺省是0，如果要创建数据报的socket，则必须指定一个端口号。
	参数2表示本socket的类型，缺省是SOCK_STREAM，表示面向连接类型。
	参数3是屏蔽位，表示希望对本socket监测的事件，缺省是FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE。
	参数4表示本socket的IP地址字符串，缺省是NULL。
	Create调用Socket函数创建一个socket，并把它捆绑在this所指对象上，监测指定的网络事件。参数2和3被传递给Socket函数，如果希望创建数据报的socket，不要使用缺省参数，指定参数2是SOCK_DGRM。
	如果上一步骤成功，则调用bind给新的socket分配端口和IP地址。
	*/
	if (m_IsServer && FALSE == m_IsServerCreatedSucceed) {
		BOOL bFlag = FALSE;
		//m_Server = new CLtSocket();
		bFlag = m_Server.Create(m_TargetPort, SOCK_STREAM, FD_CLOSE | FD_ACCEPT);	//创建Socket服务
		if (!bFlag)
		{
			m_Server.Close();
			AfxMessageBox(_T("创建数据服务套接字失败!"));
			return;
		}
		if (!m_Server.Listen(1)) {//监听
			int nErrorCode = m_Server.GetLastError();
			if (nErrorCode != WSAEWOULDBLOCK)
			{
				m_Server.Close();
				AfxMessageBox(_T("开启数据服务失败!"));
				return;
			}
		}
		m_Server.SetViewWnd(this);
		m_IsServerCreatedSucceed = TRUE;
	}
	if (m_IsServerCreatedSucceed) {
		AfxMessageBox(_T("已创建成功！"));
	}
	

	//client
	if(FALSE == m_IsServer && FALSE ==m_IsClientCreatedSucceed){
		
		DWORD dwIP;
		if (m_IPAddress.GetAddress(dwIP) < 4)	//获取目标IP地址
		{
			AfxMessageBox(_T("注意!\nIP地址填写不完整!"), MB_ICONWARNING);
			return;
		}
		unsigned char *pIP;
		pIP = (unsigned char*)&dwIP;
		unsigned char *IP1 = pIP + 3;
		unsigned char *IP2 = pIP + 2;
		unsigned char *IP3 = pIP + 1;
		unsigned char *IP4 = pIP;
		m_TargetIP.Format(_T("%u.%u.%u.%u"), *IP1, *IP2, *IP3, *IP4);
		m_TargetSocket.Create();
		m_TargetSocket.Connect(m_TargetIP, m_TargetPort);	//连接目标
		m_TargetSocket.AsyncSelect(FD_READ | FD_WRITE | FD_CLOSE);	//设置传输模式
		m_TargetSocket.SetViewWnd(this);	//设置消息返回事件的句柄
		m_IsClientCreatedSucceed = TRUE;
	}
	if (m_IsClientCreatedSucceed) {
		AfxMessageBox(_T("已创建成功！"));
	}
}
