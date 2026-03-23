#include "pch.h"
#include "MFC.h"
#include "afxdialogex.h"
#include "CMainMenuDlg.h"

IMPLEMENT_DYNAMIC(CMainMenuDlg, CDialogEx)

CMainMenuDlg::CMainMenuDlg(int storeId, const CString& storeName,
    const CString& category, const CString& storeAddress,
    const CString& bizNum, const CString& cookTime,
    const CString& minOrder, const CString& openTime,
    const CString& closeTime, const CString& ownerName,
    const CString& ownerPhone, const CString& accountNumber,
    const CString& approvalStatus, CWnd* pParent)
    : CDialogEx(IDD_MAIN_MENU, pParent)
    , m_storeId(storeId), m_storeName(storeName)
    , m_category(category), m_storeAddress(storeAddress)
    , m_bizNum(bizNum), m_cookTime(cookTime)
    , m_minOrder(minOrder), m_openTime(openTime)
    , m_closeTime(closeTime), m_ownerName(ownerName)
    , m_ownerPhone(ownerPhone), m_accountNumber(accountNumber)
    , m_approvalStatus(approvalStatus)
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

    // 상단 매장명 / 영업상태 표시
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
    m_tabCtrl.InsertItem(5, L"정산 관리");

    // Tab Control 내용 영역 계산
    CRect rcTab;
    m_tabCtrl.GetClientRect(&rcTab);
    m_tabCtrl.AdjustRect(FALSE, &rcTab);

    // ── 탭 다이얼로그 생성 및 배치 ───────────────────────────
    m_tabOrderDlg.Create(IDD_TAB_ORDER, &m_tabCtrl);
    m_tabOrderDlg.MoveWindow(&rcTab);
    m_tabOrderDlg.ShowWindow(SW_SHOW); // 0번 탭 기본 표시

    m_tabMenuDlg.Create(IDD_TAB_MENU, &m_tabCtrl);
    m_tabMenuDlg.MoveWindow(&rcTab);
    m_tabMenuDlg.ShowWindow(SW_HIDE);

    m_tabStoreDlg.Create(IDD_TAB_STORE, &m_tabCtrl);
    m_tabStoreDlg.MoveWindow(&rcTab);
    m_tabStoreDlg.ShowWindow(SW_HIDE);

    // ✅ 서버에서 받은 매장/사장님 정보 전달
    m_tabStoreDlg.SetStoreInfo(
        m_storeName, m_category, m_storeAddress, m_bizNum,
        m_cookTime, m_minOrder, m_openTime, m_closeTime,
        m_ownerName, m_ownerPhone, m_accountNumber, m_approvalStatus
    );

    m_tabReviewDlg.Create(IDD_TAB_REVIEW, &m_tabCtrl);
    m_tabReviewDlg.MoveWindow(&rcTab);
    m_tabReviewDlg.ShowWindow(SW_HIDE);

    m_tabSalesDlg.Create(IDD_TAB_SALES, &m_tabCtrl);
    m_tabSalesDlg.MoveWindow(&rcTab);
    m_tabSalesDlg.ShowWindow(SW_HIDE);

    m_tabSettlementDlg.Create(IDD_TAB_SETTLEMENT, &m_tabCtrl);
    m_tabSettlementDlg.MoveWindow(&rcTab);
    m_tabSettlementDlg.ShowWindow(SW_HIDE);

    return TRUE;
}

void CMainMenuDlg::OnTcnSelchangeTabStatusSet(NMHDR* pNMHDR, LRESULT* pResult)
{
    (void)pNMHDR;
    int nSel = m_tabCtrl.GetCurSel();

    // 모든 탭 숨김 후 선택된 탭만 표시
    m_tabOrderDlg.ShowWindow(SW_HIDE);
    m_tabMenuDlg.ShowWindow(SW_HIDE);
    m_tabStoreDlg.ShowWindow(SW_HIDE);
    m_tabReviewDlg.ShowWindow(SW_HIDE);
    m_tabSalesDlg.ShowWindow(SW_HIDE);
    m_tabSettlementDlg.ShowWindow(SW_HIDE);

    switch (nSel)
    {
    case 0: m_tabOrderDlg.ShowWindow(SW_SHOW);      break;
    case 1: m_tabMenuDlg.ShowWindow(SW_SHOW);       break;
    case 2: m_tabStoreDlg.ShowWindow(SW_SHOW);      break;
    case 3: m_tabReviewDlg.ShowWindow(SW_SHOW);     break;
    case 4: m_tabSalesDlg.ShowWindow(SW_SHOW);      break;
    case 5: m_tabSettlementDlg.ShowWindow(SW_SHOW); break;
    }

    *pResult = 0;
}

BEGIN_MESSAGE_MAP(CMainMenuDlg, CDialogEx)
    ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_STATUS_SET, &CMainMenuDlg::OnTcnSelchangeTabStatusSet)
END_MESSAGE_MAP()