#pragma once
#include "afxdialogex.h"
#include "NetworkHelper.h"

static const wchar_t* CATEGORY_LIST[] = {
    L"한식", L"중식", L"양식", L"돈까스/일식",
    L"치킨", L"피자", L"햄버거", L"족발/보쌈",
    L"도시락", L"초밥/회"
};
static const int CATEGORY_COUNT = 10;

class CSignupDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CSignupDlg)

public:
    CSignupDlg(CNetworkHelper* pNet, CWnd* pParent = nullptr);
    virtual ~CSignupDlg();

    enum { IDD = IDD_SIGNUP };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

    afx_msg void OnBnClickedBtnIdCheck();
    afx_msg void OnBnClickedBtnStoreCheck();
    afx_msg void OnBnClickedBtnSignup();
    afx_msg void OnBnClickedBtnCancel();

    // ID / 사업자번호 수정 감지
    afx_msg void OnChangeEditId();
    afx_msg void OnChangeEditStoreId();

    afx_msg LRESULT OnPacketReceived(WPARAM wParam, LPARAM lParam);
    afx_msg HBRUSH  OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

    DECLARE_MESSAGE_MAP()

private:
    bool ValidateInputs();
    void UpdateSignupButton();
    void SetIdStatus(const CString& msg, COLORREF color);
    void SetStoreStatus(const CString& msg, COLORREF color);

    CNetworkHelper* m_pNet;

    // USERS
    CEdit       m_editId;
    CEdit       m_editPw;
    CEdit       m_editPwConfirm;
    CEdit       m_editName;
    CEdit       m_editPhone;

    // STORES
    CEdit       m_editStoreName;
    CComboBox   m_comboCategory;
    CEdit       m_editOpenTime;
    CEdit       m_editCloseTime;
    CEdit       m_editStoreId;
    CEdit       m_editStoreAddress;
    // Buttons
    CButton     m_btnIdCheck;
    CButton     m_btnStoreCheck;

    // Status labels
    CStatic     m_staticIdStatus;
    CStatic     m_staticStoreStatus;

    // Colors
    COLORREF    m_idStatusColor = RGB(0, 0, 0);
    COLORREF    m_storeStatusColor = RGB(0, 0, 0);

    // Flags
    bool        m_idAvailable = false;
    bool        m_storeAvailable = false;
    bool        m_waitingResponse = false;
public:
    afx_msg void OnEnChangePhone();
};