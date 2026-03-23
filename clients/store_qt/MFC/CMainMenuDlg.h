#pragma once
#include "afxdialogex.h"
#include "CTabOrderDlg.h"
#include "CTabMenuDlg.h"
#include "CTabStoreDlg.h"
#include "CTabReviewDlg.h"
#include "CTabSalesDlg.h"
#include "CTabSettlementDlg.h"
#include "NetworkHelper.h"

class CMainMenuDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CMainMenuDlg)

public:
    CMainMenuDlg(int storeId, CNetworkHelper* pNet, 
        const CString& storeName,
        const CString& category, const CString& storeAddress,
        const CString& bizNum, const CString& cookTime,
        const CString& minOrder, const CString& openTime,
        const CString& closeTime, const CString& ownerName,
        const CString& ownerPhone, const CString& accountNumber,
        const CString& approvalStatus,
        CWnd* pParent = nullptr);

    // ✅ 소멸자 선언 추가
    virtual ~CMainMenuDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_MAIN_MENU };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    afx_msg LRESULT OnPacketReceived(WPARAM wParam, LPARAM lParam);
    afx_msg void OnTcnSelchangeTabStatusSet(NMHDR* pNMHDR, LRESULT* pResult);
    DECLARE_MESSAGE_MAP()

private:
    // ── 매장 정보 ──────────────────────────────────────────
    int             m_storeId = 0;
    CString         m_storeName;
    CString         m_category;
    CString         m_storeAddress;
    CString         m_bizNum;
    CString         m_cookTime;
    CString         m_minOrder;
    CString         m_openTime;
    CString         m_closeTime;
    CString         m_ownerName;
    CString         m_ownerPhone;
    CString         m_accountNumber;
    CString         m_approvalStatus;

    // ── 탭 다이얼로그 ───────────────────────────────────────
    CTabOrderDlg      m_tabOrderDlg;      // 0: 주문 관리
    CTabMenuDlg       m_tabMenuDlg;       // 1: 메뉴 관리
    CTabStoreDlg      m_tabStoreDlg;      // 2: 매장 설정
    CTabReviewDlg     m_tabReviewDlg;     // 3: 리뷰 관리
    CTabSalesDlg      m_tabSalesDlg;      // 4: 매출 통계
    CTabSettlementDlg m_tabSettlementDlg; // 5: 정산 관리

    // ── 컨트롤 바인딩 ───────────────────────────────────────
    CStatic     m_staticNameBar;   // IDC_STATIC_NAMEBAR
    CStatic     m_staticStatus;    // IDC_STATIC_STATUS
    CTabCtrl    m_tabCtrl;         // IDC_TAB_STATUS_SET
    CNetworkHelper* m_pNet = nullptr;
public:

};