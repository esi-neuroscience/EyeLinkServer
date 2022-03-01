================================================================================
    MICROSOFT FOUNDATION CLASS LIBRARY : EyeLinkServer Project Overview
===============================================================================

The application wizard has created this EyeLinkServer application for
you.  This application not only demonstrates the basics of using the Microsoft
Foundation Classes but is also a starting point for writing your application.

This file contains a summary of what you will find in each of the files that
make up your EyeLinkServer application.

EyeLinkServer.vcxproj
    This is the main project file for VC++ projects generated using an application wizard.
    It contains information about the version of Visual C++ that generated the file, and
    information about the platforms, configurations, and project features selected with the
    application wizard.

EyeLinkServer.vcxproj.filters
    This is the filters file for VC++ projects generated using an Application Wizard. 
    It contains information about the association between the files in your project 
    and the filters. This association is used in the IDE to show grouping of files with
    similar extensions under a specific node (for e.g. ".cpp" files are associated with the
    "Source Files" filter).

EyeLinkServer.h
    This is the main header file for the application.  It includes other
    project specific headers (including Resource.h) and declares the
    CEyeLinkServerApp application class.

EyeLinkServer.cpp
    This is the main application source file that contains the application
    class CEyeLinkServerApp.

EyeLinkServer.rc
    This is a listing of all of the Microsoft Windows resources that the
    program uses.  It includes the icons, bitmaps, and cursors that are stored
    in the RES subdirectory.  This file can be directly edited in Microsoft
    Visual C++. Your project resources are in 1033.

res\EyeLinkServer.ico
    This is an icon file, which is used as the application's icon.  This
    icon is included by the main resource file EyeLinkServer.rc.

res\EyeLinkServer.rc2
    This file contains resources that are not edited by Microsoft
    Visual C++. You should place all resources not editable by
    the resource editor in this file.

/////////////////////////////////////////////////////////////////////////////

For the main frame window:
    The project includes a standard MFC interface.

MainFrm.h, MainFrm.cpp
    These files contain the frame class CMainFrame, which is derived from
    CFrameWnd and controls all SDI frame features.

/////////////////////////////////////////////////////////////////////////////

The application wizard creates one document type and one view:

EyeLinkServerDoc.h, EyeLinkServerDoc.cpp - the document
    These files contain your CEyeLinkServerDoc class.  Edit these files to
    add your special document data and to implement file saving and loading
    (via CEyeLinkServerDoc::Serialize).

EyeLinkServerView.h, EyeLinkServerView.cpp - the view of the document
    These files contain your CEyeLinkServerView class.
    CEyeLinkServerView objects are used to view CEyeLinkServerDoc objects.





Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named EyeLinkServer.pch and a precompiled types file named StdAfx.obj.

Resource.h
    This is the standard header file, which defines new resource IDs.
    Microsoft Visual C++ reads and updates this file.

EyeLinkServer.manifest
	Application manifest files are used by Windows XP to describe an applications
	dependency on specific versions of Side-by-Side assemblies. The loader uses this
	information to load the appropriate assembly from the assembly cache or private
	from the application. The Application manifest  maybe included for redistribution
	as an external .manifest file that is installed in the same folder as the application
	executable or it may be included in the executable in the form of a resource.
/////////////////////////////////////////////////////////////////////////////

Other notes:

The application wizard uses "TODO:" to indicate parts of the source code you
should add to or customize.

If your application uses MFC in a shared DLL, you will need
to redistribute the MFC DLLs. If your application is in a language
other than the operating system's locale, you will also have to
redistribute the corresponding localized resources MFC100XXX.DLL.
For more information on both of these topics, please see the section on
redistributing Visual C++ applications in MSDN documentation.

/////////////////////////////////////////////////////////////////////////////

1.0.0.2	04-Oct-2018	MSt	Fixed transformations bug
1.0.0.3 05-Oct-2018 MSt transformation messages to EDF file
1.0.0.4 12-Oct-2018 MSt "EyeServerDone" event; various fixes
1.1.0.0 19-Dec-2018 MSt Form based layout / simulation mode
1.1.0.1 29-Jan-2019 MSt Fix: crash on wrong state
1.1.0.2 31-Jan-2019 MSt EyeServerDone signaled on startup
1.1.0.3 31-Jan-2019 MSt Drawing through Invalidate
1.1.0.4 07-Feb-2019 MSt blink events
						To be conform with the former Matlab server, In- Out-
						events are now "manual reset"
1.1.0.5 12-Feb-2019 MSt "Clear Screen" command added to EyeLink menu
						circular targets are drawn as octagons
1.1.1.0 15-Feb-2019 MSt prevent multi-threading issues in EyeLink libraries
1.1.1.1 26-Feb-2019 MSt optionally set "Out" events on blinks
1.1.2.0 13-Mar-2019 MSt allow targets to overlap
1.1.3.0 18-Mar-2019 MSt support for biquad transformations
1.1.3.1 27-Mar-2019 MSt use transformed data for display and event generation
1.1.4.0 24-Apr-2019 MSt persistent frames; initialization of m_wasInside
1.1.5.0 13-May-2019 MSt draw to tracker screen only in offline mode
1.1.5.1 21-May-2019 MSt Support for 16 bit digital inputs
1.1.6.0 28-May-2019 MSt Honor FIXUPDATES of one eye only
1.1.6.1 06-Jun-2019 MSt speedup of start recording
1.1.6.2 13-Jun-2019 MSt bugfix in "EyeLink->Connect"
1.1.6.3 21-Jun-2019 MSt code clean up
1.1.6.4 01-Jul-2019 MSt popup error if no End Parse event on "stop"
1.1.7.0 01-Jul-2019 MSt new pass "Message" command
1.1.7.1 01-Aug-2019 MSt increased pipe's input buffer size from 128 to 256
						(to allow for very long file names)
1.2.0.0 18-Dec-2019 MSt make In- Out- events mutually exclusive
						exclusive behaviour can be disabled through a command
1.2.1.0 18-Mar-2020 MSt support to receive samples for speed up
1.2.2.0 13-Apr-2020 MSt support for EndDeferredMode events
1.2.2.1 18-May 2020 MSt bug fix: exit after close
1.2.3.0 19-May 2020 MSt support for start saccade events
1.2.3.1 02-Feb-2022 MSt support for start/stop fixation events
1.2.4.0 16-Feb-2022 MSt use gaze data from fixation end and saccade end
1.2.4.1 20-Feb-2022 MSt use gav* rather than gen*
1.2.4.2 21-Feb-2022 MSt use gen* with end fixation and end saccade events
1.2.4.3 01-Mar-2022 MSt use gav* with end fixation events
1.2.4.4 01-Mar-2022 MSt no gaze evaluation on end fixation events