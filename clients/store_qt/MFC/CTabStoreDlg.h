#pragma once
#include "pch.h"
#include "MFC.h"
#include "afxdialogex.h"
#include "CTabStoreDlg.h"
#include "json.hpp"

class CTabStoreDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CTabStoreDlg)

public:
    CTabStoreDlg(CWnd* pParent = nullptr);
    virtual ~CTabStoreDlg();
    void SetUIMode(BOOL bEditMode);

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_TAB_STORE };
#endif

    // ✅ 서버에서 받은 매장/사장님 정보를 채우는 함수
   void SetStoreInfo(
        const CString& storeName, const CString& category,
        const CString& storeAddress, const CString& bizNum,
        const CString& cookTime, const CString& minOrder,
        const CString& openTime, const CString& closeTime,
        const CString& ownerName, const CString& ownerPhone,
        const CString& accountNumber, const CString& approvalStatus);

protected:
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

    // ── 수정 버튼 핸들러 ─────────────────────────────────────
    afx_msg void OnBnClickedBtnEditName();
    afx_msg void OnBnClickedBtnEditCategory();
    afx_msg void OnBnClickedBtnEditAddress();
    afx_msg void OnBnClickedBtnEditOwnerName();
    afx_msg void OnBnClickedBtnEditOwnerPhone();
    afx_msg void OnBnClickedBtnEditAccount();
    // ── 영업 상태 버튼 핸들러 ────────────────────────────────
    afx_msg void OnBnClickedBtnStoreOpen();
    afx_msg void OnBnClickedBtnStoreClose();

    // ── 저장 / 취소 버튼 핸들러 ─────────────────────────────
    afx_msg void OnBnClickedBtnSave();
    afx_msg void OnBnClickedBtnCancel();
    void BackupValues();
    void RestoreValues();
    DECLARE_MESSAGE_MAP()

private:
    int m_nScrollPos = 0;


    // ── 매장 정보 컨트롤 ─────────────────────────────────────
    CEdit       m_editStoreName;       // IDC_EDIT_STORE_NAME
    CComboBox   m_comboCategory;       // IDC_COMBO_CATEGORY
    CEdit       m_editStoreAddress;    // IDC_EDIT_STORE_ADDRESS
    CEdit       m_editStoreBiznum;     // IDC_EDIT_STORE_BIZNUM (읽기 전용)
    CEdit       m_editCookTime;        // IDC_EDIT_COOK_TIME
    CEdit       m_editMinOrder;        // IDC_EDIT_MIN_ORDER
    CEdit       m_editOpenTime;        // IDC_EDIT_OPEN_TIME
    CEdit       m_editCloseTime;       // IDC_EDIT_CLOSE_TIME

    // ── 사장님 정보 컨트롤 ───────────────────────────────────
    CEdit       m_editOwnerName;       // IDC_EDIT_OWNER_NAME
    CEdit       m_editOwnerPhone;      // IDC_EDIT_OWNER_PHONE
    CEdit       m_editAccount;         // IDC_EDIT_ACCOUNT
    CStatic     m_staticApproval;      // IDC_STATIC_APPROVAL (읽기 전용)

    // ── 버튼 ─────────────────────────────────────────────────
    CButton     m_btnEditName;
    CButton     m_btnEditCategory;
    CButton     m_btnEditAddress;
    CButton     m_btnEditOwnerName;
    CButton     m_btnEditOwnerPhone;
    CButton     m_btnEditAccount;
    CButton     m_btnStoreOpen;
    CButton     m_btnStoreClose;

    // ── 원본값 백업용 변수 ────────────────────────────────────
    CString     m_bakStoreName;
    CString     m_bakStoreAddress;
    CString     m_bakCookTime;
    CString     m_bakMinOrder;
    CString     m_bakOpenTime;
    CString     m_bakCloseTime;
    CString     m_bakOwnerName;
    CString     m_bakOwnerPhone;
    CString     m_bakAccount;
    int         m_bakCategory = 0;
};