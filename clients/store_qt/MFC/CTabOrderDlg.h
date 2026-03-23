#pragma once
#include "afxdialogex.h"
#include "COrderDetailDlg.h"

class CTabOrderDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CTabOrderDlg)

public:
    CTabOrderDlg(CWnd* pParent = nullptr);
    virtual ~CTabOrderDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_TAB_ORDER };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

    // ── 버튼 핸들러 ──────────────────────────────────────────
    afx_msg void OnBnClickedBtnOrderAccept();   // 수락
    afx_msg void OnBnClickedBtnOrderReject();   // 거절

    // ── 콤보박스 핸들러 ──────────────────────────────────────
    afx_msg void OnCbnSelchangeComboRejectReason(); // 거절 사유 선택 변경

    // ── List Control 핸들러 ──────────────────────────────────
    afx_msg void OnLvnItemchangedListOrder(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnNMDblclkListOrder(NMHDR* pNMHDR, LRESULT* pResult); // 더블클릭 → 상세보기

    DECLARE_MESSAGE_MAP()

private:
    void InitListCtrl();
    void UpdateButtonState();
    int  GetSelectedIndex();

    // ── 컨트롤 바인딩 ────────────────────────────────────────
    CListCtrl   m_listOrder;             // IDC_LIST_ORDER
    CButton     m_btnOrderAccept;        // IDC_BTN_ORDER_ACCEPT
    CButton     m_btnOrderReject;        // IDC_BTN_ORDER_REJECT
    CComboBox   m_comboRejectReason;     // IDC_COMBO_REJECT_REASON
    CEdit       m_editRejectReason;      // IDC_EDIT_REJECT_REASON (직접 입력)
};