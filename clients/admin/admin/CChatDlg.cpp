#include "pch.h"
#include "admin.h"
#include "afxdialogex.h"
#include "CChatDlg.h"

IMPLEMENT_DYNAMIC(CChatDlg, CDialogEx)

CChatDlg::CChatDlg(CWnd* pParent)
    : CDialogEx(IDD_TAB_CHAT, pParent) {
}

CChatDlg::~CChatDlg() {}

void CChatDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_CHAT_USERS, m_listChatUsers);
    DDX_Control(pDX, IDC_LIST_CHAT_LOG, m_listChatLog);
    DDX_Control(pDX, IDC_EDIT_CHAT_MSG, m_editChatMsg);
    DDX_Control(pDX, IDC_BTN_CHAT_SEND, m_btnChatSend);
}

BOOL CChatDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    InitListCtrl();

    // 전송 버튼 초기 비활성화 (유저 선택 후 활성화)
    m_btnChatSend.EnableWindow(FALSE);

    return TRUE;
}

void CChatDlg::InitListCtrl()
{
    m_listChatUsers.SetExtendedStyle(
        m_listChatUsers.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    m_listChatUsers.InsertColumn(0, L"유저ID", LVCFMT_LEFT, 90);
    m_listChatUsers.InsertColumn(1, L"상태", LVCFMT_CENTER, 40);
    m_listChatUsers.InsertColumn(1, L"매장ID", LVCFMT_CENTER, 60);
    m_listChatUsers.InsertColumn(2, L"상태", LVCFMT_CENTER, 50);
}

void CChatDlg::SetNetworkHelper(CNetworkHelper* pNet)
{
    m_pNet = pNet;
}

// =========================================================
// 새 상담 요청 수신 (REQ_CHAT_CONNECT)
// =========================================================
void CChatDlg::AddChatRequest(const json& reqJson)
{
    std::string userId = reqJson.value("userId", "");
    int  storeId = reqJson.value("storeId", 0);

    CString strUser = CA2W(userId.c_str(), CP_UTF8);
    CString strStoreId;
    strStoreId.Format(L"%d", storeId);

    int nIdx = m_listChatUsers.InsertItem(
        m_listChatUsers.GetItemCount(), strUser);
    m_listChatUsers.SetItemText(nIdx, 1, strStoreId); 
    m_listChatUsers.SetItemText(nIdx, 1, L"대기");

    // userId, storeId 함께 저장
    struct ChatUserData {
        std::string userId;
        int         storeId;
    };
    m_listChatUsers.SetItemData(nIdx,(DWORD_PTR)new ChatUserData{ userId, storeId });
}

// =========================================================
// 새 메시지 수신 (RES_CHAT_SEND)
// =========================================================
void CChatDlg::AddChatMessage(const json& msgJson)
{
    std::string senderId = msgJson.value("senderId", "");
    std::string message = msgJson.value("message", "");

    CString strLine;
    CString strSender = CA2W(senderId.c_str(), CP_UTF8);
    CString strMsg = CA2W(message.c_str(), CP_UTF8);
    strLine.Format(L"[%s] %s", (LPCTSTR)strSender, (LPCTSTR)strMsg);

    m_listChatLog.AddString(strLine);
    // 최신 메시지로 스크롤
    m_listChatLog.SetCurSel(m_listChatLog.GetCount() - 1);
}

// =========================================================
// 상담 유저 선택 변경
// =========================================================
void CChatDlg::OnLvnItemchangedListChatUsers(NMHDR* pNMHDR, LRESULT* pResult)
{
    (void)pNMHDR;
    int nIdx = GetSelectedUserIndex();
    if (nIdx != -1)
    {
        struct ChatUserData { std::string userId; int storeId; };
        auto* pData = reinterpret_cast<ChatUserData*>(
            m_listChatUsers.GetItemData(nIdx));
        if (pData)
        {
            m_selectedUserId = pData->userId;
            m_selectedStoreId = pData->storeId;  // ← 추가
        }

        m_listChatUsers.SetItemText(nIdx, 2, L"상담중");  // ← 1→2
        m_btnChatSend.EnableWindow(TRUE);
        m_listChatLog.ResetContent();
    }
    *pResult = 0;
}


// =========================================================
// 전송 버튼
// =========================================================
void CChatDlg::OnBnClickedBtnChatSend()
{
    if (!m_pNet) return;
    if (m_selectedUserId.empty()) return;

    CString strMsg;
    m_editChatMsg.GetWindowText(strMsg);
    if (strMsg.IsEmpty()) return;

    // ✅ 서버 전송
    json body;
    body["receiverId"] = m_selectedUserId;
    body["senderId"] = "admin";
    body["message"] = (const char*)CT2A(strMsg, CP_UTF8);
    m_pNet->Send(CmdID::REQ_CHAT_SEND, body);

    // 내 메시지 채팅 로그에 즉시 표시
    CString strLine;
    strLine.Format(L"[관리자] %s", (LPCTSTR)strMsg);
    m_listChatLog.AddString(strLine);
    m_listChatLog.SetCurSel(m_listChatLog.GetCount() - 1);

    // 입력창 초기화
    m_editChatMsg.SetWindowText(L"");
    m_editChatMsg.SetFocus();
}

int CChatDlg::GetSelectedUserIndex()
{
    return m_listChatUsers.GetNextItem(-1, LVNI_SELECTED);
}

BEGIN_MESSAGE_MAP(CChatDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_CHAT_SEND, &CChatDlg::OnBnClickedBtnChatSend)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_CHAT_USERS,
        &CChatDlg::OnLvnItemchangedListChatUsers)
END_MESSAGE_MAP()