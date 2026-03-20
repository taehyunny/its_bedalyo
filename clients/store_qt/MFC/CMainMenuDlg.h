#pragma once
#include "afxdialogex.h"
#include "CTabStoreDlg.h"

class CMainMenuDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CMainMenuDlg)

public:
    // ✅ 매장 정보를 받는 생성자로 변경
    CMainMenuDlg(int storeId, const CString& storeName, CWnd* pParent = nullptr);
    virtual ~CMainMenuDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_MAIN_MENU };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    DECLARE_MESSAGE_MAP()

private:
    // ── 매장 정보 ──────────────────────────────────────────
    int         m_storeId = 0;
    CString     m_storeName;
    CTabStoreDlg m_tabStoreDlg;  // 매장 설정 탭

    // ── 컨트롤 바인딩 ───────────────────────────────────────
    CStatic     m_staticNameBar;   // IDC_STATIC_NAMEBAR
    CStatic     m_staticStatus;    // IDC_STATIC_STATUS
    CTabCtrl    m_tabCtrl;         // IDC_TAB_STATUS_SET
public:
    afx_msg void OnTcnSelchangeTabStatusSet(NMHDR* pNMHDR, LRESULT* pResult);
};