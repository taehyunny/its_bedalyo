#include "pch.h"
#include "MFC.h"
#include "afxdialogex.h"
#include "CMenuEditDlg.h"

IMPLEMENT_DYNAMIC(CMenuEditDlg, CDialogEx)

CMenuEditDlg::CMenuEditDlg(int storeId, int menuId, CWnd* pParent)
    : CDialogEx(IDD_MENU_EDIT, pParent)
    , m_storeId(storeId)
    , m_menuId(menuId)
{
}

CMenuEditDlg::~CMenuEditDlg() {}

void CMenuEditDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_MENU_NAME, m_editMenuName);
    DDX_Control(pDX, IDC_EDIT_MENU_PRICE, m_editPrice);
    DDX_Control(pDX, IDC_EDIT_MENU_CAT, m_editCategory);
    DDX_Control(pDX, IDC_EDIT_MENU_DESC, m_editDescription);
    DDX_Control(pDX, IDC_CHK_POPULAR, m_chkPopular);
}

BOOL CMenuEditDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    SetWindowText(m_menuId == -1 ? L"메뉴 추가" : L"메뉴 수정");
    return TRUE;
}

void CMenuEditDlg::OnBnClickedOk()
{
    CString strName, strPrice, strCat, strDesc;
    m_editMenuName.GetWindowText(strName);
    m_editPrice.GetWindowText(strPrice);
    m_editCategory.GetWindowText(strCat);
    m_editDescription.GetWindowText(strDesc);

    if (strName.IsEmpty() || strPrice.IsEmpty())
    {
        MessageBox(L"메뉴명과 가격은 필수입니다.", L"알림", MB_OK);
        return;
    }

    // ✅ MenuUpdateReqDTO 구조에 맞게 body 생성
    m_resultBody["storeId"] = m_storeId;
    m_resultBody["actionType"] = (m_menuId == -1) ? 0 : 1; // 0:추가, 1:수정

    m_resultBody["menuData"]["menuName"] = CT2A(strName, CP_UTF8).m_psz;
    m_resultBody["menuData"]["basePrice"] = _ttoi(strPrice);
    m_resultBody["menuData"]["menuCategory"] = CT2A(strCat, CP_UTF8).m_psz;
    m_resultBody["menuData"]["description"] = CT2A(strDesc, CP_UTF8).m_psz;
    m_resultBody["menuData"]["isPopular"] = (m_chkPopular.GetCheck() == BST_CHECKED);
    m_resultBody["menuData"]["isSoldOut"] = false;

    if (m_menuId != -1)
        m_resultBody["menuData"]["menuId"] = m_menuId; // 수정 시에만 포함

    EndDialog(IDOK);
}

BEGIN_MESSAGE_MAP(CMenuEditDlg, CDialogEx)
    ON_BN_CLICKED(IDOK, &CMenuEditDlg::OnBnClickedOk)
END_MESSAGE_MAP()