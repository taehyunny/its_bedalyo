#pragma once
#include "afxdialogex.h"
#include "json.hpp"

using json = nlohmann::json;

class CMenuEditDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CMenuEditDlg)

public:
    // 추가(add): menuId = -1, 수정(edit): menuId = 실제 ID
    CMenuEditDlg(int storeId, int menuId = -1, CWnd* pParent = nullptr);
    virtual ~CMenuEditDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_MENU_EDIT };
#endif

    // 외부에서 결과 읽기용
    json GetResultBody() const { return m_resultBody; }
    bool IsEditMode()    const { return m_menuId != -1; }

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedOk();
    DECLARE_MESSAGE_MAP()

private:
    int  m_storeId;
    int  m_menuId;   // -1이면 추가, 실제값이면 수정
    json m_resultBody;

    CEdit      m_editMenuName;
    CEdit      m_editPrice;
    CEdit      m_editCategory;
    CEdit      m_editDescription;
    CButton    m_chkPopular;
};