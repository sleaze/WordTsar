//////////////////////////////////////////////////////////////////////////////
//
// WordTsar - Wordstar clone for modern systems http://wordtsar.ca
// Copyright (C) 2018 Gerald Brandt
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if !wxUSE_DEBUGREPORT
#error "This sample can't be built without wxUSE_DEBUGREPORT"
#endif // wxUSE_DEBUGREPORT

#if !wxUSE_ON_FATAL_EXCEPTION
#error "This sample can't be built without wxUSE_ON_FATAL_EXCEPTION"
#endif // wxUSE_ON_FATAL_EXCEPTION

#include <wx/datetime.h>
#include <wx/ffile.h>
#include <wx/filename.h>
#include "wx/debugrpt.h"

////@begin includes
////@end includes

#include "wordtsarapp.h"
#include "../include/version.h"
#include "../test/test.h"

////@begin XPM images
////@end XPM images


/*
 * Application instance implementation
 */

////@begin implement app
IMPLEMENT_APP( WordTsarApp )
////@end implement app


/*
 * WordTsarApp type definition
 */

IMPLEMENT_CLASS( WordTsarApp, wxApp )


/*
 * WordTsarApp event table definition
 */

BEGIN_EVENT_TABLE( WordTsarApp, wxApp )

////@begin WordTsarApp event table entries
////@end WordTsarApp event table entries

END_EVENT_TABLE()


/*
 * Constructor for WordTsarApp
 */

WordTsarApp::WordTsarApp()
{
    wxHandleFatalExceptions() ;
    Init();
}


/*
 * Member initialisation
 */

void WordTsarApp::Init()
{
////@begin WordTsarApp member initialisation
////@end WordTsarApp member initialisation
}

/*
 * Initialisation for WordTsarApp
 */

bool WordTsarApp::OnInit()
{

////@begin WordTsarApp initialisation
	// Remove the comment markers above and below this block
	// to make permanent changes to the code.

#if wxUSE_XPM
	wxImage::AddHandler(new wxXPMHandler);
#endif
#if wxUSE_LIBPNG
	wxImage::AddHandler(new wxPNGHandler);
#endif
#if wxUSE_LIBJPEG
	wxImage::AddHandler(new wxJPEGHandler);
#endif
#if wxUSE_GIF
	wxImage::AddHandler(new wxGIFHandler);
#endif

    WordTsar *mainWindow = new WordTsar( NULL );
	mainWindow->Show(true);
////@end WordTsarApp initialisation
    // someone passed a file name as an argument
    if(argc > 1)
    {
        wxString arg(argv[1]);
        mInitialFileName = arg ;
        mainWindow->SetFile(mInitialFileName) ;
    }

#ifdef DO_TEST
    cTest test ;
    test.StartTest() ;
    wxExit() ;
#endif // DO_TEST

    return true;
}


/*
 * Cleanup for WordTsarApp
 */

int WordTsarApp::OnExit()
{
////@begin WordTsarApp cleanup
	return wxApp::OnExit();
////@end WordTsarApp cleanup
}

void WordTsarApp::OnFatalException()
{
    GenerateReport(wxDebugReport::Context_Exception);
}

void WordTsarApp::GenerateReport(wxDebugReport::Context ctx)
{
    wxDebugReportCompress report ;
    wxDebugReportPreviewStd preview ;

    // add all standard files: currently this means just a minidump and an
    // XML file with system info and stack trace
    report.AddAll(ctx);

    // you can also call report->AddFile(...) with your own log files, files
    // created using wxRegKey::Export() and so on, here we just add a test
    // file containing the date of the crash
    wxFileName fn(report.GetDirectory(), wxT("version.my"));
    wxFFile file(fn.GetFullPath(), wxT("w"));
    if ( file.IsOpened() )
    {
        wxDateTime dt = wxDateTime::Now();
        file.Write(dt.FormatISODate() + wxT(' ') + dt.FormatISOTime() + wxT(' ') +  FULLVERSION_STRING);
        file.Close();
    }

    report.AddFile(fn.GetFullName(), wxT("timestamp of this report and WordTsar version number"));

    // can also add an existing file directly, it will be copied
    // automatically
#ifdef __WXMSW__
    report.AddFile(wxT("c:\\autoexec.bat"), wxT("DOS startup file"));
#elif __WXGTK__
    report.AddFile(wxT("/etc/issue"), wxT("OS Identifier"));
#endif

    // calling Show() is not mandatory, but is more polite
    if ( preview.Show(report) )
    {
        report.Process() ;

        wxString str ;
        str.Printf("Report Generated.  Please email file \"%s\" to support@wordtsar.ca", report.GetCompressedFileName()) ;
        wxMessageBox(str, "Debug Report Generated") ;
        report.Reset();
    }
}
