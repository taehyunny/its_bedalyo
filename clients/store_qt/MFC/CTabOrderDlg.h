#pragma once
#include "afxdialogex.h"
#include "COrderDetailDlg.h"
#include "NetworkHelper.h"
#include "json.hpp"

using json = nlohmann::json;

class CTabOrderDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CTabOrderDlg)

public:
    CTabOrderDlg(CWnd* pParent = nullptr);
    virtual ~CTabOrderDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_TAB_ORDER };
#endif
    void SetOrderList(const json& orderArray);
    void SetOrderInfo(int storeId, CNetworkHelper* pNet, int cookTime);
    void AddNewOrder(const json& orderJson);
    void OnOrderAcceptResult(const json& resJson);
    void OnOrderRejectResult(const json& resJson);

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

    afx_msg void OnBnClickedBtnOrderDelete();
    afx_msg void OnBnClickedBtnOrderRefresh();

    afx_msg void OnBnClickedBtnOrderAccept();
    afx_msg void OnBnClickedBtnOrderReject();
    afx_msg void OnCbnSelchangeComboRejectReason();
    afx_msg void OnLvnItemchangedListOrder(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnNMDblclkListOrder(NMHDR* pNMHDR, LRESULT* pResult);

    DECLARE_MESSAGE_MAP()

private:
    void InitListCtrl();
    void UpdateButtonState();
    int  GetSelectedIndex();

    int             m_storeId = 0;
    CNetworkHelper* m_pNet = nullptr;
    int             m_cookTime = 30;

    // 주문 상세 데이터 저장 (더블클릭 시 팝업에 전달)
    std::vector<json> m_orderDetails;

    CButton m_btnOrderDelete;   // IDC_BTN_DELETE
    CButton m_btnOrderRefresh;  // IDC_BTN_REFRESH
    CListCtrl   m_listOrder;
    CButton     m_btnOrderAccept;
    CButton     m_btnOrderReject;
    CComboBox   m_comboRejectReason;
    CEdit       m_editRejectReason;
};