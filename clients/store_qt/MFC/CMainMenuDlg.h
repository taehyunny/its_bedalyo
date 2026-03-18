#pragma once
#include "afxdialogex.h"


// CMainMenuDlg 대화 상자

class CMainMenuDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMainMenuDlg)

public:
	CMainMenuDlg(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CMainMenuDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MAIN_MENU };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
};
