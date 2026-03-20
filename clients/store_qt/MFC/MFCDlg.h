#pragma once

#include "NetworkHelper.h" 

class CMFCDlg : public CDialogEx
{
public:
    CMFCDlg(CWnd* pParent = nullptr);

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_MFC_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnEdit_ID();
    afx_msg void OnEdit_PW();
    afx_msg void OnBtnLogin();   
    afx_msg void OnBtnSign();     
    afx_msg void OnBtnCancel();

    afx_msg LRESULT OnPacketReceived(WPARAM wParam, LPARAM lParam);

protected:
    HICON m_hIcon;

private:

    CNetworkHelper  m_net;        


    CString         m_strId;
    CString         m_strPw;


    int             m_storeId   = 0;
    CString         m_storeName;


    std::string     m_serverIp  = "10.10.10.123";
    int             m_serverPort = 8000;


    bool            m_waitingResponse = false;
};
