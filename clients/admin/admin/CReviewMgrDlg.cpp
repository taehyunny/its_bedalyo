#include "pch.h"
#include "admin.h"
#include "afxdialogex.h"
#include "CReviewMgrDlg.h"

IMPLEMENT_DYNAMIC(CReviewMgrDlg, CDialogEx)

CReviewMgrDlg::CReviewMgrDlg(CWnd* pParent)
    : CDialogEx(IDD_TAB_REVIEW_MGR, pParent) {
}

CReviewMgrDlg::~CReviewMgrDlg() {}

void CReviewMgrDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_REVIEW_SEARCH, m_editReviewSearch);
    DDX_Control(pDX, IDC_BTN_REVIEW_SEARCH, m_btnReviewSearch);
    DDX_Control(pDX, IDC_LIST_REVIEW_MGR, m_listReviewMgr);
    DDX_Control(pDX, IDC_BTN_REVIEW_DELETE, m_btnReviewDelete);
}

BOOL CReviewMgrDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    InitListCtrl();
    UpdateButtonState();
    return TRUE;
}

void CReviewMgrDlg::InitListCtrl()
{
    m_listReviewMgr.SetExtendedStyle(
        m_listReviewMgr.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    m_listReviewMgr.InsertColumn(0, L"리뷰ID", LVCFMT_CENTER, 60);
    m_listReviewMgr.InsertColumn(1, L"유저ID", LVCFMT_LEFT, 90);
    m_listReviewMgr.InsertColumn(2, L"별점", LVCFMT_CENTER, 60);
    m_listReviewMgr.InsertColumn(3, L"내용", LVCFMT_LEFT, 180);
    m_listReviewMgr.InsertColumn(4, L"작성일", LVCFMT_LEFT, 110);
}

void CReviewMgrDlg::SetNetworkHelper(CNetworkHelper* pNet)
{
    m_pNet = pNet;
}

void CReviewMgrDlg::UpdateButtonState()
{
    bool bSelected = (GetSelectedIndex() != -1);
    m_btnReviewDelete.EnableWindow(bSelected);
}

int CReviewMgrDlg::GetSelectedIndex()
{
    return m_listReviewMgr.GetNextItem(-1, LVNI_SELECTED);
}

// =========================================================
// 검색 버튼 → 매장ID로 리뷰 목록 요청
// =========================================================
void CReviewMgrDlg::OnBnClickedBtnReviewSearch()
{
    if (!m_pNet) return;

    CString strStoreId;
    m_editReviewSearch.GetWindowText(strStoreId);
    if (strStoreId.IsEmpty())
    {
        MessageBox(L"매장ID를 입력해주세요.", L"알림", MB_OK);
        return;
    }

    json body;
    body["storeId"] = _ttoi(strStoreId);
    m_pNet->Send(CmdID::REQ_REVIEW_LIST, body);
}

// =========================================================
// 서버 리뷰 목록 수신 (RES_REVIEW_LIST = 2015)
// =========================================================
void CReviewMgrDlg::OnReviewListResult(const json& resJson)
{
    m_listReviewMgr.DeleteAllItems();

    if (resJson.value("status", 0) != 200)
    {
        MessageBox(L"리뷰 목록 조회에 실패했습니다.", L"오류", MB_ICONERROR);
        return;
    }

    auto toW = [](const std::string& s) -> CString {
        CString str = CA2W(s.c_str(), CP_UTF8);
        return str;
    };

    const auto& reviews = resJson["reviews"];
    for (int i = 0; i < (int)reviews.size(); i++)
    {
        const auto& rv = reviews[i];

        int         reviewId = rv.value("reviewId", 0);
        std::string userId = rv.value("userId", "");
        int         rating = rv.value("rating", 0);
        std::string content = rv.value("content", "");
        std::string createdAt = rv.value("createdAt", "");

        // 별점 ★로 변환
        CString strRating;
        for (int r = 0; r < rating; r++)  strRating += L"★";
        for (int r = rating; r < 5; r++)  strRating += L"☆";

        CString strReviewId;
        strReviewId.Format(L"%d", reviewId);

        int nIdx = m_listReviewMgr.InsertItem(i, strReviewId);
        m_listReviewMgr.SetItemText(nIdx, 1, toW(userId));
        m_listReviewMgr.SetItemText(nIdx, 2, strRating);
        m_listReviewMgr.SetItemText(nIdx, 3, toW(content));
        m_listReviewMgr.SetItemText(nIdx, 4, toW(createdAt));

        // reviewId 저장
        m_listReviewMgr.SetItemData(nIdx, (DWORD_PTR)reviewId);
    }

    UpdateButtonState();
}

// =========================================================
// 악성 리뷰 삭제 버튼
// =========================================================
void CReviewMgrDlg::OnBnClickedBtnReviewDelete()
{
    int nIdx = GetSelectedIndex();
    if (nIdx == -1) return;
    if (!m_pNet) return;

    int reviewId = (int)m_listReviewMgr.GetItemData(nIdx);

    if (MessageBox(L"해당 리뷰를 삭제하시겠습니까?\n삭제된 리뷰는 복구되지 않습니다.",
        L"악성 리뷰 삭제", MB_YESNO | MB_ICONWARNING) == IDYES)
    {
        json body;
        body["reviewId"] = reviewId;
        m_pNet->Send(CmdID::RES_REVIEW_DELETE_OK, body);

        // 로컬 리스트에서도 제거
        m_listReviewMgr.DeleteItem(nIdx);
        UpdateButtonState();
    }
}

// =========================================================
// 리스트 선택 변경
// =========================================================
void CReviewMgrDlg::OnLvnItemchangedListReviewMgr(NMHDR* pNMHDR, LRESULT* pResult)
{
    (void)pNMHDR;
    UpdateButtonState();
    *pResult = 0;
}

BEGIN_MESSAGE_MAP(CReviewMgrDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_REVIEW_SEARCH, &CReviewMgrDlg::OnBnClickedBtnReviewSearch)
    ON_BN_CLICKED(IDC_BTN_REVIEW_DELETE, &CReviewMgrDlg::OnBnClickedBtnReviewDelete)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_REVIEW_MGR,
        &CReviewMgrDlg::OnLvnItemchangedListReviewMgr)
END_MESSAGE_MAP()