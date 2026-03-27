#pragma once
#include "afxdialogex.h"
#include "json.hpp"
#include "NetworkHelper.h"

class CTabMenuDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CTabMenuDlg)

public:
    CTabMenuDlg(CWnd* pParent = nullptr);
    virtual ~CTabMenuDlg();
    void LoadMenuList(); // 서버에 메뉴 목록 요청
    void SetMenuList(const nlohmann::json& menuArray); // 서버 응답으로 리스트 채우기
    void SetMenuInfo(int storeId, CNetworkHelper* pNet);

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_TAB_MENU };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    virtual void OnCancel() {} // ESC 키 무시

    // ── 버튼 핸들러 ──────────────────────────────────────────
    afx_msg void OnBnClickedBtnMenuAdd();       // 메뉴 추가
    afx_msg void OnBnClickedBtnMenuEdit();      // 메뉴 수정
    afx_msg void OnBnClickedBtnMenuDelete();    // 메뉴 삭제
    afx_msg void OnBnClickedBtnMenuSoldout();   // 품절처리 / 판매재개

    // ── List Control 핸들러 ──────────────────────────────────
    afx_msg void OnLvnItemchangedListMenu(NMHDR* pNMHDR, LRESULT* pResult);

    DECLARE_MESSAGE_MAP()

private:
    // ── 컨트롤 바인딩 ────────────────────────────────────────
    CListCtrl   m_listMenu;            // IDC_LIST_MENU
    CButton     m_btnMenuAdd;          // IDC_BTN_MENU_ADD
    CButton     m_btnMenuEdit;         // IDC_BTN_MENU_EDIT
    CButton     m_btnMenuDelete;       // IDC_BTN_MENU_DELETE
    CButton     m_btnMenuSoldout;      // IDC_BTN_MENU_SOLDOUT
    int              m_storeId = 0;
    CNetworkHelper* m_pNet = nullptr;
    // ── 내부 함수 ────────────────────────────────────────────
    void InitListCtrl();               // List Control 컬럼 초기화
    void UpdateButtonState();          // 선택 항목 유무에 따라 버튼 활성화/비활성화
    int  GetSelectedIndex();           // 현재 선택된 항목 인덱스 반환
};