// CMainMenuDlg.cpp: 구현 파일
//

#include "pch.h"
#include "MFC.h"
#include "afxdialogex.h"
#include "CMainMenuDlg.h"


// CMainMenuDlg 대화 상자

IMPLEMENT_DYNAMIC(CMainMenuDlg, CDialogEx)

CMainMenuDlg::CMainMenuDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MAIN_MENU, pParent)
{

}

CMainMenuDlg::~CMainMenuDlg()
{
}

void CMainMenuDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMainMenuDlg, CDialogEx)
END_MESSAGE_MAP()


// CMainMenuDlg 메시지 처리기
