#include "pch.h"
#include "admin.h"
#include "afxdialogex.h"
#include "CBlacklistDlg.h"

IMPLEMENT_DYNAMIC(CBlacklistDlg, CDialogEx)

CBlacklistDlg::CBlacklistDlg(CWnd* pParent)
    : CDialogEx(IDD_TAB_BLACKLIST, pParent) {
}

CBlacklistDlg::~CBlacklistDlg() {}

void CBlacklistDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_BLACKLIST_ID, m_editBlacklistId);
    DDX_Control(pDX, IDC_EDIT_BLACKLIST_RSN, m_editBlacklistRsn);
    DDX_Control(pDX, IDC_BTN_BLACKLIST_ADD, m_btnBlacklistAdd);
    DDX_Control(pDX, IDC_LIST_BLACKLIST, m_listBlacklist);
    DDX_Control(pDX, IDC_BTN_BLACKLIST_DEL, m_btnBlacklistDel);
}

BOOL CBlacklistDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    InitListCtrl();
    UpdateButtonState();
    return TRUE;
}

void CBlacklistDlg::InitListCtrl()
{
    m_listBlacklist.SetExtendedStyle(
        m_listBlacklist.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    m_listBlacklist.InsertColumn(0, L"유저ID", LVCFMT_LEFT, 120);
    m_listBlacklist.InsertColumn(1, L"매장ID", LVCFMT_CENTER, 70);
    m_listBlacklist.InsertColumn(2, L"차단 사유", LVCFMT_LEFT, 180);
    m_listBlacklist.InsertColumn(3, L"차단 일시", LVCFMT_LEFT, 120);
}

void CBlacklistDlg::SetNetworkHelper(CNetworkHelper* pNet)
{
    m_pNet = pNet;
}

void CBlacklistDlg::UpdateButtonState()
{
    bool bSelected = (GetSelectedIndex() != -1);
    m_btnBlacklistDel.EnableWindow(bSelected);
}

int CBlacklistDlg::GetSelectedIndex()
{
    return m_listBlacklist.GetNextItem(-1, LVNI_SELECTED);
}

// =========================================================
// 차단 추가 버튼
// =========================================================
void CBlacklistDlg::OnBnClickedBtnBlacklistAdd()
{
    if (!m_pNet) return;

    CString strUserId, strReason;
    m_editBlacklistId.GetWindowText(strUserId);
    m_editBlacklistRsn.GetWindowText(strReason);

    if (strUserId.IsEmpty())
    {
        MessageBox(L"차단할 유저ID를 입력해주세요.", L"알림", MB_OK);
        return;
    }
    if (strReason.IsEmpty())
    {
        MessageBox(L"차단 사유를 입력해주세요.", L"알림", MB_OK);
        return;
    }

    if (MessageBox(
        L"해당 유저를 블랙리스트에 추가하시겠습니까?",
        L"차단 추가", MB_YESNO | MB_ICONWARNING) == IDYES)
    {
        m_editBlacklistId.GetWindowText(strUserId);
        m_editBlacklistRsn.GetWindowText(strReason);

        std::string userIdA = (const char*)CT2A(strUserId, CP_UTF8);
        std::string reasonA = (const char*)CT2A(strReason, CP_UTF8);

        json body;
        body["userId"] = userIdA;
        body["reason"] = reasonA;
        m_pNet->Send(CmdID::REQ_BLACKLIST_REQUEST, body);

        // 입력창 초기화
        m_editBlacklistId.SetWindowText(L"");
        m_editBlacklistRsn.SetWindowText(L"");
    }
}

// =========================================================
// 서버 응답 수신 (RES_BLACKLIST_REQUEST = 3121)
// =========================================================
void CBlacklistDlg::OnBlacklistResult(const json& resJson)
{
    if (resJson.value("status", 0) != 200)
    {
        MessageBox(L"처리에 실패했습니다.", L"오류", MB_ICONERROR);
        return;
    }

    // ✅ 응답에 목록이 포함된 경우 리스트 갱신
    if (!resJson.contains("blacklist")) return;

    m_listBlacklist.DeleteAllItems();

    auto toW = [](const std::string& s) -> CString {
        CString str = CA2W(s.c_str(), CP_UTF8);
        return str;
        };

    const auto& list = resJson["blacklist"];
    for (int i = 0; i < (int)list.size(); i++)
    {
        const auto& item = list[i];

        std::string userId = item.value("userId", "");
        int         storeId = item.value("storeId", 0);
        std::string reason = item.value("reason", "");
        std::string createdAt = item.value("createdAt", "");

        CString strStoreId;
        strStoreId.Format(L"%d", storeId);

        int nIdx = m_listBlacklist.InsertItem(i, toW(userId));
        m_listBlacklist.SetItemText(nIdx, 1, strStoreId);
        m_listBlacklist.SetItemText(nIdx, 2, toW(reason));
        m_listBlacklist.SetItemText(nIdx, 3, toW(createdAt));

        // userId 저장
        m_listBlacklist.SetItemData(nIdx, (DWORD_PTR)new std::string(userId));
    }

    UpdateButtonState();
}

// =========================================================
// 차단 해제 버튼
// =========================================================
void CBlacklistDlg::OnBnClickedBtnBlacklistDel()
{
    int nIdx = GetSelectedIndex();
    if (nIdx == -1) return;
    if (!m_pNet) return;

    auto* pUserId = reinterpret_cast<std::string*>(
        m_listBlacklist.GetItemData(nIdx));
    if (!pUserId) return;

    if (MessageBox(
        L"해당 유저의 차단을 해제하시겠습니까?",
        L"차단 해제", MB_YESNO | MB_ICONQUESTION) == IDYES)
    {
        json body;
        body["userId"] = *pUserId;
        body["action"] = "remove";  // 차단 해제
        m_pNet->Send(CmdID::REQ_BLACKLIST_REQUEST, body);

        // 로컬 리스트에서 제거
        delete pUserId;
        m_listBlacklist.DeleteItem(nIdx);
        UpdateButtonState();
    }
}

// =========================================================
// 리스트 선택 변경
// =========================================================
void CBlacklistDlg::OnLvnItemchangedListBlacklist(NMHDR* pNMHDR, LRESULT* pResult)
{
    (void)pNMHDR;
    UpdateButtonState();
    *pResult = 0;
}

BEGIN_MESSAGE_MAP(CBlacklistDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_BLACKLIST_ADD, &CBlacklistDlg::OnBnClickedBtnBlacklistAdd)
    ON_BN_CLICKED(IDC_BTN_BLACKLIST_DEL, &CBlacklistDlg::OnBnClickedBtnBlacklistDel)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_BLACKLIST,
        &CBlacklistDlg::OnLvnItemchangedListBlacklist)
END_MESSAGE_MAP()