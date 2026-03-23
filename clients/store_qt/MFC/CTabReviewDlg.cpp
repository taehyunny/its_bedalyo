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

    // TODO: 서버에서 REQ_REVIEW_LIST 수신 시 채우기
    // 현재는 임시 더미 데이터
    int nIdx = m_listReview.InsertItem(0, L"user123");
    m_listReview.SetItemText(nIdx, 1, L"★★★★★");
    m_listReview.SetItemText(nIdx, 2, L"떡볶이가 정말 맛있어요!");
    m_listReview.SetItemText(nIdx, 3, L"없음");
    m_listReview.SetItemText(nIdx, 4, L"2026-03-20");

    nIdx = m_listReview.InsertItem(1, L"user456");
    m_listReview.SetItemText(nIdx, 1, L"★★★☆☆");
    m_listReview.SetItemText(nIdx, 2, L"음식은 맛있는데 배달이 늦었어요.");
    m_listReview.SetItemText(nIdx, 3, L"완료");
    m_listReview.SetItemText(nIdx, 4, L"2026-03-19");
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
        // 답글 완료된 항목은 Edit에 기존 내용 표시
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

    CString strReply;
    m_editReply.GetWindowText(strReply);

    if (strReply.IsEmpty())
    {
        MessageBox(L"답글 내용을 입력하세요.", L"알림", MB_OK);
        return;
    }

    // TODO: REQ_REVIEW_REPLY 전송
    m_listReview.SetItemText(nIdx, 3, L"완료");
    m_editReply.SetWindowText(L"");
    MessageBox(L"답글이 등록되었습니다.", L"완료", MB_OK);
}

BEGIN_MESSAGE_MAP(CTabReviewDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_REPLY_SUBMIT, &CTabReviewDlg::OnBnClickedBtnReplySubmit)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_REVIEW, &CTabReviewDlg::OnLvnItemchangedListReview)
END_MESSAGE_MAP()