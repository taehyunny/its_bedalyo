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

    // ✅ 추가
    void SetOrderInfo(int storeId, CNetworkHelper* pNet);
    void AddNewOrder(const json& orderJson);       // NOTIFY_NEW_ORDER 수신 시
    void OnOrderAcceptResult(const json& resJson); // RES_ORDER_ACCEPT 수신 시
    void OnOrderRejectResult(const json& resJson); // RES_ORDER_REJECT 수신 시
    void SetOrderInfo(int storeId, CNetworkHelper* pNet, int cookTime);
protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

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
    int m_cookTime = 30;

    int             m_storeId = 0;
    CNetworkHelper* m_pNet = nullptr;

    CListCtrl   m_listOrder;
    CButton     m_btnOrderAccept;
    CButton     m_btnOrderReject;
    CComboBox   m_comboRejectReason;
    CEdit       m_editRejectReason;
};