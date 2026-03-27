#pragma once
#include "afxdialogex.h"
#include "NetworkHelper.h"
#include "json.hpp"

using json = nlohmann::json;

class CTabSettlementDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CTabSettlementDlg)

public:
    CTabSettlementDlg(CWnd* pParent = nullptr);
    virtual ~CTabSettlementDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_TAB_SETTLEMENT };
#endif

    void SetSettlementInfo(int storeId, CNetworkHelper* pNet);
    void ShowDummyData(int totalSales); // total_sales 기반 하드코딩 표시

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedBtnSettlementSearch();
    DECLARE_MESSAGE_MAP()

private:
    void InitListCtrl();


    const double COMMISSION_RATE = 0.128;

    int             m_storeId = 0;
    CNetworkHelper* m_pNet = nullptr;

    CDateTimeCtrl m_dtSetStart;
    CDateTimeCtrl m_dtSetEnd;
    CButton       m_btnSettlementSearch;
    CStatic       m_staticPendingAmount;
    CStatic       m_staticCompletedAmount;
    CListCtrl     m_listSettlement;
};