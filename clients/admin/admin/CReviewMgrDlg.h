#pragma once
#include "afxdialogex.h"
#include "NetworkHelper.h"
#include "json.hpp"

using json = nlohmann::json;

class CReviewMgrDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CReviewMgrDlg)

public:
    CReviewMgrDlg(CWnd* pParent = nullptr);
    virtual ~CReviewMgrDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_TAB_REVIEW_MGR };
#endif

    void SetNetworkHelper(CNetworkHelper* pNet);
    void OnReviewListResult(const json& resJson);  // 서버 검색 결과 수신

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    virtual void OnOK() override {}    // Enter 키 → 탭 닫힘 방지
    virtual void OnCancel() override {} // ESC 키 → 탭 닫힘 방지
    afx_msg void OnBnClickedBtnReviewSearch();
    afx_msg void OnBnClickedBtnReviewDelete();
    afx_msg void OnLvnItemchangedListReviewMgr(NMHDR* pNMHDR, LRESULT* pResult);

    DECLARE_MESSAGE_MAP()

private:
    void InitListCtrl();
    void UpdateButtonState();
    int  GetSelectedIndex();

    CNetworkHelper* m_pNet = nullptr;

    CEdit       m_editReviewSearch;   // IDC_EDIT_REVIEW_SEARCH
    CButton     m_btnReviewSearch;    // IDC_BTN_REVIEW_SEARCH
    CListCtrl   m_listReviewMgr;      // IDC_LIST_REVIEW_MGR
    CButton     m_btnReviewDelete;    // IDC_BTN_REVIEW_DELETE
};