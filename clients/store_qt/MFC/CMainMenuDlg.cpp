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
    int deliveryFee,        //  CString → int
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

    // 상단 매장명 / 영업상태 표시
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

    // 탭 항목 추가
    m_tabCtrl.InsertItem(0, L"주문 관리");
    m_tabCtrl.InsertItem(1, L"메뉴 관리");
    m_tabCtrl.InsertItem(2, L"매장 설정");
    m_tabCtrl.InsertItem(3, L"리뷰 관리");
    m_tabCtrl.InsertItem(4, L"매출 통계");
    m_tabCtrl.InsertItem(5, L"정산 관리");

    // Tab Control 내용 영역 계산
    CRect rcTab;
    m_tabCtrl.GetClientRect(&rcTab);
    m_tabCtrl.AdjustRect(FALSE, &rcTab);

    // ── 탭 다이얼로그 생성 및 배치 ───────────────────────────
    m_tabOrderDlg.Create(IDD_TAB_ORDER, &m_tabCtrl);
    m_tabOrderDlg.MoveWindow(&rcTab);
    m_tabOrderDlg.ShowWindow(SW_SHOW);

    m_tabMenuDlg.Create(IDD_TAB_MENU, &m_tabCtrl);
    m_tabMenuDlg.MoveWindow(&rcTab);
    m_tabMenuDlg.ShowWindow(SW_HIDE);

    m_tabStoreDlg.Create(IDD_TAB_STORE, &m_tabCtrl);
    m_tabStoreDlg.MoveWindow(&rcTab);
    m_tabStoreDlg.ShowWindow(SW_HIDE);

    // ✅ int → CString 변환 후 SetStoreInfo 전달
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

    // ✅ Create 먼저, SetReviewInfo 나중에
    m_tabReviewDlg.Create(IDD_TAB_REVIEW, &m_tabCtrl);
    m_tabReviewDlg.MoveWindow(&rcTab);
    m_tabReviewDlg.ShowWindow(SW_HIDE);
    m_tabReviewDlg.SetReviewInfo(m_storeId, m_pNet);

    m_tabSalesDlg.Create(IDD_TAB_SALES, &m_tabCtrl);
    m_tabSalesDlg.MoveWindow(&rcTab);
    m_tabSalesDlg.ShowWindow(SW_HIDE);
    m_tabSalesDlg.SetSalesInfo(m_storeId, m_pNet);

    // ✅ Create 먼저, SetSettlementInfo 나중에
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
        std::string msg = resJson.value("message", "");
        CString strMsg = CA2W(msg.c_str(), CP_UTF8);

        if (status == 200 || status == 202) 
        {
            MessageBox(strMsg.IsEmpty() ?
                L"관리자와 연결되었습니다.\n잠시 후 답변이 도착합니다." :
                strMsg,
                L"고객센터", MB_OK | MB_ICONINFORMATION);
        }
        else
        {
            MessageBox(strMsg.IsEmpty() ?
                L"현재 관리자가 없습니다.\n잠시 후 다시 시도해주세요." :
                strMsg,
                L"고객센터", MB_OK | MB_ICONWARNING);
            m_btnChatRequest.EnableWindow(TRUE);
            m_btnChatRequest.SetWindowText(L"고객센터");
        }
    }
    else if (pkt->cmdId == CmdID::RES_ORDER_LIST)
    {
        json resJson = json::parse(pkt->body);
        if (resJson.value("status", 0) == 200)
            m_tabOrderDlg.SetOrderList(resJson["orders"]);
    }
    else if (pkt->cmdId == CmdID::RES_CHAT_CONNECT)
    {
        json resJson = json::parse(pkt->body);
        int status = resJson.value("status", 0);

        if (status == 200 || status == 202)
        {
            // 채팅창 열기 (모달리스)
            if (!m_pChatRoomDlg)
            {
                std::string userId = (const char*)CT2A(m_ownerName, CP_UTF8);
                m_pChatRoomDlg = new CChatRoomDlg(m_pNet, userId, this);
                m_pChatRoomDlg->Create(IDD_CHAT_ROOM, this);
            }
            m_pChatRoomDlg->ShowWindow(SW_SHOW);
        }
        else
        {
            MessageBox(L"현재 관리자가 없습니다.\n잠시 후 다시 시도해주세요.",
                L"고객센터", MB_OK | MB_ICONWARNING);
            m_btnChatRequest.EnableWindow(TRUE);
            m_btnChatRequest.SetWindowText(L"고객센터");
        }
    }
    else if (pkt->cmdId == CmdID::NOTIFY_CHAT_MSG)
    {
        json resJson = json::parse(pkt->body);
        if (m_pChatRoomDlg && ::IsWindow(m_pChatRoomDlg->GetSafeHwnd()))
            m_pChatRoomDlg->AddMessage(resJson);
    }

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