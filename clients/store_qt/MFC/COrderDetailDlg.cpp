// COrderDetailDlg.cpp: 구현 파일

#include "pch.h"
#include "MFC.h"
#include "afxdialogex.h"
#include "COrderDetailDlg.h"
#include <string>

IMPLEMENT_DYNAMIC(COrderDetailDlg, CDialogEx)

COrderDetailDlg::COrderDetailDlg(CWnd* pParent)
    : CDialogEx(IDD_ORDER_DETAIL, pParent)
{
}

COrderDetailDlg::~COrderDetailDlg()
{
}

// =========================================================================
// 컨트롤 바인딩
// =========================================================================
void COrderDetailDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_ORDER_ID, m_staticOrderId);
    DDX_Control(pDX, IDC_STATIC_ORDER_ADDRESS, m_staticOrderAddress);
    DDX_Control(pDX, IDC_STATIC_ORDER_STATUS, m_staticOrderStatus);
    DDX_Control(pDX, IDC_STATIC_ORDER_TIME, m_staticOrderTime);
    DDX_Control(pDX, IDC_LIST_ORDER_ITEMS, m_listOrderItems);
    DDX_Control(pDX, IDC_STATIC_ORDER_PRICE, m_staticOrderPrice);
    DDX_Control(pDX, IDC_BTN_PRINT_RECEIPT, m_btnPrintReceipt);
    DDX_Control(pDX, IDC_BTN_DETAIL_CLOSE, m_btnDetailClose);
}

// =========================================================================
// 초기화
// =========================================================================
BOOL COrderDetailDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    InitListCtrl();

    // TODO: 선택된 주문 데이터를 받아서 채우기
    // 현재는 임시 더미 데이터
    m_staticOrderId.SetWindowText(L"ORD-20260320-001");
    m_staticOrderAddress.SetWindowText(L"서울시 광산구 ···");
    m_staticOrderStatus.SetWindowText(L"대기");
    m_staticOrderTime.SetWindowText(L"2026-03-20 14:32:10");
    m_staticOrderPrice.SetWindowText(L"18,000 원");

    int nIdx = m_listOrderItems.InsertItem(0, L"떡볶이 2인세트");
    m_listOrderItems.SetItemText(nIdx, 1, L"1");
    m_listOrderItems.SetItemText(nIdx, 2, L"18,000원");
    m_listOrderItems.SetItemText(nIdx, 3, L"맵기 보통");

    return TRUE;
}

// =========================================================================
// List Control 컬럼 초기화
// =========================================================================
void COrderDetailDlg::InitListCtrl()
{
    m_listOrderItems.SetExtendedStyle(
        m_listOrderItems.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES
    );

    m_listOrderItems.InsertColumn(0, L"메뉴명", LVCFMT_LEFT, 180);
    m_listOrderItems.InsertColumn(1, L"수량", LVCFMT_CENTER, 50);
    m_listOrderItems.InsertColumn(2, L"단가", LVCFMT_RIGHT, 90);
    m_listOrderItems.InsertColumn(3, L"옵션", LVCFMT_LEFT, 150);
}

// =========================================================================
// 영수증 재출력 - VS 출력창에 출력
// =========================================================================
void COrderDetailDlg::PrintReceipt()
{
    CString strOrderId, strAddress, strStatus, strTime, strPrice;
    m_staticOrderId.GetWindowText(strOrderId);
    m_staticOrderAddress.GetWindowText(strAddress);
    m_staticOrderStatus.GetWindowText(strStatus);
    m_staticOrderTime.GetWindowText(strTime);
    m_staticOrderPrice.GetWindowText(strPrice);

    // std::string으로 변환 후 연결
    std::string sOrderId = CT2A(strOrderId, CP_UTF8);
    std::string sAddress = CT2A(strAddress, CP_UTF8);
    std::string sStatus = CT2A(strStatus, CP_UTF8);
    std::string sTime = CT2A(strTime, CP_UTF8);
    std::string sPrice = CT2A(strPrice, CP_UTF8);

    OutputDebugStringA("========================================\n");
    OutputDebugStringA("           이츠 배달료 영수증            \n");
    OutputDebugStringA("========================================\n");
    OutputDebugStringA(("주문번호 : " + sOrderId + "\n").c_str());
    OutputDebugStringA(("주문시각 : " + sTime + "\n").c_str());
    OutputDebugStringA(("배달주소 : " + sAddress + "\n").c_str());
    OutputDebugStringA(("주문상태 : " + sStatus + "\n").c_str());
    OutputDebugStringA("----------------------------------------\n");
    OutputDebugStringA("[주문 메뉴]\n");

    for (int i = 0; i < m_listOrderItems.GetItemCount(); i++)
    {
        CString strMenu = m_listOrderItems.GetItemText(i, 0);
        CString strQty = m_listOrderItems.GetItemText(i, 1);
        CString strUnitP = m_listOrderItems.GetItemText(i, 2);
        CString strOpt = m_listOrderItems.GetItemText(i, 3);

        // ✅ CString Format에 LPCTSTR 캐스트
        CString strLine;
        strLine.Format(L"  %s x%s = %s  [옵션: %s]\n",
            (LPCTSTR)strMenu, (LPCTSTR)strQty,
            (LPCTSTR)strUnitP, (LPCTSTR)strOpt);
        OutputDebugStringA(CT2A(strLine, CP_UTF8));
    }

    OutputDebugStringA("----------------------------------------\n");
    OutputDebugStringA(("총 금액  : " + sPrice + "\n").c_str());
    OutputDebugStringA("========================================\n");
}

void COrderDetailDlg::OnBnClickedBtnPrintReceipt()
{
    PrintReceipt();
    MessageBox(L"영수증이 출력되었습니다.\n(VS 출력창에서 확인하세요)", L"영수증 재출력", MB_OK);
}

void COrderDetailDlg::OnBnClickedBtnDetailClose()
{
    EndDialog(IDCANCEL);
}

BEGIN_MESSAGE_MAP(COrderDetailDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_PRINT_RECEIPT, &COrderDetailDlg::OnBnClickedBtnPrintReceipt)
    ON_BN_CLICKED(IDC_BTN_DETAIL_CLOSE, &COrderDetailDlg::OnBnClickedBtnDetailClose)
END_MESSAGE_MAP()