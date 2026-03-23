#pragma once
#include "afxdialogex.h"

class CTabSettlementDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CTabSettlementDlg)

public:
    CTabSettlementDlg(CWnd* pParent = nullptr);
    virtual ~CTabSettlementDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_TAB_SETTLEMENT };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

    afx_msg void OnBnClickedBtnSettlementSearch(); // 조회 버튼

    DECLARE_MESSAGE_MAP()

private:
    void InitListCtrl();

    // ── 수수료율 (클라이언트 자체 계산) ─────────────────────
    const double COMMISSION_RATE = 0.128; // 12.8%

    // ── 컨트롤 바인딩 ────────────────────────────────────────
    CDateTimeCtrl m_dtSetStart;           // IDC_DATETIMEPICKER_SET_START
    CDateTimeCtrl m_dtSetEnd;             // IDC_DATETIMEPICKER_SET_END
    CButton       m_btnSettlementSearch;  // IDC_BTN_SETTLEMENT_SEARCH

    CStatic       m_staticPendingAmount;  // IDC_STATIC_PENDING_AMOUNT
    CStatic       m_staticCompletedAmount;// IDC_STATIC_COMPLETED_AMOUNT

    CListCtrl     m_listSettlement;       // IDC_LIST_SETTLEMENT
};