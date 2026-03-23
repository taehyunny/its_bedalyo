#pragma once
#include "afxdialogex.h"

class COrderDetailDlg : public CDialogEx
{
    DECLARE_DYNAMIC(COrderDetailDlg)

public:
    COrderDetailDlg(CWnd* pParent = nullptr);
    virtual ~COrderDetailDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_ORDER_DETAIL };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

    afx_msg void OnBnClickedBtnPrintReceipt(); // 영수증 재출력
    afx_msg void OnBnClickedBtnDetailClose();  // 닫기

    DECLARE_MESSAGE_MAP()

private:
    void InitListCtrl();
    void PrintReceipt();  // VS 출력창에 영수증 출력

    // ── 컨트롤 바인딩 ────────────────────────────────────────
    CStatic     m_staticOrderId;       // IDC_STATIC_ORDER_ID
    CStatic     m_staticOrderAddress;  // IDC_STATIC_ORDER_ADDRESS
    CStatic     m_staticOrderStatus;   // IDC_STATIC_ORDER_STATUS
    CStatic     m_staticOrderTime;     // IDC_STATIC_ORDER_TIME
    CListCtrl   m_listOrderItems;      // IDC_LIST_ORDER_ITEMS
    CStatic     m_staticOrderPrice;    // IDC_STATIC_ORDER_PRICE
    CButton     m_btnPrintReceipt;     // IDC_BTN_PRINT_RECEIPT
    CButton     m_btnDetailClose;      // IDC_BTN_DETAIL_CLOSE
};