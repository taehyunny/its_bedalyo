#pragma once
#include "afxdialogex.h"
#include "NetworkHelper.h"
#include "json.hpp"

using json = nlohmann::json;

class CTabSalesDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CTabSalesDlg)

public:
    CTabSalesDlg(CWnd* pParent = nullptr);
    virtual ~CTabSalesDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_TAB_SALES };
#endif

    // ✅ 서버 응답으로 매출 표시
    void SetSalesInfo(int storeId, CNetworkHelper* pNet);
    void OnSalesStatReceived(const json& resJson);

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedBtnSalesSearch();
    DECLARE_MESSAGE_MAP()

private:
    void UpdateSalesDisplay(int totalSales);

    const double COMMISSION_RATE = 0.128; // 12.8% 하드코딩

    int             m_storeId = 0;
    CNetworkHelper* m_pNet = nullptr;

    CDateTimeCtrl m_dtStart;
    CDateTimeCtrl m_dtEnd;
    CButton       m_btnSalesSearch;
    CStatic       m_staticTotalSales;
    CStatic       m_staticTotalOrder;
    CStatic       m_staticCommission;
    CStatic       m_staticNetSales;
    CListCtrl     m_listTopMenu;
};