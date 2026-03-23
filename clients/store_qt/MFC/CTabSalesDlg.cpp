// CTabSalesDlg.cpp: 구현 파일

#include "pch.h"
#include "MFC.h"
#include "afxdialogex.h"
#include "CTabSalesDlg.h"

IMPLEMENT_DYNAMIC(CTabSalesDlg, CDialogEx)

CTabSalesDlg::CTabSalesDlg(CWnd* pParent)
    : CDialogEx(IDD_TAB_SALES, pParent)
{
}

CTabSalesDlg::~CTabSalesDlg()
{
}

// =========================================================================
// 컨트롤 바인딩
// =========================================================================
void CTabSalesDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_DATETIMEPICKER_START, m_dtStart);
    DDX_Control(pDX, IDC_DATETIMEPICKER_END, m_dtEnd);
    DDX_Control(pDX, IDC_BTN_SALES_SEARCH, m_btnSalesSearch);
    DDX_Control(pDX, IDC_STATIC_TOTAL_SALES, m_staticTotalSales);
    DDX_Control(pDX, IDC_STATIC_TOTAL_ORDER, m_staticTotalOrder);
    DDX_Control(pDX, IDC_STATIC_COMMISSION, m_staticCommission);
    DDX_Control(pDX, IDC_STATIC_NET_SALES, m_staticNetSales);
    DDX_Control(pDX, IDC_LIST_TOP_MENU, m_listTopMenu);
}

// =========================================================================
// 초기화
// =========================================================================
BOOL CTabSalesDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // 날짜 초기값: 오늘 기준 이번달 1일 ~ 오늘
    CTime tmToday = CTime::GetCurrentTime();
    CTime tmStart(tmToday.GetYear(), tmToday.GetMonth(), 1, 0, 0, 0);
    m_dtStart.SetTime(&tmStart);
    m_dtEnd.SetTime(&tmToday);

    // 초기 표시값
    m_staticTotalSales.SetWindowText(L"0 원");
    m_staticTotalOrder.SetWindowText(L"0 건");
    m_staticCommission.SetWindowText(L"0 원");
    m_staticNetSales.SetWindowText(L"0 원");

    InitListCtrl();

    return TRUE;
}

// =========================================================================
// 인기 메뉴 List Control 컬럼 초기화
// =========================================================================
void CTabSalesDlg::InitListCtrl()
{
    m_listTopMenu.SetExtendedStyle(
        m_listTopMenu.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES
    );

    m_listTopMenu.InsertColumn(0, L"순위", LVCFMT_CENTER, 50);
    m_listTopMenu.InsertColumn(1, L"메뉴명", LVCFMT_LEFT, 180);
    m_listTopMenu.InsertColumn(2, L"주문 수", LVCFMT_RIGHT, 80);
    m_listTopMenu.InsertColumn(3, L"매출", LVCFMT_RIGHT, 100);
}

// =========================================================================
// 매출/수수료/정산금액 계산 후 Static에 표시
// =========================================================================
void CTabSalesDlg::UpdateSalesInfo(int nTotalSales, int nTotalOrder)
{
    // 수수료 계산 (클라이언트 자체 계산 - 12.8%)
    int nCommission = (int)(nTotalSales * COMMISSION_RATE);
    int nNetSales = nTotalSales - nCommission;

    CString str;

    str.Format(L"%d 원", nTotalSales);
    m_staticTotalSales.SetWindowText(str);

    str.Format(L"%d 건", nTotalOrder);
    m_staticTotalOrder.SetWindowText(str);

    str.Format(L"%d 원 (12.8%%)", nCommission);
    m_staticCommission.SetWindowText(str);

    str.Format(L"%d 원", nNetSales);
    m_staticNetSales.SetWindowText(str);
}

// =========================================================================
// 조회 버튼
// =========================================================================
void CTabSalesDlg::OnBnClickedBtnSalesSearch()
{
    CTime tmStart, tmEnd;
    m_dtStart.GetTime(tmStart);
    m_dtEnd.GetTime(tmEnd);

    if (tmStart > tmEnd)
    {
        MessageBox(L"시작 날짜가 종료 날짜보다 클 수 없습니다.", L"알림", MB_OK);
        return;
    }

    // TODO: REQ_SALES_STAT 전송 (startDate, endDate, storeId)
    // 현재는 임시 더미 데이터로 표시
    m_listTopMenu.DeleteAllItems();

    int nIdx = m_listTopMenu.InsertItem(0, L"1위");
    m_listTopMenu.SetItemText(nIdx, 1, L"떡볶이 2인세트");
    m_listTopMenu.SetItemText(nIdx, 2, L"87건");
    m_listTopMenu.SetItemText(nIdx, 3, L"1,566,000원");

    nIdx = m_listTopMenu.InsertItem(1, L"2위");
    m_listTopMenu.SetItemText(nIdx, 1, L"김밥 2줄");
    m_listTopMenu.SetItemText(nIdx, 2, L"54건");
    m_listTopMenu.SetItemText(nIdx, 3, L"432,000원");

    nIdx = m_listTopMenu.InsertItem(2, L"3위");
    m_listTopMenu.SetItemText(nIdx, 1, L"순대 1개");
    m_listTopMenu.SetItemText(nIdx, 2, L"41건");
    m_listTopMenu.SetItemText(nIdx, 3, L"164,000원");

    // 임시 매출 데이터로 계산
    UpdateSalesInfo(2340000, 182);
}

BEGIN_MESSAGE_MAP(CTabSalesDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_SALES_SEARCH, &CTabSalesDlg::OnBnClickedBtnSalesSearch)
END_MESSAGE_MAP()