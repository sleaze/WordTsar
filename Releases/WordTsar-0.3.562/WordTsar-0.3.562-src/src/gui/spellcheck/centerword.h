/////////////////////////////////////////////////////////////////////////////
// Name:        centerword.h
// Purpose:     
// Author:      Gerald Brandt
// Modified by: 
// Created:     Fri 12 Oct 2012 12:00:00 CDT
// RCS-ID:      
// Copyright:   (c) Majentis Technologies
// Licence:     
/////////////////////////////////////////////////////////////////////////////

/*
 *
#ifndef _CENTERWORD_H_
#define _CENTERWORD_H_



////@begin includes
////@end includes


////@begin forward declarations
////@end forward declarations


////@begin control identifiers
#define ID_CENTERWORD 10007
#define ID_TEXTCTRL1 10006
#define SYMBOL_CENTERWORD_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxTAB_TRAVERSAL
#define SYMBOL_CENTERWORD_TITLE _("Enter Word")
#define SYMBOL_CENTERWORD_IDNAME ID_CENTERWORD
#define SYMBOL_CENTERWORD_SIZE wxSize(400, 300)
#define SYMBOL_CENTERWORD_POSITION wxDefaultPosition
////@end control identifiers



class cEnterWord: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( cEnterWord )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    cEnterWord();
    cEnterWord( wxWindow* parent, wxWindowID id = SYMBOL_CENTERWORD_IDNAME, const QString& caption = SYMBOL_CENTERWORD_TITLE, const wxPoint& pos = SYMBOL_CENTERWORD_POSITION, const wxSize& size = SYMBOL_CENTERWORD_SIZE, long style = SYMBOL_CENTERWORD_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_CENTERWORD_IDNAME, const QString& caption = SYMBOL_CENTERWORD_TITLE, const wxPoint& pos = SYMBOL_CENTERWORD_POSITION, const wxSize& size = SYMBOL_CENTERWORD_SIZE, long style = SYMBOL_CENTERWORD_STYLE );

    /// Destructor
    ~cEnterWord();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin cEnterWord event handler declarations

////@end cEnterWord event handler declarations

////@begin cEnterWord member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const QString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const QString& name );
////@end cEnterWord member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

////@begin cEnterWord member variables
    wxTextCtrl* mWord;
////@end cEnterWord member variables
};

#endif
    // _CENTERWORD_H_
*/
