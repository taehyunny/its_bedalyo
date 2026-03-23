// CTabMenuDlg.cpp: 구현 파일

#include "pch.h"
#include "MFC.h"
#include "afxdialogex.h"
#include "CTabMenuDlg.h"

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

    // TODO: 서버에서 받은 메뉴 데이터로 채우기
    // 현재는 임시 더미 데이터
    int nIdx = m_listMenu.InsertItem(0, L"떡볶이 2인세트");
    m_listMenu.SetItemText(nIdx, 1, L"18,000");
    m_listMenu.SetItemText(nIdx, 2, L"메인");
    m_listMenu.SetItemText(nIdx, 3, L"판매중");
    m_listMenu.SetItemText(nIdx, 4, L"O");

    nIdx = m_listMenu.InsertItem(1, L"순대 1개");
    m_listMenu.SetItemText(nIdx, 1, L"4,000");
    m_listMenu.SetItemText(nIdx, 2, L"사이드");
    m_listMenu.SetItemText(nIdx, 3, L"판매중");
    m_listMenu.SetItemText(nIdx, 4, L"X");

    nIdx = m_listMenu.InsertItem(2, L"라면 1개");
    m_listMenu.SetItemText(nIdx, 1, L"5,000");
    m_listMenu.SetItemText(nIdx, 2, L"메인");
    m_listMenu.SetItemText(nIdx, 3, L"품절");
    m_listMenu.SetItemText(nIdx, 4, L"X");
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
    // TODO: 메뉴 추가 다이얼로그 열기 -> REQ_MENU_EDIT 전송
    MessageBox(L"메뉴 추가 기능은 준비 중입니다.", L"알림", MB_OK);
}

// =========================================================================
// 메뉴 수정 버튼
// =========================================================================
void CTabMenuDlg::OnBnClickedBtnMenuEdit()
{
    int nIdx = GetSelectedIndex();
    if (nIdx == -1) return;

    CString strName = m_listMenu.GetItemText(nIdx, 0);
    // TODO: 선택된 메뉴 수정 다이얼로그 열기 -> REQ_MENU_EDIT 전송
    MessageBox(L"메뉴 수정 기능은 준비 중입니다.", L"알림", MB_OK);
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
        // TODO: REQ_MENU_EDIT (actionType: 2 삭제) 전송
        m_listMenu.DeleteItem(nIdx); // 임시로 로컬에서만 삭제
    }
}

// =========================================================================
// 품절처리 / 판매재개 버튼
// =========================================================================
void CTabMenuDlg::OnBnClickedBtnMenuSoldout()
{
    int nIdx = GetSelectedIndex();
    if (nIdx == -1) return;

    CString strStatus = m_listMenu.GetItemText(nIdx, 3);

    if (strStatus == L"품절")
    {
        // 판매재개
        m_listMenu.SetItemText(nIdx, 3, L"판매중");
        m_btnMenuSoldout.SetWindowText(L"품절처리");
        // TODO: REQ_MENU_SOLD_OUT 전송 (isSoldOut: 0)
    }
    else
    {
        // 품절처리
        m_listMenu.SetItemText(nIdx, 3, L"품절");
        m_btnMenuSoldout.SetWindowText(L"판매재개");
        // TODO: REQ_MENU_SOLD_OUT 전송 (isSoldOut: 1)
    }
}

BEGIN_MESSAGE_MAP(CTabMenuDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_MENU_ADD, &CTabMenuDlg::OnBnClickedBtnMenuAdd)
    ON_BN_CLICKED(IDC_BTN_MENU_EDIT, &CTabMenuDlg::OnBnClickedBtnMenuEdit)
    ON_BN_CLICKED(IDC_BTN_MENU_DELETE, &CTabMenuDlg::OnBnClickedBtnMenuDelete)
    ON_BN_CLICKED(IDC_BTN_MENU_SOLDOUT, &CTabMenuDlg::OnBnClickedBtnMenuSoldout)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_MENU, &CTabMenuDlg::OnLvnItemchangedListMenu)
END_MESSAGE_MAP()