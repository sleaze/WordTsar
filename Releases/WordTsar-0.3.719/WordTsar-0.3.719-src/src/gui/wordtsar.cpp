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

#ifdef PROFILE
#include "gperftools/profiler.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
#include "wx/mstream.h"
#include "editor/editorctrl.h"
#include "ruler/rulerctrl.h"
////@end includes

#include "wordtsar.h"
#include "editor/editorctrl.h"
#include "ruler/rulerctrl.h"


////@begin XPM images
#include "images/icon64x64.inc"
////@end XPM images


#include "../../third-party/simpleini/SimpleIni.h"


const char HELPDELAY = 4 ;          // wait x * 200 ms before showing help

/*
 * WordTsar type definition
 */

IMPLEMENT_CLASS( WordTsar, wxFrame )


/*
 * WordTsar event table definition
 */

BEGIN_EVENT_TABLE( WordTsar, wxFrame )

////@begin WordTsar event table entries
    EVT_CLOSE( WordTsar::OnCloseWindow )
////@end WordTsar event table entries

END_EVENT_TABLE()


/*
 * WordTsar constructors
 */

WordTsar::WordTsar()
{
    Init();
}


WordTsar::WordTsar( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create( parent, id, caption, pos, size, style );
}


/*
 * WordTsar creator
 */

bool WordTsar::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin WordTsar creation
    wxFrame::Create( parent, id, caption, pos, size, style );

    CreateControls();
    SetIcon(GetIconResource(wxT("images/icon64x64.png")));
////@end WordTsar creation
    return true;
}


/*
 * WordTsar destructor
 */

WordTsar::~WordTsar()
{
    WriteConfig() ;
    
////@begin WordTsar destruction
////@end WordTsar destruction
#ifdef PROFILE
    ProfilerFlush();
    ProfilerStop();
#endif
}


/*
 * Member initialisation
 */

void WordTsar::Init()
{
#ifdef PROFILE
ProfilerStart("/home/gbr/gperf/dump.txt");
#endif

////@begin WordTsar member initialisation
    mEditorSizer = NULL;
    mStatusTop = NULL;
    mHelpCtrl = NULL;
    mHelpPCtrl = NULL;
    mHelpKCtrl = NULL;
    mHelpQCtrl = NULL;
    mHelpOCtrl = NULL;
    mRuler = NULL;
    mEditor = NULL;
    mStatusBottom = NULL;
////@end WordTsar member initialisation

    mStatusBottom = NULL ;
    
    mLoadFileName.clear() ;
}


/*
 * Control creation for WordTsar
 */

void WordTsar::CreateControls()
{
    static wxLog* logger = nullptr;

    if (logger == nullptr)
    {
#ifdef DEBUG
        logger = new wxLogWindow(this, "WordTsar Messages", true, false);
#else
        logger = new wxLogStderr();
#endif
    }
    wxLog::SetActiveTarget(logger);


////@begin WordTsar content construction
    WordTsar* itemFrame1 = this;

    mEditorSizer = new wxBoxSizer(wxVERTICAL);
    itemFrame1->SetSizer(mEditorSizer);

    mResizeIndicator = new wxPanel(itemFrame1, -1, wxDefaultPosition, wxSize(100, 1), wxNO_BORDER);
    mEditorSizer->Add(mResizeIndicator, 0, wxGROW | wxALL, 0);

    mStatusTop = new wxStatusBar( itemFrame1, ID_STATUSBAR, wxNO_BORDER );
    mStatusTop->Enable(false);
    mStatusTop->SetFieldsCount(11);
    int mStatusTopWidths[11];
    mStatusTopWidths[0] = -20;
    mStatusTopWidths[1] = -20;
    mStatusTopWidths[2] = 20;
    mStatusTopWidths[3] = 20;
    mStatusTopWidths[4] = 20;
    mStatusTopWidths[5] = 20;
    mStatusTopWidths[6] = 20;
    mStatusTopWidths[7] = 20;
    mStatusTopWidths[8] = 20;
    mStatusTopWidths[9] = 20;
    mStatusTopWidths[10] = 40;
    mStatusTop->SetStatusWidths(11, mStatusTopWidths);
    mEditorSizer->Add(mStatusTop, 0, wxGROW|wxALL, 0);

    mHelpCtrl = new cEditorCtrl( itemFrame1, ID_HELPCTRL, wxDefaultPosition, wxSize(100, 140), wxNO_BORDER );
    mHelpCtrl->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Andale Mono")));
    mHelpCtrl->Enable(false);
    mEditorSizer->Add(mHelpCtrl, 0, wxGROW|wxALL, 0);

    mHelpPCtrl = new cEditorCtrl( itemFrame1, ID_HELPPCTRL, wxDefaultPosition, wxSize(100, 170), wxNO_BORDER );
    mHelpPCtrl->SetForegroundColour(wxColour(0, 0, 0));
    mHelpPCtrl->SetBackgroundColour(wxColour(191, 191, 191));
    mHelpPCtrl->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Andale Mono")));
    mHelpPCtrl->Show(false);
    mHelpPCtrl->Enable(false);
    mEditorSizer->Add(mHelpPCtrl, 0, wxGROW|wxALL, 0);

    mHelpKCtrl = new cEditorCtrl( itemFrame1, ID_HELPKCTRL, wxDefaultPosition, wxSize(100, 170), wxNO_BORDER );
    mHelpKCtrl->SetForegroundColour(wxColour(0, 0, 0));
    mHelpKCtrl->SetBackgroundColour(wxColour(191, 191, 191));
    mHelpKCtrl->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Andale Mono")));
    mHelpKCtrl->Show(false);
    mHelpKCtrl->Enable(false);
    mEditorSizer->Add(mHelpKCtrl, 0, wxGROW|wxALL, 0);

    mHelpQCtrl = new cEditorCtrl( itemFrame1, ID_HELPQCTRL, wxDefaultPosition, wxSize(100, 170), wxNO_BORDER );
    mHelpQCtrl->SetForegroundColour(wxColour(0, 0, 0));
    mHelpQCtrl->SetBackgroundColour(wxColour(191, 191, 191));
    mHelpQCtrl->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Andale Mono")));
    mHelpQCtrl->Show(false);
    mHelpQCtrl->Enable(false);
    mEditorSizer->Add(mHelpQCtrl, 0, wxGROW|wxALL, 0);

    mHelpOCtrl = new cEditorCtrl( itemFrame1, ID_HELPOCTRL, wxDefaultPosition, wxSize(100, 170), wxNO_BORDER );
    mHelpOCtrl->SetForegroundColour(wxColour(0, 0, 0));
    mHelpOCtrl->SetBackgroundColour(wxColour(191, 191, 191));
    mHelpOCtrl->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Andale Mono")));
    mHelpOCtrl->Show(false);
    mHelpOCtrl->Enable(false);
    mEditorSizer->Add(mHelpOCtrl, 0, wxGROW|wxALL, 0);

    mRuler = new cRulerCtrl( itemFrame1, ID_RULERPANEL, wxDefaultPosition, wxSize(-1, 33), wxNO_BORDER );
    mRuler->SetBackgroundColour(wxColour(191, 191, 191));
    mEditorSizer->Add(mRuler, 0, wxGROW|wxALL, 0);

    mEditor = new cEditorCtrl( itemFrame1, ID_EDITORCTRL, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxWANTS_CHARS );
    mEditor->SetName(wxT("Main Editor"));
    mEditor->SetForegroundColour(wxColour(0, 0, 0));
    mEditor->SetFont(wxFont(12, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Courier 10 Pitch")));
    mEditorSizer->Add(mEditor, 1, wxGROW|wxALL, 0);

    mStatusBottom = new wxStatusBar( itemFrame1, ID_BOTTOMSTATUSBAR, wxNO_BORDER );
    mStatusBottom->Enable(false);
    mStatusBottom->SetFieldsCount(2);
    int mStatusBottomWidths[2];
    mStatusBottomWidths[0] = -25;
    mStatusBottomWidths[1] = -75;
    mStatusBottom->SetStatusWidths(2, mStatusBottomWidths);
    mEditorSizer->Add(mStatusBottom, 0, wxGROW|wxALL, 0);

////@end WordTsar content construction
    
    mEditor->SetFrame(this) ;  // for update of status
    
/// @todo temp
//menuBar->Hide() ;

	// Under Windows, specify a better font
#ifdef __WIN32__
	mHelpCtrl->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Consolas")));
	mHelpPCtrl->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Consolas")));
	mHelpKCtrl->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Consolas")));
	mHelpQCtrl->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Consolas")));
	mHelpOCtrl->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Consolas")));
#endif
    mHelpCtrl->SetEditorName("Base Help") ;
    mHelpKCtrl->SetEditorName("ctrl-K Help") ;
    mHelpOCtrl->SetEditorName("ctrl-O Help") ;
    mHelpPCtrl->SetEditorName("ctrl-P Help") ;
    mHelpQCtrl->SetEditorName("ctrl-Q Help") ;
    mEditor->SetEditorName("Main Editor") ;
    
//	mHelpCtrl->Disable() ;
//    mHelpKCtrl->Disable() ;
//    mHelpOCtrl->Disable() ;
//    mHelpPCtrl->Disable() ;
//    mHelpQCtrl->Disable() ;

    mHelpCtrl->SetAsHelp() ;
    mHelpKCtrl->SetAsHelp() ;
    mHelpOCtrl->SetAsHelp() ;
    mHelpPCtrl->SetAsHelp() ;
    mHelpQCtrl->SetAsHelp() ;

    // the ruler
    mRuler->Disable() ;
    mEditor->SetRuler(mRuler) ;

    ReadConfig() ;

    // create the bottom status bar
//    mStatusBottom = CreateStatusBar(2) ;
//    int widths[2] ;
//    widths[0] = -25 ;
//    widths[1] = -75 ;
//    SetStatusWidths(2, widths) ;

    // Help Screens
    
    // these strings are stored in internal buffer format for easy displaying
    wxString helpj(wxString::FromUTF8("                        ----- E D I T   M E N U -----\n" \
	"  CURSOR      SCROLL        DELETE      OTHER               MENUS\n" \
	" ^E up       ^W up         ^G char    ^J help             ^O onscreen format\n" \
	" ^X down     ^Z down       ^T word    ^I tab              ^K block & save\n" \
	" ^S left     ^R up screen  ^Y line    ^V turn insert off  ^M macros\n" \
	" ^D right    ^C down      Del char    F1 Settings         ^P print controls\n" \
	" ^A word left   screen     ^U unerase ^N split the line   ^Q quick functions\n" \
	" ^F word right                        ^L find/replace again  F11 Full Screen")) ;
    mHelpCtrl->SetAsHelp() ;
    mHelpCtrl->InsertWordStarString(helpj) ;

     wxString helpk(wxString::FromUTF8("                  ----- B L O C K   &   S A V E   M E N U -----\n" \
    "    SAVE                     BLOCK                       WINDOW\n" \
    "  D save                   B begin block               A copy between\n" \
    "  T save as                K end block                 G move between\n" \
    "  S save and resume        C copy                      \n" \
    "  X save and exit          V move                        CASE\n" \
    "  Q abandon changes        Y delete                    \" upper\n" \
    "    FILE                   W write to disk             \' lower\n" \
    "  O copy                   M math                      . sentence\n" \
    "  E rename                 Z sort                      \n" \
    "  J delete                 ? word count                  CURSOR\n" \
    "  P print                  H turn disp on/off        0-9 set marker\n" \
    "  \\ fax                    U mark previous block       \n" \
    "  L change drive/dir       < unmark block                SYSTEM CLIPBOARD\n" \
    "  R insert a file          N turn column mode on       [ copy from clipboard\n" \
    "  F run command            I turn column replace on    ] copy to clipboard\n")) ;
    mHelpKCtrl->SetAsHelp() ;
    mHelpKCtrl->InsertWordStarString(helpk) ;
    
    wxString helpp(wxString::FromUTF8("              ----- P R I N T   C O N T R O L S   M E N U -----\n" \
	"            BEGIN & END                                OTHER \n" \
	"    B bold         X strike out         H overprint char   O binding space\n" \
	"    S underline    D double strike      \u21b5 overprint line   C print pause\n" \
	"    V subscript    Y italics            F phantom space    I 8-column tab\n" \
	"    T superscript  K indexing           G phantom rubout   . dot leader\n" \
	"                                        * graphics tag     0 extended characters\n" \
	"               STYLE                    & start Inset\n" \
	"    =/+ select font N Normal Font\n" \
	"    - select color  A alternate font    Q W E R ! custom    ? select printer")) ;
    mHelpPCtrl->SetAsHelp() ;
    mHelpPCtrl->InsertWordStarString(helpp) ;


    wxString helpq(wxString::FromUTF8("                      ----- Q U I C K   M E N U -----\n" \
	"            CURSOR              FIND            OTHER             SPELL\n" \
	" E upper left   P previous   F find text     U align rest doc  L check document\n" \
	" X lower right  V prev find  A find/replace  M math  Q repeat  N check word\n" \
	" S begin line   B beg block  G char forward  J thesaurus       O enter word\n" \
	" D end line     K end block  H char back                         DELETE\n" \
	" R beg doc    0-9 marker     I page/line       SCROLL        Del line to left\n" \
	" C end doc                   = next font     W up, repeat      Y line to right\n" \
	"                             < next style    Z dn, repeat      T to character\n")) ;
    mHelpQCtrl->SetAsHelp() ;
    mHelpQCtrl->InsertWordStarString(helpq) ;


    wxString helpo(wxString::FromUTF8("           ----- O N S C R E E N   F O R M A T   M E N U -----\n" \
    "   MARGINS & TABS            TYPING                         DISPLAY\n" \
    " L left  R right     C center line                   P page preview\n" \
    " G temorary indent   ] right flush line              D turn command tags off\n" \
    " X release margin    V vertically center             B change screen settings\n" \
    " I set/clear tabs    E enter soft hyphen             K open or switch window\n" \
    " O ruler to text     H turn auto-hyphenation off     M size current window\n" \
    " U column layout     J turn justification on         ? status\n" \
    " Y page layout       A turn auto-align off           Z paragraph number\n" \
    " F paragraph styles  W turn word wrap off            # page numbering\n" \
    " S set line spacing  \u21b5  turn Enter closes dialog off  N notes\n")) ;
    mHelpOCtrl->SetAsHelp() ;
    mHelpOCtrl->InsertWordStarString(helpo) ;

    mEditor->SetFocusIgnoringChildren() ;
    
/*
#ifndef DOTEST
    if(mLoadFileName.length() != 0)
    {
        wxString filename = mLoadFileName ;

        // move file name and directory into holders
        int slash = filename.Find('/', true) ;
        if(slash == wxNOT_FOUND)
        {
            slash = filename.Find('\\', true) ;
        }
        if(slash != wxNOT_FOUND)
        {
            mEditor->mFileDir = filename.Left(slash + 1) ;
            mEditor->mFileName = filename.Right(filename.length() - slash) ;
        }
    }
#endif
*/
}

///< @todo loading here causes weird focus issue
void WordTsar::SetFile(wxString filename)
{
    mLoadFileName = filename ;
    wxYield() ;
    mEditor->LoadFile(mLoadFileName) ;
}

/*
 * Should we show tooltips?
 */

bool WordTsar::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap WordTsar::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin WordTsar bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end WordTsar bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon WordTsar::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin WordTsar icon retrieval
    wxUnusedVar(name);
    if (name == wxT("images/icon64x64.png"))
    {
        wxMemoryInputStream memStream(icon64x64_png, sizeof(icon64x64_png));
        wxBitmap bitmap(wxImage(memStream, wxBITMAP_TYPE_ANY, -1), -1);
        wxIcon icon;
        icon.CopyFromBitmap(bitmap);
        return icon;
    }
    return wxNullIcon;
////@end WordTsar icon retrieval

}


void WordTsar::UpdateStatus(cEditorCtrl *editor)
{
    if(editor == NULL)
    {
        return ;
    }
    
    if(mStatusBottom == NULL)
    {
        return ;
    }

    static char counter = 0 ;
    static eHelpDisplay lasthelp = HELP_MAIN ;
    
    sStatus cstatus ;
    editor->GetStatus(cstatus) ;
    
    time_t now = time(NULL) ;
    tm *t2 = localtime(&now) ;
    
    wxString status ;
    
    switch(mEditor->mMeasure)
    {
        case MSR_INCHES :
            status.Printf("Page %ld of %ld    line %ld    V:%0.2f in    column %ld    %s    %ld %s    %ld %s    %s  %s   %2d:%02d:%02d", 
                        cstatus.page, cstatus.pagecount, cstatus.line, cstatus.height / TWIPSPERINCH , cstatus.column, (cstatus.mode ? "Insert" : "Overwrite"),
                        cstatus.wordcount, (cstatus.wordcount == 1 ? "Word" : "Words"), cstatus.charcount, (cstatus.charcount == 1 ? "Character" : "Characters"), (cstatus.showcontrol ? "Show" : "Hide"),
                        (cstatus.saving ? "- Saving...    " : ""), t2->tm_hour, t2->tm_min, t2->tm_sec) ;
            break ;
        
        case MSR_MILLIMETERS :
            status.Printf("Page %ld of %ld    line %ld    V:%0.2f mm    column %ld    %s    %ld %s    %ld %s    %s  %s   %2d:%02d:%02d",
                        cstatus.page, cstatus.pagecount, cstatus.line, cstatus.height / TWIPSPERMM , cstatus.column, (cstatus.mode ? "Insert" : "Overwrite"),
                        cstatus.wordcount, (cstatus.wordcount == 1 ? "Word" : "Words"), cstatus.charcount, (cstatus.charcount == 1 ? "Character" : "Characters"), (cstatus.showcontrol ? "Show" : "Hide"),
                        (cstatus.saving ? "- Saving...    " : ""), t2->tm_hour, t2->tm_min, t2->tm_sec) ;
            break ;

        default :
            status.Printf("Page %ld of %ld    line %ld    V:%0.2f cm    column %ld    %s    %ld %s    %ld %s    %s  %s   %2d:%02d:%02d",
                        cstatus.page, cstatus.pagecount, cstatus.line, cstatus.height / TWIPSPERCM , cstatus.column, (cstatus.mode ? "Insert" : "Overwrite"),
                        cstatus.wordcount, (cstatus.wordcount == 1 ? "Word" : "Words"), cstatus.charcount, (cstatus.charcount == 1 ? "Character" : "Characters"), (cstatus.showcontrol ? "Show" : "Hide"),
                        (cstatus.saving ? "- Saving...    " : ""), t2->tm_hour, t2->tm_min, t2->tm_sec) ;
            break ;
    }
    
    mStatusBottom->SetStatusText(status, 1) ; // << status ;
    
//    if(cstatus.style != oldstatus.style || first == true)
    {
        mStatusTop->SetStatusText(cstatus.style, 0) ;
    }

 //   if(cstatus.font != oldstatus.font || first == true)
    {
        mStatusTop->SetStatusText(cstatus.font, 1) ;
    }

    mStatusTop->SetStatusText(cstatus.filled, 10) ;

    wxClientDC dc(mStatusTop) ;
//    if(cstatus.showcontrol != oldstatus.showcontrol || first == true)
    {
        if(cstatus.showcontrol)
        {
            StatusOn(5, "*", dc) ;
        }
        else
        {
            StatusOff(5, "*", dc) ;
        }
    }

//    if(cstatus.bold != oldstatus.bold || first == true)
    {
        if(cstatus.bold)
        {
            StatusOn(2, "B", dc) ;
        }
        else
        {
            StatusOff(2, "B", dc) ;
        }
    }

//    if(cstatus.italic != oldstatus.italic || first == true)
    {
        if(cstatus.italic)
        {
            StatusOn(3, "I", dc) ;
        }
        else
        {
            StatusOff(3, "I", dc) ;
        }
    }

//    if(cstatus.underline != oldstatus.underline || first == true)
    {
        if(cstatus.underline)
        {
            StatusOn(4, "U", dc) ;
        }
        else
        {
            StatusOff(4, "U", dc) ;
        }
    }

//    if(cstatus.just != oldstatus.just || first == true)
    {
        if(cstatus.just == JUST_LEFT)
        {
            StatusOn(6, "L", dc) ;
            StatusOff(7, "C", dc) ;
            StatusOff(8, "R", dc) ;
            StatusOff(9, "J", dc) ;
        }
        else if(cstatus.just == JUST_CENTER)
        {
            StatusOff(6, "L", dc) ;
            StatusOn(7, "C", dc) ;
            StatusOff(8, "R", dc) ;
            StatusOff(9, "J", dc) ;
        }
        else if(cstatus.just == JUST_RIGHT)
        {
            StatusOff(6, "L", dc) ;
            StatusOff(7, "C", dc) ;
            StatusOn(8, "R", dc) ;
            StatusOff(9, "J", dc) ;
        }
        else if(cstatus.just == JUST_JUST)
        {
            StatusOff(6, "L", dc) ;
            StatusOff(7, "C", dc) ;
            StatusOff(8, "R", dc) ;
            StatusOn(9, "J", dc) ;
        }
    }

    if(cstatus.help == HELP_NONE && lasthelp != HELP_NONE)
    {
        mHelpCtrl->HideWithEffect(wxSHOW_EFFECT_SLIDE_TO_TOP, 100) ;
        mHelpKCtrl->HideWithEffect(wxSHOW_EFFECT_SLIDE_TO_TOP, 100) ;
        mHelpPCtrl->HideWithEffect(wxSHOW_EFFECT_SLIDE_TO_TOP, 100) ;
        mHelpQCtrl->HideWithEffect(wxSHOW_EFFECT_SLIDE_TO_TOP, 100) ;
        mHelpOCtrl->HideWithEffect(wxSHOW_EFFECT_SLIDE_TO_TOP, 100) ;
        lasthelp = HELP_NONE ;
        mEditorSizer->Layout() ;
    }
    else if(cstatus.help == HELP_MAIN && lasthelp != HELP_MAIN)
    {
        mHelpCtrl->Show() ;
        mHelpKCtrl->HideWithEffect(wxSHOW_EFFECT_SLIDE_TO_TOP, 100) ;
        mHelpPCtrl->HideWithEffect(wxSHOW_EFFECT_SLIDE_TO_TOP, 100) ;
        mHelpQCtrl->HideWithEffect(wxSHOW_EFFECT_SLIDE_TO_TOP, 100) ;
        mHelpOCtrl->HideWithEffect(wxSHOW_EFFECT_SLIDE_TO_TOP, 100) ;
        lasthelp = HELP_MAIN ;
        mEditorSizer->Layout() ;
    }

    if(cstatus.help == HELP_CTRLK && lasthelp != HELP_CTRLK)
    {
        counter = 0 ;
        lasthelp = HELP_CTRLK ;
    }
    if(cstatus.help == HELP_CTRLK && counter == HELPDELAY)
    {
        mHelpCtrl->HideWithEffect(wxSHOW_EFFECT_SLIDE_TO_TOP, 100) ;
        mHelpKCtrl->Show() ;
        mHelpPCtrl->HideWithEffect(wxSHOW_EFFECT_SLIDE_TO_TOP, 100) ;
        mHelpQCtrl->HideWithEffect(wxSHOW_EFFECT_SLIDE_TO_TOP, 100) ;
        mHelpOCtrl->HideWithEffect(wxSHOW_EFFECT_SLIDE_TO_TOP, 100) ;
        lasthelp = HELP_CTRLK ;
        mEditorSizer->Layout() ;
    }

    if(cstatus.help == HELP_CTRLP && lasthelp != HELP_CTRLP)
    {
        counter = 0 ;
        lasthelp = HELP_CTRLP ;
    }
    if(cstatus.help == HELP_CTRLP && counter == HELPDELAY)
    {
        mHelpCtrl->HideWithEffect(wxSHOW_EFFECT_SLIDE_TO_TOP, 100) ;
        mHelpKCtrl->HideWithEffect(wxSHOW_EFFECT_SLIDE_TO_TOP, 100) ;
        mHelpPCtrl->Show() ;
        mHelpQCtrl->HideWithEffect(wxSHOW_EFFECT_SLIDE_TO_TOP, 100) ;
        mHelpOCtrl->HideWithEffect(wxSHOW_EFFECT_SLIDE_TO_TOP, 100) ;
        lasthelp = HELP_CTRLP ;
        mEditorSizer->Layout() ;
    }

    if(cstatus.help == HELP_CTRLQ && lasthelp != HELP_CTRLQ)
    {
        counter = 0 ;
        lasthelp = HELP_CTRLQ ;
    }
    if(cstatus.help == HELP_CTRLQ && counter == HELPDELAY)
    {
        mHelpCtrl->HideWithEffect(wxSHOW_EFFECT_SLIDE_TO_TOP, 100) ;
        mHelpKCtrl->HideWithEffect(wxSHOW_EFFECT_SLIDE_TO_TOP, 100) ;
        mHelpPCtrl->HideWithEffect(wxSHOW_EFFECT_SLIDE_TO_TOP, 100) ;
        mHelpQCtrl->Show() ;
        mHelpOCtrl->HideWithEffect(wxSHOW_EFFECT_SLIDE_TO_TOP, 100) ;
        lasthelp = HELP_CTRLQ ;
        mEditorSizer->Layout() ;
    }

    if(cstatus.help == HELP_CTRLO && lasthelp != HELP_CTRLO)
    {
        counter = 0 ;
        lasthelp = HELP_CTRLO ;
    }
    if(cstatus.help == HELP_CTRLO && counter == HELPDELAY)
    {
        mHelpCtrl->HideWithEffect(wxSHOW_EFFECT_SLIDE_TO_TOP, 100) ;
        mHelpKCtrl->HideWithEffect(wxSHOW_EFFECT_SLIDE_TO_TOP, 100) ;
        mHelpPCtrl->HideWithEffect(wxSHOW_EFFECT_SLIDE_TO_TOP, 100) ;
        mHelpQCtrl->HideWithEffect(wxSHOW_EFFECT_SLIDE_TO_TOP, 100) ;
        mHelpOCtrl->Show() ;
        lasthelp = HELP_CTRLO ;
        mEditorSizer->Layout() ;
    }
/*
    if(mEditor->mDispScrollBar)
    {
//        GetScrollbar()->Show() ;
    }
    else
    {
//        GetScrollbar()->Hide() ;
    }
    
    if(mEditor->mDispStyleBar)
    {
        mStatusTop->Show() ;
    }
    else
    {
        mStatusTop->Hide() ;
    }

    if(mEditor->mDispRuler)
    {
        mRuler->Show() ;
    }
    else
    {
        mRuler->Hide() ;
    }

    if(mEditor->mDispStatusBar)
    {
        mStatusBottom->Show() ;
    }
    else
    {
        mStatusBottom->Hide() ;
    }

    mEditorSizer->Layout() ;
*/
    counter++ ;
}



void WordTsar::SetStatus(wxString text, bool progress, int percent)
{
    static int oldpercent = 0 ;
    
    if(mStatusBottom != NULL)
    {
        if(progress == true && oldpercent != percent)
        {
            wxRect r ;
            mStatusBottom->GetFieldRect(0, r) ;
            
            wxClientDC dc(mStatusBottom) ;
            
            r.width = r.width * ((double)percent / 100.0) ;
        
            wxColor green(69, 139, 0) ;
            dc.SetPen(green) ;
            dc.SetBrush(green) ;
            dc.DrawRectangle(r) ;
            dc.DrawText(text, 5, 2) ;
            
            oldpercent = percent ;
        }
        else
        {
            mStatusBottom->SetStatusText(text, 0) ; // << status ;
        }
    }
//    wxYield() ;
}


void WordTsar::StatusOn(int index, wxString text, wxClientDC &dc)
{
//    wxClientDC dc(mStatusTop);

//    dc.SetBrush(wxColor(150, 150, 225)) ;
//#ifdef __WXOSX__
//    dc.SetTextForeground(*wxRED) ;
//    dc.SetPen(*wxRED_PEN);
//#endif // __WXOSX__
    wxRect r;
//    if ( mStatusTop->GetFieldRect(index, r))
//    {
//        dc.DrawRectangle(r);
//    }

    mStatusTop->GetFieldRect(index, r) ;
    
    dc.SetTextForeground(*wxBLACK) ;
    dc.DrawText(text, r.x + 4, r.y + 2) ;
}


void WordTsar::StatusOff(int index, wxString text, wxClientDC &dc)
{
//    wxClientDC dc(mStatusTop);
//    dc.SetPen(*wxRED_PEN);
//    dc.SetBrush( mStatusTop->GetBackgroundColour() ) ;

    wxRect r;
//    if ( mStatusTop->GetFieldRect(index, r) )
//    {
//        dc.DrawRectangle(r);
//    }

    mStatusTop->GetFieldRect(index, r) ;
    dc.SetTextForeground(wxColor(160, 160, 160)) ;
    dc.DrawText(text, r.x + 4, r.y + 2) ;
}



void WordTsar::ReadConfig(void)
{
    CSimpleIniA ini ;

    wxString dir = wxGetHomeDir() ;
    dir += "/.WordTsar" ;

    if(ini.LoadFile(dir.mb_str()) >= 0)
    {
        bool t =  ini.GetBoolValue("base", "showcontrols", true) ;
        if(t)
        {
            mEditor->SetShowControls(SHOW_ALL) ;
        }
        else
        {
            mEditor->SetShowControls(SHOW_DOT) ;
        }

        short red, green, blue ;
        red = ini.GetLongValue("base", "backgroundred", 245) ;
        green = ini.GetLongValue("base", "backgroundgreen", 245) ;
        blue = ini.GetLongValue("base", "backgroundblue", 245) ;
//        mEditor->SetBackgroundColour(wxColour(red, green, blue)) ;
        mEditor->SetBGroundColour(wxColour(red, green, blue)) ;

        red = ini.GetLongValue("base", "textred", 0) ;
        green = ini.GetLongValue("base", "textgreen", 0) ;
        blue = ini.GetLongValue("base", "textblue", 0) ;
        mEditor->SetTextColour(wxColour(red, green, blue)) ;

        red = ini.GetLongValue("base", "highlightred", 0) ;
        green = ini.GetLongValue("base", "highlightgreen", 150) ;
        blue = ini.GetLongValue("base", "highlightblue", 200) ;
        mEditor->SetHighlightColour(wxColour(red, green, blue, 127)) ;

        red = ini.GetLongValue("base", "dotred", 100) ;
        green = ini.GetLongValue("base", "dotgreen", 200) ;
        blue = ini.GetLongValue("base", "dotblue", 200) ;
        mEditor->SetDotColour(wxColour(red, green, blue, 190)) ;

        red = ini.GetLongValue("base", "blockred", 50) ;
        green = ini.GetLongValue("base", "blockgreen", 100) ;
        blue = ini.GetLongValue("base", "blockblue", 200) ;
        mEditor->SetBlockColour(wxColour(red, green, blue, 190)) ;

        red = ini.GetLongValue("base", "commentred", 255) ;
        green = ini.GetLongValue("base", "commentgreen", 178) ;
        blue = ini.GetLongValue("base", "commentblue", 102) ;
        mEditor->SetCommentColour(wxColour(red, green, blue, 190)) ;

        red = ini.GetLongValue("base", "unknownred", 194) ;
        green = ini.GetLongValue("base", "unknowngreen", 70) ;
        blue = ini.GetLongValue("base", "unknownblue", 65) ;
        mEditor->SetUnknownColour(wxColour(red, green, blue, 190)) ;

//        red = ini.GetLongValue("base", "specialred", ) ;
//        green = ini.GetLongValue("base", "specialgreen", ) ;
//        blue = ini.GetLongValue("base", "specialblue", ) ;
//        mEditor->SetSpecialColour(0, wxColour(red, green, blue, 190)) ;

        mEditor->mShortName = ini.GetValue("base", "shortname", "none") ;
        mEditor->mLongName = ini.GetValue("base", "longname", "none") ;

        long width, height ;
        width = ini.GetLongValue("base", "windowwidth", 800) ;
        height = ini.GetLongValue("base", "windowheight", 600) ;

        SetSize(width, height) ;

//        mEditor->mAutoSaveTime = ini.GetLongValue("base", "autosave", 300) ;

//        mEditor->mWrapStyle = ini.GetLongValue("base", "wrapstyle", 1) ;

        mEditor->mHelpDisplay = static_cast<eHelpDisplay>(ini.GetLongValue("base", "showhelp", HELP_MAIN)) ;

//        mEditor->mCheckForUpdates = static_cast<char>(ini.GetLongValue("base", "checkupdate", 0)) ;

//        mEditor->mMetric = ini.GetBoolValue("base", "metric", true) ;
        mEditor->SetMeasurement(ini.GetValue("base", "measurement", "0cm")) ;
        
        mEditor->mDispRuler = ini.GetBoolValue("base", "dispruler", true) ;
        mEditor->mDispScrollBar = ini.GetBoolValue("base", "dispscroll", true) ; ;
        mEditor->mDispStatusBar = ini.GetBoolValue("base", "dispstatusbottom", true) ;
        mEditor->mDispStyleBar = ini.GetBoolValue("base", "dispstyle", true) ;
        mEditor->mAlwaysDot = ini.GetBoolValue("base", "dispdotalways", true) ;
        if(mEditor->GetShowControls() == SHOW_DOT && mEditor->mAlwaysDot == false)
        {
            mEditor->SetShowDot(false) ;
        }
        mEditor->mAlwaysFlag = ini.GetBoolValue("base", "dispflagalways", true) ;
        

    }
    else
    {
        mEditor->SetShowControls(SHOW_ALL) ;
        mEditor->SetBackgroundColour(wxColour(245, 245, 245)) ;
        mEditor->SetBGroundColour(wxColour(245, 245, 245)) ;
        mEditor->SetTextColour(wxColour(0, 0, 0)) ;
        mEditor->SetHighlightColour(wxColour(0, 150, 200, 190)) ;
        mEditor->SetDotColour(wxColour(100, 200, 200, 190)) ;
        mEditor->SetBlockColour(wxColour(50, 100, 200, 190)) ;
        mEditor->SetCommentColour(wxColour(255, 178, 102, 190)) ;
        mEditor->SetUnknownColour(wxColour(194, 70, 65, 190)) ;
//        mEditor->SetSpecialColour(0, wxColour()) ;
        
//        mEditor->mMetric = true ;
        mEditor->SetMeasurement("0cm") ;

        mEditor->mDispRuler = true ;
        mEditor->mDispScrollBar = true ;
        mEditor->mDispStatusBar = true ;
        mEditor->mDispStyleBar = true ;
        mEditor->mAlwaysDot = true ;
        mEditor->mAlwaysFlag = true ;
        
//        mEditor->mAutoSaveTime = 300 ;
//        mEditor->mWrapStyle = 1 ;
    }

    if(mEditor->mDispScrollBar)
    {
        mEditor->ShowScrollbars(wxSHOW_SB_NEVER, wxSHOW_SB_ALWAYS) ;
    }
    else
    {
        mEditor->ShowScrollbars(wxSHOW_SB_NEVER, wxSHOW_SB_NEVER) ;
    }
    
    if(mEditor->mDispStyleBar)
    {
        mStatusTop->Show() ;
    }
    else
    {
        mStatusTop->Hide() ;
    }

    if(mEditor->mDispRuler)
    {
        mRuler->Show() ;
    }
    else
    {
        mRuler->Hide() ;
    }

    if(mEditor->mDispStatusBar)
    {
        mStatusBottom->Show() ;
    }
    else
    {
        mStatusBottom->Hide() ;
    }

    mEditorSizer->Layout() ;

}


void WordTsar::WriteConfig(void)
{
    CSimpleIniA ini ;

    wxString dir = wxGetHomeDir() ;
    dir += "/.WordTsar" ;

//    if(ini.LoadFile(dir.char_str()) >= 0)
    ini.LoadFile(dir.char_str()) ;

    {
        if(mEditor->GetShowControls() == SHOW_ALL)
        {
            ini.SetBoolValue("base", "showcontrols", true) ;
        }
        else
        {
            ini.SetBoolValue("base", "showcontrols", false) ;
        }
//        ini.SetLongValue("showhelp", mEditor->mHelpDisplay) ;

        short red, green, blue ;
        wxColour color = mEditor->GetBGroundColour() ;
        red = ini.SetLongValue("base", "backgroundred", color.Red()) ;
        green = ini.SetLongValue("base", "backgroundgreen", color.Green()) ;
        blue = ini.SetLongValue("base", "backgroundblue", color.Blue()) ;

        color = mEditor->GetTextColour() ;
        red = ini.SetLongValue("base", "textred", color.Red()) ;
        green = ini.SetLongValue("base", "textgreen", color.Green()) ;
        blue = ini.SetLongValue("base", "textblue", color.Blue()) ;

        color = mEditor->GetHighlightColour() ;
        red = ini.SetLongValue("base", "highlightred", color.Red()) ;
        green = ini.SetLongValue("base", "highlightgreen", color.Green()) ;
        blue = ini.SetLongValue("base", "highlightblue", color.Blue()) ;

        color = mEditor->GetDotColour() ;
        red = ini.SetLongValue("base", "dotred", color.Red()) ;
        green = ini.SetLongValue("base", "dotgreen", color.Green()) ;
        blue = ini.SetLongValue("base", "dotblue", color.Blue()) ;

        color = mEditor->GetBlockColour() ;
        red = ini.SetLongValue("base", "blockred", color.Red()) ;
        green = ini.SetLongValue("base", "blockgreen", color.Green()) ;
        blue = ini.SetLongValue("base", "blockblue", color.Blue()) ;

        color = mEditor->GetCommentColour() ;
        red = ini.SetLongValue("base", "commentred", color.Red()) ;
        green = ini.SetLongValue("base", "commentgreen", color.Green()) ;
        blue = ini.SetLongValue("base", "commentblue", color.Blue()) ;

        color = mEditor->GetUnknownColour() ;
        red = ini.SetLongValue("base", "unknownred", color.Red()) ;
        green = ini.SetLongValue("base", "unknowngreen", color.Green()) ;
        blue = ini.SetLongValue("base", "unknownblue", color.Blue()) ;

        ini.SetValue("base", "shortname", mEditor->mShortName.char_str()) ;
        ini.SetValue("base", "longname", mEditor->mLongName.char_str()) ;

        wxRect size = GetScreenRect() ;
        red = ini.SetLongValue("base", "windowwidth", size.width) ;
        blue = ini.SetLongValue("base", "windowheight", size.height) ;

//        ini.SetLongValue("base", "autosave", mEditor->mAutoSaveTime) ;

//        ini.SetLongValue("base", "wrapstyle", mEditor->mWrapStyle) ;

        ini.SetLongValue("base", "showhelp", mEditor->mHelpDisplay) ;

//        ini.SetLongValue("base", "checkupdate", mEditor->mCheckForUpdates) ;

//        ini.SetBoolValue("base", "metric", mEditor->mMetric) ;
        ini.SetValue("base", "measurement", mEditor->GetMeasurement().char_str()) ;

        ini.SetBoolValue("base", "dispruler", mEditor->mDispRuler) ;
        ini.SetBoolValue("base", "dispscroll", mEditor->mDispScrollBar) ;
        ini.SetBoolValue("base", "dispstatusbottom", mEditor->mDispStatusBar) ;
        ini.SetBoolValue("base", "dispstyle", mEditor->mDispStyleBar) ;
        ini.SetBoolValue("base", "dispdotalways", mEditor->mAlwaysDot) ;
        ini.SetBoolValue("base", "dispflagalways", mEditor->mAlwaysFlag) ;

        ini.SaveFile(dir.char_str()) ;
        // get rid of warnings until implemented
        red = green = blue ;
        blue = red ;
    }
}




/*
 * wxEVT_CLOSE_WINDOW event handler for ID_WORDTSAR
 */

void WordTsar::OnCloseWindow( wxCloseEvent& event )
{
////@begin wxEVT_CLOSE_WINDOW event handler for ID_WORDTSAR in WordTsar.
    // Before editing this code, remove the block markers.
    event.Skip();
////@end wxEVT_CLOSE_WINDOW event handler for ID_WORDTSAR in WordTsar. 
}

