// COrderDetailDlg.cpp: 구현 파일

#include "pch.h"
#include "MFC.h"
#include "afxdialogex.h"
#include "COrderDetailDlg.h"
#include <string>

IMPLEMENT_DYNAMIC(COrderDetailDlg, CDialogEx)

COrderDetailDlg::COrderDetailDlg(const json& orderJson, CWnd* pParent)
    : CDialogEx(IDD_ORDER_DETAIL, pParent)
    , m_orderJson(orderJson)
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

    auto toW = [](const std::string& s) -> CString {
        return CA2W(s.c_str(), CP_UTF8);
        };

    // 주문 기본 정보 표시
    m_staticOrderId.SetWindowText(toW(m_orderJson.value("orderId", "")));
    m_staticOrderAddress.SetWindowText(toW(m_orderJson.value("deliveryAddress", "")));

    CString strPrice;
    strPrice.Format(L"%d원", m_orderJson.value("totalPrice", 0));
    m_staticOrderPrice.SetWindowText(strPrice);
    m_staticOrderTime.SetWindowText(toW(m_orderJson.value("createdAt", "")));
    m_staticOrderStatus.SetWindowText(L"대기");

    // items 배열로 메뉴 목록 표시
    if (m_orderJson.contains("items") && m_orderJson["items"].is_array())
    {
        InitListCtrl();
        for (const auto& item : m_orderJson["items"])
        {
            int menuId = item.value("menuId", 0);
            int quantity = item.value("quantity", 0);
            int unitPrice = item.value("unitPrice", 0);

            CString strMenuId, strQty, strPrice2, strTotal;
            strMenuId.Format(L"메뉴 #%d", menuId);
            strQty.Format(L"%d개", quantity);
            strPrice2.Format(L"%d원", unitPrice);
            strTotal.Format(L"%d원", quantity * unitPrice);

            int nIdx = m_listOrderItems.InsertItem(
                m_listOrderItems.GetItemCount(), strMenuId);
            m_listOrderItems.SetItemText(nIdx, 1, strQty);
            m_listOrderItems.SetItemText(nIdx, 2, strPrice2);
            m_listOrderItems.SetItemText(nIdx, 3, strTotal);
        }
    }

    return TRUE;
}
// =========================================================================
// List Control 컬럼 초기화
// =========================================================================
void COrderDetailDlg::InitListCtrl()
{
    m_listOrderItems.SetExtendedStyle(
        m_listOrderItems.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    m_listOrderItems.InsertColumn(0, L"메뉴", LVCFMT_LEFT, 120);
    m_listOrderItems.InsertColumn(1, L"수량", LVCFMT_CENTER, 50);
    m_listOrderItems.InsertColumn(2, L"단가", LVCFMT_RIGHT, 80);
    m_listOrderItems.InsertColumn(3, L"소계", LVCFMT_RIGHT, 80);
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

    // ✅ std::string으로 변환 후 연결
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