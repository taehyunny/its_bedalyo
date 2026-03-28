#pragma once
#include "afxdialogex.h"
#include "NetworkHelper.h"
#include "json.hpp"

using json = nlohmann::json;

class CBlacklistDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CBlacklistDlg)

public:
    CBlacklistDlg(CWnd* pParent = nullptr);
    virtual ~CBlacklistDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_TAB_BLACKLIST };
#endif

    void SetNetworkHelper(CNetworkHelper* pNet);
    void OnBlacklistResult(const json& resJson);  // 서버 응답 수신

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    virtual void OnOK() override {}    // Enter 키 → 탭 닫힘 방지
    virtual void OnCancel() override {} // ESC 키 → 탭 닫힘 방지
    afx_msg void OnBnClickedBtnBlacklistAdd();
    afx_msg void OnBnClickedBtnBlacklistDel();
    afx_msg void OnLvnItemchangedListBlacklist(NMHDR* pNMHDR, LRESULT* pResult);

    DECLARE_MESSAGE_MAP()

private:
    void InitListCtrl();
    void UpdateButtonState();
    int  GetSelectedIndex();

    CNetworkHelper* m_pNet = nullptr;

    CEdit       m_editBlacklistId;   // IDC_EDIT_BLACKLIST_ID
    CEdit       m_editBlacklistRsn;  // IDC_EDIT_BLACKLIST_RSN
    CButton     m_btnBlacklistAdd;   // IDC_BTN_BLACKLIST_ADD
    CListCtrl   m_listBlacklist;     // IDC_LIST_BLACKLIST
    CButton     m_btnBlacklistDel;   // IDC_BTN_BLACKLIST_DEL
};