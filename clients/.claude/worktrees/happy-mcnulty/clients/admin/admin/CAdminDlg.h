#pragma once
#include "afxdialogex.h"
#include "NetworkHelper.h"
#include "CChatDlg.h"
#include "json.hpp"
#include "CRefundDlg.h"
#include "CReviewMgrDlg.h"
#include "CBlacklistDlg.h"

using json = nlohmann::json;

class CAdminDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CAdminDlg)

public:
    CAdminDlg(CWnd* pParent = nullptr);
    virtual ~CAdminDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_ADMIN_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

    afx_msg void OnBnClickedBtnConnect();
    afx_msg void OnBnClickedBtnConnectOff();
    afx_msg void OnTcnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LRESULT OnPacketReceived(WPARAM wParam, LPARAM lParam);

    DECLARE_MESSAGE_MAP()

private:
    // ── 네트워크 ─────────────────────────────────────────
    CNetworkHelper  m_net;
    std::string     m_serverIp = "10.10.10.123";
    int             m_serverPort = 8000;
    // ── 컨트롤 바인딩 ────────────────────────────────────
    CStatic     m_staticServerAddress;  // IDC_STATIC_SEVER_ADDRESS
    CButton     m_btnConnect;           // IDC_BTN_CONNECT
    CButton     m_btnConnectOff;        // IDC_BTN_CONNECT_OFF
    CTabCtrl    m_tabCtrl;              // IDC_TAB

    // ── 탭 다이얼로그 ────────────────────────────────────
    CChatDlg            m_tabChatDlg;           // Tab 0: 1:1 채팅 상담
    CRefundDlg          m_tabRefundDlg;         // Tab 1: 주문 취소/환불
    CReviewMgrDlg       m_tabReviewDlg;  // Tab 2: 리뷰 관리 (추후 추가)
    CBlacklistDlg       m_tabBlackDlg;   // Tab 3: 블랙리스트 (추후 추가)
};