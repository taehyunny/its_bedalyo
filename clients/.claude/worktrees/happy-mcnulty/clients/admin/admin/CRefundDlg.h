#pragma once
#include "afxdialogex.h"
#include "NetworkHelper.h"
#include "json.hpp"

using json = nlohmann::json;

class CRefundDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CRefundDlg)

public:
    CRefundDlg(CWnd* pParent = nullptr);
    virtual ~CRefundDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_TAB_REFUND };
#endif

    void SetNetworkHelper(CNetworkHelper* pNet);
    void OnSearchResult(const json& resJson);  // 서버 검색 결과 수신
    void CRefundDlg::OnCancelResult(const json& resJson);
protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    virtual void OnOK() override {}    // Enter 키 → 탭 닫힘 방지
    virtual void OnCancel() override {} // ESC 키 → 탭 닫힘 방지
    afx_msg void OnBnClickedBtnOrderSearch();
    afx_msg void OnBnClickedBtnCancelOrder();
    afx_msg void OnBnClickedBtnRefundOrder();
    afx_msg void OnLvnItemchangedListRefundOrder(NMHDR* pNMHDR, LRESULT* pResult);
    DECLARE_MESSAGE_MAP()

private:
    void InitListCtrl();
    void UpdateButtonState();
    int  GetSelectedIndex();

    CNetworkHelper* m_pNet = nullptr;

    CComboBox   m_comboSearchType;    // IDC_COMBO_SEARCH_TYPE
    CEdit       m_editOrderSearch;    // IDC_EDIT_ORDER_SEARCH
    CButton     m_btnOrderSearch;     // IDC_BTN_ORDER_SEARCH
    CListCtrl   m_listRefundOrder;    // IDC_LIST_REFUND_ORDER
    CButton     m_btnCancelOrder;     // IDC_BTN_CANCEL_ORDER
    CButton     m_btnRefundOrder;     // IDC_BTN_REFUND_ORDER
};