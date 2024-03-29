
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
float tx = 0.0f;
float ty = 0.0f;

extern CEyeLinkServerView* g_pView;

float* CEyeLinkServerApp::m_pShare;
INT16  CEyeLinkServerApp::m_useEDF;
INT16  CEyeLinkServerApp::m_eyeUsed;
INT16  CEyeLinkServerApp::m_eyeUsedConf = -1;
HANDLE CEyeLinkServerApp::m_hDone = NULL;
HANDLE CEyeLinkServerApp::m_hLinkTimer = NULL;
HANDLE CEyeLinkServerApp::m_hEndDeferredMode = NULL;
bool CEyeLinkServerApp::m_eyeLinkIsConnected = false;
bool CEyeLinkServerApp::m_sampleMode = false;
bool CEyeLinkServerApp::m_running = false;
bool CEyeLinkServerApp::m_signalSaccade = false;
bool CEyeLinkServerApp::m_reportSaccades = false;
static HANDLE hStartBlink = NULL;
static HANDLE hEndBlink = NULL;
static HANDLE hStartSacc = NULL;
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
	ON_CBN_SELCHANGE(IDC_COMBO1, &CEyeLinkServerApp::OnCbnSelchangeCombo1)
END_MESSAGE_MAP()

CEyeLinkServerDoc* pDoc;


VOID CALLBACK CEyeLinkServerApp::LinkTimerCallback(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
	static INT16 type, datatype;
	static ALLF_DATA buf;
	float x = 0.0f;
	float y = 0.0f;
	CString ErrorMsg;

	/*
	Nach einem "Abort Trial" in ELCL liefert "eyelink_get_next_data" "type=0" und "eyelink_get_float_data" "type=2" (ENDPARSE)
	*/
	EnterCriticalSection(&EyelinkCriticalSection);
	while (type = eyelink_get_next_data(NULL))
	{
		datatype = eyelink_get_float_data(&buf);
		switch (datatype)
		{
		case SAMPLE_TYPE:
//			if (buf.fe.eye != CEyeLinkServerApp::m_eyeUsed) break;
			if ((fux == buf.fs.gx[m_eyeUsed]) && (fuy == buf.fs.gy[m_eyeUsed])) break;
			fux = buf.fs.gx[m_eyeUsed];
			fuy = buf.fs.gy[m_eyeUsed];
			x = fux;
			y = fuy;
			CEyeLinkServerApp::FixationUpdate(x, y);
			TRACE("Sample; %f %f\n", x, y);
			break;
		case MESSAGEEVENT:
			TRACE("Message Event\n");
			break;
		case STARTPARSE:
			TRACE("Start Parse\n");
			break;
		case ENDPARSE:	// bei Abort Trial / end program
			TRACE("End Parse\n");
			VERIFY(SetEvent(g_hDone));
			break;
		case STARTBLINK:
			if (buf.fe.eye == m_eyeUsed)
			{
				VERIFY(SetEvent(hStartBlink));
				CEyeLinkServerDoc::OnStartBlink();
			}
			break;
		case ENDBLINK:
			if (buf.fe.eye == m_eyeUsed) VERIFY(SetEvent(hEndBlink));
			break;
		case FIXUPDATE:
			if (buf.fe.eye != m_eyeUsed) break;
			if ((fux == buf.fe.gavx) && (fuy == buf.fe.gavy)) break;
			fux = buf.fe.gavx;
			fuy = buf.fe.gavy;
			x = fux;
			y = fuy;
			CEyeLinkServerApp::FixationUpdate(x, y);
			break;
		case BUTTONEVENT:
			TRACE("Button %u\n", buf.io.data);
			break;
		case INPUTEVENT:
			TRACE("Input %u\n", buf.io.data);
			break;
		case STARTSACC:
			if (m_signalSaccade)
			{
				VERIFY(SetEvent(hStartSacc));
				VERIFY(SetEvent(theApp.m_hEndDeferredMode));
				m_signalSaccade = false;	// one shot
			}
			break;
		case ENDSACC:
			break;
		default:
			TRACE("Unsupported data type: %u\n", datatype);
			;
		}
	}
	LeaveCriticalSection(&EyelinkCriticalSection);
}


// CEyeLinkServerApp construction

CEyeLinkServerApp::CEyeLinkServerApp() noexcept
	: m_screenWidth(1680)
	, m_screenHeight(1050)
	, m_hShare(NULL)
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
	SetRegistryKey(_T("ESI_Application"));
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
//	m_pMainWnd->ShowWindow(SW_SHOWMINIMIZED);
	m_pMainWnd->ShowWindow(m_nCmdShow);
	m_pMainWnd->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	POSITION pos = pDocTemplate->GetFirstDocPosition();
	pDoc = (CEyeLinkServerDoc*)pDocTemplate->GetNextDoc(pos);

	VERIFY(g_hDone = CreateEvent(NULL, FALSE, FALSE, NULL));
	VERIFY(m_hEndDeferredMode = CreateEventA(NULL, TRUE, FALSE, "StimServerEndDeferredMode"));
	VERIFY(m_hDone = CreateEventA(NULL, FALSE, FALSE, "EyeServerDone"));

	// blink event names are chosen to be compatible with former Matlab server 
	VERIFY(hStartBlink = CreateEventA(NULL, FALSE, FALSE, "BlinkStart"));
	VERIFY(hEndBlink = CreateEventA(NULL, FALSE, FALSE, "BlinkEnd"));
	VERIFY(hStartSacc = CreateEventA(NULL, FALSE, FALSE, "StartSacc"));

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
	EyelinkStop(0, 0);
	EnterCriticalSection(&EyelinkCriticalSection);
//	if (m_hLinkTimer) VERIFY(DeleteTimerQueueTimer(NULL, m_hLinkTimer, INVALID_HANDLE_VALUE));
	if (m_eyeLinkIsConnected) close_eyelink_connection();
	LeaveCriticalSection(&EyelinkCriticalSection);
	AfxOleTerm(FALSE);
	DeleteCriticalSection(&m_criticalUpdate);
	DeleteCriticalSection(&EyelinkCriticalSection);
	VERIFY(CloseHandle(hStartBlink));
	VERIFY(CloseHandle(hEndBlink));
	VERIFY(CloseHandle(hStartSacc));
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


void CEyeLinkServerApp::EyelinkOffline()
{
	EnterCriticalSection(&EyelinkCriticalSection);
	set_offline_mode();
	LeaveCriticalSection(&EyelinkCriticalSection);
}


void CEyeLinkServerApp::EyelinkStart(unsigned char message[], DWORD messageLength)
{
	if (!m_eyeLinkIsConnected || m_running) return;

	EnterCriticalSection(&EyelinkCriticalSection);
	INT16 error;
	m_useEDF = 0;

	if (messageLength != 0)
	{
		TRACE("open_data_file %s\n", message);
		error = eyecmd_printf("open_data_file %s", message);
		if (error != 0)
		{
			LeaveCriticalSection(&EyelinkCriticalSection);
			TRACE("Open error: %d\n", error);
			AfxMessageBox(_T("Can't open EDF file"), MB_OK | MB_ICONINFORMATION);
//			EyelinkDisconnect();
			return;
		}
		m_useEDF = 1;
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
	CStringA linkEventFilter= m_sampleMode ? "" : "FIXUPDATE";
	bool reportBlinks = true;
	if (reportBlinks)
	{
		if (!linkEventFilter.IsEmpty()) linkEventFilter.AppendChar(',');
		linkEventFilter.Append("BLINK");
	}
	if (m_reportSaccades)
	{
		if (!linkEventFilter.IsEmpty()) linkEventFilter.AppendChar(',');
		linkEventFilter.Append("SACCADE");
	}
	eyecmd_printf("link_event_filter = %s", linkEventFilter);
	TRACE("link_event_filter = %s\n", linkEventFilter);
	/*
	if (m_sampleMode)
	{
		eyecmd_printf("link_event_filter = BLINK");
		TRACE("link_event_filter = BLINK");
	}
	else
	{
		eyecmd_printf("link_event_filter = FIXUPDATE%s%s",
			reportBlinks ? ",BLINK" : "",
		m_reportSaccades ? ",SACCADE" : "");
		TRACE("link_event_filter = FIXUPDATE%s\n", reportBlinks ? ",BLINK" : "");
	*/
	if (!m_sampleMode)
	{
		eyecmd_printf("fixation_update_interval = 25");
		eyecmd_printf("fixation_update_accumulate = 25");
	}
	//	eyecmd_printf("draw_cross %u %u 15", 0, 0);
	error = start_recording(m_useEDF, m_useEDF,
		m_sampleMode,	// link samples
		1				// link events
	);
	if (error) {
		LeaveCriticalSection(&EyelinkCriticalSection);
		AfxMessageBox(_T("Can't start recording"), MB_OK | MB_ICONINFORMATION);
	//	EyelinkDisconnect();
		return;
	}
	if (!eyelink_wait_for_block_start(100, m_sampleMode, 1)) //wait for link events
	{
		LeaveCriticalSection(&EyelinkCriticalSection);
		AfxMessageBox(_T("No link events received"), MB_OK | MB_ICONINFORMATION);
	//	EyelinkDisconnect();
		return;
	}
	INT16 sampleRate;
	VERIFY(0 == eyelink_mode_data(&sampleRate, NULL, NULL, NULL));
	if (m_useEDF)
	{
		VERIFY(0 == eyemsg_printf("DISPLAY_COORDS %i %i %i %i",
			(short)m_screenWidth / -2,
			m_screenHeight / 2 - 1,
			m_screenWidth / 2 - 1,
			(short)m_screenHeight / -2));
	}
	TransformMsg();
	// determine which eye(s) are available
	m_eyeUsed = eyelink_eye_available();
	ASSERT(m_eyeUsed != -1);	// no eye data available
	bool binocular = (m_eyeUsed == BINOCULAR);
	if (binocular)
	{
		char buf[6] = "";
		INT16 result;
		INT16 nRetry = 0;
		UINT32 code = 0;
		eyelink_read_request("active_eye");
		do
		{
			Sleep(21);
			result = eyelink_read_reply(buf);
			if (result == OK_RESULT)
			{
				code = *((UINT32*)&buf[0]);
				TRACE("Reply: %s %u\n", buf, code);
				switch (code)
				{
				case 1413891404:	// LEFT
					m_eyeUsed = LEFT_EYE;
					break;
				case 1212631378:	// RIGHT
					m_eyeUsed = RIGHT_EYE;
					break;
				default:
					ASSERT(false);
				}
				break;	// do - while
			}
			else
			{
				TRACE("Waiting for reply\n");
				message_pump(); // keep Windows happy
				nRetry++;
			}
		} while (nRetry < 24);
		if (nRetry == 24)	// reply timeout
		{
			ASSERT(false);
		}
	}
	if (m_eyeUsedConf == -1) m_eyeUsedConf = m_eyeUsed;
	g_pView->m_eyeSelID.SetCurSel(m_eyeUsed);
	g_pView->m_eyeSelID.ShowWindow(SW_SHOW);
	g_pView->m_eyeSelID.EnableWindow(binocular);
	EyeUsedMessage();
	LeaveCriticalSection(&EyelinkCriticalSection);

	DWORD dueTime = m_sampleMode ? 1000 / sampleRate : 25;
	if (!CreateTimerQueueTimer(
		&m_hLinkTimer,
		NULL,		// default timer queue
		(WAITORTIMERCALLBACK)LinkTimerCallback,
		NULL,
		dueTime,	// due time (first callback)
		dueTime,	// period
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
		CString ErrorMsg;
		ErrorMsg.Format(_T("EyeLinkServer failed with error %d: %s"), dw, (LPTSTR)&lpMsgBuf);
		AfxMessageBox(ErrorMsg, MB_OK | MB_ICONSTOP);

		LocalFree(lpMsgBuf);
	}
	else
	{
		m_running = true;
	}
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
	TRACE(_T("App::EyelinkConnect\r\n"));
	m_pMainWnd->EnableWindow(FALSE);	// make EyeLinks's message box "modal"
	EnterCriticalSection(&EyelinkCriticalSection);
	if (open_eyelink_connection(mode))
	{
		//		AfxMessageBox(_T("Can't connect to EyeLink"), MB_OK | MB_ICONINFORMATION);	// no need for that: EyeLink generates MessageBox itself (at least at timeout)
		LeaveCriticalSection(&EyelinkCriticalSection);
		CEyeLinkServerView::SetStateString(_T("Mouse Simulation"));
		//		m_pMainWnd->ShowWindow(SW_RESTORE);
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

//	EnterCriticalSection(&EyelinkCriticalSection);
	// Read setting of "screen_pixel_coords" from tracker
	// This allows remapping of gaze data if our display
	// has a different resolution than the connected computer
	// The read may fail with older tracker software
	char buf[40] = "";
	INT16 result;
	INT16 nRetry = 0;
	//	UINT32 t;
	set_offline_mode();
	eyelink_read_request("screen_pixel_coords");
	do
	{
		Sleep(21);
		result = eyelink_read_reply(buf);
		if (result == OK_RESULT)
		{
			TRACE("Reply: %s\n", buf);
			VERIFY(4 == sscanf(buf, "%f,%f,%f,%f",
				&tracker_pixel_left,
				&tracker_pixel_top,
				&tracker_pixel_right,
				&tracker_pixel_bottom));
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
	
	m_eyeLinkIsConnected = true;
}


void CEyeLinkServerApp::EyelinkDisconnect(void)
{
	EyelinkStop(0,0);
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
	if (m_eyeLinkIsConnected && m_running)
	{
		CString msg;
		EyelinkOffline();
		// the g_hDone event is signaled when we see an "End Parse" event from EyeLink 
		if (WAIT_OBJECT_0 != WaitForSingleObject(g_hDone, 1000))
		{
			msg.Format(_T("No 'End Parse' event from EyeLink"));
			AfxMessageBox(msg, MB_OK | MB_ICONINFORMATION);
		}
		if (m_hLinkTimer) VERIFY(DeleteTimerQueueTimer(NULL, m_hLinkTimer, INVALID_HANDLE_VALUE));
		if (m_useEDF)
		{
			EnterCriticalSection(&EyelinkCriticalSection);
			VERIFY(!close_data_file());
			m_useEDF = 0;
			INT32 nBytes = receive_data_file_dialog("", (char*)message, 0);
			LeaveCriticalSection(&EyelinkCriticalSection);
			if ((nBytes <= 0) && (nBytes != -1))	// not success or dialog aborted
			{
				msg.Format(_T("Can't transfer EDF file to %S (error %d)"), message, nBytes);
				AfxMessageBox(msg, MB_OK | MB_ICONINFORMATION);
			}
		}
		TRACE("Used: %u, Conf: %u\n", m_eyeUsed, m_eyeUsedConf);
		if (m_eyeUsed != m_eyeUsedConf)
		{
			EnterCriticalSection(&EyelinkCriticalSection);
			INT16 result = eyecmd_printf("active_eye = %s", m_eyeUsed ? "RIGHT" : "LEFT");
			LeaveCriticalSection(&EyelinkCriticalSection);
			TRACE("Result (close): %u\n", result);
		}
	}
	m_running = false;
	VERIFY(SetEvent(m_hDone));
}



// This routine is critical because it's called from various threads.
void CEyeLinkServerApp::FixationUpdate(float x, float y)
{
	EnterCriticalSection(&m_criticalUpdate);
	tx = x;
	ty = y;
	if (g_pTransformation) g_pTransformation->Apply(tx, ty);
	*m_pShare++ = tx;
	*m_pShare-- = ty;
	pDoc->FixationUpdate(tx, ty);
	LeaveCriticalSection(&m_criticalUpdate);
}


void CEyeLinkServerApp::TransformMsg()
{
	if (m_useEDF && g_pTransformation)
	{
		char msg[256];
		g_pTransformation->xMsg(&msg[0]);
		VERIFY(0 == eyemsg_printf(msg));
		g_pTransformation->yMsg(&msg[0]);
		VERIFY(0 == eyemsg_printf(msg));
	}
	float x = fux;
	float y = fuy;
//	if (g_pTransformation) g_pTransformation->Apply(&x, &y);
	FixationUpdate(x, y);
}


void CEyeLinkServerApp::TransformClearMsg()
{
	EnterCriticalSection(&EyelinkCriticalSection);
	VERIFY(0 == eyemsg_printf("Transformation cleared"));
	LeaveCriticalSection(&EyelinkCriticalSection);
	FixationUpdate(fux, fuy);
}


void CEyeLinkServerApp::EyelinkPassMessage(unsigned char message[], DWORD messageLength)
{
	EnterCriticalSection(&EyelinkCriticalSection);
	VERIFY(0 == eyemsg_printf((char*) message));
	LeaveCriticalSection(&EyelinkCriticalSection);
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
	TRACE("Current Mode: %u\n", eyelink_current_mode());
	pCmdUI->Enable(m_eyeLinkIsConnected && (eyelink_current_mode() == IN_IDLE_MODE));
}


void CEyeLinkServerApp::OnCbnSelchangeCombo1()
{
	m_eyeUsed = g_pView->m_eyeSelID.GetCurSel();
	TRACE("New Selection: %u\n", m_eyeUsed);
	EyeUsedMessage();
}


void CEyeLinkServerApp::EyeUsedMessage()
{
	if (m_useEDF)
	{
		EnterCriticalSection(&EyelinkCriticalSection);
		switch (m_eyeUsed) // select eye, add annotation to EDF file
		{
		case RIGHT_EYE:
			eyemsg_printf("EYE_USED 1 RIGHT");
			break;
		case LEFT_EYE:
			eyemsg_printf("EYE_USED 0 LEFT");
			break;
		}
		LeaveCriticalSection(&EyelinkCriticalSection);
	}
}
