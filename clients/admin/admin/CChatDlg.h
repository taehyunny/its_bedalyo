#pragma once
#include "afxdialogex.h"
#include "NetworkHelper.h"
#include "json.hpp"

using json = nlohmann::json;

class CChatDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CChatDlg)

public:
    CChatDlg(CWnd* pParent = nullptr);
    virtual ~CChatDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_TAB_CHAT };
#endif

    // 외부 호출용 함수
    void SetNetworkHelper(CNetworkHelper* pNet);
    void AddChatRequest(const json& reqJson);   // 새 상담 요청
    void AddChatMessage(const json& msgJson);   // 메시지 수신
    void RemoveChatUser(const json& reqJson);   // 상담 종료 (추가됨)

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    virtual void OnOK() override {}
    virtual void OnCancel() override {}

    afx_msg void OnBnClickedBtnChatSend();
    afx_msg void OnBnClickedBtnChatAccept();
    afx_msg void OnBnClickedBtnChatReject();
    afx_msg void OnLvnItemchangedListChatUsers(NMHDR* pNMHDR, LRESULT* pResult);

    DECLARE_MESSAGE_MAP()

private:
    // 내부 사용 구조체
    struct ChatUserData {
        std::string requesterId;    // 서버 body의 "requesterId"
        std::string requesterType;  // 서버 body의 "requesterType" (CUSTOMER / OWNER)
    };

    void InitListCtrl();
    int  GetSelectedUserIndex();

    CNetworkHelper* m_pNet = nullptr;

    std::string m_selectedRequesterId;
    std::string m_selectedRequesterType;

    CListCtrl   m_listChatUsers;    // IDC_LIST_CHAT_USERS
    CListBox    m_listChatLog;      // IDC_LIST_CHAT_LOG
    CEdit       m_editChatMsg;      // IDC_EDIT_CHAT_MSG
    CButton     m_btnChatSend;      // IDC_BTN_CHAT_SEND
    CButton     m_btnChatAccept;    // 동적 생성 - 수락
    CButton     m_btnChatReject;    // 동적 생성 - 거절
};