#pragma once
#include "afxdialogex.h"
#include "NetworkHelper.h"
#include "json.hpp"

using json = nlohmann::json;

class CRiderMainDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CRiderMainDlg)

public:
    CRiderMainDlg(CWnd* pParent = nullptr);
    virtual ~CRiderMainDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_RIDER_MAIN };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

    afx_msg void OnBnClickedBtnConnect();
    afx_msg void OnBnClickedBtnDisconnect();
    afx_msg void OnBnClickedBtnRefresh();
    afx_msg void OnBnClickedBtnDeliver();
    afx_msg void OnLvnItemchangedListOrders(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LRESULT OnPacketReceived(WPARAM wParam, LPARAM lParam);
    afx_msg void OnClose();

    DECLARE_MESSAGE_MAP()

private:
    void InitListCtrl();
    void UpdateButtonState();
    int  GetSelectedIndex();

    // 주문 목록 수신 처리
    void OnOrderListReceived(const json& resJson);
    void OnDeliveryCallReceived(const json& resJson); // NOTIFY_DELIVERY_CALL 수신
    // 배달 완료 응답 처리
    void OnDeliverResult(const json& resJson);

    CNetworkHelper  m_net;
    std::string     m_serverIp = "10.10.10.123";
    int             m_serverPort = 8000;

    CStatic     m_staticServer;     // IDC_STATIC_SERVER
    CButton     m_btnConnect;       // IDC_BTN_CONNECT
    CButton     m_btnDisconnect;    // IDC_BTN_DISCONNECT
    CButton     m_btnRefresh;       // IDC_BTN_REFRESH
    CListCtrl   m_listOrders;       // IDC_LIST_ORDERS
    CButton     m_btnDeliver;       // IDC_BTN_DELIVER
};