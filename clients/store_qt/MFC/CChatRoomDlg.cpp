#include "pch.h"
#include "MFC.h"
#include "afxdialogex.h"
#include "CChatRoomDlg.h"

IMPLEMENT_DYNAMIC(CChatRoomDlg, CDialogEx)

CChatRoomDlg::CChatRoomDlg(CNetworkHelper* pNet,
    const std::string& userId,
    CWnd* pParent)
    : CDialogEx(IDD_CHAT_ROOM, pParent)
    , m_pNet(pNet)
    , m_userId(userId)
{
}

CChatRoomDlg::~CChatRoomDlg() {}

void CChatRoomDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_CHAT_ROOM, m_listChatLog);
    DDX_Control(pDX, IDC_EDIT_CHAT_ROOM_MSG, m_editChatMsg);
    DDX_Control(pDX, IDC_BTN_CHAT_ROOM_SEND, m_btnChatSend);
    DDX_Control(pDX, IDC_BTN_CHAT_ROOM_CLOSE, m_btnChatClose);
}

BOOL CChatRoomDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    SetWindowText(L"고객센터 1:1 채팅");
    m_listChatLog.AddString(L"── 관리자와 연결되었습니다 ──");
    return TRUE;
}

// =========================================================
// 메시지 수신
// =========================================================
void CChatRoomDlg::AddMessage(const json& msgJson)
{
    std::string senderId = msgJson.value("senderId", "");
    std::string content = msgJson.value("content", "");

    // 내가 보낸 메시지는 이미 로컬에 추가했으므로 skip
    if (senderId == m_userId) return;

    CString strMsg = CA2W(content.c_str(), CP_UTF8);
    CString strLine;
    strLine.Format(L"[관리자] %s", (LPCTSTR)strMsg);
    m_listChatLog.AddString(strLine);
    m_listChatLog.SetCurSel(m_listChatLog.GetCount() - 1);
}

// =========================================================
// 전송 버튼
// =========================================================
void CChatRoomDlg::OnBnClickedBtnChatRoomSend()
{
    if (!m_pNet) return;
    if (m_roomId == -1)  //  roomId 없으면 전송 불가
    {
        MessageBox(L"채팅방이 아직 준비되지 않았습니다.", L"알림", MB_OK);
        return;
    }

    CString strMsg;
    m_editChatMsg.GetWindowText(strMsg);
    if (strMsg.IsEmpty()) return;

    //  roomId 사용, "content" 키 사용
    json body;
    body["roomId"] = m_roomId;
    body["senderId"] = m_userId;
    body["content"] = (const char*)CT2A(strMsg, CP_UTF8);
    m_pNet->Send(CmdID::REQ_CHAT_SEND, body);

    // 내 메시지 즉시 표시
    CString strLine;
    strLine.Format(L"[나] %s", (LPCTSTR)strMsg);
    m_listChatLog.AddString(strLine);
    m_listChatLog.SetCurSel(m_listChatLog.GetCount() - 1);

    m_editChatMsg.SetWindowText(L"");
    m_editChatMsg.SetFocus();
}

// =========================================================
// 닫기 버튼
// =========================================================
void CChatRoomDlg::OnBnClickedBtnChatRoomClose()
{
    if (m_pNet && m_roomId != -1)
    {
        json body;
        body["roomId"] = m_roomId;
        m_pNet->Send(CmdID::REQ_CHAT_CLOSE, body);
    }
    ShowWindow(SW_HIDE);
}

BEGIN_MESSAGE_MAP(CChatRoomDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_CHAT_ROOM_SEND, &CChatRoomDlg::OnBnClickedBtnChatRoomSend)
    ON_BN_CLICKED(IDC_BTN_CHAT_ROOM_CLOSE, &CChatRoomDlg::OnBnClickedBtnChatRoomClose)
END_MESSAGE_MAP()