// CTabOrderDlg.cpp: 구현 파일

#include "pch.h"
#include "MFC.h"
#include "afxdialogex.h"
#include "CTabOrderDlg.h"

IMPLEMENT_DYNAMIC(CTabOrderDlg, CDialogEx)

CTabOrderDlg::CTabOrderDlg(CWnd* pParent)
    : CDialogEx(IDD_TAB_ORDER, pParent)
{
}

CTabOrderDlg::~CTabOrderDlg()
{
}

// =========================================================================
// 컨트롤 바인딩
// =========================================================================
void CTabOrderDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_ORDER, m_listOrder);
    DDX_Control(pDX, IDC_BTN_ORDER_ACCEPT, m_btnOrderAccept);
    DDX_Control(pDX, IDC_BTN_ORDER_REJECT, m_btnOrderReject);
    DDX_Control(pDX, IDC_COMBO_REJECT_REASON, m_comboRejectReason);
    DDX_Control(pDX, IDC_EDIT_REJECT_REASON, m_editRejectReason);
    DDX_Control(pDX, IDC_BTN_DELETE, m_btnOrderDelete);
    DDX_Control(pDX, IDC_BTN_REFRESH, m_btnOrderRefresh);
}
void CTabOrderDlg::OnBnClickedBtnOrderRefresh()
{
    if (!m_pNet) return;

    // 리스트 초기화
    m_listOrder.DeleteAllItems();
    m_orderDetails.clear();

    json body;
    body["storeId"] = m_storeId;
    m_pNet->Send(CmdID::REQ_ORDER_LIST, body);
}
void CTabOrderDlg::UpdateOrderStatus(
    const std::string& orderId, const CString& strStatus)
{
    for (int i = 0; i < m_listOrder.GetItemCount(); i++)
    {
        auto* pId = reinterpret_cast<std::string*>(
            m_listOrder.GetItemData(i));
        if (pId && *pId == orderId)
        {
            m_listOrder.SetItemText(i, 3, strStatus);
            break;
        }
    }
}

void CTabOrderDlg::OnBnClickedBtnOrderDelete()
{
    int nIdx = GetSelectedIndex();
    if (nIdx == -1)
    {
        MessageBox(L"삭제할 주문을 선택하세요.", L"알림", MB_OK);
        return;
    }

    CString strStatus = m_listOrder.GetItemText(nIdx, 3);
    if (strStatus == L"대기")
    {
        MessageBox(L"대기 중인 주문은 삭제할 수 없습니다.", L"알림", MB_OK);
        return;
    }

    // orderId 메모리 해제
    auto* pOrderId = reinterpret_cast<std::string*>(
        m_listOrder.GetItemData(nIdx));
    if (pOrderId) delete pOrderId;

    // 상세 데이터 제거
    if (nIdx < (int)m_orderDetails.size())
        m_orderDetails.erase(m_orderDetails.begin() + nIdx);

    m_listOrder.DeleteItem(nIdx);
    UpdateButtonState();
}

// 서버 응답으로 주문 목록 채우기
void CTabOrderDlg::SetOrderList(const json& orderArray)
{
    m_listOrder.DeleteAllItems();
    m_orderDetails.clear();

    auto toW = [](const std::string& s) -> CString {
        return CA2W(s.c_str(), CP_UTF8);
        };

    for (int i = 0; i < (int)orderArray.size(); i++)
    {
        const auto& order = orderArray[i];

        std::string orderId = order.value("orderId", "");
        std::string menuSummary = order.value("menuSummary", "");
        int         totalPrice = order.value("totalPrice", 0);
        std::string createdAt = order.value("createdAt", "");

        // 서버 키 이름 호환: "orderStatus" → "state" → "status" 순서로 시도
        int status = 0;
        if (order.contains("orderStatus")) status = order["orderStatus"].get<int>();
        else if (order.contains("state"))       status = order["state"].get<int>();
        else if (order.contains("status"))      status = order["status"].get<int>();

        // 상태 텍스트 변환
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

        CString strPrice;
        strPrice.Format(L"%d원", totalPrice);

        int nIdx = m_listOrder.InsertItem(i, toW(orderId));
        m_listOrder.SetItemText(nIdx, 1, toW(menuSummary));
        m_listOrder.SetItemText(nIdx, 2, strPrice);
        m_listOrder.SetItemText(nIdx, 3, strStatus);
        m_listOrder.SetItemText(nIdx, 4, toW(createdAt));

        m_listOrder.SetItemData(nIdx, (DWORD_PTR)new std::string(orderId));
        m_orderDetails.push_back(order);
    }

    UpdateButtonState();
}
// =========================================================================
// 초기화
// =========================================================================
BOOL CTabOrderDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // 거절 사유 콤보박스 항목 추가
    m_comboRejectReason.AddString(L"품절된 메뉴가 있습니다.");
    m_comboRejectReason.AddString(L"재료 소진으로 영업이 종료되었습니다.");
    m_comboRejectReason.AddString(L"배달 불가 지역입니다.");
    m_comboRejectReason.AddString(L"최소 주문 금액 미만입니다.");
    m_comboRejectReason.AddString(L"직접 입력");
    m_comboRejectReason.SetCurSel(0);

    // 직접 입력란 초기 비활성화
    m_editRejectReason.EnableWindow(FALSE);

    InitListCtrl();
    UpdateButtonState();

    return TRUE;
}

// =========================================================================
// List Control 컬럼 초기화
// =========================================================================
void CTabOrderDlg::InitListCtrl()
{
    m_listOrder.SetExtendedStyle(
        m_listOrder.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES
    );

    m_listOrder.InsertColumn(0, L"주문번호", LVCFMT_LEFT, 140);
    m_listOrder.InsertColumn(1, L"메뉴", LVCFMT_LEFT, 160);
    m_listOrder.InsertColumn(2, L"금액", LVCFMT_RIGHT, 80);
    m_listOrder.InsertColumn(3, L"상태", LVCFMT_CENTER, 80);
    m_listOrder.InsertColumn(4, L"주문 시각", LVCFMT_LEFT, 120);
}

void CTabOrderDlg::SetOrderInfo(int storeId, CNetworkHelper* pNet, int cookTime)
{
    m_storeId = storeId;
    m_pNet = pNet;
    m_cookTime = cookTime;
}

// =========================================================
// 새 주문 수신 (NOTIFY_NEW_ORDER = 9000)
// =========================================================
void CTabOrderDlg::AddNewOrder(const json& orderJson)
{
    auto toW = [](const std::string& s) -> CString {
        return CA2W(s.c_str(), CP_UTF8);
        };

    std::string orderId = orderJson.value("orderId", "");
    std::string menuSummary = orderJson.value("menuSummary", "");
    int         totalPrice = orderJson.value("totalPrice", 0);
    std::string createdAt = orderJson.value("createdAt", "");

    CString strPrice;
    strPrice.Format(L"%d원", totalPrice);

    // 리스트에 추가 (맨 위에 최신 주문)
    int nIdx = m_listOrder.InsertItem(0, toW(orderId));
    m_listOrder.SetItemText(nIdx, 1, toW(menuSummary));
    m_listOrder.SetItemText(nIdx, 2, strPrice);
    m_listOrder.SetItemText(nIdx, 3, L"대기");
    m_listOrder.SetItemText(nIdx, 4, toW(createdAt));

    // orderId 저장 (수락/거절 시 사용)
    m_listOrder.SetItemData(nIdx, (DWORD_PTR)new std::string(orderId));

    // 상세 데이터 저장 (더블클릭 시 팝업에 전달)
    m_orderDetails.insert(m_orderDetails.begin(), orderJson);

    // 새 주문 알림음
    MessageBeep(MB_ICONEXCLAMATION);
    UpdateButtonState();
}

// =========================================================================
// 버튼 활성화/비활성화
// =========================================================================
void CTabOrderDlg::UpdateButtonState()
{
    int nIdx = GetSelectedIndex();
    if (nIdx == -1)
    {
        m_btnOrderAccept.EnableWindow(FALSE);
        m_btnOrderReject.EnableWindow(FALSE);
        return;
    }

    // 대기 상태일 때만 수락/거절 활성화
    CString strStatus = m_listOrder.GetItemText(nIdx, 3);
    bool bPending = (strStatus == L"대기");
    m_btnOrderAccept.EnableWindow(bPending);
    m_btnOrderReject.EnableWindow(bPending);
}

int CTabOrderDlg::GetSelectedIndex()
{
    return m_listOrder.GetNextItem(-1, LVNI_SELECTED);
}

// =========================================================================
// List Control 선택 변경
// =========================================================================
void CTabOrderDlg::OnLvnItemchangedListOrder(NMHDR* pNMHDR, LRESULT* pResult)
{
    (void)pNMHDR;
    UpdateButtonState();
    *pResult = 0;
}

// =========================================================================
// 더블클릭 → 주문 상세 팝업
// =========================================================================
void CTabOrderDlg::OnNMDblclkListOrder(NMHDR* pNMHDR, LRESULT* pResult)
{
    (void)pNMHDR;
    int nIdx = GetSelectedIndex();
    if (nIdx == -1) { *pResult = 0; return; }

    //  저장된 상세 데이터를 팝업에 전달
    if (nIdx < (int)m_orderDetails.size())
    {
        COrderDetailDlg detailDlg(m_orderDetails[nIdx], this);
        detailDlg.DoModal();
    }

    *pResult = 0;
}

// =========================================================================
// 거절 사유 콤보박스 변경 → 직접 입력 선택 시 Edit 활성화
// =========================================================================
void CTabOrderDlg::OnCbnSelchangeComboRejectReason()
{
    int nSel = m_comboRejectReason.GetCurSel();
    CString strSel;
    m_comboRejectReason.GetLBText(nSel, strSel);

    // "직접 입력" 선택 시에만 Edit 활성화
    bool bDirect = (strSel == L"직접 입력");
    m_editRejectReason.EnableWindow(bDirect);
    if (bDirect)
    {
        m_editRejectReason.SetWindowText(L"");
        m_editRejectReason.SetFocus();
    }
}

// =========================================================================
// 수락 버튼
// =========================================================================
void CTabOrderDlg::OnBnClickedBtnOrderAccept()
{
    int nIdx = GetSelectedIndex();
    if (nIdx == -1) return;
    if (!m_pNet) return;

    // 이미 처리된 주문인지 확인
    CString strStatus = m_listOrder.GetItemText(nIdx, 3);
    if (strStatus != L"대기")
    {
        MessageBox(L"대기 중인 주문만 수락할 수 있습니다.", L"알림", MB_OK);
        return;
    }

    // orderId 가져오기
    auto* pOrderId = reinterpret_cast<std::string*>(m_listOrder.GetItemData(nIdx));
    if (!pOrderId) return;

    json body;
    body["orderId"] = *pOrderId;
    body["estimatedTime"] = m_cookTime; // 기본 30분 (추후 입력받는 UI 추가 가능)

    m_pNet->Send(CmdID::REQ_ORDER_ACCEPT, body);
}


// =========================================================================
// 거절 버튼
// =========================================================================
void CTabOrderDlg::OnBnClickedBtnOrderReject()
{
    int nIdx = GetSelectedIndex();
    if (nIdx == -1) return;
    if (!m_pNet) return;

    CString strStatus = m_listOrder.GetItemText(nIdx, 3);
    if (strStatus != L"대기")
    {
        MessageBox(L"대기 중인 주문만 거절할 수 있습니다.", L"알림", MB_OK);
        return;
    }

    // 거절 사유 수집
    CString strReason;
    int nSel = m_comboRejectReason.GetCurSel();
    m_comboRejectReason.GetLBText(nSel, strReason);

    if (strReason == L"직접 입력")
    {
        m_editRejectReason.GetWindowText(strReason);
        if (strReason.IsEmpty())
        {
            MessageBox(L"거절 사유를 입력해주세요.", L"알림", MB_OK);
            return;
        }
    }

    auto* pOrderId = reinterpret_cast<std::string*>(m_listOrder.GetItemData(nIdx));
    if (!pOrderId) return;

    json body;
    body["orderId"] = *pOrderId;
    body["rejectReason"] = (const char*)CT2A(strReason, CP_UTF8);

    m_pNet->Send(CmdID::REQ_ORDER_REJECT, body);
}

// =========================================================
// 수락 응답 처리 (RES_ORDER_ACCEPT = 3001)
// =========================================================
void CTabOrderDlg::OnOrderAcceptResult(const json& resJson)
{
    if (resJson.value("status", 0) == 200)
    {
        std::string orderId = resJson.value("orderId", "");
        if (!orderId.empty())
        {
            for (int i = 0; i < m_listOrder.GetItemCount(); i++)
            {
                auto* pId = reinterpret_cast<std::string*>(
                    m_listOrder.GetItemData(i));
                if (pId && *pId == orderId)
                {
                    m_listOrder.SetItemText(i, 3, L"조리중");
                    break;
                }
            }
        }
        else
        {
            // orderId 없을 때 fallback
            int nIdx = GetSelectedIndex();
            if (nIdx != -1)
                m_listOrder.SetItemText(nIdx, 3, L"조리중");
        }
        MessageBox(L"주문을 수락했습니다.", L"완료", MB_OK);
    }
    else
        MessageBox(L"주문 수락에 실패했습니다.", L"오류", MB_ICONERROR);
}
// =========================================================
// 거절 응답 처리 (RES_ORDER_REJECT = 3011)
// =========================================================
void CTabOrderDlg::OnOrderRejectResult(const json& resJson)
{
    if (resJson.value("status", 0) == 200)
    {
        std::string orderId = resJson.value("orderId", "");
        if (!orderId.empty())
        {
            for (int i = 0; i < m_listOrder.GetItemCount(); i++)
            {
                auto* pId = reinterpret_cast<std::string*>(
                    m_listOrder.GetItemData(i));
                if (pId && *pId == orderId)
                {
                    m_listOrder.SetItemText(i, 3, L"거절"); // orderId로 찾아서 변경
                    break;
                }
            }
        }
        else
        {
            // orderId 없을 때 fallback
            int nIdx = GetSelectedIndex();
            if (nIdx != -1)
                m_listOrder.SetItemText(nIdx, 3, L"거절");
        }
        MessageBox(L"주문을 거절했습니다.", L"완료", MB_OK);
    }
    else
        MessageBox(L"주문 거절에 실패했습니다.", L"오류", MB_ICONERROR);
}

BEGIN_MESSAGE_MAP(CTabOrderDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_ORDER_ACCEPT, &CTabOrderDlg::OnBnClickedBtnOrderAccept)
    ON_BN_CLICKED(IDC_BTN_ORDER_REJECT, &CTabOrderDlg::OnBnClickedBtnOrderReject)
    ON_CBN_SELCHANGE(IDC_COMBO_REJECT_REASON, &CTabOrderDlg::OnCbnSelchangeComboRejectReason)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_ORDER, &CTabOrderDlg::OnLvnItemchangedListOrder)
    ON_NOTIFY(NM_DBLCLK, IDC_LIST_ORDER, &CTabOrderDlg::OnNMDblclkListOrder)
    ON_BN_CLICKED(IDC_BTN_DELETE, &CTabOrderDlg::OnBnClickedBtnOrderDelete)
    ON_BN_CLICKED(IDC_BTN_REFRESH, &CTabOrderDlg::OnBnClickedBtnOrderRefresh)
END_MESSAGE_MAP()