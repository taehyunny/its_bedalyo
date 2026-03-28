#include "pch.h"
#include "Rider.h"
#include "afxdialogex.h"
#include "CRiderMainDlg.h"
#include "Global_protocol.h"

IMPLEMENT_DYNAMIC(CRiderMainDlg, CDialogEx)

CRiderMainDlg::CRiderMainDlg(CWnd* pParent)
    : CDialogEx(IDD_RIDER_MAIN, pParent) {
}

CRiderMainDlg::~CRiderMainDlg() {}

void CRiderMainDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_BTN_CONNECT, m_btnConnect);
    DDX_Control(pDX, IDC_BTN_DISCONNECT, m_btnDisconnect);
    DDX_Control(pDX, IDC_BTN_REFRESH, m_btnRefresh);
    DDX_Control(pDX, IDC_LIST_ORDERS, m_listOrders);
    DDX_Control(pDX, IDC_BTN_DELIVER, m_btnDeliver);
    DDX_Control(pDX, IDC_BTN_PICKUP, m_btnPickup);
}

BOOL CRiderMainDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    SetWindowText(L"ITS_Bedalyo - 라이더");


    // 초기 버튼 상태
    m_btnDisconnect.EnableWindow(FALSE);
    m_btnRefresh.EnableWindow(FALSE);
    m_btnDeliver.EnableWindow(FALSE);
    m_btnPickup.EnableWindow(FALSE);

    InitListCtrl();
    return TRUE;
}

void CRiderMainDlg::InitListCtrl()
{
    m_listOrders.SetExtendedStyle(
        m_listOrders.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    m_listOrders.InsertColumn(0, L"주문번호", LVCFMT_LEFT, 140);
    m_listOrders.InsertColumn(1, L"메뉴요약", LVCFMT_LEFT, 160);
    m_listOrders.InsertColumn(2, L"배달주소", LVCFMT_LEFT, 180);
    m_listOrders.InsertColumn(3, L"금액", LVCFMT_RIGHT, 80);
    m_listOrders.InsertColumn(4, L"주문시각", LVCFMT_LEFT, 120);
}

void CRiderMainDlg::OnBnClickedBtnPickup()
{
    int nIdx = GetSelectedIndex();
    if (nIdx == -1) return;

    auto* pOrderId = reinterpret_cast<std::string*>(
        m_listOrders.GetItemData(nIdx));
    if (!pOrderId) return;

    if (MessageBox(L"해당 주문을 픽업하셨습니까?",
        L"픽업 완료", MB_YESNO | MB_ICONQUESTION) == IDYES)
    {
        json body;
        body["orderId"] = *pOrderId;
        m_net.Send(CmdID::REQ_PICKUP, body);
    }
}

void CRiderMainDlg::UpdateButtonState()
{
    bool bSelected = (GetSelectedIndex() != -1);
    m_btnDeliver.EnableWindow(bSelected);
    m_btnPickup.EnableWindow(bSelected);
}

int CRiderMainDlg::GetSelectedIndex()
{
    return m_listOrders.GetNextItem(-1, LVNI_SELECTED);
}

// =========================================================
// 연결 버튼
// =========================================================
void CRiderMainDlg::OnBnClickedBtnConnect()
{
    if (m_net.IsConnected()) return;

    if (!m_net.Connect(m_serverIp, m_serverPort, GetSafeHwnd()))
    {
        MessageBox(L"서버에 연결할 수 없습니다.", L"연결 실패", MB_ICONERROR);
        return;
    }

    m_net.SetNotifyHwnd(GetSafeHwnd());
    m_btnConnect.EnableWindow(FALSE);
    m_btnDisconnect.EnableWindow(TRUE);
    m_btnRefresh.EnableWindow(TRUE);

    // 연결 즉시 조리중 주문 목록 요청
    json body;
    m_net.Send(CmdID::REQ_RIDER_ORDER_LIST, body);
}

// =========================================================
// 해제 버튼
// =========================================================
void CRiderMainDlg::OnBnClickedBtnDisconnect()
{
    m_net.Disconnect();
    m_listOrders.DeleteAllItems();

    m_btnConnect.EnableWindow(TRUE);
    m_btnDisconnect.EnableWindow(FALSE);
    m_btnRefresh.EnableWindow(FALSE);
    m_btnDeliver.EnableWindow(FALSE);

    CString strAddr;
    CString strIp = CA2W(m_serverIp.c_str(), CP_UTF8);
    strAddr.Format(L"서버: %s : %d", (LPCTSTR)strIp, m_serverPort);
}

// =========================================================
// 새로고침 버튼
// =========================================================
void CRiderMainDlg::OnBnClickedBtnRefresh()
{
    if (!m_net.IsConnected()) return;
    m_listOrders.DeleteAllItems();

    json body;
    m_net.Send(CmdID::REQ_RIDER_ORDER_LIST, body);
}

// =========================================================
// 배달 완료 버튼
// =========================================================
void CRiderMainDlg::OnBnClickedBtnDeliver()
{
    int nIdx = GetSelectedIndex();
    if (nIdx == -1) return;

    auto* pOrderId = reinterpret_cast<std::string*>(
        m_listOrders.GetItemData(nIdx));
    if (!pOrderId) return;

    if (MessageBox(L"해당 주문을 배달 완료 처리하시겠습니까?",
        L"배달 완료", MB_YESNO | MB_ICONQUESTION) == IDYES)
    {
        json body;
        body["orderId"] = *pOrderId;
        m_net.Send(CmdID::REQ_DELIVERY_COMPLETE, body);
    }
}

// =========================================================
// 주문 목록 수신 (RES_RIDER_ORDER_LIST)
// =========================================================
void CRiderMainDlg::OnOrderListReceived(const json& resJson)
{
    m_listOrders.DeleteAllItems();

    if (resJson.value("status", 0) != 200)
    {
        MessageBox(L"주문 목록 조회에 실패했습니다.", L"오류", MB_ICONERROR);
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
        std::string menuSummary = order.value("menuSummary", "");
        std::string address = order.value("deliveryAddress", "");
        int         totalPrice = order.value("totalPrice", 0);
        std::string createdAt = order.value("createdAt", "");

        CString strPrice;
        strPrice.Format(L"%d원", totalPrice);

        int nIdx = m_listOrders.InsertItem(i, toW(orderId));
        m_listOrders.SetItemText(nIdx, 1, toW(menuSummary));
        m_listOrders.SetItemText(nIdx, 2, toW(address));
        m_listOrders.SetItemText(nIdx, 3, strPrice);
        m_listOrders.SetItemText(nIdx, 4, toW(createdAt));

        m_listOrders.SetItemData(nIdx, (DWORD_PTR)new std::string(orderId));
    }

    UpdateButtonState();
}

void CRiderMainDlg::OnDeliveryCallReceived(const json& resJson)
{
    auto toW = [](const std::string& s) -> CString {
        CString str = CA2W(s.c_str(), CP_UTF8);
        return str;
        };

    std::string orderId = resJson.value("orderId", "");
    std::string menuSummary = resJson.value("menuSummary", "");
    std::string deliveryAddress = resJson.value("deliveryAddress", "");
    int         totalPrice = resJson.value("totalPrice", 0);
    std::string createdAt = resJson.value("createdAt", "");

    CString strPrice;
    strPrice.Format(L"%d원", totalPrice);

    //  리스트 맨 위에 추가
    int nIdx = m_listOrders.InsertItem(0, toW(orderId));
    m_listOrders.SetItemText(nIdx, 1, toW(menuSummary));
    m_listOrders.SetItemText(nIdx, 2, toW(deliveryAddress));
    m_listOrders.SetItemText(nIdx, 3, strPrice);
    m_listOrders.SetItemText(nIdx, 4, toW(createdAt));

    // orderId 저장
    m_listOrders.SetItemData(nIdx, (DWORD_PTR)new std::string(orderId));

    //  알림음 + 창 활성화
    MessageBeep(MB_ICONEXCLAMATION);
    FlashWindow(TRUE);  // 작업 표시줄 깜빡임

    UpdateButtonState();
}
void CRiderMainDlg::OnPickupResult(const json& resJson)
{
    if (resJson.value("status", 0) == 200)
    {
        std::string orderId = resJson.value("orderId", "");
        // 리스트에서 상태 컬럼 "배달중"으로 변경
        for (int i = 0; i < m_listOrders.GetItemCount(); i++)
        {
            auto* pId = reinterpret_cast<std::string*>(
                m_listOrders.GetItemData(i));
            if (pId && *pId == orderId)
            {
                // 리스트에 상태 컬럼이 있다면 변경
                // 없다면 MessageBox만 표시
                break;
            }
        }
        MessageBox(L"픽업 완료 처리되었습니다.", L"완료", MB_OK);
    }
    else
        MessageBox(L"픽업 처리에 실패했습니다.", L"오류", MB_ICONERROR);
}

// =========================================================
// 배달 완료 응답 (RES_DELIVERY_COMPLETE)
// =========================================================
void CRiderMainDlg::OnDeliverResult(const json& resJson)
{
    if (resJson.value("status", 0) == 200)
    {
        std::string orderId = resJson.value("orderId", "");

        // 리스트에서 해당 항목 제거
        for (int i = 0; i < m_listOrders.GetItemCount(); i++)
        {
            auto* pId = reinterpret_cast<std::string*>(
                m_listOrders.GetItemData(i));
            if (pId && *pId == orderId)
            {
                delete pId;
                m_listOrders.DeleteItem(i);
                break;
            }
        }
        MessageBox(L"배달 완료 처리되었습니다.", L"완료", MB_OK);
    }
    else
        MessageBox(L"배달 완료 처리에 실패했습니다.", L"오류", MB_ICONERROR);

    UpdateButtonState();
}

// =========================================================
// 패킷 수신
// =========================================================
LRESULT CRiderMainDlg::OnPacketReceived(WPARAM wParam, LPARAM lParam)
{
    (void)wParam;
    auto* pkt = reinterpret_cast<ReceivedPacket*>(lParam);
    if (!pkt) return 0;

    json resJson = json::parse(pkt->body);

    if (pkt->cmdId == CmdID::RES_RIDER_ORDER_LIST)
        OnOrderListReceived(resJson);
    else if (pkt->cmdId == CmdID::RES_DELIVERY_COMPLETE)
        OnDeliverResult(resJson);
    else if (pkt->cmdId == CmdID::NOTIFY_DELIVERY_CALL) 
        OnDeliveryCallReceived(resJson);
    else if (pkt->cmdId == CmdID::RES_PICKUP)
        OnPickupResult(resJson);
    delete pkt;
    return 0;
}

// =========================================================
// 리스트 선택 변경
// =========================================================
void CRiderMainDlg::OnLvnItemchangedListOrders(NMHDR* pNMHDR, LRESULT* pResult)
{
    (void)pNMHDR;
    UpdateButtonState();
    *pResult = 0;
}

// =========================================================
// X 버튼
// =========================================================
void CRiderMainDlg::OnClose()
{
    if (MessageBox(L"종료하시겠습니까?", L"종료",
        MB_YESNO | MB_ICONQUESTION) == IDYES)
    {
        if (m_net.IsConnected()) m_net.Disconnect();
        CDialogEx::OnClose();
    }
}

BEGIN_MESSAGE_MAP(CRiderMainDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_CONNECT, &CRiderMainDlg::OnBnClickedBtnConnect)
    ON_BN_CLICKED(IDC_BTN_DISCONNECT, &CRiderMainDlg::OnBnClickedBtnDisconnect)
    ON_BN_CLICKED(IDC_BTN_REFRESH, &CRiderMainDlg::OnBnClickedBtnRefresh)
    ON_BN_CLICKED(IDC_BTN_DELIVER, &CRiderMainDlg::OnBnClickedBtnDeliver)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_ORDERS,
        &CRiderMainDlg::OnLvnItemchangedListOrders)
    ON_MESSAGE(WM_PACKET_RECEIVED, &CRiderMainDlg::OnPacketReceived)
    ON_BN_CLICKED(IDC_BTN_PICKUP, &CRiderMainDlg::OnBnClickedBtnPickup)
    ON_WM_CLOSE()
END_MESSAGE_MAP()