
// LOGFONT_to_FontFile.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CLOGFONT_to_FontFileApp:
// �йش����ʵ�֣������ LOGFONT_to_FontFile.cpp
//

class CLOGFONT_to_FontFileApp : public CWinApp
{
public:
	CLOGFONT_to_FontFileApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CLOGFONT_to_FontFileApp theApp;