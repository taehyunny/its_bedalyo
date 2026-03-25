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

    // ✅ 외부에서 호출
    void SetNetworkHelper(CNetworkHelper* pNet);
    void AddChatRequest(const json& reqJson);   // 새 상담 요청 수신
    void AddChatMessage(const json& msgJson);   // 새 메시지 수신

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    virtual void OnOK() override {}    // Enter 키 → 탭 닫힘 방지
    virtual void OnCancel() override {} // ESC 키 → 탭 닫힘 방지
    afx_msg void OnBnClickedBtnChatSend();
    afx_msg void OnLvnItemchangedListChatUsers(NMHDR* pNMHDR, LRESULT* pResult);

    DECLARE_MESSAGE_MAP()

private:
    void InitListCtrl();
    int  GetSelectedUserIndex();

    CNetworkHelper* m_pNet = nullptr;

    // 현재 선택된 상담 유저 ID
    std::string m_selectedUserId;

    CListCtrl   m_listChatUsers;   // IDC_LIST_CHAT_USERS
    CListBox    m_listChatLog;     // IDC_LIST_CHAT_LOG
    CEdit       m_editChatMsg;     // IDC_EDIT_CHAT_MSG
    CButton     m_btnChatSend;     // IDC_BTN_CHAT_SEND
};