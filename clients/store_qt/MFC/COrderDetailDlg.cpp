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

    try  // ✅ 추가
    {
        auto toW = [](const std::string& s) -> CString {
            CString str = CA2W(s.c_str(), CP_UTF8);
            return str;
            };

        m_staticOrderId.SetWindowText(toW(m_orderJson.value("orderId", "")));
        m_staticOrderAddress.SetWindowText(toW(m_orderJson.value("deliveryAddress", "")));
        m_staticOrderTime.SetWindowText(toW(m_orderJson.value("createdAt", "")));

        CString strPrice;
        strPrice.Format(L"%d원", m_orderJson.value("totalPrice", 0));
        m_staticOrderPrice.SetWindowText(strPrice);

        int status = m_orderJson.value("orderStatus", 0);
        CString strStatus;
        switch (status)
        {
        case 0: strStatus = L"대기";   break;
        case 8: strStatus = L"수락";   break;
        case 1: strStatus = L"조리중"; break;
        case 2: strStatus = L"배달중"; break;
        case 3: strStatus = L"완료";   break;
        case 9: strStatus = L"거절";   break;
        default: strStatus = L"알 수 없음"; break;
        }
        m_staticOrderStatus.SetWindowText(strStatus);

        std::string storeReq = m_orderJson.value("storeRequest", "");
        std::string riderReq = m_orderJson.value("riderRequest", "");
        m_staticStoreRequest.SetWindowText(storeReq.empty() ? L"없음" : toW(storeReq));
        m_staticRiderRequest.SetWindowText(riderReq.empty() ? L"없음" : toW(riderReq));

        if (m_orderJson.contains("items") && m_orderJson["items"].is_array())
        {
            // ── 디버그: items 전체 JSON 출력 ──────────────────────
            OutputDebugStringW(L"[DEBUG] ===== items 배열 시작 =====\n");
            for (int i = 0; i < (int)m_orderJson["items"].size(); i++)
            {
                const auto& item = m_orderJson["items"][i];

                CString strDebug;
                strDebug.Format(L"[DEBUG] item[%d] keys:", i);
                OutputDebugStringW(strDebug);

                // 각 item의 키 목록 출력
                for (auto it = item.begin(); it != item.end(); ++it)
                {
                    CString strKey = CA2W(it.key().c_str(), CP_UTF8);
                    OutputDebugStringW(L" " + strKey);
                }
                OutputDebugStringW(L"\n");

                // selectedOptions 값 출력
                if (item.contains("options"))            
                {
                    std::string optStr = item["options"].dump();
                    CString strOpt = CA2W(optStr.c_str(), CP_UTF8);
                    OutputDebugStringW(L"[DEBUG] options = " + strOpt + L"\n");
                }
                else
                {
                    OutputDebugStringW(L"[DEBUG] options 키 없음!\n");
                }
            }
            OutputDebugStringW(L"[DEBUG] ===== items 배열 끝 =====\n");

            InitListCtrl();
            for (const auto& item : m_orderJson["items"])
            {
                int         menuId = item.value("menuId", 0);
                std::string menuName = item.value("menuName", "");
                int         quantity = item.value("quantity", 0);
                int         unitPrice = item.value("unitPrice", 0);

                CString strMenu;
                if (menuName.empty())
                    strMenu.Format(L"메뉴 #%d", menuId);
                else
                    strMenu = toW(menuName);

                CString strOptions = L"없음";
                try
                {
                    if (item.contains("options") &&       
                        !item["options"].is_null() &&
                        item["options"].is_array() &&
                        !item["options"].empty())          
                    {
                        strOptions = L"";
                        for (const auto& opt : item["options"])
                        {
                            // 객체가 아닌 문자열로 바로 파싱
                            std::string optName = opt.get<std::string>();

                            if (!strOptions.IsEmpty()) strOptions += L" / ";
                            strOptions += toW(optName);      
                        }
                        if (strOptions.IsEmpty()) strOptions = L"없음";
                    }
                }
                catch (...) { strOptions = L"없음"; }

                CString strQty, strUnitPrice, strTotal;
                strQty.Format(L"%d개", quantity);
                strUnitPrice.Format(L"%d원", unitPrice);
                strTotal.Format(L"%d원", quantity * unitPrice);

                int nIdx = m_listOrderItems.InsertItem(
                    m_listOrderItems.GetItemCount(), strMenu);
                m_listOrderItems.SetItemText(nIdx, 1, strOptions);
                m_listOrderItems.SetItemText(nIdx, 2, strQty);
                m_listOrderItems.SetItemText(nIdx, 3, strUnitPrice);
                m_listOrderItems.SetItemText(nIdx, 4, strTotal);
            }
        }
    }
    catch (const std::exception& e)
    {
        OutputDebugStringA(("[COrderDetailDlg] 오류: " + std::string(e.what()) + "\n").c_str());
    }
    catch (...) {}

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
    CString strOrderId, strAddress, strStatus, strTime, strPrice;
    CString strStoreReq, strRiderReq;
    m_staticOrderId.GetWindowText(strOrderId);
    m_staticOrderAddress.GetWindowText(strAddress);
    m_staticOrderStatus.GetWindowText(strStatus);
    m_staticOrderTime.GetWindowText(strTime);
    m_staticOrderPrice.GetWindowText(strPrice);
    m_staticStoreRequest.GetWindowText(strStoreReq);
    m_staticRiderRequest.GetWindowText(strRiderReq);

    //  OutputDebugStringW 사용 (한글 지원)
    OutputDebugStringW(L"========================================\n");
    OutputDebugStringW(L"          이츠 배달료 영수증            \n");
    OutputDebugStringW(L"========================================\n");

    OutputDebugStringW((L"주문번호 : " + CString(strOrderId) + L"\n"));
    OutputDebugStringW((L"주문시각 : " + CString(strTime) + L"\n"));
    OutputDebugStringW((L"배달주소 : " + CString(strAddress) + L"\n"));
    OutputDebugStringW((L"주문상태 : " + CString(strStatus) + L"\n"));
    OutputDebugStringW((L"사장님 요청 : " + CString(strStoreReq) + L"\n"));
    OutputDebugStringW(L"----------------------------------------\n");
    OutputDebugStringW(L"[주문 메뉴]\n");

    if (m_listOrderItems.GetItemCount() == 0)
    {
        OutputDebugStringW(L"  (메뉴 정보 없음 - 새로고침 후 다시 시도하세요)\n");
    }
    else
    {
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

            OutputDebugStringW(strLine);  // W 버전
        }
    }

    OutputDebugStringW(L"----------------------------------------\n");
    OutputDebugStringW((L"총 금액     : " + CString(strPrice) + L"\n"));
    OutputDebugStringW(L"----------------------------------------\n");
    OutputDebugStringW((L"라이더 요청 : " + CString(strRiderReq) + L"\n"));
    OutputDebugStringW(L"========================================\n");
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