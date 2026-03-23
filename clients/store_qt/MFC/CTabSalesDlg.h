#pragma once
#include "afxdialogex.h"

class CTabSalesDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CTabSalesDlg)

public:
    CTabSalesDlg(CWnd* pParent = nullptr);
    virtual ~CTabSalesDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_TAB_SALES };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

    afx_msg void OnBnClickedBtnSalesSearch();  // 조회 버튼

    DECLARE_MESSAGE_MAP()

private:
    void InitListCtrl();
    void UpdateSalesInfo(int nTotalSales, int nTotalOrder); // 수수료 계산 후 표시

    // ── 수수료율 (클라이언트 자체 계산) ─────────────────────
    const double COMMISSION_RATE = 0.128; // 12.8%

    // ── 컨트롤 바인딩 ────────────────────────────────────────
    CDateTimeCtrl m_dtStart;              // IDC_DATETIMEPICKER_START
    CDateTimeCtrl m_dtEnd;                // IDC_DATETIMEPICKER_END
    CButton       m_btnSalesSearch;       // IDC_BTN_SALES_SEARCH

    CStatic       m_staticTotalSales;     // IDC_STATIC_TOTAL_SALES
    CStatic       m_staticTotalOrder;     // IDC_STATIC_TOTAL_ORDER
    CStatic       m_staticCommission;     // IDC_STATIC_COMMISSION
    CStatic       m_staticNetSales;       // IDC_STATIC_NET_SALES

    CListCtrl     m_listTopMenu;          // IDC_LIST_TOP_MENU
};