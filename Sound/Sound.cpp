// Sound.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "Sound.h"
#include "SoundShow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO:  如果此 DLL 相对于 MFC DLL 是动态链接的，
//		则从此 DLL 导出的任何调入
//		MFC 的函数必须将 AFX_MANAGE_STATE 宏添加到
//		该函数的最前面。
//
//		例如: 
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// 此处为普通函数体
//		}
//
//		此宏先于任何 MFC 调用
//		出现在每个函数中十分重要。  这意味着
//		它必须作为函数中的第一个语句
//		出现，甚至先于所有对象变量声明，
//		这是因为它们的构造函数可能生成 MFC
//		DLL 调用。
//
//		有关其他详细信息，
//		请参阅 MFC 技术说明 33 和 58。
//

// CSoundApp

BEGIN_MESSAGE_MAP(CSoundApp, CWinApp)
END_MESSAGE_MAP()


// CSoundApp 构造

CSoundApp::CSoundApp()
{
	// TODO:  在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CSoundApp 对象

CSoundApp theApp;


// CSoundApp 初始化

BOOL CSoundApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}
//创建一个算法对象，并返回算法对象指针
void *LtCreateObject()
{
	SoundShow * pAlgorithm = new SoundShow();
	pAlgorithm->Create(IDD_SOUNDSHOW,NULL);	//如果算法对象是继承自CDialog类,则需调用此行代码
	pAlgorithm->ShowWindow( SW_SHOW );		//如果算法对象是继承自CDialog类,则需调用此行代码
	return static_cast<void*>(pAlgorithm);//返回一个新的算法对象
}
//删除一个算法对象，此函数期待的参数（pObject）正是LtCreateObject()的返回值
void LtDestroyObject(void * pObject)
{
	ASSERT(pObject != NULL);
	ASSERT(!IsBadWritePtr(pObject, sizeof(SoundShow)));
	SoundShow * pAlgorithm = static_cast<SoundShow *>(pObject);
	pAlgorithm->DestroyWindow();	//如果算法对象是继承自CDialog类,则需调用此行代码
	delete pAlgorithm;	//删除一个算法对象
}
//算法主函数
//此函数第一个参数（pObject）是LtCreateObject()的返回值
//此函数第二个参数 新版本工程的函数接口
//此函数第三个参数（pdInput）是指向输入数组的指针
//此函数第四个参数（pdOutput）是指向输出数组的指针
void LtDLLMain(void * pObject, const bool *, const double * pdInput, double * pdOutput)
{
	ASSERT(pObject != NULL);
	SoundShow * pAlgorithm = static_cast<SoundShow *>(pObject);
	pAlgorithm->RunAlgrithm(pdInput, pdOutput);	//运行算法（调用RunAlgrithm()函数);
}
//显示算法操作界面(仅当算法对象是继承自CDialog类,才需编写此函数)
void LtShowWindow(void * pObject)
{
	ASSERT(pObject != NULL);
	ASSERT(!IsBadWritePtr(pObject, sizeof(SoundShow)));
	SoundShow * pAlgorithm = static_cast<SoundShow *>(pObject);
	pAlgorithm->ShowWindow( SW_SHOW || SW_RESTORE );	//如果算法对象是继承自CDialog类,则需调用此行代码
}
