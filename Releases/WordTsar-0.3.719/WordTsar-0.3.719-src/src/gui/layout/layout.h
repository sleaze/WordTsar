#ifndef LAYOUT_H
#define LAYOUT_H

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


#include <array>
#include <deque>
#include <string>
#include <vector>

#include <QFont>
#include <QColor>
#include <QPainter>
#include <QPageSize>

#include "src/core/include/config.h"
#include "src/core/document/document.h"


template< typename... Args >
std::string string_sprintf( const char* format, Args... args )
{
  int length = std::snprintf( nullptr, 0, format, args... );
  MY_ASSERT( length >= 0 )

  char* buf = new char[length + 1];
  std::snprintf( buf, length + 1, format, args... );

  std::string str( buf );
  delete[] buf;
  return str;
}


enum eHighlightType
{
    HILITE_SELECT
} ;


struct sHighlightList
{
    eHighlightType type ;
    POSITION_T start ;
    POSITION_T end ;
} ;


// a segment of a single line of text (segments containa specific font
struct sSegmentLayout
{
    PARAGRAPH_T paragraph ;         // paragraph number ( @TODO this probably shouldn't be here, put it in the paragraph structure instead)
    std::deque<std::string> glyph ;           // graphemes
    std::deque<COORD_T> position ;       // position of each glyph
    bool isBlock ;                  // is this segment in a marked block (used by display only)
    bool isSearch ;                 // is this glyph in a search block (used by display only)
    COORD_T segmentheight ;         // overall segment height
    QFont font ;                   // font for segment display
    QFont tfont ;                   // font used for font name and size display (normally same as font, except for comments and commands)
    QColor textcolor ;             // color for segment
    QColor backcolor ;             // background color
} ;

// a collection of segments that make up a single line
struct sLineLayout
{
    bool center, right, left, justify, none1, none2, none3, none4 ;     // none# is for padding
    PAGE_T pagenumber ;
    LINE_T linenumber ;             // line number (does not change for .commands)
    COORD_T cumalativeheight ;       // cumalative line height in twips (does not change for .commands)
    COORD_T x, y ;                  // x and y coordinate of start of line in twips
    COORD_T lineheight ;
    std::vector<sSegmentLayout> segments ;
} ;


// page specific info
struct sPageInfo
{
    QPageSize::PageSizeId papertype ;
    COORD_T paperwidth ;            // this can be looked up via wxPaperSize, but it's slow
    COORD_T paperheight ;           // this can be looked up via wxPaperSize, but it's slow
    bool set ;                      // the user can do multiple .pt commands per page, but only the first is actually used.

    COORD_T topmargin ;             // page's top margin
    COORD_T bottommargin ;          // page's bottom margin (real margin size, not height of print space i.e. 1440 for 1 inch margin)
    COORD_T leftmargin ;            // page's left margin
    COORD_T rightmargin ;           // pages's right margin (real margin size, not width of print space i.e. 1440 for 1 inch margin)
    COORD_T headermargin ;          // page's header margin
    COORD_T footermargin ;          // page's footer margin
} ;

// header and footer storage
struct sHeaderFooter
{
    PAGE_T page ;                   // the page number this header or footer starts on
    int headernumber ;              // header number 1 to MAX_HEADER_FOOTER
//    sParagraphLayout text ;         // The header line itself
    sLineLayout text ;              // the header line itself
} ;


struct sFontSizes
{
    double width ;
    double height ;
    double descent ;
    double externalLeading ;
} ;


// per paragraph styles
struct sModifierParagraph
{
    QFont font ;                                   ///< the font at the end of the paragraph
    QColor textcolor ;                             ///< the color of the text
    bool bold, italics, underline ;                 ///< the attributes at the end of the paragraph
    bool superscript, subscript, strikethrough ;    ///< the attributes at the end of the paragraph
    bool right, left, justify, center ;             ///< the attributes at the end of the paragraph


    double linespace ;

//    sStyle style ;                                  ///< the style of the pargraph
} ;

#ifdef CACHE_DOT
// per paragraph dot command changes (accumalated)
struct sAccumDotCommands
{
    COORD_T paragraphgmargin ;
    COORD_T pageoffseteven ;
    COORD_T pageoffsetodd ;
    COORD_T paperwidth ;
    COORD_T paperheight ;
    COORD_T leftmargin ;
    COORD_T rightmargin ;
    COORD_T topmargin ;
    COORD_T bottommargin ;
    COORD_T headermargin ;
    COORD_T footermargin ;
    COORD_T pageextend ;
    COORD_T spacebefore, spaceafter ;                   ///< space before and after a paragraph

    sPageInfo currentpage ;

    // we really don't need to save headers and footers, since it's a print only thing... unless we do true page wysiwyg
    // and if we do it, don't do it this way. Instead, save an index into a separate vector that stores the headers
    size_t lastheaderfooter ;               // index into separate header/footer storage
//    vector<sHeaderFooter> headers ;
//    vector<sHeaderFooter> footers ;
//    vector<sHeaderFooter> headersEven ;
//    vector<sHeaderFooter> footersEven ;
//    vector<sHeaderFooter> headersOdd ;
//    vector<sHeaderFooter> footersOdd ;


    bool validleftmargin ;
    bool validrightmargin ;
    bool validparagraphmargin ;
} ;
#endif

// a collection of lines that make a single paragraph
struct sParagraphLayout
{
    bool pagebreak ;                // used when looking backwards for information on .PA command (see WordWrapParagraph)
    bool isKnownDot ;               // true if this is a known dot command
    bool isCommand ;                // true if this is a dot command
    bool isComment ;                // true if this is a comment
    char isSpecialComment ;         // value 0 through 9 if valid special comment, otherwise 'x'
    PARAGRAPH_T number ;
    sModifierParagraph modifiers ;
#ifdef CACHE_DOT
    sAccumDotCommands dots ;
#endif
    std::vector<sLineLayout> lines ;
} ;

class cEditorCtrl ;

class cLayout
{
public:
    cLayout(cDocument *document, cEditorCtrl *editor, bool forprint = false);
    virtual ~cLayout();


    bool LayoutParagraph(PARAGRAPH_T paragragh);
    void FindPageBox(PAGE_T page, COORD_T &box_top, COORD_T &box_left, COORD_T &box_bottom, COORD_T &box_right) ;
    
    PARAGRAPH_T GetParagraphFromLine(LINE_T &line) ;
    LINE_T GetLineFromPosition(POSITION_T pos) ;
    
    PAGE_T GetNumberofPages(void) ;
    PARAGRAPH_T GetNumberofParagraphs(void) ;
    LINE_T GetNumberofLines(void) ;
    LINE_T GetNumberofLinesinParagraph(PARAGRAPH_T para) ;
    POSITION_T GetLineStartPosition(LINE_T line) ;
    POSITION_T GetLineEndPosition(LINE_T line) ;
    sPageInfo *GetPageInfoFromLine(LINE_T line) ;
    sPageInfo *GetPageInfo(PAGE_T page) ;
    
protected:

private:
    void SetupParagraph(PARAGRAPH_T para) ;
    void SetupPage(void) ;
    void CheckCommentorCommand(std::string &paragraphtext, std::vector<POSITION_T> &paragraphoffsets) ;

    std::vector<COORD_T> MeasureParagraphText(std::string &text, std::vector<POSITION_T> &offsets) ;

    void PrepareDotCommand(std::string &text);
    void HandleHeadersandFooters(std::string &paragraphtext, std::vector<POSITION_T> &poffsets) ;

#ifndef CACHE_DOT
	COORD_T PreviousDotCommands(PARAGRAPH_T paragraph);
#endif
    bool PerformDotCommand(std::string &command2, std::string &text) ;
    CHAR_T CheckForChanges(POSITION_T bufferpos, std::string &measure, std::vector<COORD_T> &positions, QFont &font, std::string &fontstr, double &fontchgwidth) ;
    void MeasureText(std::string &measure, std::vector<COORD_T> &positions, QFont &font);
    CHAR_T ApplyAttribute(QFont &font, eModifiers attribute, bool assumeset = false) ;

//    void WordWrapParagraph(COORD_T &pageextend, PAGE_T page, PARAGRAPH_T paragraph, vector<COORD_T> &widths, COORD_T &drawx, POSITION_T start) ;
    void WordWrapParagraph(COORD_T &pageextend, PARAGRAPH_T paragraph) ;

	bool SaveLine(bool paCommand, PAGE_T &pagenumber, COORD_T &ypos, COORD_T &tempy, COORD_T &pagetest, sLineLayout &finalline);

	void Paginate(COORD_T pageextend, PARAGRAPH_T paragraph);
    bool CreateParagraphs(PARAGRAPH_T) ;
            
    void IsCommentorCommand(sSegmentLayout &segment, bool &comment, bool &command) ;
    
    void JustifyLine(sLineLayout &line) ;
    COORD_T GetLastNonBreakOffset(sLineLayout &line) ;
    size_t GetLastNonBreakPosition(sLineLayout &line) ;

    void ParseTabs(std::string nums) ;
    
    void InsertHeaderFooter(PAGE_T page, COORD_T &tempy) ;
    void HeaderFooterReplace(sLineLayout &line, PAGE_T &pagenumber) ;
    
    void SavePageInfo(PAGE_T pagenumber) ;

    std::string GetGrapheme(ssize_t &index, std::string &text, std::vector<POSITION_T> &offsets) ;
    
public:
    std::vector<sPageInfo> mPageInfo ;                           ///< per page info
    std::vector<sParagraphLayout> mParagraphLayout ;             ///< the layout of a single paragraph

//    wxGraphicsContext *mFontGC ;                            ///< the wxGraphicsContext we use for measuring fonts

    // wordstar lays out a page differently. In something like word, you specify a page width, and then the margins
    // for example, an 8.5" papaer with 1" left and right margins gives 6.5 inches for text
    // in wordstar, you specify the page offset (left margin) and the right margin, so a left marging of 1" and a 
    // right margin of 6.5" will leave a 1" right margin on an 8.5" wide paper.
    
    COORD_T mPaperWidth ;
    COORD_T mPaperHeight ;
    COORD_T mParagraphMargin ;                              ///< .pm first line in/outdent of paragraph margin, in twips
    COORD_T mRightMargin, mLeftMargin ;                     ///< .rm .lm right and left margin on print, in twips
    COORD_T mTopMargin, mBottomMargin ;                     ///< top and bottom margin in twips
    COORD_T mPageOffsetOdd, mPageOffsetEven ;               ///< this is what Word would call a left margin in twips
    COORD_T mHeaderMargin, mFooterMargin ;                  ///< .hm .fm header and footer porsitions in twips
    COORD_T mSpaceBefore, mSpaceAfter ;                      ///< the space before and after a paragraph on twips
    bool mLandscapeMode ;                                    ///< .pr or= setting

    bool mValidParagraphMargin ;                            ///< .pm is mParagraphMargin a valid setting
    bool mValidRightMargin, mValidLeftMargin ;              ///< .rm .lm is right or left margin setting valid

    std::vector<sHeaderFooter> mHeaders ;
    std::vector<sHeaderFooter> mFooters ;
    std::vector<sHeaderFooter> mHeadersEven ;
    std::vector<sHeaderFooter> mFootersEven ;
    std::vector<sHeaderFooter> mHeadersOdd ;
    std::vector<sHeaderFooter> mFootersOdd ;

    cDocument *mDocument ;
    
#ifdef LAYOUT_TIMER
    qint64 prelimt ;    // preliminary timer
    qint64 wwt ;        // word wrap timer
    qint64 pt  ;        // paragraph timer
    qint64 mt ;         // measure timer`
    qint64 pdt ;        // previous dot commands timer
    qint64 postt ;      // post layout work
#endif

#ifdef DETAIL_LAYOUT_TIMER
    qint64 pdct ;       // previous dot command timer
    qint64 prepdct ;    // prepare dot comman timer
    qint64 perfdct ;    // perform dot command timer
    qint64 measurecount ;
    qint64 percall;     // only measure time in the actual measure call
#endif

private:
    cEditorCtrl *mEditor ;

//    wxBitmap mBitmap ;
    
    
    COORD_T mBoxTop, mBoxLeft, mBoxBottom, mBoxRight ;      ///< The coordinates of the current drawing box
    
    QFont mCurrentFont ;                                   ///< current font in use
    QColor mCurrentColor ;                                 ///< current color in use
    bool mIsItalics ;                                       ///< true if currently doing italics
    bool mIsBold ;                                          ///< true if currently doing bold
    bool mIsUnderline ;                                     ///< true if currently doing underline
    bool mIsSuperscript ;                                   ///< true if currently doing superscript
    bool mIsSubscript ;                                     ///< true if currently doing supscript
    bool mIsStrikethrough ;                                 ///< true if currently doing strikethrough

    bool mCenterLine ;                                      ///< should we draw a centered line
    bool mRightLine ;                                       ///< should we draw a right justified line
    bool mLeftLine ;                                        ///< should we draw a left justified line (default)
    bool mJustifyLine ;                                     ///< should we draw a justified line

    bool mTabCenterLine ;                                   ///< center line for single line only ^OC
    bool mTabRightLine ;                                    ///< right align for single line only ^OJ

    bool mFinalParaLine ;                                   ///< true if this is the last line in the paragraph
    
    double mLineSpace ;                                     ///< line spacing
    COORD_T mTabOffset ;                                    ///< if we have a tab, this is the next character offset
    
    COORD_T mDrawx ;
//    COORD_T mDrawy ;
    COORD_T mLayoutHeight ;
    PARAGRAPH_T mCurrentParagraph ;                         ///< the paragraph number in cDocument we are working on
    
    std::deque<sSegmentLayout> mTempSegmentList ;               ///< our temporary segment list
    std::vector<sLineLayout> mTempLineList ;                     ///< our temporary line list
    
    COORD_T mLastGlyphWidth ;                               ///< used to keep glyph width between segments
    
    bool mPageBreak ;                                       ///< true if this line is a page break (.pa) else false
    bool mDoNewPage ;                                       ///< if the previous paragraph was a .pa, increase page number
    PAGE_T mPageNumber ;
    LINE_T mLineNumber ;
    COORD_T mCumulativeLineHeight ;                          ///< cumulative height of current line (sum of lines above)
    
    bool mIsCommand ;                                       ///< this paragraph is a dot command
    bool mIsComment ;                                       ///< this paragraph is a comment
    bool mIsSpecialComment ;                                ///< this paragraph is a special comment (different color)
    bool mIsKnownDotCommand ;                               ///< true if this is a dot command we know, else false
    bool mInMarkedBlock ;                                   ///< are we laying out a marked block
    bool mInMarkedSearchBlock ;                             ///< are we laying out a  search block
    POSITION_T mParagraphStart ;                            ///< where this paragraph starts in the buffer
    POSITION_T mParagraphEnd ;                              ///< where this paragraph ends in the buffer

    bool mForPrint ;                                        ///< true if we are doing layout for printing
    
    int mHeaderValue ;                                      ///< 0 if paragraph is not a header, else 1 through 5
    int mFooterValue ;                                      ///< 0 if paragraph is not a footer, else 1 through 5
    
    std::array<sLineLayout, MAX_HEADER_FOOTER> mStoreHeader ;                    ///< storage of header info for use when breaking a page
    std::array<sLineLayout, MAX_HEADER_FOOTER> mStoreFooter ;                    ///< storage of header info for use when breaking a page
    std::array<sLineLayout, MAX_HEADER_FOOTER> mStoreHeaderEven ;                    ///< storage of header info for use when breaking a page
    std::array<sLineLayout, MAX_HEADER_FOOTER> mStoreFooterEven ;                    ///< storage of header info for use when breaking a page
    std::array<sLineLayout, MAX_HEADER_FOOTER> mStoreHeaderOdd ;                    ///< storage of header info for use when breaking a page
    std::array<sLineLayout, MAX_HEADER_FOOTER> mStoreFooterOdd ;                    ///< storage of header info for use when breaking a page
    
    sPageInfo mCurrentPage ;                                ///< the current pages structure
    
    void GetLastYPosition();
};

#endif // LAYOUT_H
