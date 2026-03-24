// CTabReviewDlg.cpp: 구현 파일

#include "pch.h"
#include "MFC.h"
#include "afxdialogex.h"
#include "CTabReviewDlg.h"

IMPLEMENT_DYNAMIC(CTabReviewDlg, CDialogEx)

CTabReviewDlg::CTabReviewDlg(CWnd* pParent)
    : CDialogEx(IDD_TAB_REVIEW, pParent)
{
}

CTabReviewDlg::~CTabReviewDlg()
{
}

// =========================================================================
// 컨트롤 바인딩
// =========================================================================
void CTabReviewDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_REVIEW, m_listReview);
    DDX_Control(pDX, IDC_EDIT_REPLY, m_editReply);
    DDX_Control(pDX, IDC_BTN_REPLY_SUBMIT, m_btnReplySubmit);
}

// =========================================================================
// 초기화
// =========================================================================
BOOL CTabReviewDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    InitListCtrl();

    // 초기에는 답글 버튼 비활성화 (리뷰 선택 후 활성화)
    m_btnReplySubmit.EnableWindow(FALSE);

    return TRUE;
}

// =========================================================================
// List Control 컬럼 초기화
// =========================================================================
void CTabReviewDlg::InitListCtrl()
{
    m_listReview.SetExtendedStyle(
        m_listReview.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES
    );

    m_listReview.InsertColumn(0, L"고객명", LVCFMT_LEFT, 100);
    m_listReview.InsertColumn(1, L"별점", LVCFMT_CENTER, 60);
    m_listReview.InsertColumn(2, L"내용", LVCFMT_LEFT, 250);
    m_listReview.InsertColumn(3, L"답글 여부", LVCFMT_CENTER, 80);
    m_listReview.InsertColumn(4, L"작성일", LVCFMT_LEFT, 100);

}

void CTabReviewDlg::SetReviewInfo(int storeId, CNetworkHelper* pNet)
{
    m_storeId = storeId;
    m_pNet = pNet;
    LoadReviewList(); // 저장 즉시 서버에 요청
}
void CTabReviewDlg::SetReviewList(const nlohmann::json& reviewArray)
{
    m_listReview.DeleteAllItems();

    auto toW = [](const std::string& s) -> CString {
        return CA2W(s.c_str(), CP_UTF8);
        };

    for (int i = 0; i < (int)reviewArray.size(); i++)
    {
        const auto& rv = reviewArray[i];

        CString userName = toW(rv.value("userId", ""));
        int     rating = rv.value("rating", 0);
        CString content = toW(rv.value("content", ""));
        CString createdAt = toW(rv.value("createdAt", ""));
        bool    hasReply = !rv.value("ownerReply", "").empty();

        // 별점을 ★로 변환
        CString strRating;
        for (int r = 0; r < rating; r++)    strRating += L"★";
        for (int r = rating; r < 5; r++)    strRating += L"☆";

        int nIdx = m_listReview.InsertItem(i, userName);
        m_listReview.SetItemText(nIdx, 1, strRating);
        m_listReview.SetItemText(nIdx, 2, content);
        m_listReview.SetItemText(nIdx, 3, hasReply ? L"완료" : L"없음");
        m_listReview.SetItemText(nIdx, 4, createdAt);

        // ✅ reviewId 저장 (답글 전송 시 필요)
        m_listReview.SetItemData(nIdx, rv.value("reviewId", 0));
    }
}

void CTabReviewDlg::LoadReviewList()
{
    if (!m_pNet) return;
    json body;
    body["storeId"] = m_storeId;
    m_pNet->Send(CmdID::REQ_REVIEW_LIST, body);
}
int CTabReviewDlg::GetSelectedIndex()
{
    return m_listReview.GetNextItem(-1, LVNI_SELECTED);
}

// =========================================================================
// List Control 선택 변경 → 기존 답글 내용 표시
// =========================================================================
void CTabReviewDlg::OnLvnItemchangedListReview(NMHDR* pNMHDR, LRESULT* pResult)
{
    (void)pNMHDR;
    int nIdx = GetSelectedIndex();
    bool bSelected = (nIdx != -1);
    m_btnReplySubmit.EnableWindow(bSelected);

    if (bSelected)
    {
        //  선택된 리뷰 ID 저장
        m_selectedReviewId = (int)m_listReview.GetItemData(nIdx);

        CString strReplied = m_listReview.GetItemText(nIdx, 3);
        if (strReplied == L"완료")
            m_editReply.SetWindowText(L"이미 답글이 등록된 리뷰입니다.");
        else
            m_editReply.SetWindowText(L"");
    }
    *pResult = 0;
}

// =========================================================================
// 답글 등록 버튼
// =========================================================================
void CTabReviewDlg::OnBnClickedBtnReplySubmit()
{
    int nIdx = GetSelectedIndex();
    if (nIdx == -1) return;

    // 이미 답글 있는 리뷰는 중복 등록 방지
    CString strReplied = m_listReview.GetItemText(nIdx, 3);
    if (strReplied == L"완료") {
        MessageBox(L"이미 답글이 등록된 리뷰입니다.", L"알림", MB_OK);
        return;
    }

    CString strReply;
    m_editReply.GetWindowText(strReply);
    if (strReply.IsEmpty()) {
        MessageBox(L"답글 내용을 입력하세요.", L"알림", MB_OK);
        return;
    }

    if (!m_pNet) return;

    //  실제 서버 전송
    json body;
    body["reviewId"] = m_selectedReviewId;
    body["storeId"] = m_storeId;
    body["ownerReply"] = CT2A(strReply, CP_UTF8).m_psz;
    m_pNet->Send(CmdID::REQ_REVIEW_REPLY, body);

    // UI 즉시 반영
    m_listReview.SetItemText(nIdx, 3, L"완료");
    m_editReply.SetWindowText(L"");
    MessageBox(L"답글이 등록되었습니다.", L"완료", MB_OK);
}

BEGIN_MESSAGE_MAP(CTabReviewDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_REPLY_SUBMIT, &CTabReviewDlg::OnBnClickedBtnReplySubmit)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_REVIEW, &CTabReviewDlg::OnLvnItemchangedListReview)
END_MESSAGE_MAP()