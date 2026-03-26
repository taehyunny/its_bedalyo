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
        auto* pData = reinterpret_cast<ChatUserData*>(m_listChatUsers.GetItemData(i));
        if (pData) delete pData;
    }
}

void CChatDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_CHAT_USERS, m_listChatUsers);
    DDX_Control(pDX, IDC_LIST_CHAT_LOG, m_listChatLog);
    DDX_Control(pDX, IDC_EDIT_CHAT_MSG, m_editChatMsg);
    DDX_Control(pDX, IDC_BTN_CHAT_SEND, m_btnChatSend);
}

BEGIN_MESSAGE_MAP(CChatDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_CHAT_SEND, &CChatDlg::OnBnClickedBtnChatSend)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_CHAT_USERS, &CChatDlg::OnLvnItemchangedListChatUsers)
END_MESSAGE_MAP()

BOOL CChatDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    InitListCtrl();
    m_btnChatSend.EnableWindow(FALSE);
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
    std::string userId = reqJson.value("userId", "");
    int storeId = reqJson.value("storeId", 0);

    for (int i = 0; i < m_listChatUsers.GetItemCount(); ++i) {
        auto* pData = reinterpret_cast<ChatUserData*>(m_listChatUsers.GetItemData(i));
        if (pData && pData->userId == userId) return;
    }

    CString strUser = CA2W(userId.c_str(), CP_UTF8);
    CString strStoreId;
    strStoreId.Format(L"%d", storeId);

    int nIdx = m_listChatUsers.InsertItem(m_listChatUsers.GetItemCount(), strUser);
    m_listChatUsers.SetItemText(nIdx, 1, strStoreId);
    m_listChatUsers.SetItemText(nIdx, 2, L"대기중");
    m_listChatUsers.SetItemData(nIdx, (DWORD_PTR)new ChatUserData{ userId, storeId });
}

void CChatDlg::AddChatMessage(const json& msgJson)
{
    std::string senderId = msgJson.value("senderId", "");
    std::string message = msgJson.value("message", "");

    if (senderId == m_selectedUserId) {
        CString strLine;
        strLine.Format(L"[%S] %S", senderId.c_str(), message.c_str());
        m_listChatLog.AddString(strLine);
        m_listChatLog.SetCurSel(m_listChatLog.GetCount() - 1);
    }
}

// 오류 C2039 해결: 클래스 멤버로 구현
void CChatDlg::RemoveChatUser(const json& reqJson)
{
    std::string userId = reqJson.value("userId", "");
    for (int i = 0; i < m_listChatUsers.GetItemCount(); ++i) {
        auto* pData = reinterpret_cast<ChatUserData*>(m_listChatUsers.GetItemData(i));
        if (pData && pData->userId == userId) {
            delete pData;
            m_listChatUsers.DeleteItem(i);
            if (m_selectedUserId == userId) {
                m_selectedUserId = "";
                m_listChatLog.AddString(L"── 유저가 퇴장했습니다 ──");
                m_btnChatSend.EnableWindow(FALSE);
            }
            break;
        }
    }
}

void CChatDlg::OnLvnItemchangedListChatUsers(NMHDR* pNMHDR, LRESULT* pResult)
{
    int nIdx = m_listChatUsers.GetNextItem(-1, LVNI_SELECTED);
    if (nIdx != -1) {
        auto* pData = reinterpret_cast<ChatUserData*>(m_listChatUsers.GetItemData(nIdx));
        if (pData) {
            m_selectedUserId = pData->userId;
            m_selectedStoreId = pData->storeId;
        }
        m_listChatUsers.SetItemText(nIdx, 2, L"상담중");
        m_btnChatSend.EnableWindow(TRUE);
        m_listChatLog.ResetContent();
    }
    *pResult = 0;
}

void CChatDlg::OnBnClickedBtnChatSend()
{
    if (!m_pNet || m_selectedUserId.empty()) return;
    CString strMsg;
    m_editChatMsg.GetWindowText(strMsg);
    if (strMsg.IsEmpty()) return;

    json body;
    body["receiverId"] = m_selectedUserId;
    body["senderId"] = "admin";
    body["message"] = (const char*)CT2A(strMsg, CP_UTF8);
    m_pNet->Send(CmdID::REQ_CHAT_SEND, body);

    CString strLine;
    strLine.Format(L"[관리자] %s", (LPCTSTR)strMsg);
    m_listChatLog.AddString(strLine);
    m_listChatLog.SetCurSel(m_listChatLog.GetCount() - 1);
    m_editChatMsg.SetWindowText(L"");
}