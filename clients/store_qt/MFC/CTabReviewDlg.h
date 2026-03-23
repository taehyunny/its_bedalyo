#pragma once
#include "afxdialogex.h"

class CTabReviewDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CTabReviewDlg)

public:
    CTabReviewDlg(CWnd* pParent = nullptr);
    virtual ~CTabReviewDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_TAB_REVIEW };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

    afx_msg void OnBnClickedBtnReplySubmit();  // 답글 등록
    afx_msg void OnLvnItemchangedListReview(NMHDR* pNMHDR, LRESULT* pResult);

    DECLARE_MESSAGE_MAP()

private:
    void InitListCtrl();
    int  GetSelectedIndex();

    // ── 컨트롤 바인딩 ────────────────────────────────────────
    CListCtrl   m_listReview;          // IDC_LIST_REVIEW
    CEdit       m_editReply;           // IDC_EDIT_REPLY
    CButton     m_btnReplySubmit;      // IDC_BTN_REPLY_SUBMIT
};