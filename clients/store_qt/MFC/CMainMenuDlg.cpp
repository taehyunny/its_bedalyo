#include "pch.h"
#include "MFC.h"
#include "afxdialogex.h"
#include "CMainMenuDlg.h"
#include "NetworkHelper.h"
#include "CTabSettlementDlg.h"

IMPLEMENT_DYNAMIC(CMainMenuDlg, CDialogEx)

CMainMenuDlg::CMainMenuDlg(int storeId, CNetworkHelper* pNet,
    const CString& storeName,
    const CString& category, const CString& storeAddress,
    const CString& bizNum, const CString& cookTime,
    const CString& minOrder, const CString& openTime,
    const CString& closeTime, const CString& ownerName,
    const CString& ownerPhone, const CString& accountNumber,
    const CString& approvalStatus,
    int deliveryFee,
    CWnd* pParent)
    : CDialogEx(IDD_MAIN_MENU, pParent)
    , m_storeId(storeId), m_pNet(pNet), m_storeName(storeName)
    , m_category(category), m_storeAddress(storeAddress)
    , m_bizNum(bizNum), m_cookTime(cookTime)
    , m_minOrder(minOrder), m_openTime(openTime)
    , m_closeTime(closeTime), m_ownerName(ownerName)
    , m_ownerPhone(ownerPhone), m_accountNumber(accountNumber)
    , m_approvalStatus(approvalStatus)
    , m_deliveryFee(deliveryFee)
{
}

CMainMenuDlg::~CMainMenuDlg()
{
    if (m_pChatRoomDlg)
    {
        m_pChatRoomDlg->DestroyWindow();
        delete m_pChatRoomDlg;
        m_pChatRoomDlg = nullptr;
    }
}

void CMainMenuDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_NAMEBAR, m_staticNameBar);
    DDX_Control(pDX, IDC_STATIC_STATUS, m_staticStatus);
    DDX_Control(pDX, IDC_TAB_STATUS_SET, m_tabCtrl);
    DDX_Control(pDX, IDC_BTN_CHAT_REQUEST, m_btnChatRequest);
}

BOOL CMainMenuDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    m_pNet->SetNotifyHwnd(GetSafeHwnd());

    if (m_storeId == 0 || m_storeName.IsEmpty())
    {
        m_staticNameBar.SetWindowText(L"매장 정보가 없습니다.");
        m_staticStatus.SetWindowText(L"-");
    }
    else
    {
        m_staticNameBar.SetWindowText(m_storeName);
        m_staticStatus.SetWindowText(L"영업중");
    }

    m_tabCtrl.InsertItem(0, L"주문 관리");
    m_tabCtrl.InsertItem(1, L"메뉴 관리");
    m_tabCtrl.InsertItem(2, L"매장 설정");
    m_tabCtrl.InsertItem(3, L"리뷰 관리");
    m_tabCtrl.InsertItem(4, L"매출 통계");
    m_tabCtrl.InsertItem(5, L"정산 관리");

    CRect rcTab;
    m_tabCtrl.GetClientRect(&rcTab);
    m_tabCtrl.AdjustRect(FALSE, &rcTab);

    m_tabOrderDlg.Create(IDD_TAB_ORDER, &m_tabCtrl);
    m_tabOrderDlg.MoveWindow(&rcTab);
    m_tabOrderDlg.ShowWindow(SW_SHOW);

    m_tabMenuDlg.Create(IDD_TAB_MENU, &m_tabCtrl);
    m_tabMenuDlg.MoveWindow(&rcTab);
    m_tabMenuDlg.ShowWindow(SW_HIDE);

    m_tabStoreDlg.Create(IDD_TAB_STORE, &m_tabCtrl);
    m_tabStoreDlg.MoveWindow(&rcTab);
    m_tabStoreDlg.ShowWindow(SW_HIDE);

    CString strDeliveryFee;
    strDeliveryFee.Format(L"%d", m_deliveryFee);

    m_tabStoreDlg.SetStoreInfo(
        m_storeId, m_pNet,
        m_storeName, m_category, m_storeAddress, m_bizNum,
        m_cookTime, m_minOrder, m_openTime, m_closeTime,
        m_ownerName, m_ownerPhone, m_accountNumber,
        m_approvalStatus, strDeliveryFee
    );

    m_tabMenuDlg.SetMenuInfo(m_storeId, m_pNet);

    m_tabReviewDlg.Create(IDD_TAB_REVIEW, &m_tabCtrl);
    m_tabReviewDlg.MoveWindow(&rcTab);
    m_tabReviewDlg.ShowWindow(SW_HIDE);
    m_tabReviewDlg.SetReviewInfo(m_storeId, m_pNet);

    m_tabSalesDlg.Create(IDD_TAB_SALES, &m_tabCtrl);
    m_tabSalesDlg.MoveWindow(&rcTab);
    m_tabSalesDlg.ShowWindow(SW_HIDE);
    m_tabSalesDlg.SetSalesInfo(m_storeId, m_pNet);

    m_tabSettlementDlg.Create(IDD_TAB_SETTLEMENT, &m_tabCtrl);
    m_tabSettlementDlg.MoveWindow(&rcTab);
    m_tabSettlementDlg.ShowWindow(SW_HIDE);
    m_tabSettlementDlg.SetSettlementInfo(m_storeId, m_pNet);

    m_tabOrderDlg.SetOrderInfo(m_storeId, m_pNet, _ttoi(m_cookTime));

    return TRUE;
}

void CMainMenuDlg::OnTcnSelchangeTabStatusSet(NMHDR* pNMHDR, LRESULT* pResult)
{
    (void)pNMHDR;
    int nSel = m_tabCtrl.GetCurSel();

    m_tabOrderDlg.ShowWindow(SW_HIDE);
    m_tabMenuDlg.ShowWindow(SW_HIDE);
    m_tabStoreDlg.ShowWindow(SW_HIDE);
    m_tabReviewDlg.ShowWindow(SW_HIDE);
    m_tabSalesDlg.ShowWindow(SW_HIDE);
    m_tabSettlementDlg.ShowWindow(SW_HIDE);

    switch (nSel)
    {
    case 0: m_tabOrderDlg.ShowWindow(SW_SHOW);      break;
    case 1: m_tabMenuDlg.ShowWindow(SW_SHOW);       break;
    case 2: m_tabStoreDlg.ShowWindow(SW_SHOW);      break;
    case 3: m_tabReviewDlg.ShowWindow(SW_SHOW);     break;
    case 4: m_tabSalesDlg.ShowWindow(SW_SHOW);      break;
    case 5: m_tabSettlementDlg.ShowWindow(SW_SHOW); break;
    }

    *pResult = 0;
}

void CMainMenuDlg::OnClose()
{
    if (MessageBox(L"종료하시겠습니까?", L"종료", MB_YESNO | MB_ICONQUESTION) == IDYES)
    {
        if (m_pNet)
        {
            json body;
            m_pNet->Send(CmdID::REQ_LOGOUT, body);
        }
        CDialogEx::OnClose();
        AfxGetMainWnd()->PostMessage(WM_QUIT);
    }
}

LRESULT CMainMenuDlg::OnPacketReceived(WPARAM wParam, LPARAM lParam)
{
    (void)wParam;
    auto* pkt = reinterpret_cast<ReceivedPacket*>(lParam);
    if (!pkt) return 0;

    // [디버그 수정한 부분] 한글 깨짐 방지 처리
    CString strUnicodeBody = CA2W(pkt->body.c_str(), CP_UTF8);
    CString dbgPkt;
    dbgPkt.Format(L"[DEBUG] RECV Packet ID: %d, Body: %s\n", pkt->cmdId, (LPCTSTR)strUnicodeBody);
    OutputDebugString(dbgPkt);

    if (pkt->cmdId == CmdID::RES_STORE_INFO_UPDATE)
    {
        json resJson = json::parse(pkt->body);
        if (resJson.value("status", 0) == 200)
            m_tabStoreDlg.OnStoreUpdateSuccess();
        else
            MessageBox(L"저장에 실패했습니다.", L"오류", MB_ICONERROR);
    }
    else if (pkt->cmdId == CmdID::RES_STORE_STATUS_SET)
    {
        json resJson = json::parse(pkt->body);
        if (resJson.value("status", 0) != 200)
            MessageBox(L"영업 상태 변경에 실패했습니다.", L"오류", MB_ICONERROR);
    }
    else if (pkt->cmdId == CmdID::RES_MENU_LIST)
    {
        json resJson = json::parse(pkt->body);
        if (resJson.value("status", 0) == 200)
            m_tabMenuDlg.SetMenuList(resJson["menus"]);
    }
    else if (pkt->cmdId == CmdID::RES_MENU_EDIT)
    {
        json res = json::parse(pkt->body);
        if (res.value("status", 0) == 200)
        {
            m_tabMenuDlg.LoadMenuList();
            MessageBox(L"메뉴가 변경되었습니다.", L"알림", MB_OK);
        }
        else
            MessageBox(L"메뉴 변경에 실패했습니다.", L"오류", MB_ICONERROR);
    }
    else if (pkt->cmdId == CmdID::RES_MENU_SOLD_OUT)
    {
        json res = json::parse(pkt->body);
        if (res.value("status", 0) != 200)
            MessageBox(L"품절 상태 변경에 실패했습니다.", L"오류", MB_ICONERROR);
    }
    else if (pkt->cmdId == CmdID::RES_REVIEW_LIST)
    {
        json resJson = json::parse(pkt->body);
        if (resJson.value("status", 0) == 200)
            m_tabReviewDlg.SetReviewList(resJson["reviews"]);
    }
    else if (pkt->cmdId == CmdID::RES_REVIEW_REPLY)
    {
        json resJson = json::parse(pkt->body);
        if (resJson.value("status", 0) != 200)
            MessageBox(L"답글 등록에 실패했습니다.", L"오류", MB_ICONERROR);
    }
    else if (pkt->cmdId == CmdID::NOTIFY_NEW_ORDER)
    {
        json resJson = json::parse(pkt->body);
        m_tabOrderDlg.AddNewOrder(resJson);

        m_tabCtrl.SetCurSel(0);
        m_tabOrderDlg.ShowWindow(SW_SHOW);
        m_tabMenuDlg.ShowWindow(SW_HIDE);
        m_tabStoreDlg.ShowWindow(SW_HIDE);
        m_tabReviewDlg.ShowWindow(SW_HIDE);
        m_tabSalesDlg.ShowWindow(SW_HIDE);
        m_tabSettlementDlg.ShowWindow(SW_HIDE);
    }
    else if (pkt->cmdId == CmdID::RES_ORDER_ACCEPT)
    {
        json resJson = json::parse(pkt->body);
        m_tabOrderDlg.OnOrderAcceptResult(resJson);
    }
    else if (pkt->cmdId == CmdID::RES_ORDER_REJECT)
    {
        json resJson = json::parse(pkt->body);
        m_tabOrderDlg.OnOrderRejectResult(resJson);
    }
    else if (pkt->cmdId == CmdID::RES_SALES_STAT)
    {
        json resJson = json::parse(pkt->body);
        if (resJson.value("status", 0) == 200)
        {
            int totalSales = resJson.value("totalSales", 0);
            m_tabSalesDlg.OnSalesStatReceived(resJson);
            m_tabSettlementDlg.ShowDummyData(totalSales);
        }
        else
            MessageBox(L"매출 조회에 실패했습니다.", L"오류", MB_ICONERROR);
    }
    else if (pkt->cmdId == CmdID::RES_CHAT_CONNECT)
    {
        json resJson = json::parse(pkt->body);
        int status = resJson.value("status", 0);
        int roomId = resJson.value("roomId", -1);

        if (status == 202)
        {
            // 대기 중
            MessageBox(
                L"관리자에게 요청을 보냈습니다.\n수락 시 채팅창이 자동으로 열립니다.",
                L"고객센터", MB_OK | MB_ICONINFORMATION);
        }
        else if (status == 200 && roomId != -1)
        {
            // 수락됨 - 채팅창 오픈
            if (!m_pChatRoomDlg)
            {
                std::string userId = (const char*)CT2A(m_ownerName, CP_UTF8);
                m_pChatRoomDlg = new CChatRoomDlg(m_pNet, userId, this);
                m_pChatRoomDlg->Create(IDD_CHAT_ROOM, this);
            }
            m_pChatRoomDlg->SetRoomId(roomId);
            m_pChatRoomDlg->ShowWindow(SW_SHOW);
            m_btnChatRequest.EnableWindow(FALSE);
        }
        else if (status == 200 && roomId == -1)
        {
            // 관리자 있지만 대기 중 (roomId 아직 미배정)
            MessageBox(
                L"관리자에게 요청을 보냈습니다.\n수락 시 채팅창이 자동으로 열립니다.",
                L"고객센터", MB_OK | MB_ICONINFORMATION);
        }
        else
        {
            MessageBox(
                L"현재 관리자가 없습니다.\n잠시 후 다시 시도해주세요.",
                L"고객센터", MB_OK | MB_ICONWARNING);
            m_btnChatRequest.EnableWindow(TRUE);
            m_btnChatRequest.SetWindowText(L"고객센터");
        }
    }
        //  RES_REQUEST_OK (5000) - 관리자 수락 시 채팅창 오픈
    else if (pkt->cmdId == CmdID::RES_REQUEST_OK)
    {
        json resJson = json::parse(pkt->body);
        int roomId = resJson.value("roomId", -1);

        // 채팅창 생성 (없을 때만)
        if (!m_pChatRoomDlg)
        {
            std::string userId = (const char*)CT2A(m_ownerName, CP_UTF8);
            m_pChatRoomDlg = new CChatRoomDlg(m_pNet, userId, this);
            m_pChatRoomDlg->Create(IDD_CHAT_ROOM, this);
        }

        //  roomId 전달 후 창 오픈
        m_pChatRoomDlg->SetRoomId(roomId);
        m_pChatRoomDlg->ShowWindow(SW_SHOW);
        }

        //  NOTIFY_CHAT_MSG (9030) - 메시지 수신 시 채팅창에 추가
    else if (pkt->cmdId == CmdID::NOTIFY_CHAT_MSG)
    {
        json resJson = json::parse(pkt->body);
        if (m_pChatRoomDlg && m_pChatRoomDlg->IsWindowVisible())
            m_pChatRoomDlg->AddMessage(resJson);
    }
    else if (pkt->cmdId == CmdID::RES_ORDER_LIST)
    {
        json resJson = json::parse(pkt->body);
        if (resJson.value("status", 0) == 200)
            m_tabOrderDlg.SetOrderList(resJson["orders"]);
    }
    else if (pkt->cmdId == CmdID::NOTIFY_ORDER_STATE)
    {
        json resJson = json::parse(pkt->body);
        std::string orderId = resJson.value("orderId", "");
        int state = resJson.value("state", 0);

        CString strStatus;
        switch (state)
        {
        case 2: strStatus = L"배달중"; break;
        case 3: strStatus = L"완료";   break;
        default: strStatus = L"알 수 없음"; break;
        }

        // orderId로 리스트 탐색 후 상태 변경
        m_tabOrderDlg.UpdateOrderStatus(orderId, strStatus);
    }
    // [새로 추가] 서버에서 채팅 종료 통보를 보냈을 때 (예: CmdID::NOTIFY_CHAT_EXIT)
    //else if (pkt->cmdId == CmdID::NOTIFY_CHAT_EXIT)
    //{
    //    if (m_pChatRoomDlg && ::IsWindow(m_pChatRoomDlg->GetSafeHwnd()))
    //    {
    //        m_pChatRoomDlg->ShowWindow(SW_HIDE);
    //        MessageBox(L"관리자에 의해 상담이 종료되었습니다.", L"알림", MB_OK | MB_ICONINFORMATION);
    //    }
    //    m_btnChatRequest.EnableWindow(TRUE);
    //    m_btnChatRequest.SetWindowText(L"고객센터");
    //}

    delete pkt;
    return 0;
}

void CMainMenuDlg::OnBnClickedBtnChatRequest()
{
    if (MessageBox(
        L"관리자에게 1:1 문의를 요청하시겠습니까?",
        L"고객센터",
        MB_YESNO | MB_ICONQUESTION) == IDYES)
    {
        if (!m_pNet) return;

        json body;
        body["storeId"] = m_storeId;
        body["userId"] = (const char*)CT2A(m_ownerName, CP_UTF8);

        std::string dumped = body.dump();
        CString strUnicodeSend = CA2W(dumped.c_str(), CP_UTF8);
        CString dbgMsg;
        dbgMsg.Format(L"[DEBUG] SEND REQ_CHAT_CONNECT: %s\n", (LPCTSTR)strUnicodeSend);
        OutputDebugString(dbgMsg);

        m_pNet->Send(CmdID::REQ_CHAT_CONNECT, body);

        m_btnChatRequest.EnableWindow(FALSE);
        m_btnChatRequest.SetWindowText(L"문의중...");
    }
}

BEGIN_MESSAGE_MAP(CMainMenuDlg, CDialogEx)
    ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_STATUS_SET, &CMainMenuDlg::OnTcnSelchangeTabStatusSet)
    ON_MESSAGE(WM_PACKET_RECEIVED, &CMainMenuDlg::OnPacketReceived)
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_BTN_CHAT_REQUEST, &CMainMenuDlg::OnBnClickedBtnChatRequest)
END_MESSAGE_MAP()