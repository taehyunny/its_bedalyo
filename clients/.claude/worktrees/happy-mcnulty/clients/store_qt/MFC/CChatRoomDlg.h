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

    void AddMessage(const json& msgJson);
    void SetRoomId(int roomId) { m_roomId = roomId; }  // ✅ roomId 설정

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    virtual void OnOK() override {}
    virtual void OnCancel() override {}

    afx_msg void OnBnClickedBtnChatRoomSend();
    afx_msg void OnBnClickedBtnChatRoomClose();

    DECLARE_MESSAGE_MAP()

private:
    CNetworkHelper* m_pNet = nullptr;
    std::string     m_userId;
    int             m_roomId = -1;  // ✅ roomId 추가

    CListBox    m_listChatLog;
    CEdit       m_editChatMsg;
    CButton     m_btnChatSend;
    CButton     m_btnChatClose;
};