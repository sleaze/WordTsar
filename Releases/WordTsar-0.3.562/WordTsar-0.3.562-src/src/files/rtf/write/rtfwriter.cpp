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

#include <QMessageBox>

#include "rtfwriter.h"
#include "src/core/include/version.h"
#include "src/core/include/utils.h"

#include "src/gui/layout/layout.h"
#include "src/gui/editor/editorctrl.h"
#include "src/core/codepage/cp437.h"

using namespace std ;


//extern sExtendedChars gCodePage437[] ;
//extern int gExtendedSize ;
extern sSeqNewColor gBaseWSColors[] ;


const int ON = 1 ;
const int OFF = 0 ;


// wxWidgets compatibility functions

/////////////////////////////////////////////////////////////////////////////
///
/// @param  str - [in] the string to search
/// @param  ch  - [in] the char to look for
///
/// @return QString - the string after the last ch or the whole string if ch is not found
///
/// @brief
/// Gets all the charcaters after the last occurance of ch
/////////////////////////////////////////////////////////////////////////////
QString AfterLast(QString &str, QChar ch)
{
    QString ret = str ;

    int index = str.lastIndexOf(ch) ;
    if(index != -1)
    {
        int length = str.length() ;

        ret = str.right(length - index + 1) ;
    }

    return ret ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @param  str  - [in] the string to search
/// @param  ch   - [in] the character to look for
///
/// @return QString - the string after the first ch or empty string if ch is not found
///
/// @brief
/// Gets all the characters after the first occurrence of ch.
/////////////////////////////////////////////////////////////////////////////
QString AfterFirst(QString &str, QChar ch)
{
    QString ret("") ;

    int index = str.indexOf(ch) ;
    if(index != -1)
    {
        int length = str.length() ;

        ret = str.right(length - index - 1) ;
    }

    return ret ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @param  str     - [in] the string to search
/// @param  ch      - [in] the charcater to look for
/// @param  rest    - [out] -Filled with the part of the string following the first occurrence of ch or cleared if it was not found.
///
/// @return QString - part of the string before the first occurrence of ch or whole string if ch not found
///
/// @brief
/// Gets all characters before the first occurrence of ch.
/////////////////////////////////////////////////////////////////////////////
QString BeforeFirst(QString &str, QChar ch, QString &rest)
{
    QString ret = str ;

    int index = str.indexOf(ch) ;
    if(index != -1)
    {
        int length = str.length() ;

        ret = str.left(index) ;
        rest = str.right(length - index + 1) ;
    }
    else
    {
        rest.clear() ;
    }

    return ret ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @param  text    - [in] the string to search
/// @param  incomp  - [in] the string to look for
/// @param  rest    - [out] Filled with the part of the string following the first occurrence of incomp or unmodified rest
///
/// @return bool - true if found, else false
///
/// @brief
/// This function can be used to test if the string starts with the specified prefix.
///
/// If it does, the function will return true and put the rest of the string (i.e. after the prefix) into rest string if it is not NULL. Otherwise, the function returns false and doesn't modify the rest.
/////////////////////////////////////////////////////////////////////////////
bool StartsWith(QString &text, string incomp, QString *rest = nullptr)
{
    QString comp = QString::fromStdString(incomp) ;
    if(text.startsWith(comp))
    {
        if(rest != nullptr)
        {
            *rest = text.right(text.length() - comp.length()) ;
        }
        return true ;
    }
    else
    {
//        if(rest != nullptr)
//        {
//            rest->clear() ;
//        }
        return false ;
    }
}


cRTFWriter::cRTFWriter(cEditorCtrl *editor)
{
    mEditor = editor ;
    mDocument = &mEditor->mDocument ;
    
    mCommentCount = 1 ;
    mCurrentLeftMargin = 0 ;
    mCurrentParagraphMargin = 0 ;

    mGroupCount = 0 ;
    mNewLine = false ;
    
    mDoIndex = false ;
    mDonefacingp = false ;
}

cRTFWriter::~cRTFWriter()
{
    if(mGroupCount != 0)
    {
        QMessageBox msgBox(QMessageBox::Critical, "Error", "Unmatched braces in RTF file", QMessageBox::Ok, mEditor) ;
        msgBox.exec() ;
    }
}


bool cRTFWriter::Start(string &infilename)
{
    bool retval = false ;
    
    QString filename = QString::fromStdString(infilename) ;
    paragraph.character.bold = false ;
    paragraph.character.italics = false ;
    paragraph.character.underline = false ;
    paragraph.character.subscript = false ;
    paragraph.character.superscript = false ;
    paragraph.character.strikethrough = false ;
    paragraph.character.smallcaps = false ;

    mFile.setFileName(filename) ;
    if(mFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        CreateRTFHeader() ;
        CreateFontTable() ;
        CreateColorTable() ;
        CreateStyleSheet() ;
        
        CreateGenerator() ;

        CreateRTF() ;


        CreateRTFClose() ;

        mFile.close() ;

        retval = true ;
    }

    return retval ;
}


void cRTFWriter::CreateRTFHeader(void)
{
    StartGroup() ;
    ControlWord("rtf", 1) ;                                 // RTF version number (always 1)
    ControlWord("pc") ;                                     // character set (pc == codepage 437)
    ControlWord("deff", 0) ;                                // default font number
    NewLine() ;
}



void cRTFWriter::CreateRTFClose(void)
{
    EndGroup() ;
}



void cRTFWriter::CreateFontTable(void)
{
    vector<sInternalFonts> fontlist ;
    mDocument->GetFontList(fontlist) ;

    mFinalfont.clear() ;
    
    // build a map of fonts used in document
    for(auto &fontiter : fontlist)
    {
        string fonttype, fontname ;
        QFont nfont(QString::fromStdString(fontiter.name), static_cast<int>(fontiter.size)) ;
        QFont::StyleHint hint = nfont.styleHint() ;
        
        switch(hint)
        {
            case QFont::Times :
                fonttype = "froman " ;
                break ;
                
             case QFont::Decorative :
                fonttype = "fdecor " ;
                break ;
                
            case QFont::Cursive :
                fonttype = "fscript " ;
                break ;
                
            case QFont::SansSerif :
                fonttype = "fswiss " ;
                break ;
                
            case QFont::Monospace :
                fonttype = "fmodern " ;
                break ;
                
            default :
                fonttype = "ftech " ;
                break ;
        } ;
        
        fontname = nfont.family().toUtf8().constData() ;
        
        mFinalfont[fontname] = fonttype ;
    }

    StartGroup() ;
    ControlWord("fonttbl") ;                    // start the font table
    NewLine() ;

    // add default font
    StartGroup() ;
    ControlWord("f", 0) ;               // add a font
    ControlWord("froman ") ;
    ControlText("Times New Roman") ;
    ControlText(";") ;
    EndGroup() ;

    int count = 1 ;
    for(auto &fiter : mFinalfont)
    {
        StartGroup() ;
        ControlWord("f", count) ;               // add a font
        ControlWord(QString::fromStdString(fiter.second)) ;
        ControlText(QString::fromStdString(fiter.first)) ;
        ControlText(";") ;
        EndGroup() ;

        count++ ;
    }

    EndGroup() ;
    
    ControlWord("f", 0) ;                       // specify the default font
    ControlWord("fs", 24) ;                     // specifiy default font size
    paragraph.fontindex = 0 ;
    paragraph.fontsize = 24 ;
    NewLine() ;
}


// dumps the wordstar color table out
void cRTFWriter::CreateColorTable(void)
{
    StartGroup() ;
    ControlWord("colortbl") ;
    ControlText(";") ;

    for(int i = 0; i < 16; i++)
    {
        NewLine() ;
        ControlWord("red", gBaseWSColors[i].red) ;
        ControlWord("green", gBaseWSColors[i].green) ;
        ControlWord("blue", gBaseWSColors[i].blue) ;
        ControlText(";") ;
    }
    EndGroup() ;
}


void cRTFWriter::CreateStyleSheet(void)
{
    StartGroup() ;
    ControlWord("stylesheet") ;
    NewLine() ;
    
    // WordTsar doesn't do styles yet
    
    EndGroup() ;
}



void cRTFWriter::CreateGenerator(void)
{    
    StartGroup() ;
    ControlWord("*") ;
    ControlWord("generator WordTsar/") ;
    ControlText(FULLVERSION_STRING) ;
    ControlText("/") ;

    QSysInfo info ;
    ControlText(info.productType()) ;
    EndGroup() ;
}



void cRTFWriter::CreateRTF(void)
{
    ControlWord("paperw", static_cast<int>(mEditor->mLayout->mPageInfo[0].paperwidth)) ;
    ControlWord("paperh", static_cast<int>(mEditor->mLayout->mPageInfo[0].paperheight)) ;
    
    
    mCurrentLeftMargin = static_cast<size_t>(mEditor->mLayout->mPageInfo[0].leftmargin) ;
    mCurrentRightMargin = static_cast<size_t>(mEditor->mLayout->mPageInfo[0].rightmargin) ;
    mCurrentTopMargin = static_cast<size_t>(mEditor->mLayout->mPageInfo[0].topmargin) ;
    mCurrentBottomMargin = static_cast<size_t>(mEditor->mLayout->mPageInfo[0].bottommargin) ;

    // set initial margins to 1"
    ControlWord("margl", static_cast<int>(mCurrentLeftMargin)) ;
    ControlWord("margr", static_cast<int>(mCurrentRightMargin)) ;
    ControlWord("margt", static_cast<int>(mCurrentTopMargin)) ;
    ControlWord("margb", static_cast<int>(mCurrentBottomMargin)) ;
    NewLine() ;

//    PAGE_T oldpage = 0 ;
    
    // go through the text a paragraph at a time
    size_t paras = mDocument->GetNumberofParagraphs() ;
    for(size_t loop = 0; loop < paras; loop++)
    {        
        POSITION_T start, end ;
        QString text = QString::fromStdString(mDocument->GetParagraphText(loop)) ;
        mDocument->GetParagraphStartandEnd(loop, start, end) ;
        mCurrentPosition = start ;
        
        // make sure we are not a comment paragraph
        if(text[0] != '.')
        {
            CreateText(text) ;
        }
        else
        {
            CreateDot(text) ;
        }
    }
}



void cRTFWriter::CreateText(QString &text)
{
     // go through a character at a time
    for(size_t pos = 0; pos < text.length(); pos++, mCurrentPosition++)
    {
//        if(text.at(pos) < STYLE_END_OF_STYLES)
        if(text.at(pos) == static_cast<QChar>(MARKER_CHAR))
        {
            QChar ch = static_cast<QChar>(mDocument->GetControlChar(mCurrentPosition)) ;
            CreateModifiers(ch) ;
//            CreateModifiers(text.at(pos)) ;
            ControlSpace() ;
        }
        else if(text.at(pos) > static_cast<QChar>(255)) ///< @todo This is wrong (RTF 1.9.1 spec page 14)
        {
            ControlWord("u", text.at(pos)) ;
            ControlText("?") ;
            ControlSpace() ;
        }
        else
        {
            if(pos == 0)
            {
                ControlSpace() ;
            }
            bool d = false ;
            
            // check if this is a extended char.
            if(text.at(pos) < static_cast<QChar>(32) || text.at(pos) > static_cast<QChar>(126))
            {
                cCodePage437 cp ;
                unsigned char chws = cp.toChar(text.at(pos).toLatin1()) ;
                if(chws != 0)
                {
                    ControlWord("u", (int)text.at(pos).unicode()) ;
                    ControlText("?") ;
                    d = true ;
                    break ;
                }
/*
                for(int loop = 0; loop < gExtendedSize; loop++)
                {
                    QString temp ;
                    if(text.at(pos) == gCodePage437[loop].utf8char)
                    {
                        ControlWord("u", (int)text.at(pos).unicode()) ;
                        ControlText("?") ;
                        d = true ;
                        break ;
                    }
                }
*/
            }
            if(d == false)
            {
                char w = text.at(pos).unicode() ;
                mFile.write(&w, 1) ;
            }
        }
    }
    NewLine() ;
    ControlWord("par") ;   
}


void cRTFWriter::CreateModifiers(QChar ch)
{
    switch(ch.unicode())
    {
        case STYLE_BOLD :
            if(paragraph.character.bold == false)
            {
                ControlWord("b", ON) ;
//                ControlSpace() ;
                paragraph.character.bold = true ;
            }
            else
            {
                ControlWord("b", OFF) ;
//                ControlSpace() ;
                paragraph.character.bold = false ;
            }
            break ;

        case STYLE_ITALICS :
            if(paragraph.character.italics == false)
            {
                ControlWord("i", ON) ;
//                ControlSpace() ;
                paragraph.character.italics = true ;
            }
            else
            {
                ControlWord("i", OFF) ;
//                ControlSpace() ;
                paragraph.character.italics = false ;
            }
            break ;

        case STYLE_UNDERLINE :
            if(paragraph.character.underline == false)
            {
                ControlWord("ul", ON) ;
//                ControlSpace() ;
                paragraph.character.underline = true ;
            }
            else
            {
                ControlWord("ul0", OFF) ;
//                ControlSpace() ;
                paragraph.character.underline = false ;
            }
            break ;

        case STYLE_SUBSCRIPT :
            if(paragraph.character.subscript == false)
            {
                ControlWord("sub") ;
//                ControlSpace() ;
                paragraph.character.subscript = true ;
            }
            else
            {
                ControlWord("nosupersub") ;
//                ControlSpace() ;
                paragraph.character.subscript = false ;
            }
            break ;

        case STYLE_SUPERSCRIPT :
            if(paragraph.character.superscript == false)
            {
                ControlWord("super") ;
//                ControlSpace() ;
                paragraph.character.superscript = true ;
            }
            else
            {
                ControlWord("nosupersub") ;
//                ControlSpace() ;
                paragraph.character.superscript = false ;
            }
            break ;

        case STYLE_STRIKETHROUGH :
            if(paragraph.character.strikethrough == false)
            {
                ControlWord("strike", ON) ;
//                ControlSpace() ;
                paragraph.character.strikethrough = true ;
            }
            else
            {
                ControlWord("strike", OFF) ;
//                ControlSpace() ;
                paragraph.character.strikethrough = false ;
            }
            break ;

        case STYLE_TAB :
            ControlWord("tab") ;             /// @todo - style of TAB!
            break ;

        case STYLE_FONT1 :
            CreateFont1() ;
            break ;
            
        case STYLE_INTERNAL_COLOR :
            CreateColor() ;
            break ;
    }
}



void cRTFWriter::CreateFont1(void)
{
    QString temp ;
    sInternalFonts internalfont ;
    if(mDocument->GetFont(mCurrentPosition, internalfont) == true)
    {
        size_t fontnum = 1 ;
        QFont nfont(QString::fromStdString(internalfont.name), static_cast<int>(internalfont.size)) ;
        for(auto &ffiter : mFinalfont)
        {
            if(nfont.family().toUtf8().constData() == ffiter.first)
            {
                ControlWord("f", fontnum) ;
                paragraph.fontindex = fontnum ;
                break ;
            }
            fontnum++ ;
        }
        
        ControlWord("fs", nfont.pointSize() * 2) ;
        paragraph.fontsize = nfont.pointSize() * 2 ;
    }
}



void cRTFWriter::CreateColor(void)
{
    sWSColor color ;
    
    mDocument->GetColor(mCurrentPosition, color) ;
    
    ControlWord("cf", color.colornumber + 1) ;
}


void cRTFWriter::CreateHeadersFooters(int index, enum eHeaderFooter which, QString &rest)
{
    sHeaderFooter current ;
    index -- ;

    rest.replace("#", "\\chpgn ") ;
    
    sRTFParaFormat para = paragraph ;
            
    switch(which)
    {
        case HEADER_BOTH :
            if(mHeaders[index].text != rest)
            {
                mHeaders[index].text = rest ;
                mHeaders[index].paraformat = paragraph ;
                
                StartGroup() ;
                ControlWord("header") ;

//                StartGroup() ;
                for(int loop = 0 ; loop < MAX_HEADER_FOOTER; loop++)
                {
                    if(mHeaders[loop].text.length() != 0)
                    {
                        paragraph = mHeaders[loop].paraformat ;
                        ParagraphFormat() ;
                        
                        CreateText(mHeaders[loop].text) ;
                    }
                }                
//                EndGroup() ;
                
                EndGroup() ;
            }
            break ;

        case HEADER_EVEN :
            if(mHeadersEven[index].text != rest)
            {
                mHeadersEven[index].text = rest ;
                mHeadersEven[index].paraformat = paragraph ;
                
                StartGroup() ;
                ControlWord("headerr") ;

//                StartGroup() ;
                for(int loop = 0 ; loop < MAX_HEADER_FOOTER; loop++)
                {
                    if(mHeadersEven[loop].text.length() != 0)
                    {
                        paragraph = mHeadersEven[loop].paraformat ;
                        ParagraphFormat() ;
                        
                        CreateText(mHeadersEven[loop].text) ;
                    }
                }                
//                EndGroup() ;
                
                EndGroup() ;
            }
            break ;
            
        case HEADER_ODD :
            if(mHeadersOdd[index].text != rest)
            {
                mHeadersOdd[index].text = rest ;
                mHeadersOdd[index].paraformat = paragraph ;
                
                StartGroup() ;
                ControlWord("headerl") ;

//                StartGroup() ;
                for(int loop = 0 ; loop < MAX_HEADER_FOOTER; loop++)
                {
                    if(mHeadersOdd[loop].text.length() != 0)
                    {
                        paragraph = mHeadersOdd[loop].paraformat ;
                        ParagraphFormat() ;
                        
                        CreateText(mHeadersOdd[loop].text) ;
                    }
                }                
//               EndGroup() ;
                
                EndGroup() ;
            }
            break ;
            
        case FOOTER_BOTH :
            if(mFooters[index].text != rest)
            {
                mFooters[index].text = rest ;
                mFooters[index].paraformat = paragraph ;
                
                StartGroup() ;
                ControlWord("footer") ;

//                StartGroup() ;
                for(int loop = 0 ; loop < MAX_HEADER_FOOTER; loop++)
                {
                    if(mFooters[loop].text.length() != 0)
                    {
                        paragraph = mFooters[loop].paraformat ;
                        ParagraphFormat() ;
                        
                        CreateText(mFooters[loop].text) ;
                    }
                }                
//                EndGroup() ;
                
                EndGroup() ;
            }
            break ;

        case FOOTER_EVEN :
            if(mFootersEven[index].text != rest)
            {
                mFootersEven[index].text = rest ;
                mFootersEven[index].paraformat = paragraph ;
                
                StartGroup() ;
                ControlWord("footerl") ;

//                StartGroup() ;
                for(int loop = 0 ; loop < MAX_HEADER_FOOTER; loop++)
                {
                    if(mFootersEven[loop].text.length() != 0)
                    {
                        paragraph = mFootersEven[loop].paraformat ;
                        ParagraphFormat() ;
                        
                        CreateText(mFootersEven[loop].text) ;
                    }
                }                
//                EndGroup() ;
                
                EndGroup() ;
            }
            break ;
            
        case FOOTER_ODD :
            if(mFootersOdd[index].text != rest)
            {
                mFootersOdd[index].text = rest ;
                mFootersOdd[index].paraformat = paragraph ;
                
                StartGroup() ;
                ControlWord("footerr") ;

//                StartGroup() ;
                for(int loop = 0 ; loop < MAX_HEADER_FOOTER; loop++)
                {
                    if(mFootersOdd[loop].text.length() != 0)
                    {
                        paragraph = mFootersOdd[loop].paraformat ;
                        ParagraphFormat() ;
                        
                        CreateText(mFootersOdd[loop].text) ;
                    }
                }                
//                EndGroup() ;
                
                EndGroup() ;
            }
            break ;
    }
    
    paragraph = para ;
}




void cRTFWriter::CreateDot(QString &text)
{
    QString rest ;
    QString lowtext = text.toLower() ;
    bool incdec ;
    
    QChar check = lowtext[1] ;
    QString start ;
    switch(check.unicode())
    {
        case 'c' :
            // columns
            if(StartsWith(lowtext, ".co", &rest)) //    lowtext.startsWith(".co"))
            {
                // we start column sections with a new section, so if columns start in middle of page, it works
                ControlWord("sect") ;
                ControlWord("sectd") ;
                ControlWord("sbknone") ;
                int value = static_cast<int>(mDocument->GetValue(text.mid(3).toStdString(), incdec));
                
                // lets see if theres a space between columns value
                text = text.mid(3) ;
                text = AfterLast(text, ' ') ; // text.right(text.)   text.AfterLast(' ') ;
                text = AfterLast(text, ',') ; // text.AfterLast(',') ;
                double w = mDocument->GetValue(text.toStdString(), incdec) ;
                char t = mDocument->GetType(text.toStdString()) ;
                if(value != 1)
                {
                    if(!qFuzzyIsNull(w))
//                    if(w != 0)
                    {
                        COORD_T twips = mDocument->ConvertToTwips(w, t) ;
                        ControlWord("cols", value) ;
                        ControlWord("colsx", static_cast<int>(twips)) ;
                        ControlWord("ri", 0) ;                      // put this after cols, so rtf loader can ignore it
                    }
                    else
                    {
                        ControlWord("cols", value) ;
                        ControlWord("ri", 0) ;                      // put this after cols, so rtf loader can ignore it
                    }
                }
                NewLine() ;
            }
            break ;
            
        case 'f' :
            // footer
printf("footer\n") ;
//            if((lowtext.startsWith(".f1 ", &rest)) || (lowtext.startsWith(".fo ", &rest)) ||
//               (lowtext.startsWith(".f2 ", &rest)) || (lowtext.startsWith(".f3 ", &rest)) ||
//               (lowtext.startsWith(".f4 ", &rest)) || (lowtext.startsWith(".f5 ", &rest)))
            if(StartsWith(lowtext, ".f1 ", &rest) || StartsWith(lowtext, ".fo ", &rest) ||
               StartsWith(lowtext, ".f2 ", &rest) || StartsWith(lowtext, ".f3 ", &rest) ||
               StartsWith(lowtext, ".f4 ", &rest) || StartsWith(lowtext, ".f5 ", &rest))
            {
                rest = AfterFirst(text, ' ') ; // text.After(' ') ;
                int index ;
                index = (char)lowtext.at(2).toLatin1() - 48 ;
                if(index >= MAX_HEADER_FOOTER)
                {
                    index = 0 ;
                }
                CreateHeadersFooters(index, FOOTER_BOTH, rest) ;
            }
//            else if((lowtext.startsWith(".f1e ", &rest)) || (lowtext.startsWith(".foe ", &rest)) ||
//               (lowtext.startsWith(".f2e ", &rest)) || (lowtext.startsWith(".f3e ", &rest)) ||
//               (lowtext.startsWith(".f4e ", &rest)) || (lowtext.startsWith(".f5e ", &rest)))
            else if(StartsWith(lowtext, ".f1e ", &rest) || StartsWith(lowtext, ".foe ", &rest) ||
               StartsWith(lowtext, ".f2e ", &rest) || StartsWith(lowtext, ".f3e ", &rest) ||
               StartsWith(lowtext, ".f4e ", &rest) || StartsWith(lowtext, ".f5e ", &rest))
            {
                rest = AfterFirst(text, ' ') ; //  text.After(' ') ;
                int index ;
                index = (char)lowtext.at(2).toLatin1() - 48 ;
                if(index >= MAX_HEADER_FOOTER)
                {
                    index = 0 ;
                }
                
                if(mDonefacingp == false)
                {
                    mDonefacingp = true ;
                    ControlWord("facingp") ;
                }
                CreateHeadersFooters(index, FOOTER_EVEN, rest) ;
            }
//            else if((lowtext.startsWith(".f1o ", &rest)) || (lowtext.startsWith(".foo ", &rest)) ||
//               (lowtext.startsWith(".f2o ", &rest)) || (lowtext.startsWith(".f3o ", &rest)) ||
//               (lowtext.startsWith(".f4o ", &rest)) || (lowtext.startsWith(".f5o ", &rest)))
            else if(StartsWith(lowtext, ".f1o ", &rest) || StartsWith(lowtext, ".foo ", &rest) ||
               StartsWith(lowtext, ".f2o ", &rest) || StartsWith(lowtext, ".f3o ", &rest) ||
               StartsWith(lowtext, ".f4o ", &rest) || StartsWith(lowtext, ".f5o ", &rest))
            {
                rest = AfterFirst(text, ' ') ;  //   text.After(' ') ;
                int index ;
                index = (char)lowtext.at(2).toLatin1() - 48 ;
                if(index >= MAX_HEADER_FOOTER)
                {
                    index = 0 ;
                }
                if(mDonefacingp == false)
                {
                    mDonefacingp = true ;
                    ControlWord("facingp") ;
                }
                CreateHeadersFooters(index, FOOTER_ODD, rest) ;
            }
            break ;
            
        case 'h' :
            // header
//            if((lowtext.startsWith(".h1e ", &rest)) || (lowtext.startsWith(".hoe ", &rest)) ||
//               (lowtext.startsWith(".h2e ", &rest)) || (lowtext.startsWith(".h3e ", &rest)) ||
//               (lowtext.startsWith(".h4e ", &rest)) || (lowtext.startsWith(".h5e ", &rest)))
            if(StartsWith(lowtext, ".h1e ", &rest) || StartsWith(lowtext, ".hoe ", &rest) ||
               StartsWith(lowtext, ".h2e ", &rest) || StartsWith(lowtext, ".h3e ", &rest) ||
               StartsWith(lowtext, ".h4e ", &rest) || StartsWith(lowtext, ".h5e ", &rest))
            {
//                rest = text.After(' ') ;
                rest = AfterFirst(text, ' ') ;
                int index ;
                index = (char)lowtext.at(2).unicode() - 48 ;
                if(index >= MAX_HEADER_FOOTER)
                {
                    index = 0 ;
                }
                if(mDonefacingp == false)
                {
                    mDonefacingp = true ;
                    ControlWord("facingp") ;
                }
                CreateHeadersFooters(index, HEADER_EVEN, rest) ;
            }
//            else if((lowtext.startsWith(".h1o ", &rest)) || (lowtext.startsWith(".hoo ", &rest)) ||
//               (lowtext.startsWith(".h2o ", &rest)) || (lowtext.startsWith(".h3o ", &rest)) ||
//               (lowtext.startsWith(".h4o ", &rest)) || (lowtext.startsWith(".h5o ", &rest)))
            else if(StartsWith(lowtext, ".h1o ", &rest) || StartsWith(lowtext, ".hoo ", &rest) ||
               StartsWith(lowtext, ".h2o ", &rest) || StartsWith(lowtext, ".h3o ", &rest) ||
               StartsWith(lowtext, ".h4o ", &rest) || StartsWith(lowtext, ".h5o ", &rest))
            {
//                rest = text.After(' ') ;
                rest = AfterFirst(text, ' ') ;
                int index ;
                index = (char)lowtext.at(2).unicode() - 48 ;
                if(index >= MAX_HEADER_FOOTER)
                {
                    index = 0 ;
                }
                if(mDonefacingp == false)
                {
                    mDonefacingp = true ;
                    ControlWord("facingp") ;
                }
                CreateHeadersFooters(index, HEADER_ODD, rest) ;
            }
//                        else if((lowtext.startsWith(".h1 ", &rest)) || (lowtext.startsWith(".ho ", &rest)) ||
//               (lowtext.startsWith(".h2 ", &rest)) || (lowtext.startsWith(".h3 ", &rest)) ||
//               (lowtext.startsWith(".h4 ", &rest)) || (lowtext.startsWith(".h5 ", &rest)))
            else if(StartsWith(lowtext, ".h1 ", &rest) || StartsWith(lowtext, ".ho ", &rest) ||
               StartsWith(lowtext, ".h2 ", &rest) || StartsWith(lowtext, ".h3 ", &rest) ||
               StartsWith(lowtext, ".h4 ", &rest) || StartsWith(lowtext, ".h5 ", &rest))
            {
//                rest = text.After(' ') ;
                rest = AfterFirst(text, ' ') ;
                int index ;
                index = (char)lowtext.at(2).unicode() - 48 ;
                if(index >= MAX_HEADER_FOOTER)
                {
                    index = 0 ;
                }
                CreateHeadersFooters(index, HEADER_BOTH, rest) ;
            }

            // hyphenate on/off
//            else if(lowtext.startsWith(".hy", &rest))
            else if(StartsWith(lowtext, ".hy", &rest))
            {
                if(rest.contains("on"))
                {
                    ControlWord("hyphauto", 1) ;
                }
                else
                {
                    ControlWord("hyphauto", 0) ;
                }
            }
            break ;
            
        case 'i' :
            // index
//            if(lowtext.startsWith(".ix", &rest))
            if(StartsWith(lowtext, ".ix", &rest))
            {
                StartGroup() ;
//                ControlWord("xe") ;
//                ControlWord("v") ;
                IndexText(text) ;
                EndGroup() ;
                mDoIndex = true ;
            }
            break ;
            
        case 'k' :
            // kerning
//            if(lowtext.startsWith(".kr", & rest))
            if(StartsWith(lowtext, ".kr", &rest))
            {
                if(rest.contains("off"))
                {
                    ControlWord("kerning", 0) ;
                }
                else
                {
                    ControlWord("kerning", 16) ;    // turn on kerning for font 8 points or large
                }
            }
            break ;
            
        case 'l' :
            // line numbering
//            if(lowtext.startsWith(".l#", &rest))
            if(StartsWith(lowtext, ".l#", &rest))
            {
                ControlWord("linemod", 1) ;
                ControlWord("linex", 0) ;
                
                lowtext = lowtext.right(lowtext.length() - 3) ;
                lowtext = lowtext.simplified() ;
                if(lowtext[0] == 'p')
                {
                    ControlWord("lineppage") ;
                }
//                ControlWord("linecont") ;
            }
            // line spacing
//            else if(lowtext.startsWith(".ls", &rest))
            else if(StartsWith(lowtext, ".ls", &rest))
            {
                double space ;
                space = rest.toDouble() ;
                long value = static_cast<long>(space * 240.0) ;

                ControlWord("sl", value) ;
                ControlWord("smulti") ;
                NewLine() ;
            }

            // left margin
//            else if(lowtext.startsWith(".lm", &rest))
            else if(StartsWith(lowtext, ".lm", &rest))
            {
                double value = mDocument->GetValue(text.mid(3).toStdString(), incdec);
                char type = mDocument->GetType(text.mid(3).toStdString());

                size_t newlm;
                newlm = mDocument->ConvertToTwips(value, type);

                ControlWord("li", newlm) ;
                mCurrentLeftMargin = newlm ;

                size_t newpm = mCurrentParagraphMargin - mCurrentLeftMargin ;
                ControlWord("fi", newpm) ;
            }
            break ; 
            
        case 'm' :
            // bottom margin
//            if(lowtext.startsWith(".mb", &rest))
            if(StartsWith(lowtext, ".mb", &rest))
            {
                double value = mDocument->GetValue(text.mid(3).toStdString(), incdec);
                char type = mDocument->GetType(text.mid(3).toStdString());

                mCurrentBottomMargin = mDocument->ConvertToTwips(value, type);

                ControlWord("margb", mCurrentBottomMargin) ;
            }

            // top margin
//            else if(lowtext.startsWith(".mt", &rest))
            else if(StartsWith(lowtext, ".mt", &rest))
            {
                double value = mDocument->GetValue(text.mid(3).toStdString(), incdec);
                char type = mDocument->GetType(text.mid(3).toStdString());

                mCurrentTopMargin = mDocument->ConvertToTwips(value, type);

                ControlWord("margt", mCurrentTopMargin) ;
            }
            break ;
            
        case 'o' :
            // justification
//            if(lowtext.startsWith(".oj", &rest))
            if(StartsWith(lowtext, ".oj", &rest))
            {
//                if(rest.startsWith("c"))
                if(StartsWith(rest, "c"))
                {
                    ControlWord("qc") ;
                    paragraph.align = ALIGNCENTER ;
                }
//                else if(rest.startsWith("r"))
                else if(StartsWith(rest, "r"))
                {
                    ControlWord("qr") ;
                    paragraph.align = ALIGNRIGHT ;
                }
//                else if(rest.startsWith("j"))
                else if(StartsWith(rest, "j"))
                {
                    ControlWord("qj") ;
                    paragraph.align = ALIGNJUSTIFY ;
                }
                else
                {
                    ControlWord("ql") ;
                    paragraph.align = ALIGNLEFT ;
                }
            }

            // center
//            else if(lowtext.startsWith(".oc", &rest))
            else if(StartsWith(lowtext, ".oc", &rest))
            {
                // get rid of extraneous from left and right
                rest = rest.simplified() ;
//                rest = rest.Trim(false) ;
//                if(rest.startsWith("off"))
                if(StartsWith(rest, "off"))
                {
                    ControlWord("ql") ;
                    paragraph.align = ALIGNLEFT ;
                }
                else
                {
                    ControlWord("qc") ;
                    paragraph.align = ALIGNCENTER ;
                }
            }
            break ;
            
        case 'p' :
            // page break
//            if(lowtext.startsWith(".pa"))
             if(StartsWith(lowtext, ".pa"))
            {
                ControlWord("page") ;
            }

            // paragraph margin
//            else if(lowtext.startsWith(".pm", &rest))
             else if(StartsWith(lowtext, ".pm", &rest))
            {
                double value = mDocument->GetValue(text.mid(3).toStdString(), incdec);
                char type = mDocument->GetType(text.mid(3).toStdString());

                size_t newpm ;
                newpm = mDocument->ConvertToTwips(value, type);
                mCurrentParagraphMargin = newpm ;
                ControlWord("fi", newpm) ;
            }
            
            // paragraph numbering
//            else if(lowtext.startsWith(".pn", &rest))
             else if(StartsWith(lowtext, ".pn", &rest))
            {
                long number ;
                rest = rest.simplified() ;
//                rest.Trim(false) ;
                if(rest.toLong())
                {
                    number = rest.toLong() ;

                    // we start page numbering with a new section
                    ControlWord("sect") ;
                    ControlWord("sectd") ;
                    ControlWord("sbknone") ;
                    ControlWord("pgnstarts", number) ;
                    ControlWord("pgnrestart") ;
                }
            }
            
            // page offset
//            else if(lowtext.startsWith(".po", &rest))
             else if(StartsWith(lowtext, ".po", &rest))
            {
                // RTF uses the same page offset (gutter) for odd and even pages
                // In order to look right, we only use the .poo and never look at .poe
                if((rest[0] == 'o') ) //  || (rest[0] == 'e') )              // .poo or .poe
                {
                    rest = rest.right(rest.length() -1) ;

                    double value = mDocument->GetValue(rest.toStdString(), incdec);
                    char type = mDocument->GetType(rest.toStdString());
                    size_t twips = mDocument->ConvertToTwips(value, type) ;
                    
                    // we start pageoffset sections with a new section, so if offsets change in middle of page, it works
                    ControlWord("sect") ;
                    ControlWord("sectd") ;
                    ControlWord("sbknone") ;
                    ControlWord("facingp") ;
                    ControlWord("gutter", twips) ;
                }
                else                            // .po
                {
                    double value = mDocument->GetValue(rest.toStdString(), incdec);
                    char type = mDocument->GetType(rest.toStdString());
                    size_t twips = mDocument->ConvertToTwips(value, type) ;
                    
                    // we start pageoffset sections with a new section, so if offsets change in middle of page, it works
                    ControlWord("sect") ;
                    ControlWord("sectd") ;
                    ControlWord("sbknone") ;
                    ControlWord("gutter", twips) ;
                }
            }
            break ;
            
        case 'r' :
            // right margin  (really right indent)
//            if(lowtext.startsWith(".rm", &rest))
            if(StartsWith(lowtext, ".rm", &rest))
            {
                double value = mDocument->GetValue(text.mid(3).toStdString(), incdec);
                char type = mDocument->GetType(text.mid(3).toStdString());

                size_t newrm;
                newrm = mDocument->ConvertToTwips(value, type);
                
                QString out ;
                if(newrm == 0)
                {
                    ControlWord("ri", newrm) ;
                }
                else
                {
                    // should be paperwidth - new right margin - leftmargin -> RTF right margin
                    LINE_T line = mEditor->mLayout->GetLineFromPosition(mCurrentPosition) ;
                    sPageInfo *pageinfo = mEditor->mLayout->GetPageInfoFromLine(line) ;
                    unsigned long twips = pageinfo->paperwidth - newrm - mCurrentLeftMargin - mCurrentRightMargin ;
                    
                    // we only remember the margin here, since we don't set it if the next dot command is a column one
//                    mNewRightMargin = twips ;
//                    mNewRightMarginSet = false ;
                    ControlWord("ri", twips) ;
                }
            }
            break ;
            
        case 't' :
            //tabs
//            if(lowtext.startsWith(".tb", &rest))
            if(StartsWith(lowtext, ".tb", &rest))
            {
                    ControlWord("tx", 0) ;
                    
                    QString nums = rest ;
                    
                    do
                    {
                        QString rest ;
//                        QString item = nums.BeforeFirst(',', &rest) ;
                        QString item = BeforeFirst(nums, ',', rest) ;
                        nums = rest ;
                        
                        double value = mDocument->GetValue(item.toStdString(), incdec);
                        char type = mDocument->GetType(item.toStdString());

                        double newtab;
                        newtab = mDocument->ConvertToTwips(value, type);
                        
                        ControlWord("tx", newtab) ;
                    } while(nums.length() != 0) ;

            }
            break  ;
            
    }

    // comment
//    if(lowtext.startsWith("..", &rest) || lowtext.startsWith(".ig", &rest))
    if(StartsWith(lowtext, "..", &rest) || StartsWith(lowtext, ".ig", &rest))
    {
        StartGroup() ;
        ControlWord("*") ;
        ControlWord("atrfstart", mCommentCount) ;
        EndGroup() ;
        
        mFile.write(".") ;              // we write out a single dot for the commented text. Reading RTF will parse this back out. (Word needs it)
        
        StartGroup() ;
        ControlWord("*") ;
        ControlWord("atrfend", mCommentCount) ;
        EndGroup() ;
        
        StartGroup() ;
        ControlWord("*") ;
        ControlWord("atnid", "wordstar") ;           // @TODO atnid
        EndGroup() ;

        StartGroup() ;
        ControlWord("*") ;
        ControlWord("atnauthor", "WordTsar") ;       // @TODO atnauthor
        EndGroup() ;

        NewLine() ;

        StartGroup() ;
        ControlWord("*") ;
        ControlWord("annotation") ;
        
        StartGroup() ;
        ControlWord("*") ;
        ControlWord("atnref", mCommentCount) ;
        EndGroup() ;

        ControlWord("pard") ;
        ControlWord("plain", text.right(rest.length())) ;
        EndGroup() ;

        mCommentCount++ ;
    }
}



void cRTFWriter::ControlWord(const QString control)
{
    QString out = strformat("\\%s", control.toUtf8().constData()).c_str() ;
    mFile.write(out.toUtf8().constData()) ;
    mNewLine = false ;
}


void cRTFWriter::ControlWord(const QString control, const int parameter)
{
    QString out = strformat("\\%s%d", control.toUtf8().constData(), parameter).c_str() ;
    mFile.write(out.toUtf8().constData()) ;
    mNewLine = false ;
}



void cRTFWriter::ControlWord(const QString control, const QString text)
{
    QString out = strformat("\\%s %s", control.toUtf8().constData(), text.toUtf8().constData()).c_str() ;
    mFile.write(out.toUtf8().constData()) ;
    mNewLine = false ;
}


void cRTFWriter::ControlText(const QString text)
{
    mFile.write(text.toUtf8().constData()) ;
    mNewLine = false ;
}


void cRTFWriter::ControlSpace(void)
{
    if(mNewLine == false)
    {
        mFile.write(" ") ;
    }
    mNewLine = false ;
}


void cRTFWriter::StartGroup(void)
{
    mFile.write("{") ;
    mGroupCount++ ;
    mNewLine = false ;
}


void cRTFWriter::EndGroup(void)
{
    mFile.write("}\n") ;
    mGroupCount-- ;
    mNewLine = true ;
}


void cRTFWriter::NewLine(void)
{
    mFile.write("\n") ;
    mNewLine = true ;
}


// returns a string with the paragraph formatting
void cRTFWriter::ParagraphFormat(void)
{
    if(paragraph.character.bold)
    {
        ControlWord("b", ON);
    }
    if(paragraph.character.italics)
    {
        ControlWord("i", ON);
    }
    if(paragraph.character.underline)
    {
        ControlWord("u", ON);
    }
    if(paragraph.character.subscript)
    {
        ControlWord("sub");
    }
    if(paragraph.character.superscript)
    {
        ControlWord("super");
    }
    if(paragraph.character.strikethrough)
    {
        ControlWord("strike");
    }

    ControlWord("f", paragraph.fontindex) ;
    ControlWord("fs", paragraph.fontsize) ;
    
    switch(paragraph.align)
    {
        case ALIGNCENTER :
            ControlWord("qc") ;
            break ;
            
        case ALIGNJUSTIFY :
            ControlWord("qj") ;
            break ;
            
        case ALIGNLEFT :
            ControlWord("ql") ;
            break ;
            
        case ALIGNRIGHT :
            ControlWord("qr") ;
            break ;
    } ;

    mNewLine = false ;
}


void cRTFWriter::IndexText(QString text)
{
    bool cross = false ;
    bool sub = false ;
    bool bold = false ;
    
    // strip off command
    text = text.right(text.length() - 4 ) ;
    text = text.simplified() ;

    // now see if this is a cross reference
    if(text[0] == '-')
    {
        sub = true ;
        text = text.right(text.length() - 1) ;
    }
    // or the page number must be in bold
    else if(text[0] == '+')
    {
        bold = true ;
        text = text.right(text.length() - 1) ;
    }
    
/* TODO QT
    // now check if this is a cross reference
    QString crosstext = text ;
    int found = 0 ;
    while(found != wxNOT_FOUND)
    {
        found = crosstext.Find(',') ;
        if(found != wxNOT_FOUND)
        {
            if(crosstext[found - 1] != '\\') 
                
            {
                break ;
            }
        }
    }

    
    QString left, right ;
    if(found != wxNOT_FOUND)
    {
        if(sub != true)
        {
            cross = true ;
        }
        
        left = text.Left(found) ;
        right = text.right(text.length() - found - 1) ;
    }
    
    left.Replace("\\", "") ;
    right.Replace("\\", "") ;
    text.Replace("\\", "") ;

    ControlWord("xe") ;
    ControlWord("v ") ;
    
    StartGroup() ;
    
    if(!sub && !cross)
    {
        ControlText(text) ;
    }
    
    if(cross)
    {
        QString ftext = left + "\\:" + right ;
        ControlText(ftext) ;
    }
    
    if(sub)
    {
        ControlText(left) ;
    }
    
    EndGroup() ;
    
    if(sub)
    {
        StartGroup() ;
        ControlWord("txe ") ;
        ControlText(right) ;
        EndGroup() ;
    }
    
    if(bold)
    {
        StartGroup() ;
        ControlWord("bxe") ;
        EndGroup() ;
    }
*/
}



