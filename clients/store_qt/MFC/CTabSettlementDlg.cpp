// CTabSettlementDlg.cpp: 구현 파일

#include "pch.h"
#include "MFC.h"
#include "afxdialogex.h"
#include "CTabSettlementDlg.h"

IMPLEMENT_DYNAMIC(CTabSettlementDlg, CDialogEx)

CTabSettlementDlg::CTabSettlementDlg(CWnd* pParent)
    : CDialogEx(IDD_TAB_SETTLEMENT, pParent)
{
}

CTabSettlementDlg::~CTabSettlementDlg()
{
}

// =========================================================================
// 컨트롤 바인딩
// =========================================================================
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

// =========================================================================
// 초기화
// =========================================================================
BOOL CTabSettlementDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // 날짜 초기값: 이번달 1일 ~ 오늘
    CTime tmToday = CTime::GetCurrentTime();
    CTime tmStart(tmToday.GetYear(), tmToday.GetMonth(), 1, 0, 0, 0);
    m_dtSetStart.SetTime(&tmStart);
    m_dtSetEnd.SetTime(&tmToday);

    // 초기 표시값
    m_staticPendingAmount.SetWindowText(L"0 원");
    m_staticCompletedAmount.SetWindowText(L"0 원");

    InitListCtrl();

    return TRUE;
}

// =========================================================================
// List Control 컬럼 초기화
// =========================================================================
void CTabSettlementDlg::InitListCtrl()
{
    m_listSettlement.SetExtendedStyle(
        m_listSettlement.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES
    );

    m_listSettlement.InsertColumn(0, L"정산일", LVCFMT_LEFT, 100);
    m_listSettlement.InsertColumn(1, L"주문 건수", LVCFMT_RIGHT, 80);
    m_listSettlement.InsertColumn(2, L"총 매출", LVCFMT_RIGHT, 110);
    m_listSettlement.InsertColumn(3, L"수수료", LVCFMT_RIGHT, 110);
    m_listSettlement.InsertColumn(4, L"정산 금액", LVCFMT_RIGHT, 110);
    m_listSettlement.InsertColumn(5, L"상태", LVCFMT_CENTER, 70);
}

// =========================================================================
// 조회 버튼
// =========================================================================
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

    // TODO: REQ_SETTLEMENT_INFO 전송 (startDate, endDate, storeId)
    // 현재는 임시 더미 데이터
    m_listSettlement.DeleteAllItems();

    // 임시 일별 데이터 추가
    struct DummyData { LPCWSTR date; int orders; int sales; };
    DummyData data[] = {
        { L"2026-03-20", 12, 187500 },
        { L"2026-03-19", 9,  142000 },
        { L"2026-03-18", 15, 231000 },
    };

    int nPending = 0, nCompleted = 0;

    for (int i = 0; i < 3; i++)
    {
        int nCommission = (int)(data[i].sales * COMMISSION_RATE);
        int nNet = data[i].sales - nCommission;

        CString strOrders, strSales, strComm, strNet;
        strOrders.Format(L"%d건", data[i].orders);
        strSales.Format(L"%d원", data[i].sales);
        strComm.Format(L"%d원", nCommission);
        strNet.Format(L"%d원", nNet);

        int nIdx = m_listSettlement.InsertItem(i, data[i].date);
        m_listSettlement.SetItemText(nIdx, 1, strOrders);
        m_listSettlement.SetItemText(nIdx, 2, strSales);
        m_listSettlement.SetItemText(nIdx, 3, strComm);
        m_listSettlement.SetItemText(nIdx, 4, strNet);
        m_listSettlement.SetItemText(nIdx, 5, i == 0 ? L"예정" : L"완료");

        if (i == 0) nPending += nNet;
        else        nCompleted += nNet;
    }

    CString str;
    str.Format(L"%d 원", nPending);
    m_staticPendingAmount.SetWindowText(str);
    str.Format(L"%d 원", nCompleted);
    m_staticCompletedAmount.SetWindowText(str);
}

BEGIN_MESSAGE_MAP(CTabSettlementDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_SETTLEMENT_SEARCH, &CTabSettlementDlg::OnBnClickedBtnSettlementSearch)
END_MESSAGE_MAP()