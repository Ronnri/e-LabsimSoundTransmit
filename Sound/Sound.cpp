// Sound.cpp : ���� DLL �ĳ�ʼ�����̡�
//

#include "stdafx.h"
#include "Sound.h"
#include "SoundShow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO:  ����� DLL ����� MFC DLL �Ƕ�̬���ӵģ�
//		��Ӵ� DLL �������κε���
//		MFC �ĺ������뽫 AFX_MANAGE_STATE ����ӵ�
//		�ú�������ǰ�档
//
//		����: 
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// �˴�Ϊ��ͨ������
//		}
//
//		�˺������κ� MFC ����
//		������ÿ��������ʮ����Ҫ��  ����ζ��
//		��������Ϊ�����еĵ�һ�����
//		���֣������������ж������������
//		������Ϊ���ǵĹ��캯���������� MFC
//		DLL ���á�
//
//		�й�������ϸ��Ϣ��
//		����� MFC ����˵�� 33 �� 58��
//

// CSoundApp

BEGIN_MESSAGE_MAP(CSoundApp, CWinApp)
END_MESSAGE_MAP()


// CSoundApp ����

CSoundApp::CSoundApp()
{
	// TODO:  �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CSoundApp ����

CSoundApp theApp;


// CSoundApp ��ʼ��

BOOL CSoundApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}
//����һ���㷨���󣬲������㷨����ָ��
void *LtCreateObject()
{
	SoundShow * pAlgorithm = new SoundShow();
	pAlgorithm->Create(IDD_SOUNDSHOW,NULL);	//����㷨�����Ǽ̳���CDialog��,������ô��д���
	pAlgorithm->ShowWindow( SW_SHOW );		//����㷨�����Ǽ̳���CDialog��,������ô��д���
	return static_cast<void*>(pAlgorithm);//����һ���µ��㷨����
}
//ɾ��һ���㷨���󣬴˺����ڴ��Ĳ�����pObject������LtCreateObject()�ķ���ֵ
void LtDestroyObject(void * pObject)
{
	ASSERT(pObject != NULL);
	ASSERT(!IsBadWritePtr(pObject, sizeof(SoundShow)));
	SoundShow * pAlgorithm = static_cast<SoundShow *>(pObject);
	pAlgorithm->DestroyWindow();	//����㷨�����Ǽ̳���CDialog��,������ô��д���
	delete pAlgorithm;	//ɾ��һ���㷨����
}
//�㷨������
//�˺�����һ��������pObject����LtCreateObject()�ķ���ֵ
//�˺����ڶ������� �°汾���̵ĺ����ӿ�
//�˺���������������pdInput����ָ�����������ָ��
//�˺������ĸ�������pdOutput����ָ����������ָ��
void LtDLLMain(void * pObject, const bool *, const double * pdInput, double * pdOutput)
{
	ASSERT(pObject != NULL);
	SoundShow * pAlgorithm = static_cast<SoundShow *>(pObject);
	pAlgorithm->RunAlgrithm(pdInput, pdOutput);	//�����㷨������RunAlgrithm()����);
}
//��ʾ�㷨��������(�����㷨�����Ǽ̳���CDialog��,�����д�˺���)
void LtShowWindow(void * pObject)
{
	ASSERT(pObject != NULL);
	ASSERT(!IsBadWritePtr(pObject, sizeof(SoundShow)));
	SoundShow * pAlgorithm = static_cast<SoundShow *>(pObject);
	pAlgorithm->ShowWindow( SW_SHOW || SW_RESTORE );	//����㷨�����Ǽ̳���CDialog��,������ô��д���
}
