#include "pch.h"
#include "MFC.h"
#include "afxdialogex.h"
#include "CTabSettlementDlg.h"

IMPLEMENT_DYNAMIC(CTabSettlementDlg, CDialogEx)

CTabSettlementDlg::CTabSettlementDlg(CWnd* pParent)
    : CDialogEx(IDD_TAB_SETTLEMENT, pParent) {
}

CTabSettlementDlg::~CTabSettlementDlg() {}

void CTabSettlementDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_DATETIMEPICKER_SET_START, m_dtSetStart);
    DDX_Control(pDX, IDC_DATETIMEPICKER_SET_END, m_dtSetEnd);
    DDX_Control(pDX, IDC_BTN_SETTLEMENT_SEARCH, m_btnSettlementSearch);
    DDX_Control(pDX, IDC_STATIC_PENDING_AMOUNT, m_staticPendingAmount);
    DDX_Control(pDX, IDC_STATIC_COMPLETED_AMOUNT, m_staticCompletedAmount);
    DDX_Control(pDX, IDC_LIST_SETTLEMENT, m_listSettlement);
}

BOOL CTabSettlementDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    CTime tmToday = CTime::GetCurrentTime();
    CTime tmStart(tmToday.GetYear(), tmToday.GetMonth(), 1, 0, 0, 0);
    m_dtSetStart.SetTime(&tmStart);
    m_dtSetEnd.SetTime(&tmToday);

    m_staticPendingAmount.SetWindowText(L"- 원");
    m_staticCompletedAmount.SetWindowText(L"- 원");

    InitListCtrl();
    return TRUE;
}

void CTabSettlementDlg::InitListCtrl()
{
    m_listSettlement.SetExtendedStyle(
        m_listSettlement.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    m_listSettlement.InsertColumn(0, L"정산일", LVCFMT_LEFT, 100);
    m_listSettlement.InsertColumn(1, L"주문 건수", LVCFMT_RIGHT, 80);
    m_listSettlement.InsertColumn(2, L"총 매출", LVCFMT_RIGHT, 110);
    m_listSettlement.InsertColumn(3, L"수수료", LVCFMT_RIGHT, 110);
    m_listSettlement.InsertColumn(4, L"정산 금액", LVCFMT_RIGHT, 110);
    m_listSettlement.InsertColumn(5, L"상태", LVCFMT_CENTER, 70);
}

void CTabSettlementDlg::SetSettlementInfo(int storeId, CNetworkHelper* pNet)
{
    m_storeId = storeId;
    m_pNet = pNet;
}

// =========================================================
// total_sales 기반 하드코딩 정산 표시
// =========================================================
void CTabSettlementDlg::ShowDummyData(int totalSales)
{
    m_listSettlement.DeleteAllItems();

    // total_sales를 3등분해서 날짜별 더미 데이터 생성
    int sale1 = (int)(totalSales * 0.4);
    int sale2 = (int)(totalSales * 0.35);
    int sale3 = totalSales - sale1 - sale2;

    struct DummyData { LPCWSTR date; int orders; int sales; int status; };
    DummyData data[] = {
        { L"2026-03-24", 12, sale1, 0 }, // 예정
        { L"2026-03-23",  9, sale2, 1 }, // 완료
        { L"2026-03-22", 15, sale3, 1 }, // 완료
    };

    int nPending = 0, nCompleted = 0;

    for (int i = 0; i < 3; i++)
    {
        int nComm = (int)(data[i].sales * COMMISSION_RATE);
        int nNet = data[i].sales - nComm;

        CString strOrders, strSales, strComm, strNet;
        strOrders.Format(L"%d건", data[i].orders);
        strSales.Format(L"%d원", data[i].sales);
        strComm.Format(L"%d원", nComm);
        strNet.Format(L"%d원", nNet);

        int nIdx = m_listSettlement.InsertItem(i, data[i].date);
        m_listSettlement.SetItemText(nIdx, 1, strOrders);
        m_listSettlement.SetItemText(nIdx, 2, strSales);
        m_listSettlement.SetItemText(nIdx, 3, strComm);
        m_listSettlement.SetItemText(nIdx, 4, strNet);
        m_listSettlement.SetItemText(nIdx, 5, data[i].status == 0 ? L"예정" : L"완료");

        if (data[i].status == 0) nPending += nNet;
        else                     nCompleted += nNet;
    }

    CString str;
    str.Format(L"%d 원", nPending);
    m_staticPendingAmount.SetWindowText(str);
    str.Format(L"%d 원", nCompleted);
    m_staticCompletedAmount.SetWindowText(str);
}

// =========================================================
// 조회 버튼
// =========================================================
void CTabSettlementDlg::OnBnClickedBtnSettlementSearch()
{
    CTime tmStart, tmEnd;
    m_dtSetStart.GetTime(tmStart);
    m_dtSetEnd.GetTime(tmEnd);

    if (tmStart > tmEnd)
    {
        MessageBox(L"시작 날짜가 종료 날짜보다 클 수 없습니다.", L"알림", MB_OK);
        return;
    }

    // ✅ total_sales 조회 요청
    if (!m_pNet) return;
    json body;
    body["storeId"] = m_storeId;
    m_pNet->Send(CmdID::REQ_SALES_STAT, body);
}

BEGIN_MESSAGE_MAP(CTabSettlementDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_SETTLEMENT_SEARCH, &CTabSettlementDlg::OnBnClickedBtnSettlementSearch)
END_MESSAGE_MAP()