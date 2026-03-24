#include "pch.h"
#include "MFC.h"
#include "afxdialogex.h"
#include "CTabSalesDlg.h"

IMPLEMENT_DYNAMIC(CTabSalesDlg, CDialogEx)

CTabSalesDlg::CTabSalesDlg(CWnd* pParent)
    : CDialogEx(IDD_TAB_SALES, pParent) {
}

CTabSalesDlg::~CTabSalesDlg() {}

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

BOOL CTabSalesDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // 날짜 초기값: 이번달 1일 ~ 오늘
    CTime tmToday = CTime::GetCurrentTime();
    CTime tmStart(tmToday.GetYear(), tmToday.GetMonth(), 1, 0, 0, 0);
    m_dtStart.SetTime(&tmStart);
    m_dtEnd.SetTime(&tmToday);

    // 초기 표시
    m_staticTotalSales.SetWindowText(L"- 원");
    m_staticTotalOrder.SetWindowText(L"- 건");
    m_staticCommission.SetWindowText(L"- 원");
    m_staticNetSales.SetWindowText(L"- 원");

    // 리스트 컬럼 초기화
    m_listTopMenu.SetExtendedStyle(
        m_listTopMenu.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    m_listTopMenu.InsertColumn(0, L"순위", LVCFMT_CENTER, 50);
    m_listTopMenu.InsertColumn(1, L"메뉴명", LVCFMT_LEFT, 180);
    m_listTopMenu.InsertColumn(2, L"주문 수", LVCFMT_RIGHT, 80);
    m_listTopMenu.InsertColumn(3, L"매출", LVCFMT_RIGHT, 100);

    return TRUE;
}

void CTabSalesDlg::SetSalesInfo(int storeId, CNetworkHelper* pNet)
{
    m_storeId = storeId;
    m_pNet = pNet;
}

// =========================================================
// 서버 응답 수신 (RES_SALES_STAT = 3091)
// =========================================================
void CTabSalesDlg::OnSalesStatReceived(const json& resJson)
{
    int totalSales = resJson.value("totalSales", 0);
    UpdateSalesDisplay(totalSales);
}

// =========================================================
// 매출/수수료/정산액 계산 및 표시
// =========================================================
void CTabSalesDlg::UpdateSalesDisplay(int totalSales)
{
    int commission = (int)(totalSales * COMMISSION_RATE);
    int netAmount = totalSales - commission;

    CString str;
    str.Format(L"%d 원", totalSales);
    m_staticTotalSales.SetWindowText(str);

    // 주문 건수는 서버에서 안 주므로 숨김 처리
    m_staticTotalOrder.SetWindowText(L"- 건");

    str.Format(L"%d 원 (12.8%%)", commission);
    m_staticCommission.SetWindowText(str);

    str.Format(L"%d 원", netAmount);
    m_staticNetSales.SetWindowText(str);
}

// =========================================================
// 조회 버튼 → REQ_SALES_STAT 전송
// =========================================================
void CTabSalesDlg::OnBnClickedBtnSalesSearch()
{
    if (!m_pNet) return;

    // storeId만 담아서 전송 (total_sales 조회)
    json body;
    body["storeId"] = m_storeId;
    m_pNet->Send(CmdID::REQ_SALES_STAT, body);
}

BEGIN_MESSAGE_MAP(CTabSalesDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_SALES_SEARCH, &CTabSalesDlg::OnBnClickedBtnSalesSearch)
END_MESSAGE_MAP()