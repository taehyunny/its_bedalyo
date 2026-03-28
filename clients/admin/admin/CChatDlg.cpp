#include "pch.h"
#include "admin.h"
#include "afxdialogex.h"
#include "CChatDlg.h"

IMPLEMENT_DYNAMIC(CChatDlg, CDialogEx)

CChatDlg::CChatDlg(CWnd* pParent)
    : CDialogEx(IDD_TAB_CHAT, pParent) {
}

CChatDlg::~CChatDlg() {

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

void CChatDlg::OnDestroy()
{
    // HWND가 살아있는 시점에 정리
    for (int i = 0; i < m_listChatUsers.GetItemCount(); ++i) {
        auto* pData = reinterpret_cast<ChatUserData*>(m_listChatUsers.GetItemData(i));
        if (pData) delete pData;
    }
    CDialogEx::OnDestroy();
}

void CChatDlg::UpdateRoomId(const std::string& requesterId, int roomId)
{
    for (int i = 0; i < m_listChatUsers.GetItemCount(); ++i)
    {
        auto* pData = reinterpret_cast<ChatUserData*>(m_listChatUsers.GetItemData(i));
        if (pData && pData->requesterId == requesterId)
        {
            pData->roomId = roomId;
            if (m_selectedRequesterId == requesterId)
                m_selectedRoomId = roomId;
            break;
        }
    }
}

std::string CChatDlg::GetSelectedRequesterId() const
{
    return m_selectedRequesterId;
}
void CChatDlg::AddChatRequest(const json& reqJson)
{
    // userId / requesterId 둘 다 대응
    std::string requesterId = reqJson.contains("userId")
        ? reqJson.value("userId", "")
        : reqJson.value("requesterId", "");

    // role(int) / requesterType(string) 둘 다 대응
    std::string requesterType;
    if (reqJson.contains("requesterType"))
        requesterType = reqJson.value("requesterType", "");
    else if (reqJson.contains("role"))
    {
        int role = reqJson.value("role", 0);
        requesterType = (role == 1) ? "STORE_OWNER" : "CUSTOMER";
    }

    if (requesterId.empty()) return;

    // 중복 방지
    for (int i = 0; i < m_listChatUsers.GetItemCount(); ++i) {
        auto* pData = reinterpret_cast<ChatUserData*>(m_listChatUsers.GetItemData(i));
        if (pData && pData->requesterId == requesterId) return;
    }

    CString strRequester = CA2W(requesterId.c_str(), CP_UTF8);
    CString strType = CA2W(requesterType.c_str(), CP_UTF8);

    int nIdx = m_listChatUsers.InsertItem(m_listChatUsers.GetItemCount(), strRequester);
    if (nIdx >= 0)
    {
        m_listChatUsers.SetItemText(nIdx, 1, strType);
        m_listChatUsers.SetItemText(nIdx, 2, L"대기중");
        m_listChatUsers.SetItemData(nIdx, (DWORD_PTR)new ChatUserData{ requesterId, requesterType });
    }
}
void CChatDlg::RemoveChatUserByRoomId(int roomId)
{
    for (int i = 0; i < m_listChatUsers.GetItemCount(); ++i)
    {
        auto* pData = reinterpret_cast<ChatUserData*>(m_listChatUsers.GetItemData(i));
        if (pData && pData->roomId == roomId)
        {
            if (m_selectedRequesterId == pData->requesterId)
            {
                m_selectedRequesterId.clear();
                m_selectedRequesterType.clear();
                m_selectedRoomId = -1;
                m_listChatLog.ResetContent();
                m_btnChatSend.EnableWindow(FALSE);
                m_btnChatAccept.EnableWindow(FALSE);
                m_btnChatReject.EnableWindow(FALSE);
            }
            delete pData;
            m_listChatUsers.DeleteItem(i);
            break;
        }
    }
}
void CChatDlg::AddChatMessage(const json& msgJson)
{
    std::string senderId = msgJson.value("senderId", "");
    std::string message = msgJson.value("content", "");  // content 키 사용

    if (senderId.empty() || message.empty()) return;

    CString strSender = CA2W(senderId.c_str(), CP_UTF8);
    CString strMessage = CA2W(message.c_str(), CP_UTF8);
    CString strLine;
    strLine.Format(L"[%s] %s", (LPCTSTR)strSender, (LPCTSTR)strMessage);
    m_listChatLog.AddString(strLine);
    m_listChatLog.SetCurSel(m_listChatLog.GetCount() - 1);
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
    body["requesterId"] = m_selectedRequesterId;
    body["requesterType"] = m_selectedRequesterType;
    body["result"] = "ACCEPT";
    m_pNet->Send(CmdID::RES_REQUEST_OK, body);

    int nIdx = m_listChatUsers.GetNextItem(-1, LVNI_SELECTED);
    if (nIdx != -1)
        m_listChatUsers.SetItemText(nIdx, 2, L"상담중");

    m_btnChatAccept.EnableWindow(FALSE);
    m_btnChatReject.EnableWindow(TRUE);
    m_btnChatSend.EnableWindow(TRUE); 
}

// =========================================================================
// 거절 버튼 → RES_REQUEST_NO(5001) 서버 전송
// =========================================================================
void CChatDlg::OnBnClickedBtnChatReject()
{
    if (!m_pNet || m_selectedRequesterId.empty()) return;

    CString strStatus;
    int nIdx = m_listChatUsers.GetNextItem(-1, LVNI_SELECTED);
    if (nIdx != -1)
        strStatus = m_listChatUsers.GetItemText(nIdx, 2);

    if (strStatus == L"대기중")
    {
        // 요청 거절
        json body;
        body["requesterId"] = m_selectedRequesterId;
        body["requesterType"] = m_selectedRequesterType;
        body["result"] = "REJECT";
        m_pNet->Send(CmdID::RES_REQUEST_NO, body);

        if (nIdx != -1)
        {
            delete reinterpret_cast<ChatUserData*>(m_listChatUsers.GetItemData(nIdx));
            m_listChatUsers.DeleteItem(nIdx);
        }
    }
    else if (strStatus == L"상담중")
    {
        // 채팅 종료
        if (m_selectedRoomId != -1)
        {
            json body;
            body["roomId"] = m_selectedRoomId;
            m_pNet->Send(CmdID::REQ_CHAT_CLOSE, body);
        }
    }

    m_selectedRequesterId.clear();
    m_selectedRequesterType.clear();
    m_selectedRoomId = -1;
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
            m_selectedRoomId = pData->roomId;
        }

        // 아직 대기중인 항목만 수락/거절 활성화
        CString strStatus = m_listChatUsers.GetItemText(nIdx, 2);
        bool bPending = (strStatus == L"대기중");
        m_btnChatAccept.EnableWindow(bPending);
        m_btnChatReject.EnableWindow(TRUE);
        m_btnChatSend.EnableWindow(!bPending); // 상담중일 때만 전송 가능

        m_listChatLog.ResetContent();
    }
    *pResult = 0;
}

void CChatDlg::OnBnClickedBtnChatSend()
{
    if (!m_pNet || m_selectedRequesterId.empty() || m_selectedRoomId == -1) return;
    CString strMsg;
    m_editChatMsg.GetWindowText(strMsg);
    if (strMsg.IsEmpty()) return;

    json body;
    body["roomId"] = m_selectedRoomId;           
    body["senderId"] = "admin";
    body["content"] = (const char*)CT2A(strMsg, CP_UTF8); 
    m_pNet->Send(CmdID::REQ_CHAT_SEND, body);

    CString strLine;
    strLine.Format(L"[관리자] %s", (LPCTSTR)strMsg);
    m_listChatLog.AddString(strLine);
    m_listChatLog.SetCurSel(m_listChatLog.GetCount() - 1);
    m_editChatMsg.SetWindowText(L"");
}