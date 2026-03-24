#pragma once
#include "afxdialogex.h"
#include "json.hpp"

using json = nlohmann::json;

class CMenuOptionDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CMenuOptionDlg)

public:
    // 기존 menu_options JSON을 받아서 편집
    CMenuOptionDlg(const json& existingOptions, CWnd* pParent = nullptr);
    virtual ~CMenuOptionDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_MENU_OPTION };
#endif

    // 확인 후 외부에서 결과 JSON 읽기
    json GetResultOptions() const { return m_optionsJson; }

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

    // 그룹 버튼 핸들러
    afx_msg void OnBnClickedBtnOptGroupAdd();
    afx_msg void OnBnClickedBtnOptGroupDel();

    // 아이템 버튼 핸들러
    afx_msg void OnBnClickedBtnOptItemAdd();
    afx_msg void OnBnClickedBtnOptItemDel();

    // 그룹 리스트 선택 변경 → 아이템 리스트 갱신
    afx_msg void OnLvnItemchangedListOptGroup(NMHDR* pNMHDR, LRESULT* pResult);

    afx_msg void OnBnClickedOk();

    DECLARE_MESSAGE_MAP()

private:
    void InitListCtrls();
    void RefreshGroupList();
    void RefreshItemList(int groupIndex);
    int  GetSelectedGroupIndex();
    int  GetSelectedItemIndex();

    // 현재 편집 중인 옵션 데이터
    // 구조:
    // [
    //   {
    //     "groupId": 1,
    //     "groupName": "맛 선택",
    //     "isRequired": true,
    //     "options": [
    //       {"optionId": 1, "optionName": "순한맛", "additionalPrice": 0},
    //       {"optionId": 2, "optionName": "매운맛", "additionalPrice": 500}
    //     ]
    //   }
    // ]
    json m_optionsJson;

    // 다음 임시 ID (로컬에서만 사용)
    int m_nextGroupId = 1;
    int m_nextItemId = 1;

    // 컨트롤 바인딩
    CEdit       m_editGroupName;   // IDC_EDIT_OPT_GROUP_NAME
    CButton     m_chkRequired;     // IDC_CHK_OPT_REQUIRED
    CListCtrl   m_listGroup;       // IDC_LIST_OPT_GROUP
    CEdit       m_editItemName;    // IDC_EDIT_OPT_ITEM_NAME
    CEdit       m_editItemPrice;   // IDC_EDIT_OPT_ITEM_PRICE
    CListCtrl   m_listItem;        // IDC_LIST_OPT_ITEM
};