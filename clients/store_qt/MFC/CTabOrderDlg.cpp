// CTabOrderDlg.cpp: 구현 파일

#include "pch.h"
#include "MFC.h"
#include "afxdialogex.h"
#include "CTabOrderDlg.h"

IMPLEMENT_DYNAMIC(CTabOrderDlg, CDialogEx)

CTabOrderDlg::CTabOrderDlg(CWnd* pParent)
    : CDialogEx(IDD_TAB_ORDER, pParent)
{
}

CTabOrderDlg::~CTabOrderDlg()
{
}

// =========================================================================
// 컨트롤 바인딩
// =========================================================================
void CTabOrderDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_ORDER, m_listOrder);
    DDX_Control(pDX, IDC_BTN_ORDER_ACCEPT, m_btnOrderAccept);
    DDX_Control(pDX, IDC_BTN_ORDER_REJECT, m_btnOrderReject);
    DDX_Control(pDX, IDC_COMBO_REJECT_REASON, m_comboRejectReason);
    DDX_Control(pDX, IDC_EDIT_REJECT_REASON, m_editRejectReason);
}

// =========================================================================
// 초기화
// =========================================================================
BOOL CTabOrderDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // 거절 사유 콤보박스 항목 추가
    m_comboRejectReason.AddString(L"품절된 메뉴가 있습니다.");
    m_comboRejectReason.AddString(L"재료 소진으로 영업이 종료되었습니다.");
    m_comboRejectReason.AddString(L"배달 불가 지역입니다.");
    m_comboRejectReason.AddString(L"최소 주문 금액 미만입니다.");
    m_comboRejectReason.AddString(L"직접 입력");
    m_comboRejectReason.SetCurSel(0);

    // 직접 입력란 초기 비활성화
    m_editRejectReason.EnableWindow(FALSE);

    InitListCtrl();
    UpdateButtonState();

    return TRUE;
}

// =========================================================================
// List Control 컬럼 초기화
// =========================================================================
void CTabOrderDlg::InitListCtrl()
{
    m_listOrder.SetExtendedStyle(
        m_listOrder.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES
    );

    m_listOrder.InsertColumn(0, L"주문번호", LVCFMT_LEFT, 140);
    m_listOrder.InsertColumn(1, L"메뉴", LVCFMT_LEFT, 160);
    m_listOrder.InsertColumn(2, L"금액", LVCFMT_RIGHT, 80);
    m_listOrder.InsertColumn(3, L"상태", LVCFMT_CENTER, 80);
    m_listOrder.InsertColumn(4, L"주문 시각", LVCFMT_LEFT, 120);

    // TODO: 서버에서 NOTIFY_NEW_ORDER 수신 시 항목 추가
    // 현재는 임시 더미 데이터
    int nIdx = m_listOrder.InsertItem(0, L"ORD-20260320-001");
    m_listOrder.SetItemText(nIdx, 1, L"떡볶이 2인세트");
    m_listOrder.SetItemText(nIdx, 2, L"18,000");
    m_listOrder.SetItemText(nIdx, 3, L"대기");
    m_listOrder.SetItemText(nIdx, 4, L"14:32:10");
}

// =========================================================================
// 버튼 활성화/비활성화
// =========================================================================
void CTabOrderDlg::UpdateButtonState()
{
    bool bSelected = (GetSelectedIndex() != -1);
    m_btnOrderAccept.EnableWindow(bSelected);
    m_btnOrderReject.EnableWindow(bSelected);
}

int CTabOrderDlg::GetSelectedIndex()
{
    return m_listOrder.GetNextItem(-1, LVNI_SELECTED);
}

// =========================================================================
// List Control 선택 변경
// =========================================================================
void CTabOrderDlg::OnLvnItemchangedListOrder(NMHDR* pNMHDR, LRESULT* pResult)
{
    (void)pNMHDR;
    UpdateButtonState();
    *pResult = 0;
}

// =========================================================================
// 더블클릭 → 주문 상세 팝업
// =========================================================================
void CTabOrderDlg::OnNMDblclkListOrder(NMHDR* pNMHDR, LRESULT* pResult)
{
    (void)pNMHDR;
    int nIdx = GetSelectedIndex();
    if (nIdx == -1) { *pResult = 0; return; }

    // TODO: 선택된 주문 데이터를 COrderDetailDlg에 전달
    COrderDetailDlg detailDlg(this);
    detailDlg.DoModal();

    *pResult = 0;
}

// =========================================================================
// 거절 사유 콤보박스 변경 → 직접 입력 선택 시 Edit 활성화
// =========================================================================
void CTabOrderDlg::OnCbnSelchangeComboRejectReason()
{
    int nSel = m_comboRejectReason.GetCurSel();
    CString strSel;
    m_comboRejectReason.GetLBText(nSel, strSel);

    // "직접 입력" 선택 시에만 Edit 활성화
    bool bDirect = (strSel == L"직접 입력");
    m_editRejectReason.EnableWindow(bDirect);
    if (bDirect)
    {
        m_editRejectReason.SetWindowText(L"");
        m_editRejectReason.SetFocus();
    }
}

// =========================================================================
// 수락 버튼
// =========================================================================
void CTabOrderDlg::OnBnClickedBtnOrderAccept()
{
    int nIdx = GetSelectedIndex();
    if (nIdx == -1) return;

    CString strOrderId = m_listOrder.GetItemText(nIdx, 0);
    // TODO: REQ_ORDER_ACCEPT 전송
    m_listOrder.SetItemText(nIdx, 3, L"수락");
    MessageBox(strOrderId + L" 주문을 수락했습니다.", L"주문 수락", MB_OK);
}

// =========================================================================
// 거절 버튼
// =========================================================================
void CTabOrderDlg::OnBnClickedBtnOrderReject()
{
    int nIdx = GetSelectedIndex();
    if (nIdx == -1) return;

    // 거절 사유 수집
    CString strReason;
    int nSel = m_comboRejectReason.GetCurSel();
    m_comboRejectReason.GetLBText(nSel, strReason);

    if (strReason == L"직접 입력")
    {
        m_editRejectReason.GetWindowText(strReason);
        if (strReason.IsEmpty())
        {
            MessageBox(L"거절 사유를 입력해주세요.", L"알림", MB_OK);
            return;
        }
    }

    CString strOrderId = m_listOrder.GetItemText(nIdx, 0);
    // TODO: REQ_ORDER_REJECT 전송 (rejectReason: strReason)
    m_listOrder.SetItemText(nIdx, 3, L"거절");
    MessageBox(strOrderId + L" 주문을 거절했습니다.\n사유: " + strReason, L"주문 거절", MB_OK);
}

BEGIN_MESSAGE_MAP(CTabOrderDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_ORDER_ACCEPT, &CTabOrderDlg::OnBnClickedBtnOrderAccept)
    ON_BN_CLICKED(IDC_BTN_ORDER_REJECT, &CTabOrderDlg::OnBnClickedBtnOrderReject)
    ON_CBN_SELCHANGE(IDC_COMBO_REJECT_REASON, &CTabOrderDlg::OnCbnSelchangeComboRejectReason)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_ORDER, &CTabOrderDlg::OnLvnItemchangedListOrder)
    ON_NOTIFY(NM_DBLCLK, IDC_LIST_ORDER, &CTabOrderDlg::OnNMDblclkListOrder)
END_MESSAGE_MAP()