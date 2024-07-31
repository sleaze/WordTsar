#ifndef CRTFWRITER_H
#define CRTFWRITER_H

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


#include <QString>
#include <QFile>


#include "src/core/document/document.h"
#include "src/gui/layout/layout.h"
//#include "src/files/rtf/read/rtfparser.h"

#include "src/files/rtf/structs.h"

struct sRTFHeaderFooter
{
    QString text ;
    sRTFParaFormat paraformat ;
} ;


class cRTFWriter
{
public:
    cRTFWriter(cEditorCtrl *editor);
    ~cRTFWriter();
    
    bool Start(std::string &filename) ;

private :
    void CreateRTFHeader(void) ;
    void CreateRTFClose(void) ;
    void CreateFontTable(void) ;
    void CreateColorTable(void) ;
    void CreateStyleSheet(void) ;
    void CreateGenerator(void) ;
    void CreateRTF(void) ;
    void CreateText(QString &text) ;
    void CreateDot(QString &dot) ;

    void CreateModifiers(QChar ch) ;
    void CreateFont1(void) ;                // name changed to CreateFont1 from CreateFont, because VS2019 screws up and spews tons of errors about CreatFontW
    void CreateColor(void) ;
    void CreateHeadersFooters(PAGE_T page) ;
    void CreateHeadersFooters(int index, enum eHeaderFooter which, QString &rest) ;
    
    void ControlWord(const QString control) ;
    void ControlWord(const QString control, const int parameter) ;
    void ControlWord(const QString control, const QString text) ;
    void ControlText(const QString) ;
    void ControlSpace(void) ;
    void StartGroup(void) ;
    void EndGroup(void) ;
    void NewLine(void) ;
    
    void ParagraphFormat(void) ;
    void IndexText(QString text) ;
    
private :
    QFile mFile ;
    cDocument *mDocument ;
    cEditorCtrl *mEditor ;
    
//    bool mBoldOn, mItalicsOn, mUnderlineOn, mSubscriptOn, mSuperscriptOn, mStrikeOn ;
    unsigned long mCommentCount ;
    unsigned long mGroupCount ;
    bool mNewLine ;
    
    POSITION_T mCurrentPosition ;

    size_t mCurrentParagraphMargin ;
    size_t mCurrentLeftMargin ;
    size_t mCurrentRightMargin ;
    size_t mCurrentTopMargin ;
    size_t mCurrentBottomMargin ;

    std::map<std::string, std::string> mFinalfont ;     // we use a map because it autmatically gets rid of duplicates for us

    QString mFootnote1, mFootnote2, mFootnote3, mFootnote4, mFootnote5 ;

    sRTFParaFormat paragraph ;
    
    std::array<sRTFHeaderFooter, MAX_HEADER_FOOTER> mHeaders ;
    std::array<sRTFHeaderFooter, MAX_HEADER_FOOTER> mFooters ;
    std::array<sRTFHeaderFooter, MAX_HEADER_FOOTER> mHeadersEven ;
    std::array<sRTFHeaderFooter, MAX_HEADER_FOOTER> mFootersEven ;
    std::array<sRTFHeaderFooter, MAX_HEADER_FOOTER> mHeadersOdd ;
    std::array<sRTFHeaderFooter, MAX_HEADER_FOOTER> mFootersOdd ;

    bool mDoIndex ;             // create index at end of document?
    bool mDonefacingp ;         // have we done a facingp?
};


#endif // CRTFWRITER_H
