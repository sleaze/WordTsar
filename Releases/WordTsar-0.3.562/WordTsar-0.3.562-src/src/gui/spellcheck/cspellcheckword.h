/////////////////////////////////////////////////////////////////////////////
// Name:        cspellcheckword.h
// Purpose:     
// Author:      Gerald Brandt
// Modified by: 
// Created:     Fri 12 Oct 2012 12:01:08 CDT
// RCS-ID:      
// Copyright:   (c) Majentis Technologies
// Licence:     
/////////////////////////////////////////////////////////////////////////////

/*
#ifndef _CSPELLCHECKWORD_H_
#define _CSPELLCHECKWORD_H_




////@begin includes
////@end includes



////@begin forward declarations
////@end forward declarations



////@begin control identifiers
#define ID_CSPELLCHECKWORD 10008
#define ID_TEXTCTRL1 10006
#define SYMBOL_CSPELLCHECKWORD_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CSPELLCHECKWORD_TITLE _("Spell Check Word")
#define SYMBOL_CSPELLCHECKWORD_IDNAME ID_CSPELLCHECKWORD
#define SYMBOL_CSPELLCHECKWORD_SIZE wxSize(400, 300)
#define SYMBOL_CSPELLCHECKWORD_POSITION wxDefaultPosition
////@end control identifiers




class cSpellCheckWord: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( cSpellCheckWord )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    cSpellCheckWord();
    cSpellCheckWord( wxWindow* parent, wxWindowID id = SYMBOL_CSPELLCHECKWORD_IDNAME, const QString& caption = SYMBOL_CSPELLCHECKWORD_TITLE, const wxPoint& pos = SYMBOL_CSPELLCHECKWORD_POSITION, const wxSize& size = SYMBOL_CSPELLCHECKWORD_SIZE, long style = SYMBOL_CSPELLCHECKWORD_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CSPELLCHECKWORD_IDNAME, const QString& caption = SYMBOL_CSPELLCHECKWORD_TITLE, const wxPoint& pos = SYMBOL_CSPELLCHECKWORD_POSITION, const wxSize& size = SYMBOL_CSPELLCHECKWORD_SIZE, long style = SYMBOL_CSPELLCHECKWORD_STYLE );

    /// Destructor
    ~cSpellCheckWord();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin cSpellCheckWord event handler declarations

////@end cSpellCheckWord event handler declarations

////@begin cSpellCheckWord member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const QString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const QString& name );
////@end cSpellCheckWord member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin cSpellCheckWord member variables
    wxTextCtrl* mWord;
////@end cSpellCheckWord member variables
};

#endif
    // _CSPELLCHECKWORD_H_

    */
