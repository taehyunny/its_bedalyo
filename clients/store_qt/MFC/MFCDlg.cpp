#include "pch.h"
#include "framework.h"
#include "MFC.h"
#include "MFCDlg.h"
#include "afxdialogex.h"
#include "CMainMenuDlg.h"
#include "SignupDlg.h"
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
    // ✅ camelCase 키 + CP_UTF8 변환 / role 제거 (LoginReqDTO에 없는 필드)
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
    if (signupDlg.DoModal() == IDOK)
        MessageBox(L"가입이 완료됐습니다. 로그인해주세요.", L"안내", MB_OK);
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
        OutputDebugStringA("[OnPacketReceived] RES_LOGIN 수신!\n");
        OutputDebugStringA(("[OnPacketReceived] body: " + pkt->body + "\n").c_str());

        try
        {
            json resJson = json::parse(pkt->body);

            // ✅ AuthResDTO 기준: "success" 없음 → status == 200 으로 판단
            bool success = (resJson.value("status", 0) == 200);

            if (success)
            {
                m_storeId = resJson.value("storeId", 0);
                std::string name = resJson.value("storeName", "");
                m_storeName = CA2W(name.c_str());

                ShowWindow(SW_HIDE);

                CMainMenuDlg mainDlg(m_storeId, m_storeName, this);
                mainDlg.DoModal();

                ShowWindow(SW_SHOW);
            }
            else
            {
                std::string msg = resJson.value("message", "로그인에 실패했습니다.");

                // CP_UTF8 추가
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

// ── 취소(나가기) 버튼 ────────────────────────────────────────────────────
void CMFCDlg::OnBtnCancel()
{
    m_net.Disconnect();
    CDialogEx::OnCancel();
}