#ifndef _WORDTSAR_H_
#define _WORDTSAR_H_

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

/*!
 * Includes
 */

////@begin includes
#include "wx/frame.h"
#include "wx/statusbr.h"
////@end includes

#include <string>

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
class wxStatusBar;
class cEditorCtrl;
class cRulerCtrl;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_WORDTSAR 10000
#define ID_STATUSBAR 10002
#define ID_HELPCTRL 10003
#define ID_HELPPCTRL 10004
#define ID_HELPKCTRL 10005
#define ID_HELPQCTRL 10006
#define ID_HELPOCTRL 10007
#define ID_EDITORCTRL 10008
#define ID_BOTTOMSTATUSBAR 10001
#define SYMBOL_WORDTSAR_STYLE wxDEFAULT_FRAME_STYLE|wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_WORDTSAR_TITLE _("WordTsar")
#define SYMBOL_WORDTSAR_IDNAME ID_WORDTSAR
#define SYMBOL_WORDTSAR_SIZE wxSize(800, 800)
#define SYMBOL_WORDTSAR_POSITION wxDefaultPosition
////@end control identifiers



/*!
 * WordTsar class declaration
 */

class WordTsar: public wxFrame
{
    DECLARE_CLASS( WordTsar )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    WordTsar();
    WordTsar( wxWindow* parent, wxWindowID id = SYMBOL_WORDTSAR_IDNAME, const wxString& caption = SYMBOL_WORDTSAR_TITLE, const wxPoint& pos = SYMBOL_WORDTSAR_POSITION, const wxSize& size = SYMBOL_WORDTSAR_SIZE, long style = SYMBOL_WORDTSAR_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_WORDTSAR_IDNAME, const wxString& caption = SYMBOL_WORDTSAR_TITLE, const wxPoint& pos = SYMBOL_WORDTSAR_POSITION, const wxSize& size = SYMBOL_WORDTSAR_SIZE, long style = SYMBOL_WORDTSAR_STYLE );

    /// Destructor
    ~WordTsar();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();
    
    void SetFile(wxString filename) ;

////@begin WordTsar event handler declarations

    /// wxEVT_CLOSE_WINDOW event handler for ID_WORDTSAR
    void OnCloseWindow( wxCloseEvent& event );

////@end WordTsar event handler declarations

////@begin WordTsar member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end WordTsar member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    void UpdateStatus(cEditorCtrl *editor) ;
    void SetStatus(wxString text, bool progress = false, int percent = 0) ;
    
    
    void ReadConfig(void) ;
    void WriteConfig(void) ;

    wxPanel *mResizeIndicator;

////@begin WordTsar member variables
    wxBoxSizer* mEditorSizer;
    wxStatusBar* mStatusTop;
    cEditorCtrl* mHelpCtrl;
    cEditorCtrl* mHelpPCtrl;
    cEditorCtrl* mHelpKCtrl;
    cEditorCtrl* mHelpQCtrl;
    cEditorCtrl* mHelpOCtrl;
    cRulerCtrl* mRuler;
    cEditorCtrl* mEditor;
    wxStatusBar* mStatusBottom;
////@end WordTsar member variables

private:
//    wxStatusBar* mStatusBottom ;
    std::string mLoadFileName ;
    
    void StatusOn(int index, wxString text, wxClientDC &dc) ;
    void StatusOff(int index, wxString text, wxClientDC &dc) ;


};



#endif
    // _WORDTSAR_H_
