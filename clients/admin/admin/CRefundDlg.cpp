#include "pch.h"
#include "admin.h"
#include "afxdialogex.h"
#include "CRefundDlg.h"

IMPLEMENT_DYNAMIC(CRefundDlg, CDialogEx)

CRefundDlg::CRefundDlg(CWnd* pParent)
    : CDialogEx(IDD_TAB_REFUND, pParent) {
}

CRefundDlg::~CRefundDlg() {}

void CRefundDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_SEARCH_TYPE, m_comboSearchType);
    DDX_Control(pDX, IDC_EDIT_ORDER_SEARCH, m_editOrderSearch);
    DDX_Control(pDX, IDC_BTN_ORDER_SEARCH, m_btnOrderSearch);
    DDX_Control(pDX, IDC_LIST_REFUND_ORDER, m_listRefundOrder);
    DDX_Control(pDX, IDC_BTN_CANCEL_ORDER, m_btnCancelOrder);
    DDX_Control(pDX, IDC_BTN_REFUND_ORDER, m_btnRefundOrder);
}

BOOL CRefundDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // 콤보박스 항목 추가
    m_comboSearchType.AddString(L"주문번호");
    m_comboSearchType.AddString(L"유저ID");
    m_comboSearchType.SetCurSel(0);

    InitListCtrl();
    UpdateButtonState();

    return TRUE;
}

void CRefundDlg::InitListCtrl()
{
    m_listRefundOrder.SetExtendedStyle(
        m_listRefundOrder.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    m_listRefundOrder.InsertColumn(0, L"주문번호", LVCFMT_LEFT, 140);
    m_listRefundOrder.InsertColumn(1, L"유저ID", LVCFMT_LEFT, 90);
    m_listRefundOrder.InsertColumn(2, L"금액", LVCFMT_RIGHT, 80);
    m_listRefundOrder.InsertColumn(3, L"상태", LVCFMT_CENTER, 70);
    m_listRefundOrder.InsertColumn(4, L"주문시각", LVCFMT_LEFT, 120);
}

void CRefundDlg::SetNetworkHelper(CNetworkHelper* pNet)
{
    m_pNet = pNet;
}

void CRefundDlg::UpdateButtonState()
{
    bool bSelected = (GetSelectedIndex() != -1);
    m_btnCancelOrder.EnableWindow(bSelected);
    m_btnRefundOrder.EnableWindow(bSelected);
}

int CRefundDlg::GetSelectedIndex()
{
    return m_listRefundOrder.GetNextItem(-1, LVNI_SELECTED);
}

// =========================================================
// 검색 버튼
// =========================================================
void CRefundDlg::OnBnClickedBtnOrderSearch()
{
    if (!m_pNet) return;

    CString strKeyword;
    m_editOrderSearch.GetWindowText(strKeyword);
    if (strKeyword.IsEmpty())
    {
        MessageBox(L"검색어를 입력해주세요.", L"알림", MB_OK);
        return;
    }

    int nSel = m_comboSearchType.GetCurSel();
    CString strType;
    m_comboSearchType.GetLBText(nSel, strType);

    json body;
    std::string strKeywordA = (const char*)CT2A(strKeyword, CP_UTF8);
    body["keyword"] = strKeywordA;
    body["searchType"] = (strType == L"주문번호") ? "orderId" : "userId";

    m_pNet->Send(CmdID::REQ_ADMIN_ORDER_LIST, body);
}

// =========================================================
// 서버 검색 결과 수신 (RES_ADMIN_ORDER_LIST = 5021)
// =========================================================
void CRefundDlg::OnSearchResult(const json& resJson)
{
    m_listRefundOrder.DeleteAllItems();

    if (resJson.value("status", 0) != 200)
    {
        MessageBox(L"검색 결과가 없습니다.", L"알림", MB_OK);
        return;
    }

    auto toW = [](const std::string& s) -> CString {
        CString str = CA2W(s.c_str(), CP_UTF8);
        return str;
        };

    const auto& orders = resJson["orders"];
    for (int i = 0; i < (int)orders.size(); i++)
    {
        const auto& order = orders[i];

        std::string orderId = order.value("orderId", "");
        std::string userId = order.value("userId", "");
        int         price = order.value("totalPrice", 0);
        int         status = order.value("orderStatus", 0);
        std::string created = order.value("createdAt", "");

        CString strStatus;
        switch (status)
        {
        case 0: strStatus = L"대기";   break;
        case 1: strStatus = L"조리중"; break;
        case 4: strStatus = L"완료";   break;
        case 9: strStatus = L"취소";   break;
        default: strStatus = L"-";     break;
        }

        CString strPrice;
        strPrice.Format(L"%d원", price);

        int nIdx = m_listRefundOrder.InsertItem(i, toW(orderId));
        m_listRefundOrder.SetItemText(nIdx, 1, toW(userId));
        m_listRefundOrder.SetItemText(nIdx, 2, strPrice);
        m_listRefundOrder.SetItemText(nIdx, 3, strStatus);
        m_listRefundOrder.SetItemText(nIdx, 4, toW(created));

        // orderId 저장
        m_listRefundOrder.SetItemData(nIdx, (DWORD_PTR)new std::string(orderId));
    }

    UpdateButtonState();
}

// =========================================================
// 주문 취소 버튼
// =========================================================
void CRefundDlg::OnBnClickedBtnCancelOrder()
{
    int nIdx = GetSelectedIndex();
    if (nIdx == -1) return;
    if (!m_pNet) return;

    auto* pOrderId = reinterpret_cast<std::string*>(
        m_listRefundOrder.GetItemData(nIdx));
    if (!pOrderId) return;

    if (MessageBox(L"해당 주문을 취소하시겠습니까?",
        L"주문 취소", MB_YESNO | MB_ICONQUESTION) == IDYES)
    {
        json body;
        body["orderId"] = *pOrderId;
        m_pNet->Send(CmdID::REQ_CANCEL, body);
    }
}

// =========================================================
// 환불 처리 버튼
// =========================================================
void CRefundDlg::OnBnClickedBtnRefundOrder()
{
    int nIdx = GetSelectedIndex();
    if (nIdx == -1) return;
    if (!m_pNet) return;

    auto* pOrderId = reinterpret_cast<std::string*>(
        m_listRefundOrder.GetItemData(nIdx));
    if (!pOrderId) return;

    if (MessageBox(L"해당 주문을 환불 처리하시겠습니까?",
        L"환불 처리", MB_YESNO | MB_ICONQUESTION) == IDYES)
    {
        json body;
        body["orderId"] = *pOrderId;
        m_pNet->Send(CmdID::REQ_REFUND, body);
    }
}

// =========================================================
// 리스트 선택 변경
// =========================================================
void CRefundDlg::OnLvnItemchangedListRefundOrder(NMHDR* pNMHDR, LRESULT* pResult)
{
    (void)pNMHDR;
    UpdateButtonState();
    *pResult = 0;
}

BEGIN_MESSAGE_MAP(CRefundDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_ORDER_SEARCH, &CRefundDlg::OnBnClickedBtnOrderSearch)
    ON_BN_CLICKED(IDC_BTN_CANCEL_ORDER, &CRefundDlg::OnBnClickedBtnCancelOrder)
    ON_BN_CLICKED(IDC_BTN_REFUND_ORDER, &CRefundDlg::OnBnClickedBtnRefundOrder)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_REFUND_ORDER,
        &CRefundDlg::OnLvnItemchangedListRefundOrder)
END_MESSAGE_MAP()