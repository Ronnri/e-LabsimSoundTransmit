// Sound.h : Sound DLL ����ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CSoundApp
// �йش���ʵ�ֵ���Ϣ������� Sound.cpp
//

class CSoundApp : public CWinApp
{
public:
	CSoundApp();

// ��д
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
