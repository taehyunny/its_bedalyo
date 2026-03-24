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
    CString strUser = CA2W(userId.c_str(), CP_UTF8);

    int nIdx = m_listChatUsers.InsertItem(
        m_listChatUsers.GetItemCount(), strUser);
    m_listChatUsers.SetItemText(nIdx, 1, L"대기");

    // userId 저장
    m_listChatUsers.SetItemData(nIdx,
        (DWORD_PTR)new std::string(userId));
}

// =========================================================
// 새 메시지 수신 (RES_CHAT_SEND)
// =========================================================
void CChatDlg::AddChatMessage(const json& msgJson)
{
    std::string senderId = msgJson.value("senderId", "");
    std::string message = msgJson.value("message", "");

    CString strLine;
    strLine.Format(L"[%s] %s",
        CA2W(senderId.c_str(), CP_UTF8),
        CA2W(message.c_str(), CP_UTF8));

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
        // 선택된 유저 ID 저장
        auto* pUserId = reinterpret_cast<std::string*>(
            m_listChatUsers.GetItemData(nIdx));
        if (pUserId)
            m_selectedUserId = *pUserId;

        // 상태 → 상담중으로 변경
        m_listChatUsers.SetItemText(nIdx, 1, L"상담중");

        // 전송 버튼 활성화
        m_btnChatSend.EnableWindow(TRUE);

        // 채팅 로그 초기화 (다른 유저 선택 시)
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