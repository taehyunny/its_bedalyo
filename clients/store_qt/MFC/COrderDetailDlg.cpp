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
    DDX_Control(pDX, IDC_STORE_REQUEST, m_staticStoreRequest);
    DDX_Control(pDX, IDC_RIDER_REQUEST, m_staticRiderRequest);
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
    m_staticOrderTime.SetWindowText(toW(m_orderJson.value("createdAt", "")));

    CString strPrice;
    strPrice.Format(L"%d원", m_orderJson.value("totalPrice", 0));
    m_staticOrderPrice.SetWindowText(strPrice);

    // 주문 상태 텍스트 변환
    int status = m_orderJson.value("orderStatus", 0);
    CString strStatus;
    switch (status)
    {
    case 0: strStatus = L"대기";   break;
    case 1: strStatus = L"수락";   break;
    case 2: strStatus = L"조리중"; break;
    case 3: strStatus = L"배달중"; break;
    case 4: strStatus = L"완료";   break;
    case 9: strStatus = L"거절";   break;
    default: strStatus = L"알 수 없음"; break;
    }
    m_staticOrderStatus.SetWindowText(strStatus);

    // ── 요청사항 표시 ──────────────────────────────────────
    std::string storeReq = m_orderJson.value("storeRequest", "");
    std::string riderReq = m_orderJson.value("riderRequest", "");

    m_staticStoreRequest.SetWindowText(
        storeReq.empty() ? L"없음" : toW(storeReq));
    m_staticRiderRequest.SetWindowText(
        riderReq.empty() ? L"없음" : toW(riderReq));

    // ── 주문 메뉴 목록 ─────────────────────────────────────
    if (m_orderJson.contains("items") && m_orderJson["items"].is_array())
    {
        InitListCtrl();
        for (const auto& item : m_orderJson["items"])
        {
            int         menuId = item.value("menuId", 0);
            std::string menuName = item.value("menuName", "");
            int         quantity = item.value("quantity", 0);
            int         unitPrice = item.value("unitPrice", 0);

            // 메뉴명 처리
            CString strMenu;
            if (menuName.empty())
                strMenu.Format(L"메뉴 #%d", menuId);
            else
                strMenu = CA2W(menuName.c_str(), CP_UTF8);

            // 옵션 문자열 조합 + 옵션 추가금액 합산
            CString strOptions;
            int optionTotalPrice = 0;

            if (item.contains("options") && item["options"].is_array())
            {
                for (const auto& opt : item["options"])
                {
                    std::string optName = opt.value("optionName", "");
                    std::string optValue = opt.value("optionValue", "");
                    int         optPrice = opt.value("optionPrice", 0);

                    if (!strOptions.IsEmpty()) strOptions += L" / ";
                    CString strOpt;
                    strOpt.Format(L"%s:%s(+%d원)",
                        (LPCTSTR)CA2W(optName.c_str(), CP_UTF8),
                        (LPCTSTR)CA2W(optValue.c_str(), CP_UTF8),
                        optPrice);
                    strOptions += strOpt;
                    optionTotalPrice += optPrice;
                }
            }
            if (strOptions.IsEmpty()) strOptions = L"없음";

            // 단가 = 메뉴 기본가 + 옵션 추가금액
            int realUnitPrice = unitPrice + optionTotalPrice;

            CString strQty, strUnitPrice, strTotal;
            strQty.Format(L"%d개", quantity);
            strUnitPrice.Format(L"%d원", realUnitPrice);
            strTotal.Format(L"%d원", quantity * realUnitPrice);

            int nIdx = m_listOrderItems.InsertItem(
                m_listOrderItems.GetItemCount(), strMenu);
            m_listOrderItems.SetItemText(nIdx, 1, strOptions);
            m_listOrderItems.SetItemText(nIdx, 2, strQty);
            m_listOrderItems.SetItemText(nIdx, 3, strUnitPrice);
            m_listOrderItems.SetItemText(nIdx, 4, strTotal);
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
    m_listOrderItems.InsertColumn(1, L"옵션", LVCFMT_LEFT, 160); 
    m_listOrderItems.InsertColumn(2, L"수량", LVCFMT_CENTER, 50);
    m_listOrderItems.InsertColumn(3, L"단가", LVCFMT_RIGHT, 80);
    m_listOrderItems.InsertColumn(4, L"소계", LVCFMT_RIGHT, 80);
}

// =========================================================================
// 영수증 출력 (VS 출력 콘솔)
// =========================================================================
void COrderDetailDlg::PrintReceipt()
{
    // Static에서 텍스트 수집
    CString strOrderId, strAddress, strStatus, strTime, strPrice;
    CString strStoreReq, strRiderReq;
    m_staticOrderId.GetWindowText(strOrderId);
    m_staticOrderAddress.GetWindowText(strAddress);
    m_staticOrderStatus.GetWindowText(strStatus);
    m_staticOrderTime.GetWindowText(strTime);
    m_staticOrderPrice.GetWindowText(strPrice);
    m_staticStoreRequest.GetWindowText(strStoreReq);
    m_staticRiderRequest.GetWindowText(strRiderReq);

    // CString → std::string 변환
    std::string sOrderId = CT2A(strOrderId, CP_UTF8);
    std::string sAddress = CT2A(strAddress, CP_UTF8);
    std::string sStatus = CT2A(strStatus, CP_UTF8);
    std::string sTime = CT2A(strTime, CP_UTF8);
    std::string sPrice = CT2A(strPrice, CP_UTF8);
    std::string sStoreReq = CT2A(strStoreReq, CP_UTF8);
    std::string sRiderReq = CT2A(strRiderReq, CP_UTF8);

    OutputDebugStringA("========================================\n");
    OutputDebugStringA("          이츠 배달료 영수증            \n");
    OutputDebugStringA("========================================\n");
    OutputDebugStringA(("주문번호 : " + sOrderId + "\n").c_str());
    OutputDebugStringA(("주문시각 : " + sTime + "\n").c_str());
    OutputDebugStringA(("배달주소 : " + sAddress + "\n").c_str());
    OutputDebugStringA(("주문상태 : " + sStatus + "\n").c_str());
    OutputDebugStringA(("사장님 요청  : " + sStoreReq + "\n").c_str());
    OutputDebugStringA("----------------------------------------\n");
    OutputDebugStringA("[주문 메뉴]\n");

    for (int i = 0; i < m_listOrderItems.GetItemCount(); i++)
    {
        CString strMenu = m_listOrderItems.GetItemText(i, 0);
        CString strOptions = m_listOrderItems.GetItemText(i, 1);
        CString strQty = m_listOrderItems.GetItemText(i, 2);
        CString strUnit = m_listOrderItems.GetItemText(i, 3);
        CString strTotal = m_listOrderItems.GetItemText(i, 4);

        CString strLine;
        strLine.Format(L"  %s %s  단가: %s  소계: %s\n  옵션: %s\n",
            (LPCTSTR)strMenu, (LPCTSTR)strQty,
            (LPCTSTR)strUnit, (LPCTSTR)strTotal,
            (LPCTSTR)strOptions);
        OutputDebugStringA(CT2A(strLine, CP_UTF8));
    }

    OutputDebugStringA("----------------------------------------\n");
    OutputDebugStringA(("총 금액      : " + sPrice + "\n").c_str());
    OutputDebugStringA("----------------------------------------\n");
    OutputDebugStringA(("라이더 요청  : " + sRiderReq + "\n").c_str());  
    OutputDebugStringA("========================================\n");
}

void COrderDetailDlg::OnBnClickedBtnPrintReceipt()
{
    PrintReceipt();
    MessageBox(L"영수증이 출력되었습니다.\n(VS 출력창에서 확인하세요)",
        L"영수증 재출력", MB_OK);
}

void COrderDetailDlg::OnBnClickedBtnDetailClose()
{
    EndDialog(IDCANCEL);
}

BEGIN_MESSAGE_MAP(COrderDetailDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_PRINT_RECEIPT, &COrderDetailDlg::OnBnClickedBtnPrintReceipt)
    ON_BN_CLICKED(IDC_BTN_DETAIL_CLOSE, &COrderDetailDlg::OnBnClickedBtnDetailClose)
END_MESSAGE_MAP()