#include "pch.h"
#include "framework.h"
#include "MFC.h"
#include "MFCDlg.h"
#include "afxdialogex.h"
#include "CMainMenuDlg.h"
#include "SignupDlg.h"
#include "NetworkHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CMFCDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_EN_CHANGE(IDC_EDIT1, &CMFCDlg::OnEdit_ID)
    ON_EN_CHANGE(IDC_EDIT2, &CMFCDlg::OnEdit_PW)
    ON_BN_CLICKED(IDC_BUTTON1, &CMFCDlg::OnBtnLogin)
    ON_BN_CLICKED(IDC_BUTTON2, &CMFCDlg::OnBtnSign)
    ON_BN_CLICKED(IDCANCEL, &CMFCDlg::OnBtnCancel)
    ON_MESSAGE(WM_PACKET_RECEIVED, &CMFCDlg::OnPacketReceived)
END_MESSAGE_MAP()

CMFCDlg::CMFCDlg(CWnd* pParent)
    : CDialogEx(IDD_MFC_DIALOG, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BOOL CMFCDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != nullptr)
    {
        CString strAboutMenu;
        if (strAboutMenu.LoadString(IDS_ABOUTBOX) && !strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    SetIcon(m_hIcon, TRUE);
    SetIcon(m_hIcon, FALSE);
    SetWindowText(L"ITS_Bedalyo - Owner Login");

    return TRUE;
}

void CMFCDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    CDialogEx::OnSysCommand(nID, lParam);
}

void CMFCDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this);
        SendMessage(WM_ICONERASEBKGND,
            reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        dc.DrawIcon((rect.Width() - cxIcon + 1) / 2,
            (rect.Height() - cyIcon + 1) / 2, m_hIcon);
    }
    else
    {
        CDialogEx::OnPaint();
    }
}

HCURSOR CMFCDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CMFCDlg::OnEdit_ID()
{
    GetDlgItemText(IDC_EDIT1, m_strId);
}

void CMFCDlg::OnEdit_PW()
{
    GetDlgItemText(IDC_EDIT2, m_strPw);
}

void CMFCDlg::OnBtnLogin()
{
    GetDlgItemText(IDC_EDIT1, m_strId);
    GetDlgItemText(IDC_EDIT2, m_strPw);

    if (m_strId.IsEmpty() || m_strPw.IsEmpty())
    {
        MessageBox(L"아이디와 비밀번호를 입력하세요.", L"알림", MB_OK);
        return;
    }

    if (m_waitingResponse) return;

    if (!m_net.IsConnected())
    {
        if (!m_net.Connect(m_serverIp, m_serverPort, GetSafeHwnd()))
        {
            MessageBox(L"서버에 연결할 수 없습니다.\nIP/Port를 확인하세요.",
                L"연결 실패", MB_ICONERROR);
            return;
        }
    }

    json body;
    body["userId"] = CT2A(m_strId, CP_UTF8);
    body["password"] = CT2A(m_strPw, CP_UTF8);

    m_net.Send(CmdID::REQ_LOGIN, body);

    m_waitingResponse = true;
    GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
}

void CMFCDlg::OnBtnSign()
{
    if (!m_net.IsConnected())
    {
        if (!m_net.Connect(m_serverIp, m_serverPort, GetSafeHwnd()))
        {
            MessageBox(L"서버에 연결할 수 없습니다.", L"연결 실패", MB_ICONERROR);
            return;
        }
    }

    CSignupDlg signupDlg(&m_net, this);

    // SignupDlg 열기 전 HWND 교체 → 패킷이 SignupDlg로 전달됨
    m_net.SetNotifyHwnd(signupDlg.GetSafeHwnd());

    if (signupDlg.DoModal() == IDOK)
        MessageBox(L"가입이 완료됐습니다. 로그인해주세요.", L"안내", MB_OK);

    // 모달 닫힌 후 MFCDlg로 복구
    m_net.SetNotifyHwnd(GetSafeHwnd());
}

LRESULT CMFCDlg::OnPacketReceived(WPARAM wParam, LPARAM lParam)
{
    (void)wParam;
    auto* pkt = reinterpret_cast<ReceivedPacket*>(lParam);
    if (!pkt) return 0;

    m_waitingResponse = false;
    GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE);

    if (pkt->cmdId == CmdID::RES_LOGIN)
    {
        try
        {
            json resJson = json::parse(pkt->body);
            OutputDebugStringA("[OnPacketReceived] RES_LOGIN 수신!\n");
            OutputDebugStringA(("[OnPacketReceived] body: " + pkt->body + "\n").c_str());

            bool success = (resJson.value("status", 0) == 200);

            if (success)
            {
                // UTF-8 → CString 변환 헬퍼
                auto toW = [](const std::string& s) -> CString {
                    return CA2W(s.c_str(), CP_UTF8);
                    };

                // ── 매장 정보 수신 ──────────────────────────────────
                int     storeId = resJson.value("storeId", 0);
                CString storeName = toW(resJson.value("storeName", ""));
                CString category = toW(resJson.value("category", ""));
                CString storeAddress = toW(resJson.value("storeAddress", ""));
                CString cookTime = toW(resJson.value("cookTime", ""));
                CString minOrder = toW(resJson.value("minOrderAmount", ""));
                CString openTime = toW(resJson.value("openTime", ""));
                CString closeTime = toW(resJson.value("closeTime", ""));

                // 1 사업자번호 = 1 매장 → storeId를 문자열로 변환해서 bizNum으로 사용
                CString bizNum;
                bizNum.Format(L"%d", storeId);

                // ── 사장님 정보 수신 ────────────────────────────────
                CString ownerName = toW(resJson.value("userName", ""));
                CString ownerPhone = toW(resJson.value("phoneNumber", ""));
                CString accountNumber = toW(resJson.value("accountNumber", ""));

                // ✅ 삼항 연산자 대신 if문으로 변경 (C4927 변환 오류 해결)
                int nApproval = resJson.value("approvalStatus", 0);
                CString approvalStatus = (nApproval == 1) ? L"승인" : L"대기";

                ShowWindow(SW_HIDE);

                CMainMenuDlg mainDlg(
                    storeId, storeName, category, storeAddress,
                    bizNum, cookTime, minOrder, openTime,
                    closeTime, ownerName, ownerPhone, accountNumber,
                    approvalStatus, this
                );
                mainDlg.DoModal();

                ShowWindow(SW_SHOW);
            }
            else
            {
                std::string msg = resJson.value("message", "로그인에 실패했습니다.");
                MessageBox(CA2W(msg.c_str(), CP_UTF8), L"로그인 실패", MB_ICONWARNING);
            }
        }
        catch (...)
        {
            MessageBox(L"서버 응답을 처리하는 중 오류가 발생했습니다.",
                L"오류", MB_ICONERROR);
        }
    }

    delete pkt;
    return 0;
}

void CMFCDlg::OnBtnCancel()
{
    m_net.Disconnect();
    CDialogEx::OnCancel();
}