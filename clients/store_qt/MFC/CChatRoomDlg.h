#pragma once
#include "afxdialogex.h"
#include "NetworkHelper.h"
#include "json.hpp"

using json = nlohmann::json;

class CChatRoomDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CChatRoomDlg)

public:
    CChatRoomDlg(CNetworkHelper* pNet,
        const std::string& userId,
        CWnd* pParent = nullptr);
    virtual ~CChatRoomDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_CHAT_ROOM };
#endif

    // CMainMenuDlg에서 NOTIFY_CHAT_MSG 수신 시 호출
    void AddMessage(const json& msgJson);

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    virtual void OnOK() override {}      // Enter 키 닫힘 방지
    virtual void OnCancel() override {}  // ESC 키 닫힘 방지

    afx_msg void OnBnClickedBtnChatRoomSend();
    afx_msg void OnBnClickedBtnChatRoomClose();

    DECLARE_MESSAGE_MAP()

private:
    CNetworkHelper* m_pNet = nullptr;
    std::string     m_userId;           // 내 userId (= ownerName)

    CListBox    m_listChatLog;      // IDC_LIST_CHAT_ROOM
    CEdit       m_editChatMsg;      // IDC_EDIT_CHAT_ROOM_MSG
    CButton     m_btnChatSend;      // IDC_BTN_CHAT_ROOM_SEND
    CButton     m_btnChatClose;     // IDC_BTN_CHAT_ROOM_CLOSE
};