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
	m_nTestCount = 0;

}

SoundShow::~SoundShow()
{
}

void SoundShow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(SoundShow, CDialog)
	ON_WM_SYSCOMMAND()
	ON_MESSAGE(MM_WIM_OPEN, OnMM_WIM_OPEN)
	ON_MESSAGE(MM_WIM_DATA, OnMM_WIM_DATA)
	ON_MESSAGE(MM_WIM_CLOSE, OnMM_WIM_CLOSE)
	ON_MESSAGE(MM_WOM_OPEN, OnMM_WOM_OPEN)
	ON_MESSAGE(MM_WOM_DONE, OnMM_WOM_DONE)
	ON_MESSAGE(MM_WOM_CLOSE, OnMM_WOM_CLOSE)
	ON_BN_CLICKED(IDC_SAVE, &SoundShow::OnBnClickedSave)
	ON_BN_CLICKED(IDC_OPEN, &SoundShow::OnBnClickedOpen)
	ON_BN_CLICKED(IDC_RECORD_BEG, &SoundShow::OnBnClickedRecordBeg)
	ON_BN_CLICKED(IDC_RECORD_END, &SoundShow::OnBnClickedRecordEnd)
	ON_BN_CLICKED(IDC_PLAY_BEG, &SoundShow::OnBnClickedPlayBeg)
	ON_BN_CLICKED(IDC_PLAY_END, &SoundShow::OnBnClickedPlayEnd)
	ON_BN_CLICKED(IDC_QUIT, &SoundShow::OnBnClickedQuit)
	ON_BN_CLICKED(IDC_SEND, &SoundShow::OnBnClickedSend)
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


		return TRUE;
		// return TRUE unless you set the focus to a control
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
				if (m_nRevByteCount > 10*1024*1024)//max = 10M
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

void SoundShow::OnBnClickedSend()
{
	// TODO: 在此添加控件通知处理程序代码

	m_nSendBufferSize = dwDataLength;
	m_nSendBuffer = new char[m_nSendBufferSize];

	OnBnClickedSave();
	FILE *pfile = fopen("c:\\test.wave", "rb");
	if (pfile == NULL) return;
	fwrite(m_nSendBuffer, sizeof(m_nSendBuffer), m_nSendBufferSize, pfile);
	fclose(pfile);

}
