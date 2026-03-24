// CTabMenuDlg.cpp: 구현 파일

#include "pch.h"
#include "MFC.h"
#include "afxdialogex.h"
#include "CTabMenuDlg.h"
#include "CMenuEditDlg.h"

IMPLEMENT_DYNAMIC(CTabMenuDlg, CDialogEx)

CTabMenuDlg::CTabMenuDlg(CWnd* pParent)
    : CDialogEx(IDD_TAB_MENU, pParent)
{
}

CTabMenuDlg::~CTabMenuDlg()
{
}

// =========================================================================
// 컨트롤 바인딩
// =========================================================================
void CTabMenuDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_MENU, m_listMenu);
    DDX_Control(pDX, IDC_BTN_MENU_ADD, m_btnMenuAdd);
    DDX_Control(pDX, IDC_BTN_MENU_EDIT, m_btnMenuEdit);
    DDX_Control(pDX, IDC_BTN_MENU_DELETE, m_btnMenuDelete);
    DDX_Control(pDX, IDC_BTN_MENU_SOLDOUT, m_btnMenuSoldout);
}

// =========================================================================
// 초기화
// =========================================================================
BOOL CTabMenuDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    InitListCtrl();
    UpdateButtonState(); // 초기에는 선택 없으므로 수정/삭제/품절 비활성화

    return TRUE;
}


// =========================================================================
// List Control 컬럼 초기화
// =========================================================================
void CTabMenuDlg::InitListCtrl()
{
    // 전체 선택 스타일 적용
    m_listMenu.SetExtendedStyle(
        m_listMenu.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES
    );

    // 컬럼 추가 (헤더명, 너비)
    m_listMenu.InsertColumn(0, L"메뉴명", LVCFMT_LEFT, 180);
    m_listMenu.InsertColumn(1, L"가격", LVCFMT_RIGHT, 80);
    m_listMenu.InsertColumn(2, L"카테고리", LVCFMT_LEFT, 100);
    m_listMenu.InsertColumn(3, L"상태", LVCFMT_CENTER, 80);
    m_listMenu.InsertColumn(4, L"인기메뉴", LVCFMT_CENTER, 80);

}

void CTabMenuDlg::LoadMenuList()
{
    if (!m_pNet) return;

    json body;
    body["storeId"] = m_storeId;  // ✅ 멤버변수 사용
    m_pNet->Send(CmdID::REQ_MENU_LIST, body);
}

void CTabMenuDlg::SetMenuInfo(int storeId, CNetworkHelper* pNet)
{
    m_storeId = storeId;
    m_pNet = pNet;
    LoadMenuList(); // 저장 즉시 서버에 요청
}

void CTabMenuDlg::SetMenuList(const nlohmann::json& menuArray)
{
    m_listMenu.DeleteAllItems();

    auto toW = [](const std::string& s) -> CString {
        return CA2W(s.c_str(), CP_UTF8);
        };

    for (int i = 0; i < (int)menuArray.size(); i++)
    {
        const auto& menu = menuArray[i];

        CString menuName = toW(menu.value("menuName", ""));
        int     price = menu.value("basePrice", 0);
        CString category = toW(menu.value("menuCategory", ""));
        bool    soldOut = menu.value("isSoldOut", false);
        bool    popular = menu.value("isPopular", false);

        CString strPrice;
        strPrice.Format(L"%d", price);

        int nIdx = m_listMenu.InsertItem(i, menuName);
        m_listMenu.SetItemText(nIdx, 1, strPrice);
        m_listMenu.SetItemText(nIdx, 2, category);
        m_listMenu.SetItemText(nIdx, 3, soldOut ? L"품절" : L"판매중");
        m_listMenu.SetItemText(nIdx, 4, popular ? L"O" : L"X");

        // menuId를 나중에 수정/삭제 시 사용하기 위해 저장
        m_listMenu.SetItemData(nIdx, menu.value("menuId", 0));
    }
}
// =========================================================================
// 선택 항목 유무에 따라 버튼 활성화/비활성화
// =========================================================================
void CTabMenuDlg::UpdateButtonState()
{
    bool bSelected = (GetSelectedIndex() != -1);
    m_btnMenuEdit.EnableWindow(bSelected);
    m_btnMenuDelete.EnableWindow(bSelected);
    m_btnMenuSoldout.EnableWindow(bSelected);
}

// =========================================================================
// 현재 선택된 항목 인덱스 반환 (-1 = 선택 없음)
// =========================================================================
int CTabMenuDlg::GetSelectedIndex()
{
    return m_listMenu.GetNextItem(-1, LVNI_SELECTED);
}

// =========================================================================
// List Control 선택 변경 이벤트
// =========================================================================
void CTabMenuDlg::OnLvnItemchangedListMenu(NMHDR* pNMHDR, LRESULT* pResult)
{
    UpdateButtonState();

    // 품절 버튼 텍스트 동적 변경
    int nIdx = GetSelectedIndex();
    if (nIdx != -1)
    {
        CString strStatus = m_listMenu.GetItemText(nIdx, 3);
        m_btnMenuSoldout.SetWindowText(strStatus == L"품절" ? L"판매재개" : L"품절처리");
    }

    *pResult = 0;
}

// =========================================================================
// 메뉴 추가 버튼
// =========================================================================
void CTabMenuDlg::OnBnClickedBtnMenuAdd()
{
    CMenuEditDlg dlg(m_storeId, -1, this); // -1 = 추가 모드
    if (dlg.DoModal() == IDOK)
    {
        m_pNet->Send(CmdID::REQ_MENU_EDIT, dlg.GetResultBody());
        // 서버 응답(RES_MENU_EDIT) 후 LoadMenuList()로 목록 갱신
    }
}

// =========================================================================
// 메뉴 수정 버튼
// =========================================================================
void CTabMenuDlg::OnBnClickedBtnMenuEdit()
{
    int nIdx = GetSelectedIndex();
    if (nIdx == -1) return;

    int menuId = (int)m_listMenu.GetItemData(nIdx);

    CMenuEditDlg dlg(m_storeId, menuId, this); // 실제 menuId = 수정 모드
    if (dlg.DoModal() == IDOK)
    {
        m_pNet->Send(CmdID::REQ_MENU_EDIT, dlg.GetResultBody());
    }
}

// =========================================================================
// 메뉴 삭제 버튼
// =========================================================================
void CTabMenuDlg::OnBnClickedBtnMenuDelete()
{
    int nIdx = GetSelectedIndex();
    if (nIdx == -1) return;

    CString strName = m_listMenu.GetItemText(nIdx, 0);
    CString strMsg;
    strMsg.Format(L"'%s' 메뉴를 삭제하시겠습니까?", (LPCTSTR)strName);

    if (MessageBox(strMsg, L"메뉴 삭제", MB_YESNO | MB_ICONQUESTION) == IDYES)
    {
        int menuId = (int)m_listMenu.GetItemData(nIdx);

        // MenuUpdateReqDTO 구조에 맞게 전송
        // actionType: 2 = 삭제, menuId만 필요
        json body;
        body["storeId"] = m_storeId;
        body["actionType"] = 2;  // 삭제
        body["menuData"]["menuId"] = menuId;  // 삭제는 menuId만 필요

        m_pNet->Send(CmdID::REQ_MENU_EDIT, body);

        // UI에서 즉시 제거
        m_listMenu.DeleteItem(nIdx);
        UpdateButtonState();
    }
}

// =========================================================================
// 품절처리 / 판매재개 버튼
// =========================================================================
void CTabMenuDlg::OnBnClickedBtnMenuSoldout()
{
    int nIdx = GetSelectedIndex();
    if (nIdx == -1) return;

    int menuId = (int)m_listMenu.GetItemData(nIdx); // SetItemData로 저장해둔 menuId
    CString strStatus = m_listMenu.GetItemText(nIdx, 3);
    bool bSoldOut = (strStatus != L"품절"); // 현재 판매중이면 품절로 변경

    //  서버 전송
    json body;
    body["menuId"] = menuId;
    body["isSoldOut"] = bSoldOut;  // true: 품절, false: 판매재개
    body["storeId"] = m_storeId;
    m_pNet->Send(CmdID::REQ_MENU_SOLD_OUT, body);

    // UI 즉시 반영
    m_listMenu.SetItemText(nIdx, 3, bSoldOut ? L"품절" : L"판매중");
    m_btnMenuSoldout.SetWindowText(bSoldOut ? L"판매재개" : L"품절처리");
}

BEGIN_MESSAGE_MAP(CTabMenuDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_MENU_ADD, &CTabMenuDlg::OnBnClickedBtnMenuAdd)
    ON_BN_CLICKED(IDC_BTN_MENU_EDIT, &CTabMenuDlg::OnBnClickedBtnMenuEdit)
    ON_BN_CLICKED(IDC_BTN_MENU_DELETE, &CTabMenuDlg::OnBnClickedBtnMenuDelete)
    ON_BN_CLICKED(IDC_BTN_MENU_SOLDOUT, &CTabMenuDlg::OnBnClickedBtnMenuSoldout)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_MENU, &CTabMenuDlg::OnLvnItemchangedListMenu)
END_MESSAGE_MAP()