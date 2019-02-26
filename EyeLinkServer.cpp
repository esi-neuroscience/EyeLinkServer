
// EyeLinkServer.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "EyeLinkServer.h"
#include "MainFrm.h"

#include "EyeLinkServerDoc.h"
#include "EyeLinkServerView.h"
#include <core_expt.h>
#include "PipeProcedure.h"
#include <w32_dialogs.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

float fux = 0.1f;
float fuy = 0.1f;

extern CEyeLinkServerView* g_pView;

float* CEyeLinkServerApp::m_pShare;
INT16  CEyeLinkServerApp::m_useEDF;
HANDLE CEyeLinkServerApp::m_hDone = NULL;
bool CEyeLinkServerApp::m_eyeLinkIsConnected = false;
static HANDLE hStartBlink = NULL;
static HANDLE hEndBlink = NULL;
CRITICAL_SECTION CEyeLinkServerApp::m_criticalUpdate;
CRITICAL_SECTION EyelinkCriticalSection;	// protect calls to Eyelink library

CTransformation* g_pTransformation = NULL;
HANDLE g_hDone = NULL;

// CEyeLinkServerApp

BEGIN_MESSAGE_MAP(CEyeLinkServerApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CEyeLinkServerApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinApp::OnFileOpen)
	ON_COMMAND(ID_EYELINK_CONNECT, &CEyeLinkServerApp::OnEyelinkConnect)
	ON_UPDATE_COMMAND_UI(ID_EYELINK_CONNECT, &CEyeLinkServerApp::OnUpdateEyelinkConnect)
	ON_COMMAND(ID_EYELINK_CLEARSCREEN, &CEyeLinkServerApp::OnEyelinkClearscreen)
	ON_UPDATE_COMMAND_UI(ID_EYELINK_CLEARSCREEN, &CEyeLinkServerApp::OnUpdateEyelinkClearscreen)
END_MESSAGE_MAP()

CEyeLinkServerDoc* pDoc;

VOID CALLBACK LinkTimerCallback(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
//	static bool inUse = FALSE;
	static INT16 type, datatype;
	static ALLF_DATA buf;
	//	static CString logLine;
	int		seq = 0;
	bool haveFixUpdate = false;
	// bool haveBlink = false;
	float x = 0.0f;
	float y = 0.0f;
	CString ErrorMsg;

//	if (inUse) return;
//	inUse = true;
	/*
	Nach einem "Abort Trial" in ELCL liefert "eyelink_get_next_data" "type=0" und "eyelink_get_float_data" "type=2" (ENDPARSE)
	*/
	/*
	INT16 count = eyelink_data_count(0, 1);
	if (count == 0) {
		inUse = false;
		return;
	}
	*/
	EnterCriticalSection(&EyelinkCriticalSection);
	while (type = eyelink_get_next_data(NULL))
	{
		//	for (int i=1; i<=count; i++) type = eyelink_get_next_data(NULL);
		datatype = eyelink_get_float_data(&buf);
		/*
		if (datatype != type) {
		ErrorMsg.Format(_T("Inconsistant Types: %u, %u"), type, datatype);
		AfxMessageBox(ErrorMsg, MB_OK | MB_ICONSTOP);
		}
		*/
		//		TRACE("%u: %u\n", seq, type);
		switch (datatype)
		{
		case MESSAGEEVENT:
			TRACE("Message Event\n");
			//			logLine.SetString((LPCTSTR) &buf.im.text, buf.im.length);
			//			CLog::AddToLog(logLine);
			break;
		case STARTPARSE:
			TRACE("Start Parse\n");
			break;
		case ENDPARSE:	// bei Abort Trial / end program
			TRACE("End Parse\n");
			VERIFY(SetEvent(g_hDone));
			//			ErrorMsg.Format(_T("End Parse"));
//			AfxMessageBox(ErrorMsg, MB_OK | MB_ICONSTOP);
			break;
		case STARTBLINK:
			VERIFY(SetEvent(hStartBlink));
			CEyeLinkServerDoc::OnStartBlink();
			break;
		case ENDBLINK:
			VERIFY(SetEvent(hEndBlink));
			break;
		case FIXUPDATE:
			if ((fux == buf.fe.gavx) && (fuy == buf.fe.gavy)) break;
			fux = buf.fe.gavx;
			fuy = buf.fe.gavy;
			x = fux;
			y = fuy;
//			TRACE("Fixation Update (pre): %f, %f\n", x, y);
			if (g_pTransformation) g_pTransformation->Apply(&x, &y);
//			TRACE("Fixation Update (inter): %f, %f\n", x, y);
			CEyeLinkServerApp::FixationUpdate(x, y);
			//			CEyeLinkServerApp::FixationUpdate(buf.fe.gavx, buf.fe.gavy);
			//			haveFixUpdate = true;
			//			x = buf.fe.gavx;
			//			y = buf.fe.gavy;
//			TRACE("Fixation Update (post): %f, %f\n", x, y);
			//			pDoc->FixationUpdate(buf.fe.gavx, buf.fe.gavy);
						//			logLine.Format(_T("%i: Eye: %i; x: %f, y: %f, Time: %u, Start: %u, End: %u, Status: %u"), seq++, buf.fe.eye, buf.fe.gavx, buf.fe.gavy,
						//				buf.fe.time, buf.fe.sttime, buf.fe.entime, buf.fe.status);
						//			CLog::AddToLog(logLine);
			break;
		default:
			//			logLine.Format(_T("Type: %i"), type);
			//			CLog::AddToLog(logLine);
			TRACE("Default\n");
			;
		}
	}
	LeaveCriticalSection(&EyelinkCriticalSection);
	//	pDoc->BlinkUpdate(haveBlink);
	//	if (haveFixUpdate) pDoc->FixationUpdate(x, y);
//	inUse = false;
	//	logLine.Format(_T("count: %u, n: %u"), count, seq);
//	CLog::AddToLog(logLine);
}


// CEyeLinkServerApp construction

CEyeLinkServerApp::CEyeLinkServerApp() noexcept
	: m_screenWidth(1680)
	, m_screenHeight(1050)
{
	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("EyeLinkServer.AppID.NoVersion"));

	InitializeCriticalSection(&m_criticalUpdate);
	InitializeCriticalSection(&EyelinkCriticalSection);
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CEyeLinkServerApp object

CEyeLinkServerApp theApp;
// CEyeLinkServerView theView;


// CEyeLinkServerApp initialization

BOOL CEyeLinkServerApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction(FALSE);

	// AfxInitRichEdit2() is required to use RichEdit control
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)


	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CEyeLinkServerDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CEyeLinkServerView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);


	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);


	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOWMINIMIZED);
	m_pMainWnd->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	POSITION pos = pDocTemplate->GetFirstDocPosition();
	pDoc = (CEyeLinkServerDoc*)pDocTemplate->GetNextDoc(pos);

	VERIFY(g_hDone = CreateEvent(NULL, FALSE, FALSE, NULL));
	//	VERIFY(m_hDone = CreateEvent(NULL, FALSE, FALSE, _T("EyeServerDone")));
	VERIFY(m_hDone = CreateEventA(NULL, FALSE, FALSE, "EyeServerDone"));

	// blink event names are chosen to be compatible with former Matlab server 
	VERIFY(hStartBlink = CreateEventA(NULL, FALSE, FALSE, "BlinkStart"));
	VERIFY(hEndBlink = CreateEventA(NULL, FALSE, FALSE, "BlinkEnd"));

	//	m_pPipeThread = AfxBeginThread(PipeProcedure, NULL);
	AfxBeginThread(PipeProcedure, (LPVOID)pDoc);

	// Create File Mapping (Shared Memory)
	m_hShare = CreateFileMapping(
		INVALID_HANDLE_VALUE,	// use page file
		NULL,
		PAGE_READWRITE,
		0,
		16,
		_T("EyePosition"));
	if (m_hShare == NULL)
	{
		DWORD error = GetLastError();
		ASSERT(false);
	}
	m_pShare = (float*)MapViewOfFile(m_hShare, FILE_MAP_WRITE, 0, 0, 0);
	if (m_pShare == NULL) ASSERT(false);

	OnEyelinkConnect();		// try an initial connection
	VERIFY(SetEvent(m_hDone));

	return TRUE;
}


int CEyeLinkServerApp::ExitInstance()
{
	//TODO: handle additional resources you may have added
	AfxOleTerm(FALSE);
	DeleteTimerQueueTimer(NULL, m_hLinkTimer, INVALID_HANDLE_VALUE);
	DeleteCriticalSection(&m_criticalUpdate);
	DeleteCriticalSection(&EyelinkCriticalSection);
	VERIFY(CloseHandle(hStartBlink));
	VERIFY(CloseHandle(hEndBlink));
	VERIFY(CloseHandle(g_hDone));
	VERIFY(CloseHandle(m_hDone));
	return CWinApp::ExitInstance();
}


void CEyeLinkServerApp::EyelinkCommand(const char* command)
{
	//	set_offline_mode();
	EnterCriticalSection(&EyelinkCriticalSection);
	eyecmd_printf(command);
	LeaveCriticalSection(&EyelinkCriticalSection);
	TRACE("%s\n", command);
}


void CEyeLinkServerApp::EyelinkStart(unsigned char message[], DWORD messageLength)
{
	if (!m_eyeLinkIsConnected)
	{
		ASSERT(false);
		return;
	}
	EnterCriticalSection(&EyelinkCriticalSection);
	INT16 error;
	m_useEDF = 0;
	if (messageLength != 0)
	{
		TRACE("open_data_file %s\n", message);
		error = eyecmd_printf("open_data_file %s", message);
		if (error == 0)
		{
			error = file_exists((char*)message);
			TRACE("File exists: %u\n", error);
			if (error == 0)
			{
				Sleep(5000);
				error = file_exists((char*)message);
				TRACE("File exists: %u\n", error);
			}
			m_useEDF = 1;
		}
		else
		{
			LeaveCriticalSection(&EyelinkCriticalSection);
			TRACE("Open error: %d\n", error);
			AfxMessageBox(_T("Can't open EDF file"), MB_OK | MB_ICONINFORMATION);
//			EyelinkDisconnect();
			return;
		}
	}
	//	eyecmd_printf("screen_pixel_coords = -840 524 839 -525");
	eyecmd_printf("screen_pixel_coords = %i %i %i %i",
		(short)m_screenWidth / -2,
		m_screenHeight / 2 - 1,
		m_screenWidth / 2 - 1,
		(short)m_screenHeight / -2);
	TRACE("screen_pixel_coords = %i %i %i %i\n",
		(short)m_screenWidth / -2,
		m_screenHeight / 2 - 1,
		m_screenWidth / 2 - 1,
		(short)m_screenHeight / -2);
	//	eyecmd_printf("link_event_filter = LEFT,RIGHT,FIXUPDATE,MESSAGE");
	//	eyecmd_printf("link_event_filter = RIGHT,FIXUPDATE");
	bool lefteye = false;
	bool reportBlinks = true;
	eyecmd_printf("link_event_filter = %s,FIXUPDATE%s", lefteye ? "LEFT" : "RIGHT", reportBlinks ? ",BLINK" : "");
	TRACE("link_event_filter = %s,FIXUPDATE%s\n", lefteye ? "LEFT" : "RIGHT", reportBlinks ? ",BLINK" : "");
	eyecmd_printf("fixation_update_interval = 50");
	eyecmd_printf("fixation_update_accumulate = 50");
	//	eyecmd_printf("draw_cross %u %u 15", 0, 0);
	error = start_recording(m_useEDF, m_useEDF, 0, 1);
	if (error) {
		LeaveCriticalSection(&EyelinkCriticalSection);
		AfxMessageBox(_T("Can't start recording"), MB_OK | MB_ICONINFORMATION);
	//	EyelinkDisconnect();
		return;
	}
	if (!eyelink_wait_for_block_start(100, 0, 1)) //wait for link events
	{
		LeaveCriticalSection(&EyelinkCriticalSection);
		AfxMessageBox(_T("No link events received"), MB_OK | MB_ICONINFORMATION);
	//	EyelinkDisconnect();
		return;
	}
	if (m_useEDF)
	{
		VERIFY(0 == eyemsg_printf("DISPLAY_COORDS %i %i %i %i",
			(short)m_screenWidth / -2,
			m_screenHeight / 2 - 1,
			m_screenWidth / 2 - 1,
			(short)m_screenHeight / -2));
	}
	TransformMsg();
	LeaveCriticalSection(&EyelinkCriticalSection);
}

// CEyeLinkServerApp message handlers

void CEyeLinkServerApp::OnEyelinkConnect()
{
	//	INT16 error;
#ifdef _DEBUG
//#define mode 1	// Simulated Link Mode
#define mode 0	// currently we have an EyeLink system for developement
#else
#define mode 0
#endif
//	CScreen* pScreen = &g_pView->m_screen;
	TRACE(_T("App::EyelinkConnect\r\n"));
	m_pMainWnd->EnableWindow(FALSE);	// make EyeLinks's message box "modal"
	if (open_eyelink_connection(mode)) {
		//		AfxMessageBox(_T("Can't connect to EyeLink"), MB_OK | MB_ICONINFORMATION);	// no need for that: EyeLink generates MessageBox itself (at least at timeout)
		CEyeLinkServerView::SetStateString(_T("Mouse Simulation"));
		m_pMainWnd->ShowWindow(SW_RESTORE);
		m_pMainWnd->EnableWindow();
		g_pView->m_screen.EnableWindow();
		return;
	}
	m_pMainWnd->EnableWindow();
	g_pView->m_screen.EnableWindow(FALSE);

	CEyeLinkServerView::SetStateString(_T("Connected to EyeLink"));
	float tracker_pixel_left = 0; // tracker gaze coord system
	float tracker_pixel_top = 0; // used to remap gaze data
	float tracker_pixel_right = 0; // to match our display resolution
	float tracker_pixel_bottom = 0;

	EnterCriticalSection(&EyelinkCriticalSection);
	// Read setting of "screen_pixel_coords" from tracker
	// This allows remapping of gaze data if our display
	// has a different resolution than the connected computer
	// The read may fail with older tracker software
	char buf[40] = "";
	INT16 result;
	INT16 nRetry = 0;
	//	UINT32 t;
	eyelink_read_request("screen_pixel_coords");
	do
	{
		Sleep(21);
		result = eyelink_read_reply(buf);
		if (result == OK_RESULT)
		{
			TRACE("Reply: %s\n", buf);
			sscanf(buf, "%f,%f,%f,%f",
				&tracker_pixel_left,
				&tracker_pixel_top,
				&tracker_pixel_right,
				&tracker_pixel_bottom);
			TRACE("Screen Coords: %f, %f, %f, %f\n",
				tracker_pixel_left,
				tracker_pixel_top,
				tracker_pixel_right,
				tracker_pixel_bottom);
			break;
		}
		else
		{
			TRACE("Waiting for reply\n");
			message_pump(); // keep Windows happy
			nRetry++;
		}
	} while (nRetry < 24);
	LeaveCriticalSection(&EyelinkCriticalSection);
	if (result != OK_RESULT)
	{
		AfxMessageBox(_T("Can't read screen size from EyeLink config."), MB_OK | MB_ICONINFORMATION);
		EyelinkDisconnect();
		return;
	}
	m_screenWidth = (short)(tracker_pixel_right - tracker_pixel_left);
	m_screenWidth += (m_screenWidth & 1) ? 1 : 0;
	m_screenHeight = (short)abs(tracker_pixel_top - tracker_pixel_bottom);
	m_screenHeight += (m_screenHeight & 1) ? 1 : 0;
	/*
	eyecmd_printf("screen_pixel_coords = -840 524 839 -525");
//	eyecmd_printf("link_event_filter = LEFT,RIGHT,FIXUPDATE,MESSAGE");
//	eyecmd_printf("link_event_filter = RIGHT,FIXUPDATE");
	bool lefteye = false;
	bool reportBlinks = true;
	eyecmd_printf("link_event_filter = %s,FIXUPDATE%s", lefteye ? "LEFT" : "RIGHT", reportBlinks ? ",BLINK" : "");
	TRACE("link_event_filter = %s,FIXUPDATE%s\n", lefteye ? "LEFT" : "RIGHT", reportBlinks ? ",BLINK" : "");
	eyecmd_printf("fixation_update_interval = 100");
	eyecmd_printf("fixation_update_accumulate = 100");
	eyecmd_printf("draw_cross %u %u 15", 0, 0);
	error = start_recording(0, 0, 0, 1);
	if (error) {
		AfxMessageBox(_T("Can't start recording"), MB_OK | MB_ICONINFORMATION);
		EyelinkDisconnect();
		return;
	}
	if(!eyelink_wait_for_block_start(100, 0, 1)) //wait for link events
	{
		AfxMessageBox(_T("No link events received"), MB_OK | MB_ICONINFORMATION);
		EyelinkDisconnect();
		return;
	}
	*/
	m_eyeLinkIsConnected = true;
	if (!CreateTimerQueueTimer(
		&m_hLinkTimer,
		NULL,		// default timer queue
		(WAITORTIMERCALLBACK)LinkTimerCallback,
		NULL,
		100,	// due time (first callback)
		50,	// period
		WT_EXECUTELONGFUNCTION))
	{
		LPVOID lpMsgBuf;
		DWORD dw = GetLastError();
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0, NULL);
		//		lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
		//			(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR)); 
		//		StringCchPrintf((LPTSTR)lpDisplayBuf, 
		//			LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		//			TEXT("%s failed with error %d: %s"), 
		//			lpszFunction, dw, lpMsgBuf); 
		//		MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);
		CString ErrorMsg;
//		ErrorMsg.Format(_T("Init Instance of EyeLinkServer failed with error %d: %s"), dw, lpMsgBuf);
		ErrorMsg.Format(_T("EyeLinkServer failed with error %d: %s"), dw, lpMsgBuf);
		AfxMessageBox(ErrorMsg, MB_OK | MB_ICONSTOP);

		LocalFree(lpMsgBuf);
		//		LocalFree(lpDisplayBuf);
		//return FALSE;
	}
}


void CEyeLinkServerApp::EyelinkDisconnect(void)
{
	EnterCriticalSection(&EyelinkCriticalSection);
	eyecmd_printf("link_event_filter = LEFT,RIGHT,FIXATION");
	eyecmd_printf("fixation_update_interval = 0");
	eyecmd_printf("fixation_update_accumulate = 0");
	close_eyelink_connection();
	LeaveCriticalSection(&EyelinkCriticalSection);
	m_eyeLinkIsConnected = false;
}


void CEyeLinkServerApp::EyelinkStop(unsigned char message[], DWORD messageLength)
{
	//	Sleep(100);
	set_offline_mode();
	// the g_hDone event is signaled when we see an "End Parse" event from EyeLink 
	VERIFY(WAIT_OBJECT_0 == WaitForSingleObject(g_hDone, 1000));
	if (m_useEDF)
	{
		//	eyecmd_printf("close_data_file");
		VERIFY(!close_data_file());
		//		Sleep(100);
		m_useEDF = 0;
		INT32 nBytes = receive_data_file_dialog("", (char*)message, 0);
		if ((nBytes <= 0) && (nBytes != -1))	// not success or dialog aborted
		{
			CString msg;
			msg.Format(_T("Can't transfer EDF file to %S (error %d)"), message, nBytes);
			AfxMessageBox(msg, MB_OK | MB_ICONINFORMATION);
		}
	}
	VERIFY(SetEvent(m_hDone));
}


void CEyeLinkServerApp::FixationUpdate(float x, float y)
{
	EnterCriticalSection(&m_criticalUpdate);
	//	if ((x == m_x) && (y == m_y)) return;
	//	m_x = x;
	//	m_y = y;
	*m_pShare++ = x;
	*m_pShare-- = y;
	pDoc->FixationUpdate(x, y);
	LeaveCriticalSection(&m_criticalUpdate);
}


void CEyeLinkServerApp::TransformMsg()
{
	if (m_useEDF && g_pTransformation)
	{
		char msg[80];
		g_pTransformation->xMsg(&msg[0]);
		VERIFY(0 == eyemsg_printf(msg));
		g_pTransformation->yMsg(&msg[0]);
		VERIFY(0 == eyemsg_printf(msg));
	}
	float x = fux;
	float y = fuy;
	if (g_pTransformation) g_pTransformation->Apply(&x, &y);
	FixationUpdate(x, y);
}


void CEyeLinkServerApp::TransformClearMsg()
{
	EnterCriticalSection(&EyelinkCriticalSection);
	VERIFY(0 == eyemsg_printf("Transformation cleared"));
	LeaveCriticalSection(&EyelinkCriticalSection);
	FixationUpdate(fux, fuy);
}


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg() noexcept;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() noexcept : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// App command to run the dialog
void CEyeLinkServerApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CEyeLinkServerApp message handlers





void CEyeLinkServerApp::OnUpdateEyelinkConnect(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(!m_eyeLinkIsConnected);
}


void CEyeLinkServerApp::OnEyelinkClearscreen()
{
	EyelinkCommand("clear_screen 7");
}


void CEyeLinkServerApp::OnUpdateEyelinkClearscreen(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_eyeLinkIsConnected);
}
