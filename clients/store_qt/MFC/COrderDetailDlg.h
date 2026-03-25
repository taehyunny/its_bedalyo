#pragma once
#include "afxdialogex.h"
#include "json.hpp"

using json = nlohmann::json;

class COrderDetailDlg : public CDialogEx
{
    DECLARE_DYNAMIC(COrderDetailDlg)

public:
    // ✅ 기본 생성자 제거, json 받는 생성자만 유지
    COrderDetailDlg(const json& orderJson, CWnd* pParent = nullptr);
    virtual ~COrderDetailDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_ORDER_DETAIL };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

    afx_msg void OnBnClickedBtnPrintReceipt();
    afx_msg void OnBnClickedBtnDetailClose();

    DECLARE_MESSAGE_MAP()

private:
    void InitListCtrl();
    void PrintReceipt();

    // ✅ 주문 데이터 저장
    json m_orderJson;

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