#include "pch.h"
#include "MFC.h"
#include "afxdialogex.h"
#include "CMenuOptionDlg.h"

IMPLEMENT_DYNAMIC(CMenuOptionDlg, CDialogEx)

CMenuOptionDlg::CMenuOptionDlg(const json& existingOptions, CWnd* pParent)
    : CDialogEx(IDD_MENU_OPTION, pParent)
    , m_optionsJson(existingOptions)
{
}

CMenuOptionDlg::~CMenuOptionDlg() {}

void CMenuOptionDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_OPT_GROUP_NAME, m_editGroupName);
    DDX_Control(pDX, IDC_CHK_OPT_REQUIRED, m_chkRequired);
    DDX_Control(pDX, IDC_LIST_OPT_GROUP, m_listGroup);
    DDX_Control(pDX, IDC_EDIT_OPT_ITEM_NAME, m_editItemName);
    DDX_Control(pDX, IDC_EDIT_OPT_ITEM_PRICE, m_editItemPrice);
    DDX_Control(pDX, IDC_LIST_OPT_ITEM, m_listItem);
}

BOOL CMenuOptionDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    InitListCtrls();

    // 기존 옵션이 있으면 불러오기
    if (!m_optionsJson.is_null() && m_optionsJson.is_array())
    {
        // nextId 계산 (기존 ID와 겹치지 않도록)
        for (auto& group : m_optionsJson)
        {
            int gid = group.value("groupId", 0);
            if (gid >= m_nextGroupId) m_nextGroupId = gid + 1;

            for (auto& item : group["options"])
            {
                int iid = item.value("optionId", 0);
                if (iid >= m_nextItemId) m_nextItemId = iid + 1;
            }
        }
        RefreshGroupList();
    }
    else
    {
        // 없으면 빈 배열 초기화
        m_optionsJson = json::array();
    }

    return TRUE;
}

void CMenuOptionDlg::InitListCtrls()
{
    // 그룹 리스트
    m_listGroup.SetExtendedStyle(
        m_listGroup.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    m_listGroup.InsertColumn(0, L"그룹명", LVCFMT_LEFT, 150);
    m_listGroup.InsertColumn(1, L"필수여부", LVCFMT_CENTER, 70);

    // 아이템 리스트
    m_listItem.SetExtendedStyle(
        m_listItem.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    m_listItem.InsertColumn(0, L"옵션명", LVCFMT_LEFT, 150);
    m_listItem.InsertColumn(1, L"추가금액", LVCFMT_RIGHT, 80);
}

// =========================================================
// 그룹 리스트 갱신
// =========================================================
void CMenuOptionDlg::RefreshGroupList()
{
    m_listGroup.DeleteAllItems();
    for (int i = 0; i < (int)m_optionsJson.size(); i++)
    {
        auto& group = m_optionsJson[i];
        CString strName = CA2W(group.value("groupName", "").c_str(), CP_UTF8);
        bool bRequired = group.value("isRequired", false);

        int nIdx = m_listGroup.InsertItem(i, strName);
        m_listGroup.SetItemText(nIdx, 1, bRequired ? L"필수" : L"선택");
    }
    // 아이템 리스트 초기화
    m_listItem.DeleteAllItems();
}

// =========================================================
// 선택된 그룹의 아이템 리스트 갱신
// =========================================================
void CMenuOptionDlg::RefreshItemList(int groupIndex)
{
    m_listItem.DeleteAllItems();
    if (groupIndex < 0 || groupIndex >= (int)m_optionsJson.size()) return;

    auto& options = m_optionsJson[groupIndex]["options"];
    for (int i = 0; i < (int)options.size(); i++)
    {
        auto& item = options[i];
        CString strName = CA2W(item.value("optionName", "").c_str(), CP_UTF8);
        int price = item.value("additionalPrice", 0);

        CString strPrice;
        strPrice.Format(L"%d", price);

        int nIdx = m_listItem.InsertItem(i, strName);
        m_listItem.SetItemText(nIdx, 1, strPrice);
    }
}

int CMenuOptionDlg::GetSelectedGroupIndex()
{
    return m_listGroup.GetNextItem(-1, LVNI_SELECTED);
}

int CMenuOptionDlg::GetSelectedItemIndex()
{
    return m_listItem.GetNextItem(-1, LVNI_SELECTED);
}

// =========================================================
// 그룹 추가
// =========================================================
void CMenuOptionDlg::OnBnClickedBtnOptGroupAdd()
{
    CString strName;
    m_editGroupName.GetWindowText(strName);

    if (strName.IsEmpty())
    {
        MessageBox(L"그룹명을 입력하세요.", L"알림", MB_OK);
        return;
    }

    bool bRequired = (m_chkRequired.GetCheck() == BST_CHECKED);

    json newGroup;
    newGroup["groupId"] = m_nextGroupId++;
    newGroup["groupName"] = (const char*)CT2A(strName, CP_UTF8);
    newGroup["isRequired"] = bRequired;
    newGroup["maxCount"] = 1;
    newGroup["options"] = json::array();

    m_optionsJson.push_back(newGroup);

    // UI 초기화
    m_editGroupName.SetWindowText(L"");
    m_chkRequired.SetCheck(BST_UNCHECKED);

    RefreshGroupList();
}

// =========================================================
// 그룹 삭제
// =========================================================
void CMenuOptionDlg::OnBnClickedBtnOptGroupDel()
{
    int nIdx = GetSelectedGroupIndex();
    if (nIdx == -1)
    {
        MessageBox(L"삭제할 그룹을 선택하세요.", L"알림", MB_OK);
        return;
    }

    CString strName = m_listGroup.GetItemText(nIdx, 0);
    CString strMsg;
    strMsg.Format(L"'%s' 그룹을 삭제하시겠습니까?", (LPCTSTR)strName);

    if (MessageBox(strMsg, L"그룹 삭제", MB_YESNO | MB_ICONQUESTION) == IDYES)
    {
        m_optionsJson.erase(m_optionsJson.begin() + nIdx);
        RefreshGroupList();
        m_listItem.DeleteAllItems();
    }
}

// =========================================================
// 옵션 아이템 추가
// =========================================================
void CMenuOptionDlg::OnBnClickedBtnOptItemAdd()
{
    int nGroupIdx = GetSelectedGroupIndex();
    if (nGroupIdx == -1)
    {
        MessageBox(L"먼저 그룹을 선택하세요.", L"알림", MB_OK);
        return;
    }

    CString strName, strPrice;
    m_editItemName.GetWindowText(strName);
    m_editItemPrice.GetWindowText(strPrice);

    if (strName.IsEmpty())
    {
        MessageBox(L"옵션명을 입력하세요.", L"알림", MB_OK);
        return;
    }

    int nPrice = strPrice.IsEmpty() ? 0 : _ttoi(strPrice);

    json newItem;
    newItem["optionId"] = m_nextItemId++;
    newItem["optionName"] = (const char*)CT2A(strName, CP_UTF8);
    newItem["additionalPrice"] = nPrice;

    m_optionsJson[nGroupIdx]["options"].push_back(newItem);

    // UI 초기화
    m_editItemName.SetWindowText(L"");
    m_editItemPrice.SetWindowText(L"");

    RefreshItemList(nGroupIdx);
}

// =========================================================
// 옵션 아이템 삭제
// =========================================================
void CMenuOptionDlg::OnBnClickedBtnOptItemDel()
{
    int nGroupIdx = GetSelectedGroupIndex();
    int nItemIdx = GetSelectedItemIndex();

    if (nGroupIdx == -1 || nItemIdx == -1)
    {
        MessageBox(L"삭제할 옵션을 선택하세요.", L"알림", MB_OK);
        return;
    }

    m_optionsJson[nGroupIdx]["options"].erase(
        m_optionsJson[nGroupIdx]["options"].begin() + nItemIdx);

    RefreshItemList(nGroupIdx);
}

// =========================================================
// 그룹 선택 변경 → 아이템 리스트 갱신
// =========================================================
void CMenuOptionDlg::OnLvnItemchangedListOptGroup(NMHDR* pNMHDR, LRESULT* pResult)
{
    (void)pNMHDR;
    int nIdx = GetSelectedGroupIndex();
    if (nIdx != -1)
        RefreshItemList(nIdx);
    *pResult = 0;
}

// =========================================================
// 확인 버튼
// =========================================================
void CMenuOptionDlg::OnBnClickedOk()
{
    // m_optionsJson이 결과값으로 외부에서 GetResultOptions()로 읽음
    EndDialog(IDOK);
}

BEGIN_MESSAGE_MAP(CMenuOptionDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_OPT_GROUP_ADD, &CMenuOptionDlg::OnBnClickedBtnOptGroupAdd)
    ON_BN_CLICKED(IDC_BTN_OPT_GROUP_DEL, &CMenuOptionDlg::OnBnClickedBtnOptGroupDel)
    ON_BN_CLICKED(IDC_BTN_OPT_ITEM_ADD, &CMenuOptionDlg::OnBnClickedBtnOptItemAdd)
    ON_BN_CLICKED(IDC_BTN_OPT_ITEM_DEL, &CMenuOptionDlg::OnBnClickedBtnOptItemDel)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_OPT_GROUP, &CMenuOptionDlg::OnLvnItemchangedListOptGroup)
    ON_BN_CLICKED(IDOK, &CMenuOptionDlg::OnBnClickedOk)
END_MESSAGE_MAP()