
#include "pch.h"
#include "framework.h"
#include "MFC.h"
#include "MFCDlg.h"     

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CMFCApp, CWinApp)
    ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

CMFCApp::CMFCApp()
{
    m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
}

CMFCApp theApp;

BOOL CMFCApp::InitInstance()
{
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);
    InitCtrls.dwICC  = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);

    CWinApp::InitInstance();
    AfxEnableControlContainer();

    CShellManager* pShellManager = new CShellManager;
    CMFCVisualManager::SetDefaultManager(
        RUNTIME_CLASS(CMFCVisualManagerWindows));

   
    SetRegistryKey(_T("ITS_Bedalyo_Owner"));

    CMFCDlg dlg;


    INT_PTR nResponse = dlg.DoModal();

    if (nResponse == -1)
    {
        TRACE(traceAppMsg, 0,
            "경고: 대화 상자를 만들지 못했으므로 애플리케이션이 예기치 않게 종료됩니다.\n");
    }

    if (pShellManager != nullptr)
        delete pShellManager;

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
    ControlBarCleanUp();
#endif

    return FALSE;
}
