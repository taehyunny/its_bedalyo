#pragma once
#include "afxdialogex.h"
#include "json.hpp"
#include "CMenuOptionDlg.h"

using json = nlohmann::json;

class CMenuEditDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CMenuEditDlg)

public:
    CMenuEditDlg(int storeId, int menuId = -1, CWnd* pParent = nullptr);
    virtual ~CMenuEditDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_MENU_EDIT };
#endif

    json GetResultBody() const { return m_resultBody; }
    bool IsEditMode()    const { return m_menuId != -1; }

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedOptionEdit();  // ✅ 추가

    DECLARE_MESSAGE_MAP()

private:
    int  m_storeId;
    int  m_menuId;
    json m_resultBody;
    json m_optionsJson = json::array();  // ✅ 빈 배열로 초기화

    CEdit      m_editMenuName;
    CEdit      m_editPrice;
    CEdit      m_editCategory;
    CEdit      m_editDescription;
    CButton    m_chkPopular;
};