// CTabStoreDlg.cpp: 구현 파일

#include "pch.h"
#include "MFC.h"
#include "afxdialogex.h"
#include "CTabStoreDlg.h"
#include "json.hpp"

IMPLEMENT_DYNAMIC(CTabStoreDlg, CDialogEx)

CTabStoreDlg::CTabStoreDlg(CWnd* pParent)
    : CDialogEx(IDD_TAB_STORE, pParent)
{
}

CTabStoreDlg::~CTabStoreDlg()
{
}

void CTabStoreDlg::SetUIMode(BOOL bEditMode)
{
    BOOL bReadOnly = !bEditMode;

    // --- [그룹 1] 수정 버튼을 눌러야 활성화되는 항목들 ---
    m_editStoreName.SetReadOnly(bReadOnly);
    m_editStoreAddress.SetReadOnly(bReadOnly);
    m_editOwnerName.SetReadOnly(bReadOnly);
    m_editOwnerPhone.SetReadOnly(bReadOnly);
    m_editAccount.SetReadOnly(bReadOnly);
    m_comboCategory.EnableWindow(bEditMode);

    // --- [그룹 2] 항상 활성화 (자주 바뀌는 설정) ---
    // 이 항목들은 bReadOnly 값과 상관없이 항상 FALSE(입력가능) / TRUE(활성)
    m_editCookTime.SetReadOnly(FALSE);
    m_editMinOrder.SetReadOnly(FALSE);
    m_editOpenTime.SetReadOnly(FALSE);
    m_editCloseTime.SetReadOnly(FALSE);
    m_btnStoreOpen.EnableWindow(TRUE);
    m_btnStoreClose.EnableWindow(TRUE);

    // --- [그룹 3] 항상 읽기 전용 (관리자 문의 대상) ---
    m_editStoreBiznum.SetReadOnly(TRUE);
    m_staticApproval.EnableWindow(FALSE); // 승인 상태 등

    // --- [그룹 4] 수정 버튼 자체 제어 ---
    // 수정 중일 때는 다른 '수정' 버튼을 못 누르게 막음
    m_btnEditName.EnableWindow(bReadOnly);
    m_btnEditCategory.EnableWindow(bReadOnly);
    m_btnEditAddress.EnableWindow(bReadOnly);
    m_btnEditOwnerName.EnableWindow(bReadOnly);
    m_btnEditOwnerPhone.EnableWindow(bReadOnly);
    m_btnEditAccount.EnableWindow(bReadOnly);
}
// =========================================================================
// 컨트롤 바인딩
// =========================================================================
void CTabStoreDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_EDIT_STORE_NAME, m_editStoreName);
    DDX_Control(pDX, IDC_COMBO_CATEGORY, m_comboCategory);
    DDX_Control(pDX, IDC_EDIT_STORE_ADDRESS, m_editStoreAddress);
    DDX_Control(pDX, IDC_EDIT_STORE_BIZNUM, m_editStoreBiznum);
    DDX_Control(pDX, IDC_EDIT_COOK_TIME, m_editCookTime);
    DDX_Control(pDX, IDC_EDIT_MIN_ORDER, m_editMinOrder);
    DDX_Control(pDX, IDC_EDIT_OPEN_TIME, m_editOpenTime);
    DDX_Control(pDX, IDC_EDIT_CLOSE_TIME, m_editCloseTime);

    DDX_Control(pDX, IDC_EDIT_OWNER_NAME, m_editOwnerName);
    DDX_Control(pDX, IDC_EDIT_OWNER_PHONE, m_editOwnerPhone);
    DDX_Control(pDX, IDC_EDIT_ACCOUNT, m_editAccount);
    DDX_Control(pDX, IDC_STATIC_APPROVAL, m_staticApproval);

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

    // ✅ 더미 데이터 제거 - SetStoreInfo()에서 채워짐

    // 스크롤 범위 설정
    CRect rcClient;
    GetClientRect(&rcClient);

    SCROLLINFO si;
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_RANGE | SIF_PAGE;
    si.nMin = 0;
    si.nMax = 600;
    si.nPage = rcClient.Height();
    SetScrollInfo(SB_VERT, &si);

    return TRUE;
}

// =========================================================================
// ✅ 서버에서 받은 데이터로 컨트롤 채우기
// =========================================================================
void CTabStoreDlg::SetStoreInfo(
    int storeId,
    CNetworkHelper* pNet,
    const CString& storeName, const CString& category,
    const CString& storeAddress, const CString& bizNum,
    const CString& cookTime, const CString& minOrder,
    const CString& openTime, const CString& closeTime,
    const CString& ownerName, const CString& ownerPhone,
    const CString& accountNumber, const CString& approvalStatus)
{
    m_storeId = storeId;
    m_pNet = pNet;  
    m_editStoreName.SetWindowText(storeName);
    m_editStoreAddress.SetWindowText(storeAddress);
    m_editStoreBiznum.SetWindowText(bizNum);
    m_editCookTime.SetWindowText(cookTime);
    m_editMinOrder.SetWindowText(minOrder);
    m_editOpenTime.SetWindowText(openTime);
    m_editCloseTime.SetWindowText(closeTime);
    m_editOwnerName.SetWindowText(ownerName);
    m_editOwnerPhone.SetWindowText(ownerPhone);
    m_editAccount.SetWindowText(accountNumber);
    m_staticApproval.SetWindowText(approvalStatus);

    // 카테고리 콤보박스 선택
    int nCount = m_comboCategory.GetCount();
    for (int i = 0; i < nCount; i++)
    {
        CString str;
        m_comboCategory.GetLBText(i, str);
        if (str == category)
        {
            m_comboCategory.SetCurSel(i);
            break;
        }
    }

    // 초기값을 백업해둠 (취소 시 원래대로 복원)
    BackupValues();
}

// =========================================================================
// 스크롤
// =========================================================================
void CTabStoreDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    int nDelta = 0;
    int nMaxPos = 600 - (int)GetScrollLimit(SB_VERT);

    switch (nSBCode)
    {
    case SB_LINEUP:     nDelta = -10; break;
    case SB_LINEDOWN:   nDelta = 10; break;
    case SB_PAGEUP:     nDelta = -50; break;
    case SB_PAGEDOWN:   nDelta = 50; break;
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
    (void)nFlags;
    (void)pt;
    OnVScroll(zDelta < 0 ? SB_LINEDOWN : SB_LINEUP, 0, nullptr);
    return TRUE;
}

// =========================================================================
// 원본값 백업 / 복원
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

    m_editStoreName.SetReadOnly(TRUE);
    m_editStoreAddress.SetReadOnly(TRUE);
    m_editOwnerName.SetReadOnly(TRUE);
    m_editOwnerPhone.SetReadOnly(TRUE);
    m_editAccount.SetReadOnly(TRUE);
}

// =========================================================================
// 수정 버튼 핸들러
// =========================================================================
void CTabStoreDlg::OnBnClickedBtnEditName()
{
    m_editStoreName.SetReadOnly(FALSE);
    m_editStoreName.SetFocus();
}

void CTabStoreDlg::OnBnClickedBtnEditCategory()
{
    m_comboCategory.EnableWindow(TRUE);
}

void CTabStoreDlg::OnBnClickedBtnEditAddress()
{
    m_editStoreAddress.SetReadOnly(FALSE);
    m_editStoreAddress.SetFocus();
}

void CTabStoreDlg::OnBnClickedBtnEditOwnerName()
{
    m_editOwnerName.SetReadOnly(FALSE);
    m_editOwnerName.SetFocus();
}

void CTabStoreDlg::OnBnClickedBtnEditOwnerPhone()
{
    m_editOwnerPhone.SetReadOnly(FALSE);
    m_editOwnerPhone.SetFocus();
}

void CTabStoreDlg::OnBnClickedBtnEditAccount()
{
    m_editAccount.SetReadOnly(FALSE);
    m_editAccount.SetFocus();
}

// =========================================================================
// 영업 상태 버튼
// =========================================================================
void CTabStoreDlg::OnBnClickedBtnStoreOpen()
{
    // TODO: REQ_STORE_STATUS_SET 전송 (status: 1)
    MessageBox(L"영업을 시작합니다.", L"영업 상태", MB_OK);
}

void CTabStoreDlg::OnBnClickedBtnStoreClose()
{
    // TODO: REQ_STORE_STATUS_SET 전송 (status: 0)
    MessageBox(L"영업을 종료합니다.", L"영업 상태", MB_OK);
}

// =========================================================================
// 저장 / 취소 버튼
// =========================================================================
void CTabStoreDlg::OnBnClickedBtnSave()
{
    // 1. 변경된 값을 담을 JSON 객체 생성
    nlohmann::json updateBody;

    // 2. 각 필드별로 백업값(원본)과 현재 입력값을 비교 (Dirty Check)
    CString currentVal;

    m_editStoreName.GetWindowText(currentVal);
    if (currentVal != m_bakStoreName)
        updateBody["storeName"] = CT2A(currentVal, CP_UTF8).m_psz;

    int curSel = m_comboCategory.GetCurSel();
    if (curSel != m_bakCategory) {
        CString strCat;
        m_comboCategory.GetLBText(curSel, strCat);
        updateBody["category"] = CT2A(strCat, CP_UTF8).m_psz;
    }

    m_editStoreAddress.GetWindowText(currentVal);
    if (currentVal != m_bakStoreAddress)
        updateBody["storeAddress"] = CT2A(currentVal, CP_UTF8).m_psz;

    m_editCookTime.GetWindowText(currentVal);
    if (currentVal != m_bakCookTime)
        updateBody["cookTime"] = CT2A(currentVal, CP_UTF8).m_psz;

    m_editMinOrder.GetWindowText(currentVal);
    if (currentVal != m_bakMinOrder)
        updateBody["minOrderAmount"] = _ttoi(currentVal); // ✅ minOrderPrice → minOrderAmount

    m_editOpenTime.GetWindowText(currentVal);
    if (currentVal != m_bakOpenTime)
        updateBody["openTime"] = CT2A(currentVal, CP_UTF8).m_psz;

    m_editCloseTime.GetWindowText(currentVal);
    if (currentVal != m_bakCloseTime)
        updateBody["closeTime"] = CT2A(currentVal, CP_UTF8).m_psz;

    m_editOwnerName.GetWindowText(currentVal);
    if (currentVal != m_bakOwnerName)
        updateBody["ownerName"] = CT2A(currentVal, CP_UTF8).m_psz;

    m_editOwnerPhone.GetWindowText(currentVal);
    if (currentVal != m_bakOwnerPhone)
        updateBody["ownerPhone"] = CT2A(currentVal, CP_UTF8).m_psz;

    m_editAccount.GetWindowText(currentVal);
    if (currentVal != m_bakAccount)
        updateBody["accountNumber"] = CT2A(currentVal, CP_UTF8).m_psz;

    // 3. 서버 전송
    if (!updateBody.empty())
    {
        updateBody["storeId"] = m_storeId; // ✅ 주석 해제 - 어떤 매장인지 서버에 알려야 함

        m_pNet->Send(CmdID::REQ_STORE_INFO_UPDATE, updateBody); // ✅ 실제 전송

        // ✅ BackupValues() 제거 → 서버 성공 응답(OnStoreUpdateSuccess)에서 처리
        // ✅ MessageBox 제거 → 서버 응답 후 띄워야 함
    }
    else
    {
        MessageBox(L"변경사항이 없습니다.", L"알림", MB_OK | MB_ICONINFORMATION);
        SetUIMode(FALSE);
    }
}

void CTabStoreDlg::OnStoreUpdateSuccess() {
    BackupValues();  // ✅ 여기서 갱신
    SetUIMode(FALSE);
    MessageBox(L"저장되었습니다.", L"알림", MB_OK);
}
void CTabStoreDlg::OnBnClickedBtnCancel()
{
    RestoreValues();
    SetUIMode(FALSE);
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