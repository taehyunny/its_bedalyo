#pragma once
#include "afxdialogex.h"
#include "NetworkHelper.h"
#include "json.hpp"

using json = nlohmann::json;

class CTabReviewDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CTabReviewDlg)

public:
    CTabReviewDlg(CWnd* pParent = nullptr);
    virtual ~CTabReviewDlg();

    // ✅ public - CMainMenuDlg에서 호출하는 함수들
    void SetReviewInfo(int storeId, CNetworkHelper* pNet);
    void SetReviewList(const nlohmann::json& reviewArray);
    void LoadReviewList();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_TAB_REVIEW };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedBtnReplySubmit();
    afx_msg void OnLvnItemchangedListReview(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnBnClickedBtnReviewRefresh();

    DECLARE_MESSAGE_MAP()
private:
    void InitListCtrl();
    int  GetSelectedIndex();
    std::vector<CString> m_ownerReplies;
    int             m_storeId = 0;
    CNetworkHelper* m_pNet = nullptr;
    int             m_selectedReviewId = -1;
    CButton m_btnReviewRefresh;
    CListCtrl   m_listReview;
    CEdit       m_editReply;
    CButton     m_btnReplySubmit;
};