/////////////////////////////////////////////////////////////////////////////
// Name:        centerword.cpp
// Purpose:
// Author:      Gerald Brandt
// Modified by:
// Created:     Fri 12 Oct 2012 12:00:00 CDT
// RCS-ID:
// Copyright:   (c) Majentis Technologies
// Licence:
/////////////////////////////////////////////////////////////////////////////

/*
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes
#include <wx/accel.h>


#include "centerword.h"

////@begin XPM images
////@end XPM images




IMPLEMENT_DYNAMIC_CLASS( cEnterWord, wxDialog )




BEGIN_EVENT_TABLE( cEnterWord, wxDialog )

////@begin cEnterWord event table entries
////@end cEnterWord event table entries

END_EVENT_TABLE()




cEnterWord::cEnterWord()
{
    Init();
}

cEnterWord::cEnterWord( wxWindow* parent, wxWindowID id, const QString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    Init();
    Create(parent, id, caption, pos, size, style);
}




bool cEnterWord::Create( wxWindow* parent, wxWindowID id, const QString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin cEnterWord creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end cEnterWord creation
    return true;
}




cEnterWord::~cEnterWord()
{
////@begin cEnterWord destruction
////@end cEnterWord destruction
}




void cEnterWord::Init()
{
////@begin cEnterWord member initialisation
    mWord = NULL;
////@end cEnterWord member initialisation

    wxAcceleratorEntry entries[1];
    entries[0].Set(wxACCEL_NORMAL, WXK_RETURN, wxID_OK);
    wxAcceleratorTable accel(1, entries);
    SetAcceleratorTable(accel);

}




void cEnterWord::CreateControls()
{
////@begin cEnterWord content construction
    cEnterWord* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxGROW|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _("Enter Word: "), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    mWord = new wxTextCtrl( itemDialog1, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0 );
    itemBoxSizer3->Add(mWord, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer3->Add(itemBoxSizer6, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton7 = new wxButton( itemDialog1, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton7, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxButton* itemButton8 = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer6->Add(itemButton8, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end cEnterWord content construction

    mWord->SetFocus() ;
}



bool cEnterWord::ShowToolTips()
{
    return true;
}



wxBitmap cEnterWord::GetBitmapResource( const QString& name )
{
    // Bitmap retrieval
////@begin cEnterWord bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end cEnterWord bitmap retrieval
}



wxIcon cEnterWord::GetIconResource( const QString& name )
{
    // Icon retrieval
////@begin cEnterWord icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end cEnterWord icon retrieval
}

*/
