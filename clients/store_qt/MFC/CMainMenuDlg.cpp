#include "pch.h"
#include "MFC.h"
#include "afxdialogex.h"
#include "CMainMenuDlg.h"

IMPLEMENT_DYNAMIC(CMainMenuDlg, CDialogEx)

// 매장 정보를 받는 생성자
CMainMenuDlg::CMainMenuDlg(int storeId, const CString& storeName, CWnd* pParent)
    : CDialogEx(IDD_MAIN_MENU, pParent)
    , m_storeId(storeId)
    , m_storeName(storeName)
{
}

CMainMenuDlg::~CMainMenuDlg()
{
}

void CMainMenuDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_NAMEBAR, m_staticNameBar);
    DDX_Control(pDX, IDC_STATIC_STATUS, m_staticStatus);
    DDX_Control(pDX, IDC_TAB_STATUS_SET, m_tabCtrl);
}

BOOL CMainMenuDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // 매장 정보 유무에 따라 분기
    if (m_storeId == 0 || m_storeName.IsEmpty())
    {
        m_staticNameBar.SetWindowText(L"매장 정보가 없습니다.");
        m_staticStatus.SetWindowText(L"-");
    }
    else
    {
        m_staticNameBar.SetWindowText(m_storeName);
        m_staticStatus.SetWindowText(L"영업중"); // 추후 서버 status 값으로 변경
    }

    // 탭 항목 추가
    m_tabCtrl.InsertItem(0, L"주문 관리");
    m_tabCtrl.InsertItem(1, L"메뉴 관리");
    m_tabCtrl.InsertItem(2, L"매장 설정");
    m_tabCtrl.InsertItem(3, L"리뷰 관리");
    m_tabCtrl.InsertItem(4, L"매출 통계");

    // Tab Control 내용 영역 계산 후 자식 다이얼로그 배치
    CRect rcTab;
    m_tabCtrl.GetClientRect(&rcTab);
    m_tabCtrl.AdjustRect(FALSE, &rcTab); // 탭 헤더 높이 제외한 내용 영역

    m_tabStoreDlg.Create(IDD_TAB_STORE, &m_tabCtrl);
    m_tabStoreDlg.MoveWindow(&rcTab);
    m_tabStoreDlg.ShowWindow(SW_HIDE); // 처음엔 숨김 (기본 탭이 0번이므로)

    return TRUE;
}

void CMainMenuDlg::OnTcnSelchangeTabStatusSet(NMHDR* pNMHDR, LRESULT* pResult)
{
    int nSel = m_tabCtrl.GetCurSel();

    // 현재는 매장 설정(2번 탭)만 있으므로
    // 추후 다른 탭 다이얼로그 추가 시 여기에 ShowWindow 추가
    m_tabStoreDlg.ShowWindow(nSel == 2 ? SW_SHOW : SW_HIDE);

    *pResult = 0;
}

BEGIN_MESSAGE_MAP(CMainMenuDlg, CDialogEx)
    ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_STATUS_SET, &CMainMenuDlg::OnTcnSelchangeTabStatusSet)
END_MESSAGE_MAP()