#include "pch.h"
#include "admin.h"
#include "afxdialogex.h"
#include "CChatDlg.h"

IMPLEMENT_DYNAMIC(CChatDlg, CDialogEx)

CChatDlg::CChatDlg(CWnd* pParent)
    : CDialogEx(IDD_TAB_CHAT, pParent) {
}

CChatDlg::~CChatDlg() {
    for (int i = 0; i < m_listChatUsers.GetItemCount(); ++i) {
        delete reinterpret_cast<ChatUserData*>(m_listChatUsers.GetItemData(i));
    }
}

void CChatDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_CHAT_USERS, m_listChatUsers);
    DDX_Control(pDX, IDC_LIST_CHAT_LOG, m_listChatLog);
    DDX_Control(pDX, IDC_EDIT_CHAT_MSG, m_editChatMsg);
    DDX_Control(pDX, IDC_BTN_CHAT_SEND, m_btnChatSend);
    DDX_Control(pDX, IDC_BTN_CHAT_ACCEPT, m_btnChatAccept);
    DDX_Control(pDX, IDC_BTN_CHAT_REJECT, m_btnChatReject);
}

BEGIN_MESSAGE_MAP(CChatDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_CHAT_SEND,   &CChatDlg::OnBnClickedBtnChatSend)
    ON_BN_CLICKED(IDC_BTN_CHAT_ACCEPT, &CChatDlg::OnBnClickedBtnChatAccept)
    ON_BN_CLICKED(IDC_BTN_CHAT_REJECT, &CChatDlg::OnBnClickedBtnChatReject)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_CHAT_USERS, &CChatDlg::OnLvnItemchangedListChatUsers)
END_MESSAGE_MAP()

BOOL CChatDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    InitListCtrl();

    // 선택 전까지 비활성화
    m_btnChatSend.EnableWindow(FALSE);
    m_btnChatAccept.EnableWindow(FALSE);
    m_btnChatReject.EnableWindow(FALSE);

    return TRUE;
}

void CChatDlg::InitListCtrl()
{
    m_listChatUsers.SetExtendedStyle(m_listChatUsers.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    m_listChatUsers.InsertColumn(0, L"유저ID", LVCFMT_LEFT, 90);
    m_listChatUsers.InsertColumn(1, L"매장ID", LVCFMT_CENTER, 60);
    m_listChatUsers.InsertColumn(2, L"상태", LVCFMT_CENTER, 70);
}

void CChatDlg::SetNetworkHelper(CNetworkHelper* pNet) { m_pNet = pNet; }

void CChatDlg::AddChatRequest(const json& reqJson)
{
    std::string requesterId   = reqJson.value("requesterId",   "");
    std::string requesterType = reqJson.value("requesterType", "");

    OutputDebugStringA(("[AddChatRequest] called. requesterId=" + requesterId + "\n").c_str());
    OutputDebugStringA(("[AddChatRequest] listHWND valid=" + std::to_string(::IsWindow(m_listChatUsers.GetSafeHwnd())) + "\n").c_str());

    if (requesterId.empty()) { OutputDebugStringA("[AddChatRequest] requesterId empty, return\n"); return; }

    // 중복 요청 방지
    for (int i = 0; i < m_listChatUsers.GetItemCount(); ++i) {
        auto* pData = reinterpret_cast<ChatUserData*>(m_listChatUsers.GetItemData(i));
        if (pData && pData->requesterId == requesterId) { OutputDebugStringA("[AddChatRequest] duplicate, return\n"); return; }
    }

    CString strRequester = CA2W(requesterId.c_str(),   CP_UTF8);
    CString strType      = CA2W(requesterType.c_str(), CP_UTF8);

    int nIdx = m_listChatUsers.InsertItem(m_listChatUsers.GetItemCount(), strRequester);
    OutputDebugStringA(("[AddChatRequest] InsertItem result nIdx=" + std::to_string(nIdx) + "\n").c_str());

    if (nIdx >= 0)
    {
        m_listChatUsers.SetItemText(nIdx, 1, strType);
        m_listChatUsers.SetItemText(nIdx, 2, L"대기중");
        m_listChatUsers.SetItemData(nIdx, (DWORD_PTR)new ChatUserData{ requesterId, requesterType });
    }
}

void CChatDlg::AddChatMessage(const json& msgJson)
{
    std::string senderId = msgJson.value("senderId", "");
    std::string message = msgJson.value("message", "");

    if (senderId.empty()) return;  // 가드 추가

    if (senderId == m_selectedRequesterId)
    {
        CString strSender = CA2W(senderId.c_str(), CP_UTF8);
        CString strMessage = CA2W(message.c_str(), CP_UTF8);
        CString strLine;
        strLine.Format(L"[%s] %s", (LPCTSTR)strSender, (LPCTSTR)strMessage);
        m_listChatLog.AddString(strLine);
        m_listChatLog.SetCurSel(m_listChatLog.GetCount() - 1);
    }
}

// 오류 C2039 해결: 클래스 멤버로 구현
void CChatDlg::RemoveChatUser(const json& reqJson)
{
    std::string requesterId = reqJson.value("requesterId", "");
    for (int i = 0; i < m_listChatUsers.GetItemCount(); ++i) {
        auto* pData = reinterpret_cast<ChatUserData*>(m_listChatUsers.GetItemData(i));
        if (pData && pData->requesterId == requesterId) {
            delete pData;
            m_listChatUsers.DeleteItem(i);
            if (m_selectedRequesterId == requesterId) {
                m_selectedRequesterId = "";
                m_listChatLog.AddString(L"── 유저가 퇴장했습니다 ──");
                m_btnChatSend.EnableWindow(FALSE);
            }
            break;
        }
    }
}

// =========================================================================
// 수락 버튼 → RES_REQUEST_OK(5000) 서버 전송
// 서버가 requester에게 푸시 → 가게 사장님 채팅창 오픈
// =========================================================================
void CChatDlg::OnBnClickedBtnChatAccept()
{
    if (!m_pNet || m_selectedRequesterId.empty()) return;

    json body;
    body["requesterId"]   = m_selectedRequesterId;
    body["requesterType"] = m_selectedRequesterType;
    body["result"]        = "ACCEPT";
    m_pNet->Send(CmdID::RES_REQUEST_OK, body);

    // 목록 상태 "상담중" 으로 변경, 버튼 비활성화
    int nIdx = m_listChatUsers.GetNextItem(-1, LVNI_SELECTED);
    if (nIdx != -1)
        m_listChatUsers.SetItemText(nIdx, 2, L"상담중");

    m_btnChatAccept.EnableWindow(FALSE);
    m_btnChatReject.EnableWindow(FALSE);
}

// =========================================================================
// 거절 버튼 → RES_REQUEST_NO(5001) 서버 전송
// =========================================================================
void CChatDlg::OnBnClickedBtnChatReject()
{
    if (!m_pNet || m_selectedRequesterId.empty()) return;

    json body;
    body["requesterId"]   = m_selectedRequesterId;
    body["requesterType"] = m_selectedRequesterType;
    body["result"]        = "REJECT";
    m_pNet->Send(CmdID::RES_REQUEST_NO, body);

    // 목록에서 제거
    int nIdx = m_listChatUsers.GetNextItem(-1, LVNI_SELECTED);
    if (nIdx != -1)
    {
        delete reinterpret_cast<ChatUserData*>(m_listChatUsers.GetItemData(nIdx));
        m_listChatUsers.DeleteItem(nIdx);
    }

    m_selectedRequesterId.clear();
    m_selectedRequesterType.clear();
    m_btnChatAccept.EnableWindow(FALSE);
    m_btnChatReject.EnableWindow(FALSE);
    m_btnChatSend.EnableWindow(FALSE);
    m_listChatLog.ResetContent();
}

void CChatDlg::OnLvnItemchangedListChatUsers(NMHDR* pNMHDR, LRESULT* pResult)
{
    int nIdx = m_listChatUsers.GetNextItem(-1, LVNI_SELECTED);
    if (nIdx != -1)
    {
        auto* pData = reinterpret_cast<ChatUserData*>(m_listChatUsers.GetItemData(nIdx));
        if (pData)
        {
            m_selectedRequesterId   = pData->requesterId;
            m_selectedRequesterType = pData->requesterType;
        }

        // 아직 대기중인 항목만 수락/거절 활성화
        CString strStatus = m_listChatUsers.GetItemText(nIdx, 2);
        bool bPending = (strStatus == L"대기중");
        m_btnChatAccept.EnableWindow(bPending);
        m_btnChatReject.EnableWindow(bPending);
        m_btnChatSend.EnableWindow(!bPending); // 상담중일 때만 전송 가능

        m_listChatLog.ResetContent();
    }
    *pResult = 0;
}

void CChatDlg::OnBnClickedBtnChatSend()
{
    if (!m_pNet || m_selectedRequesterId.empty()) return;
    CString strMsg;
    m_editChatMsg.GetWindowText(strMsg);
    if (strMsg.IsEmpty()) return;

    json body;
    body["receiverId"] = m_selectedRequesterId;
    body["senderId"]   = "admin";
    body["message"]    = (const char*)CT2A(strMsg, CP_UTF8);
    m_pNet->Send(CmdID::REQ_CHAT_SEND, body);

    CString strLine;
    strLine.Format(L"[관리자] %s", (LPCTSTR)strMsg);
    m_listChatLog.AddString(strLine);
    m_listChatLog.SetCurSel(m_listChatLog.GetCount() - 1);
    m_editChatMsg.SetWindowText(L"");
}