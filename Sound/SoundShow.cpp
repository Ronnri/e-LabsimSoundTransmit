// SoundShow.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Sound.h"
#include "SoundShow.h"
#include "afxdialogex.h"
#include "g711.h"
#include "utility.h"

// SoundShow �Ի���

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

// SoundShow ��Ϣ�������

//���豸ʱ��Ϣ���ڴ��ڼ����ǿ��Խ���һЩ��ʼ������
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
	//����ʼ¼���� ��buffer�����󣬽��յ�MM_WIN_DATA��Ϣ���������Ϣ���Ա�����¼�õ�����

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
	//����Ƶ�ļ�
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

//�����ļ�
void SoundShow::OnBnClickedSave()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//�����ļ�
	FILE *pfile = fopen("c:\\wave.pcm", "wb");
	if (pfile == NULL) return ;
	fwrite(pSaveBuffer, sizeof(BYTE), dwDataLength, pfile);
	fclose(pfile);
	g711encode();
}

//���ļ�
void SoundShow::OnBnClickedOpen()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//���ļ�
	FILE *pfile = fopen("c:\\wave.pcm", "rb");
	if (pfile == NULL) return  ;
	dwDataLength = 1024 * 1024 * 30;
	//  fread(&dwDataLength,sizeof(DWORD),1,pfile);
	pSaveBuffer = (PBYTE)realloc(pSaveBuffer, dwDataLength);
	dwDataLength = fread(pSaveBuffer, sizeof(BYTE), dwDataLength, pfile);
	fclose(pfile);
	//�ļ�����(���ݳ��Ȳ�Ϊ��) ʹ�ܲ��ְ���
	if (dwDataLength > 0)
	{
		((CWnd *)(this->GetDlgItem(IDC_RECORD_BEG)))->EnableWindow(TRUE);
		((CWnd *)(this->GetDlgItem(IDC_RECORD_END)))->EnableWindow(FALSE);
		((CWnd *)(this->GetDlgItem(IDC_PLAY_BEG)))->EnableWindow(TRUE);
		((CWnd *)(this->GetDlgItem(IDC_PLAY_END)))->EnableWindow(FALSE);
	}
	bRecording = FALSE;
}

//��ʼ¼��
void SoundShow::OnBnClickedRecordBeg()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	//��ʼ¼�� �����ڴ�  ˫ͨ��
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
	waveform.wFormatTag = WAVE_FORMAT_PCM;//���ò��������ĸ�ʽ
	waveform.nChannels = 1;//������Ƶ�ļ���ͨ������
	waveform.nSamplesPerSec = 8000;//����ÿ���������źͼ�¼ʱ������Ƶ��
	waveform.wBitsPerSample = 16;
	waveform.nBlockAlign = waveform.nChannels * (waveform.wBitsPerSample / 8);//���ֽ�Ϊ��λ���ÿ����
	waveform.nAvgBytesPerSec = waveform.nSamplesPerSec * waveform.nBlockAlign; //���������ƽ�����ݴ�����,��λbyte/s�����ֵ���ڴ��������С�Ǻ����õ�
	waveform.cbSize = 0;//������Ϣ�Ĵ�С
	/*
	waveInOpen������
		��1��������ʾ��һ���ض���¼���豸ָ�룬����豸�����ɹ����ò�����ֵ���ᱻ��ֵΪ�������豸
		��2������ʾ����Ҫ�������豸ID��һ�㲻���ֶ���ָ��ĳ���豸������ͨ������WAVE_MAPPER��ͨ��ϵͳ���ҿ��õ��豸
		��3��������ʾ����Ƶ����Ϣ�����ָ�롣��������������ǵ�һ�����õĶ���
		��4��������ʾ��¼����Ϣ�Ĵ�����򣬿�������Ϊһ�������������¼���������ھ����һ���ض����̡߳�Ҳ����˵¼����Ϣ�����������������Ӧ��ֵ���������Ϣ�������ر�¼���������������������豸��
		��5��������ʾ�����ĸ������Ĳ����б�
		��6��������ʾ�����豸�ı�ʾ������Ӧ���ĸ�������������ĸ���������Ϊ���������6��������ֵΪCALLBACK_FUNCTION��������¼�����ΪCALLBACK_EVENT��
													���Ϊ����������5����������Ϊ0������ΪCALLBACK_WINDOW��
													�������Ϊ0����ΪCALLBACK_NULL�����Ϊ�̣߳���ΪCALLBACK_THREAD
		��ʾ����˼���ǵ�¼��������Ϣ����˭��������Ӧ����Ϣ��
		ע�⣺Ҫ��ú����ɹ�ִ�У������ڿ�ʼ֮ǰ����¼���豸�Ĵ��ڣ�̨ʽ����һ��Ҫ������˷�ſ��Ա���⵽��*/
	MMRESULT mRet = waveInOpen(&hWaveIn, WAVE_MAPPER, &waveform, (DWORD)m_hWnd, NULL, CALLBACK_WINDOW);
	if (mRet != MMSYSERR_NOERROR)
	{
		free(pBuffer1);
		free(pBuffer2);
		MessageBeep(MB_ICONEXCLAMATION);
		AfxMessageBox(TEXT("Message error!"));
		return;
	}

	//����ͷ��ֵ
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

//����¼��
void SoundShow::OnBnClickedRecordEnd()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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

//��ʼ����
void SoundShow::OnBnClickedPlayBeg()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	waveform.wFormatTag = WAVE_FORMAT_PCM;//���ò��������ĸ�ʽ
	waveform.nChannels = 1;//������Ƶ�ļ���ͨ������
	waveform.nSamplesPerSec = 8000;//����ÿ���������źͼ�¼ʱ������Ƶ��
	waveform.wBitsPerSample = 16;
	waveform.nBlockAlign = waveform.nChannels * (waveform.wBitsPerSample / 8);//���ֽ�Ϊ��λ���ÿ����
	waveform.nAvgBytesPerSec = waveform.nSamplesPerSec * waveform.nBlockAlign; //���������ƽ�����ݴ�����,��λbyte/s�����ֵ���ڴ��������С�Ǻ����õ�
	waveform.cbSize = 0;//������Ϣ�Ĵ�С

	//MMRESULT mRet = waveOutOpen(&hWaveOut, WAVE_MAPPER, &waveform, (DWORD)MYCALLBACK_FUN, (DWORD)this, CALLBACK_FUNCTION);
	if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &waveform, (DWORD)m_hWnd, NULL, CALLBACK_WINDOW))
	{
		MessageBeep(MB_ICONEXCLAMATION);
		AfxMessageBox(TEXT("Audio output error"));
	}
}

//��������
void SoundShow::OnBnClickedPlayEnd()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	bEnding = TRUE;
	waveOutReset(hWaveOut);
	//waveOutClose(hWaveOut);
}

//���ڳ�ʼ��
BOOL SoundShow::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	
	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	try {
		waveform.wFormatTag = WAVE_FORMAT_PCM;//���ò��������ĸ�ʽ
		waveform.nChannels = 1; //������Ƶ�ļ���ͨ������
		waveform.nSamplesPerSec = 8000; // ����ÿ���������źͼ�¼ʱ������Ƶ��
		waveform.wBitsPerSample = 16;	//����ÿ��������λ���ÿ�β��������Ĵ�С����bitΪ��λ��
		waveform.nBlockAlign = (waveform.wBitsPerSample * waveform.nChannels) >> 3;//���ֽ�Ϊ��λ���ÿ����
		waveform.nAvgBytesPerSec = waveform.nBlockAlign * waveform.nSamplesPerSec;//���������ƽ�����ݴ�����,��λbyte/s�����ֵ���ڴ��������С�Ǻ����õ�
		waveform.cbSize = 0; //������Ϣ�Ĵ�С
		
		((CWnd *)(this->GetDlgItem(IDC_RECORD_BEG)))->EnableWindow(TRUE);
		((CWnd *)(this->GetDlgItem(IDC_RECORD_END)))->EnableWindow(FALSE);
		((CWnd *)(this->GetDlgItem(IDC_PLAY_BEG)))->EnableWindow(TRUE);
		((CWnd *)(this->GetDlgItem(IDC_PLAY_END)))->EnableWindow(FALSE);


		//Ϊͷ������ռ�
		pWaveHdr1 = (PWAVEHDR)malloc(sizeof(WAVEHDR));
		pWaveHdr2 = (PWAVEHDR)malloc(sizeof(WAVEHDR));
		//�洢buff�����ڴ�
		pSaveBuffer = (PBYTE)malloc(1);
		dwDataLength = 0;


		return TRUE;
		// return TRUE unless you set the focus to a control
	}
	catch(WAVEFORMATEX){
		return FALSE;// �쳣: OCX ����ҳӦ���� FALSE
	}	 
}

//������Ҫ�㷨
//pdInput[IN1]		���Ͷ�ʱ��
//pdInput[IN2]		���ն�ʱ��
//pdInput[IN3]		֡����
//pdOutput[OUT1]	���Ͷ�ת����bit stream
//pdOutput[OUT2]	���ն˻�ԭ��bit stream
void SoundShow::RunAlgrithm(const double * pdInput, double * pdOutput) {
	//����ʱ��
	//���ݷ���
	double nClk = pdInput[IN1];

#if 1//����bit��ת��
	//����֡��ʽ�� 2B֡ͷ + ���ݶδ�С�ɱ� + 2B֡β
	if (nClk > 1.0 && m_nClkState < 1.0)	//ʱ���������������
	{
		if (m_nOutCount == 0 || m_nOutCount == 1)	//���¿�ʼ����,���ȿ�ʼ����֡ͷ,֡ͷΪ0xffff��ռ2�ֽ�
		{
			pdOutput[OUT1] = 3.3;
			m_nFrameHeaderCount++;
			if (m_nFrameHeaderCount > 15)
				m_nOutCount = 2;

		}
		else if (m_nOutCount < m_nSendBufferSize + 2)	//�������ݣ�����2B֡ͷ���ʴ��ڼӼ�2�޸����
		{
			int nBitData[8] = { 0 };
			int nData = m_nSendBuffer[m_nOutCount - 2];//��ȡ1B
			for (int i = 0; i < 8; ++i)//��bit��ֵ
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
		else if (m_nOutCount > m_nSendBufferSize + 1)//Ҳ��m_nOutCount >= m_nSendBufferSize + 2
		{
			//������Ϻ󣬷���֡β,֡βΪ0xfffe,ռ2�ֽ�
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

	//���ݽ���
	//pdInput[IN2]Ϊʱ���ź�
	//pdInput[IN3]Ϊ������
	//pdOutput[OUT2]Ϊ�ָ���������
	double nReClk = pdInput[IN2];
	if (nReClk < 1.0 && m_nReClkState > 1.0)	//ʱ���½��ش�������
	{
		//���ջ���
		//����else����֡ͷ0xffff,��������֡ͷ��������8bitΪ��λ���һ���ֽڷ���char���黺����
		//֡βΪ0xfffe
		if (m_bFrameHeader == TRUE)
		{
			if (pdInput[IN3] > 1.0)
				m_nRevBitBuffer[m_nRevBitCount] = 1;
			else
				m_nRevBitBuffer[m_nRevBitCount] = 0;
			pdOutput[OUT2] = m_nRevBitBuffer[m_nRevBitCount] * 3.3;
			m_nRevBitCount++;
			if (m_nRevBitCount > 7)	//������һ�ֽ�
			{
				m_RevByteBuffer[m_nRevByteCount] = m_nRevBitBuffer[0] * 128 + m_nRevBitBuffer[1] * 64 + m_nRevBitBuffer[2] * 32 + m_nRevBitBuffer[3] * 16 + m_nRevBitBuffer[4] * 8 + m_nRevBitBuffer[5] * 4 + m_nRevBitBuffer[6] * 2 + m_nRevBitBuffer[7];
				m_nRevBitCount = 0;
				if (m_RevByteBuffer[m_nRevByteCount] == 0xfe && m_RevByteBuffer[m_nRevByteCount - 1] == 0xff)	//���յ�֡β,һ֡���ݽ������
				{
					//�����ݸ�ֵ���ַ���
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
		// �Ƚ���֡ͷ
		else
		{
			if (pdInput[IN3] > 1.0)
				m_nFrameRevCount++;
			else
				m_nFrameRevCount = 0;
			if (m_nFrameRevCount < 0)
				m_nFrameRevCount = 0;
			if (m_nFrameRevCount == 16)//֡ͷ������ɣ���ʼ����ʼ��������
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	m_nSendBufferSize = dwDataLength;
	m_nSendBuffer = new char[m_nSendBufferSize];

	OnBnClickedSave();
	FILE *pfile = fopen("c:\\test.wave", "rb");
	if (pfile == NULL) return;
	fwrite(m_nSendBuffer, sizeof(m_nSendBuffer), m_nSendBufferSize, pfile);
	fclose(pfile);

}
