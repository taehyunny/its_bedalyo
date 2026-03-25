#include "pch.h"
#include "admin.h"
#include "afxdialogex.h"
#include "CAdminDlg.h"

IMPLEMENT_DYNAMIC(CAdminDlg, CDialogEx)

CAdminDlg::CAdminDlg(CWnd* pParent)
    : CDialogEx(IDD_ADMIN_DIALOG, pParent)
    , m_tabChatDlg(this)
{
}

CAdminDlg::~CAdminDlg() {}

void CAdminDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_SEVER_ADDRESS, m_staticServerAddress);
    DDX_Control(pDX, IDC_BTN_CONNECT, m_btnConnect);
    DDX_Control(pDX, IDC_BTN_CONNECT_OFF, m_btnConnectOff);
    DDX_Control(pDX, IDC_TAB, m_tabCtrl);
}

BOOL CAdminDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    SetWindowText(L"ITS_Bedalyo - 관리자");

    // 서버 주소 표시
    CString strAddr;
    CString strIp = CA2W(m_serverIp.c_str(), CP_UTF8);
    strAddr.Format(L"서버: %s : %d", (LPCTSTR)strIp, m_serverPort);
    m_staticServerAddress.SetWindowText(strAddr);

    // 연결 해제 버튼 초기 비활성화
    m_btnConnectOff.EnableWindow(FALSE);

    // 탭 항목 추가
    m_tabCtrl.InsertItem(0, L"1:1 채팅 상담");
    m_tabCtrl.InsertItem(1, L"주문 취소/환불");
    m_tabCtrl.InsertItem(2, L"리뷰 관리");
    m_tabCtrl.InsertItem(3, L"블랙리스트");

    // 탭 내용 영역 계산
    CRect rcTab;
    m_tabCtrl.GetClientRect(&rcTab);
    m_tabCtrl.AdjustRect(FALSE, &rcTab);

    m_tabRefundDlg.Create(IDD_TAB_REFUND, &m_tabCtrl);
    m_tabRefundDlg.MoveWindow(&rcTab);
    m_tabRefundDlg.ShowWindow(SW_HIDE);


    m_tabReviewDlg.Create(IDD_TAB_REVIEW_MGR, &m_tabCtrl);
    m_tabReviewDlg.MoveWindow(&rcTab);
    m_tabReviewDlg.ShowWindow(SW_HIDE);

    m_tabBlackDlg.Create(IDD_TAB_BLACKLIST, &m_tabCtrl);
    m_tabBlackDlg.MoveWindow(&rcTab);
    m_tabBlackDlg.ShowWindow(SW_HIDE);

    // ── 탭 다이얼로그 생성 및 배치 ───────────────────────
    m_tabChatDlg.Create(IDD_TAB_CHAT, &m_tabCtrl);
    m_tabChatDlg.MoveWindow(&rcTab);
    m_tabChatDlg.ShowWindow(SW_SHOW); // 0번 탭 기본 표시

    return TRUE;
}

// =========================================================
// 서버 연결 버튼
// =========================================================
void CAdminDlg::OnBnClickedBtnConnect()
{
    if (m_net.IsConnected()) return;

    if (!m_net.Connect(m_serverIp, m_serverPort, GetSafeHwnd()))
    {
        MessageBox(L"서버에 연결할 수 없습니다.", L"연결 실패", MB_ICONERROR);
        return;
    }

    m_net.SetNotifyHwnd(GetSafeHwnd());
    m_tabChatDlg.SetNetworkHelper(&m_net);
    m_tabRefundDlg.SetNetworkHelper(&m_net);
    m_btnConnect.EnableWindow(FALSE);
    m_btnConnectOff.EnableWindow(TRUE);
    m_staticServerAddress.SetWindowText(L"✅ 서버 연결됨");
    m_tabReviewDlg.SetNetworkHelper(&m_net);
    m_tabBlackDlg.SetNetworkHelper(&m_net);

    MessageBox(L"서버에 연결되었습니다.", L"연결 성공", MB_OK);
}

// =========================================================
// 연결 해제 버튼
// =========================================================
void CAdminDlg::OnBnClickedBtnConnectOff()
{
    m_net.Disconnect();

    m_btnConnect.EnableWindow(TRUE);
    m_btnConnectOff.EnableWindow(FALSE);

    CString strAddr;
    CString strIp = CA2W(m_serverIp.c_str(), CP_UTF8);
    strAddr.Format(L"서버: %s : %d", (LPCTSTR)strIp, m_serverPort);
    m_staticServerAddress.SetWindowText(strAddr);
}

// =========================================================
// 탭 변경
// =========================================================
void CAdminDlg::OnTcnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult)
{
    (void)pNMHDR;
    int nSel = m_tabCtrl.GetCurSel();

    m_tabChatDlg.ShowWindow(SW_HIDE);
    m_tabRefundDlg.ShowWindow(SW_HIDE);
    m_tabReviewDlg.ShowWindow(SW_HIDE);
    m_tabBlackDlg.ShowWindow(SW_HIDE);

    switch (nSel)
    {
    case 0: m_tabChatDlg.ShowWindow(SW_SHOW);   break;
    case 1: m_tabRefundDlg.ShowWindow(SW_SHOW); break;
    case 2: m_tabReviewDlg.ShowWindow(SW_SHOW); break;
    case 3: m_tabBlackDlg.ShowWindow(SW_SHOW);  break;
    }

    *pResult = 0;
}

// =========================================================
// 패킷 수신
// =========================================================
LRESULT CAdminDlg::OnPacketReceived(WPARAM wParam, LPARAM lParam)
{
    (void)wParam;
    auto* pkt = reinterpret_cast<ReceivedPacket*>(lParam);
    if (!pkt) return 0;

    json resJson = json::parse(pkt->body);

    if (pkt->cmdId == CmdID::REQ_CHAT_CONNECT)
    {
        // ✅ 사장님/고객 채팅 요청 → 채팅 탭에 추가
        m_tabChatDlg.AddChatRequest(resJson);

        // 채팅 탭으로 자동 전환
        m_tabCtrl.SetCurSel(0);
        m_tabChatDlg.ShowWindow(SW_SHOW);
    }
    else if (pkt->cmdId == CmdID::NOTIFY_CHAT_MSG)
    {
        // ✅ 채팅 메시지 수신
        m_tabChatDlg.AddChatMessage(resJson);
    }
    else if (pkt->cmdId == CmdID::RES_ADMIN_ORDER_LIST)
    {
        m_tabRefundDlg.OnSearchResult(resJson);
    }
    else if (pkt->cmdId == CmdID::RES_REVIEW_LIST)
    {
        m_tabReviewDlg.OnReviewListResult(resJson);
    }
    else if (pkt->cmdId == CmdID::RES_BLACKLIST_REQUEST)
    {
        m_tabBlackDlg.OnBlacklistResult(resJson);
    }

    delete pkt;
    return 0;
}

BEGIN_MESSAGE_MAP(CAdminDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_CONNECT, &CAdminDlg::OnBnClickedBtnConnect)
    ON_BN_CLICKED(IDC_BTN_CONNECT_OFF, &CAdminDlg::OnBnClickedBtnConnectOff)
    ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &CAdminDlg::OnTcnSelchangeTab)
    ON_MESSAGE(WM_PACKET_RECEIVED, &CAdminDlg::OnPacketReceived)
END_MESSAGE_MAP()