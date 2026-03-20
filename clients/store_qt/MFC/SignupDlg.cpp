#include "pch.h"
#include "MFC.h"
#include "afxdialogex.h"
#include "SignupDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CSignupDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CSignupDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BtnIDCHECK, &CSignupDlg::OnBnClickedBtnIdCheck)
    ON_BN_CLICKED(IDC_BTNSTORECHECK, &CSignupDlg::OnBnClickedBtnStoreCheck)
    ON_BN_CLICKED(IDOK, &CSignupDlg::OnBnClickedBtnSignup)
    ON_BN_CLICKED(IDCANCEL, &CSignupDlg::OnBnClickedBtnCancel)
    ON_EN_CHANGE(IDC_EDIT_USER_ID, &CSignupDlg::OnChangeEditId)
    ON_EN_CHANGE(IDC_STOREID, &CSignupDlg::OnChangeEditStoreId)
    ON_MESSAGE(WM_PACKET_RECEIVED, &CSignupDlg::OnPacketReceived)
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

CSignupDlg::CSignupDlg(CNetworkHelper* pNet, CWnd* pParent)
    : CDialogEx(IDD_SIGNUP, pParent), m_pNet(pNet) {
}

CSignupDlg::~CSignupDlg() {}

// =========================================================================
// 컨트롤 바인딩
// =========================================================================
void CSignupDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);

    // ── USERS ────────────────────────────────────────────────────────────
    DDX_Control(pDX, IDC_EDIT_USER_ID, m_editId);
    DDX_Control(pDX, IDC_PWEdit, m_editPw);
    DDX_Control(pDX, IDC_REPWEdit, m_editPwConfirm);
    DDX_Control(pDX, IDC_NAMEEDIT, m_editName);
    DDX_Control(pDX, IDC_PHONE, m_editPhone);

    // ── STORES ───────────────────────────────────────────────────────────
    DDX_Control(pDX, IDC_STORENAME, m_editStoreName);
    DDX_Control(pDX, IDC_COMBO_CATEGORY, m_comboCategory);
    DDX_Control(pDX, IDC_EDIT_STOREADDRESS, m_editStoreAddress);
    DDX_Control(pDX, IDC_STOREID, m_editStoreId);

    // ── 버튼 / 상태 라벨 ─────────────────────────────────────────────────
    DDX_Control(pDX, IDC_BtnIDCHECK, m_btnIdCheck);
    DDX_Control(pDX, IDC_BTNSTORECHECK, m_btnStoreCheck);
    DDX_Control(pDX, IDC_IDCHECK, m_staticIdStatus);
    DDX_Control(pDX, IDC_STOREID_CHECK, m_staticStoreStatus);
}

BOOL CSignupDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    SetWindowText(L"ITS_Bedalyo - 사장님 회원가입");

    for (int i = 0; i < CATEGORY_COUNT; i++)
        m_comboCategory.AddString(CATEGORY_LIST[i]);
    m_comboCategory.SetCurSel(0);

    //GetDlgItem(IDOK)->EnableWindow(FALSE);
    return TRUE;
}

// =========================================================================
// ID 입력 변경 감지
// =========================================================================
void CSignupDlg::OnChangeEditId()
{
    if (m_idAvailable)
    {
        m_idAvailable = false;
        SetIdStatus(L"ID가 변경되었습니다. 다시 중복 확인하세요.", RGB(255, 140, 0));
        UpdateSignupButton();
    }
}

// =========================================================================
// 사업자번호 입력 변경 감지
// =========================================================================
void CSignupDlg::OnChangeEditStoreId()
{
    if (m_storeAvailable)
    {
        m_storeAvailable = false;
        SetStoreStatus(L"사업자번호가 변경되었습니다. 다시 조회하세요.", RGB(255, 140, 0));
        UpdateSignupButton();
    }
}

// =========================================================================
// ID 중복 확인 버튼
// REQ_AUTH_CHECK(1040) → RES_AUTH_CHECK(1041), check_type: "userId"
// =========================================================================
void CSignupDlg::OnBnClickedBtnIdCheck()
{
    CString strId;
    m_editId.GetWindowText(strId);
    strId.Trim();

    if (strId.IsEmpty())
    {
        SetIdStatus(L"ID를 입력하세요.", RGB(255, 0, 0));
        return;
    }
    if (m_waitingResponse) return;

    json body;
    body["check_type"] = "userId";
    body["userId"] = CT2A(strId);

    m_pNet->Send(CmdID::REQ_AUTH_CHECK, body);
    m_waitingResponse = true;
    m_btnIdCheck.EnableWindow(FALSE);
    SetIdStatus(L"확인 중...", RGB(0, 0, 255));
}

// =========================================================================
// 사업자번호 조회 버튼
// REQ_BUISNESS_NUM_CHECK(2006) → RES_BUISNESS_NUM_CHECK(2007)
// =========================================================================
void CSignupDlg::OnBnClickedBtnStoreCheck()
{
    CString strBizNum;
    m_editStoreId.GetWindowText(strBizNum);

    if (strBizNum.IsEmpty())
    {
        SetStoreStatus(L"사업자번호를 입력하세요.", RGB(255, 0, 0));
        return;
    }
    if (m_waitingResponse) return;

    json body;
    body["biz_num"] = CT2A(strBizNum);

    m_pNet->Send(CmdID::REQ_BUISNESS_NUM_CHECK, body);
    m_waitingResponse = true;
    m_btnStoreCheck.EnableWindow(FALSE);
    SetStoreStatus(L"조회 중...", RGB(0, 0, 255));
}

// =========================================================================
// 완료(가입하기) 버튼
// REQ_SIGNUP(1020) → RES_SIGNUP(1021)
// =========================================================================
void CSignupDlg::OnBnClickedBtnSignup()
{
    if (!ValidateInputs())  return;
    if (m_waitingResponse)  return;

    CString strId, strPw, strName, strPhone;
    CString strStoreName, strStoreAddress, strBizNum;
    int nCategory = m_comboCategory.GetCurSel();

    m_editId.GetWindowText(strId);
    m_editPw.GetWindowText(strPw);
    m_editName.GetWindowText(strName);
    m_editPhone.GetWindowText(strPhone);
    m_editStoreName.GetWindowText(strStoreName);
    m_editStoreAddress.GetWindowText(strStoreAddress);
    m_editStoreId.GetWindowText(strBizNum);

    CString strCategory;
    m_comboCategory.GetLBText(nCategory, strCategory);

    json body;
    // ── USERS (공통) ──────────────────────────────────────────────────────
    body["userId"] = CT2A(strId, CP_UTF8);
    body["password"] = CT2A(strPw, CP_UTF8);
    body["userName"] = CT2A(strName, CP_UTF8);
    body["phoneNumber"] = CT2A(strPhone, CP_UTF8);
    body["role"] = 1;

    // ── 고객 전용 (Role 0) - 사장님이라 빈값으로 채움 ─────────────────────
    body["address"] = "";

    // ── 사장님 & 매장 전용 (Role 1) ───────────────────────────────────────
    body["businessNumber"] = CT2A(strBizNum, CP_UTF8);  // ✅ storeId → businessNumber
    body["accountNumber"] = "";                               // ✅ 추가 (일단 빈값)
    body["storeName"] = CT2A(strStoreName, CP_UTF8);
    body["category"] = CT2A(strCategory, CP_UTF8);
    body["storeAddress"] = CT2A(strStoreAddress, CP_UTF8);
    m_pNet->Send(CmdID::REQ_SIGNUP, body);

    m_waitingResponse = true;
    GetDlgItem(IDOK)->EnableWindow(FALSE);
}

// =========================================================================
// 입력값 검증
// =========================================================================
bool CSignupDlg::ValidateInputs()
{
    //if (!m_idAvailable)
    //{
    //    SetIdStatus(L"ID 중복 확인을 먼저 해주세요.", RGB(255, 0, 0));
    //    return false;
    //}
    //if (!m_storeAvailable)
    //{
    //    SetStoreStatus(L"사업자번호 조회를 먼저 해주세요.", RGB(255, 0, 0));
    //    return false;
    //}

    CString strPw, strPwConfirm, strName, strPhone, strStoreName, strStoreAddress;
    m_editPw.GetWindowText(strPw);
    m_editPwConfirm.GetWindowText(strPwConfirm);
    m_editName.GetWindowText(strName);
    m_editPhone.GetWindowText(strPhone);
    m_editStoreName.GetWindowText(strStoreName);
    m_editStoreAddress.GetWindowText(strStoreAddress);

    if (strPw.IsEmpty() || strName.IsEmpty() ||
        strPhone.IsEmpty() || strStoreName.IsEmpty() || strStoreAddress.IsEmpty())
    {
        MessageBox(L"모든 필수 항목을 입력하세요.", L"알림", MB_OK);
        return false;
    }
    if (strPw != strPwConfirm)
    {
        MessageBox(L"비밀번호가 일치하지 않습니다.", L"알림", MB_OK);
        return false;
    }
    if (strPw.GetLength() < 4)
    {
        MessageBox(L"비밀번호는 4자 이상이어야 합니다.", L"알림", MB_OK);
        return false;
    }

    return true;
}

// =========================================================================
// 서버 응답 처리
// - ID 확인   : RES_AUTH_CHECK → check_type 으로 분기
// - 사업자번호 : RES_BUISNESS_NUM_CHECK → CmdID 로 직접 분기
// - 회원가입   : RES_SIGNUP → CmdID 로 직접 분기
// =========================================================================
LRESULT CSignupDlg::OnPacketReceived(WPARAM /*wParam*/, LPARAM lParam)
{
    auto* pkt = reinterpret_cast<ReceivedPacket*>(lParam);
    if (!pkt) return 0;

    m_waitingResponse = false;

    try
    {
        json resJson = json::parse(pkt->body);
        bool success = resJson.value("success", false);

        // ── ID 중복 확인 응답 ────────────────────────────────────────────
        if (pkt->cmdId == CmdID::RES_AUTH_CHECK)
        {
            m_btnIdCheck.EnableWindow(TRUE);
            m_idAvailable = success;
            if (success)
                SetIdStatus(L"사용 가능한 ID입니다.", RGB(0, 150, 0));
            else
                SetIdStatus(L"이미 사용 중인 ID입니다.", RGB(255, 0, 0));
            UpdateSignupButton();
        }
        // ── 사업자번호 조회 응답 ─────────────────────────────────────────
        else if (pkt->cmdId == CmdID::RES_BUISNESS_NUM_CHECK)
        {
            m_btnStoreCheck.EnableWindow(TRUE);
            m_storeAvailable = success;
            if (success)
                SetStoreStatus(L"사용 가능한 사업자번호입니다.", RGB(0, 150, 0));
            else
                SetStoreStatus(L"이미 등록된 사업자번호입니다.", RGB(255, 0, 0));
            UpdateSignupButton();
        }
        // ── 회원가입 응답 ────────────────────────────────────────────────
        else if (pkt->cmdId == CmdID::RES_SIGNUP)
        {
            GetDlgItem(IDOK)->EnableWindow(TRUE);
            if (success)
            {
                MessageBox(L"회원가입이 완료되었습니다.", L"가입 완료", MB_OK);
                EndDialog(IDOK);
            }
            else
            {
                std::string msg = resJson.value("message", "회원가입에 실패했습니다.");
                MessageBox(CA2W(msg.c_str()), L"가입 실패", MB_ICONWARNING);
            }
        }
    }
    catch (...) {}

    delete pkt;
    return 0;
}

// =========================================================================
// 완료 버튼 활성화 (ID + 사업자번호 둘 다 확인 완료 시)
// =========================================================================
void CSignupDlg::UpdateSignupButton()
{
    GetDlgItem(IDOK)->EnableWindow(
        (m_idAvailable && m_storeAvailable) ? TRUE : FALSE);
}

// =========================================================================
// 상태 메시지 표시
// =========================================================================
void CSignupDlg::SetIdStatus(const CString& msg, COLORREF color)
{
    m_idStatusColor = color;
    m_staticIdStatus.SetWindowText(msg);
    m_staticIdStatus.Invalidate();
}

void CSignupDlg::SetStoreStatus(const CString& msg, COLORREF color)
{
    m_storeStatusColor = color;
    m_staticStoreStatus.SetWindowText(msg);
    m_staticStoreStatus.Invalidate();
}

// =========================================================================
// 색상 처리
// =========================================================================
HBRUSH CSignupDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

    if (pWnd->GetDlgCtrlID() == IDC_IDCHECK)
    {
        pDC->SetTextColor(m_idStatusColor);
        pDC->SetBkMode(TRANSPARENT);
    }
    else if (pWnd->GetDlgCtrlID() == IDC_STOREID_CHECK)
    {
        pDC->SetTextColor(m_storeStatusColor);
        pDC->SetBkMode(TRANSPARENT);
    }

    return hbr;
}

void CSignupDlg::OnBnClickedBtnCancel()
{
    EndDialog(IDCANCEL);
}