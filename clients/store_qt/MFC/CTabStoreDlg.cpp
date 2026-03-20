// CTabStoreDlg.cpp: 구현 파일

#include "pch.h"
#include "MFC.h"
#include "afxdialogex.h"
#include "CTabStoreDlg.h"

IMPLEMENT_DYNAMIC(CTabStoreDlg, CDialogEx)

CTabStoreDlg::CTabStoreDlg(CWnd* pParent)
    : CDialogEx(IDD_TAB_STORE, pParent)
{
}

CTabStoreDlg::~CTabStoreDlg()
{
}

// =========================================================================
// 컨트롤 바인딩
// =========================================================================
void CTabStoreDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);

    // ── 매장 정보 ─────────────────────────────────────────────────────────
    DDX_Control(pDX, IDC_EDIT_STORE_NAME, m_editStoreName);
    DDX_Control(pDX, IDC_COMBO_CATEGORY, m_comboCategory);
    DDX_Control(pDX, IDC_EDIT_STORE_ADDRESS, m_editStoreAddress);
    DDX_Control(pDX, IDC_EDIT_STORE_BIZNUM, m_editStoreBiznum);
    DDX_Control(pDX, IDC_EDIT_COOK_TIME, m_editCookTime);
    DDX_Control(pDX, IDC_EDIT_MIN_ORDER, m_editMinOrder);
    DDX_Control(pDX, IDC_EDIT_OPEN_TIME, m_editOpenTime);
    DDX_Control(pDX, IDC_EDIT_CLOSE_TIME, m_editCloseTime);

    // ── 사장님 정보 ───────────────────────────────────────────────────────
    DDX_Control(pDX, IDC_EDIT_OWNER_NAME, m_editOwnerName);
    DDX_Control(pDX, IDC_EDIT_OWNER_PHONE, m_editOwnerPhone);
    DDX_Control(pDX, IDC_EDIT_ACCOUNT, m_editAccount);
    DDX_Control(pDX, IDC_STATIC_APPROVAL, m_staticApproval);

    // ── 버튼 ──────────────────────────────────────────────────────────────
    DDX_Control(pDX, IDC_BTN_EDIT_NAME, m_btnEditName);
    DDX_Control(pDX, IDC_BTN_EDIT_CATEGORY, m_btnEditCategory);
    DDX_Control(pDX, IDC_BTN_EDIT_ADDRESS, m_btnEditAddress);
    DDX_Control(pDX, IDC_BTN_EDIT_OWNER_NAME, m_btnEditOwnerName);
    DDX_Control(pDX, IDC_BTN_EDIT_OWNER_PHONE, m_btnEditOwnerPhone);
    DDX_Control(pDX, IDC_BTN_EDIT_ACCOUNT, m_btnEditAccount);
    DDX_Control(pDX, IDC_BTN_STORE_OPEN, m_btnStoreOpen);
    DDX_Control(pDX, IDC_BTN_STORE_CLOSE, m_btnStoreClose);
}

// =========================================================================
// 초기화
// =========================================================================
BOOL CTabStoreDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // 카테고리 콤보박스 항목 추가
    m_comboCategory.AddString(L"한식");
    m_comboCategory.AddString(L"중식");
    m_comboCategory.AddString(L"양식");
    m_comboCategory.AddString(L"돈까스/일식");
    m_comboCategory.AddString(L"치킨");
    m_comboCategory.AddString(L"피자");
    m_comboCategory.AddString(L"햄버거");
    m_comboCategory.AddString(L"족발/보쌈");
    m_comboCategory.AddString(L"도시락");
    m_comboCategory.AddString(L"초밥/회");
    m_comboCategory.SetCurSel(0);

    // 모든 Edit 초기 비활성화 (수정 버튼 누를 때 활성화)
    m_editStoreName.SetReadOnly(TRUE);
    m_editStoreAddress.SetReadOnly(TRUE);
    m_editOwnerName.SetReadOnly(TRUE);
    m_editOwnerPhone.SetReadOnly(TRUE);
    m_editAccount.SetReadOnly(TRUE);

    // TODO: 서버에서 받은 데이터로 채우기
    // 현재는 임시 빈값 표시
    m_staticApproval.SetWindowText(L"대기");

    //  스크롤 범위 설정
    CRect rcClient;
    GetClientRect(&rcClient);

    SCROLLINFO si;
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_RANGE | SIF_PAGE;
    si.nMin = 0;
    si.nMax = 600;          // 전체 컨텐츠 높이 (컨트롤 수에 맞게 조정)
    si.nPage = rcClient.Height();
    SetScrollInfo(SB_VERT, &si);

    return TRUE;
}
void CTabStoreDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    int nDelta = 0;
    int nMaxPos = 600 - (int)GetScrollLimit(SB_VERT);

    switch (nSBCode)
    {
    case SB_LINEUP:   nDelta = -10;  break;
    case SB_LINEDOWN: nDelta = 10;  break;
    case SB_PAGEUP:   nDelta = -50;  break;
    case SB_PAGEDOWN: nDelta = 50;  break;
    case SB_THUMBTRACK: nDelta = (int)nPos - m_nScrollPos; break;
    default: return;
    }

    nDelta = max(-m_nScrollPos, min(nDelta, nMaxPos - m_nScrollPos));
    if (nDelta == 0) return;

    m_nScrollPos += nDelta;
    SetScrollPos(SB_VERT, m_nScrollPos);
    ScrollWindow(0, -nDelta);

    CDialogEx::OnVScroll(nSBCode, nPos, pScrollBar);
}

BOOL CTabStoreDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    OnVScroll(zDelta < 0 ? SB_LINEDOWN : SB_LINEUP, 0, nullptr);
    return TRUE;
}

// =========================================================================
// 원본값 백업 (저장 버튼 누르기 전 / 취소 시 복원용)
// =========================================================================
void CTabStoreDlg::BackupValues()
{
    m_editStoreName.GetWindowText(m_bakStoreName);
    m_editStoreAddress.GetWindowText(m_bakStoreAddress);
    m_editCookTime.GetWindowText(m_bakCookTime);
    m_editMinOrder.GetWindowText(m_bakMinOrder);
    m_editOpenTime.GetWindowText(m_bakOpenTime);
    m_editCloseTime.GetWindowText(m_bakCloseTime);
    m_editOwnerName.GetWindowText(m_bakOwnerName);
    m_editOwnerPhone.GetWindowText(m_bakOwnerPhone);
    m_editAccount.GetWindowText(m_bakAccount);
    m_bakCategory = m_comboCategory.GetCurSel();
}

void CTabStoreDlg::RestoreValues()
{
    m_editStoreName.SetWindowText(m_bakStoreName);
    m_editStoreAddress.SetWindowText(m_bakStoreAddress);
    m_editCookTime.SetWindowText(m_bakCookTime);
    m_editMinOrder.SetWindowText(m_bakMinOrder);
    m_editOpenTime.SetWindowText(m_bakOpenTime);
    m_editCloseTime.SetWindowText(m_bakCloseTime);
    m_editOwnerName.SetWindowText(m_bakOwnerName);
    m_editOwnerPhone.SetWindowText(m_bakOwnerPhone);
    m_editAccount.SetWindowText(m_bakAccount);
    m_comboCategory.SetCurSel(m_bakCategory);

    // 복원 후 다시 읽기 전용으로
    m_editStoreName.SetReadOnly(TRUE);
    m_editStoreAddress.SetReadOnly(TRUE);
    m_editOwnerName.SetReadOnly(TRUE);
    m_editOwnerPhone.SetReadOnly(TRUE);
    m_editAccount.SetReadOnly(TRUE);
}

// =========================================================================
// 수정 버튼 핸들러 - 클릭 시 해당 Edit 활성화
// =========================================================================
void CTabStoreDlg::OnBnClickedBtnEditName()
{
    BackupValues();
    m_editStoreName.SetReadOnly(FALSE);
    m_editStoreName.SetFocus();
}

void CTabStoreDlg::OnBnClickedBtnEditCategory()
{
    BackupValues();
    m_comboCategory.EnableWindow(TRUE);
}

void CTabStoreDlg::OnBnClickedBtnEditAddress()
{
    BackupValues();
    m_editStoreAddress.SetReadOnly(FALSE);
    m_editStoreAddress.SetFocus();
}

void CTabStoreDlg::OnBnClickedBtnEditOwnerName()
{
    BackupValues();
    m_editOwnerName.SetReadOnly(FALSE);
    m_editOwnerName.SetFocus();
}

void CTabStoreDlg::OnBnClickedBtnEditOwnerPhone()
{
    BackupValues();
    m_editOwnerPhone.SetReadOnly(FALSE);
    m_editOwnerPhone.SetFocus();
}

void CTabStoreDlg::OnBnClickedBtnEditAccount()
{
    BackupValues();
    m_editAccount.SetReadOnly(FALSE);
    m_editAccount.SetFocus();
}

// =========================================================================
// 영업 상태 버튼
// =========================================================================
void CTabStoreDlg::OnBnClickedBtnStoreOpen()
{
    // TODO: REQ_STORE_STATUS_SET 전송 (status: 1 영업중)
    MessageBox(L"영업을 시작합니다.", L"영업 상태", MB_OK);
}

void CTabStoreDlg::OnBnClickedBtnStoreClose()
{
    // TODO: REQ_STORE_STATUS_SET 전송 (status: 0 영업종료)
    MessageBox(L"영업을 종료합니다.", L"영업 상태", MB_OK);
}

// =========================================================================
// 저장 버튼 - 변경사항 서버에 전송
// =========================================================================
void CTabStoreDlg::OnBnClickedBtnSave()
{
    // TODO: 변경된 값 서버에 REQ_PROFILE_UPDATE 전송
    MessageBox(L"저장되었습니다.", L"저장", MB_OK);

    // 저장 후 다시 읽기 전용으로
    m_editStoreName.SetReadOnly(TRUE);
    m_editStoreAddress.SetReadOnly(TRUE);
    m_editOwnerName.SetReadOnly(TRUE);
    m_editOwnerPhone.SetReadOnly(TRUE);
    m_editAccount.SetReadOnly(TRUE);
}

// =========================================================================
// 취소 버튼 - 원본값으로 복원
// =========================================================================
void CTabStoreDlg::OnBnClickedBtnCancel()
{
    RestoreValues();
}

BEGIN_MESSAGE_MAP(CTabStoreDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_EDIT_NAME, &CTabStoreDlg::OnBnClickedBtnEditName)
    ON_BN_CLICKED(IDC_BTN_EDIT_CATEGORY, &CTabStoreDlg::OnBnClickedBtnEditCategory)
    ON_BN_CLICKED(IDC_BTN_EDIT_ADDRESS, &CTabStoreDlg::OnBnClickedBtnEditAddress)
    ON_BN_CLICKED(IDC_BTN_EDIT_OWNER_NAME, &CTabStoreDlg::OnBnClickedBtnEditOwnerName)
    ON_BN_CLICKED(IDC_BTN_EDIT_OWNER_PHONE, &CTabStoreDlg::OnBnClickedBtnEditOwnerPhone)
    ON_BN_CLICKED(IDC_BTN_EDIT_ACCOUNT, &CTabStoreDlg::OnBnClickedBtnEditAccount)
    ON_BN_CLICKED(IDC_BTN_STORE_OPEN, &CTabStoreDlg::OnBnClickedBtnStoreOpen)
    ON_BN_CLICKED(IDC_BTN_STORE_CLOSE, &CTabStoreDlg::OnBnClickedBtnStoreClose)
    ON_BN_CLICKED(IDC_BTN_SAVE, &CTabStoreDlg::OnBnClickedBtnSave)
    ON_BN_CLICKED(IDC_BTN_CANCEL, &CTabStoreDlg::OnBnClickedBtnCancel)
    ON_WM_VSCROLL()
    ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()