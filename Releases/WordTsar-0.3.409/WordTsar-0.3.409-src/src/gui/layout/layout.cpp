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

#include <QElapsedTimer>
#include <QMessageBox>

#include "layout.h"

#include "src/gui/editor/editorctrl.h"

#include <cstdio>
#include <string>
#include <cassert>

using namespace std ;


extern sSeqNewColor gBaseWSColors[] ;

cLayout::cLayout(cDocument *document, cEditorCtrl *editor, bool forprint)
{
    mDocument = document ;
    mEditor = editor ;
    mForPrint = forprint ;
    
//    mEditor->SetShowControls(SHOW_ALL) ;

    mPageBreak = false ;
    mDoNewPage = false ;
    
    mLineSpace = 1.0 ;
    
    mInMarkedBlock = false ;
    mInMarkedSearchBlock = false ;

printf("sSegmentLayout size %ld\n", sizeof(sSegmentLayout)) ;
fflush(stdout) ;
}



cLayout::~cLayout()
{
}



bool cLayout::LayoutParagraph(PARAGRAPH_T paragraph)
{
#ifdef LAYOUT_TIMER
    QElapsedTimer sw ;
    QElapsedTimer measuretimer ;

    sw.start() ;
#endif

    // no layout if no text
    if(mDocument->GetTextSize() == 0)
    {
        return false ;
    }

    // can't format a paragraph we don't have
    PARAGRAPH_T paracount = mDocument->GetNumberofParagraphs();
    if(paragraph >= paracount || paragraph < 0)
    {
        return false ;
    }

    mCurrentParagraph = paragraph ;
    mHeaderValue = 0 ;
    mFooterValue = 0 ;
    mLandscapeMode = false ;
    
//    MY_ASSERT(paragraph <= mParagraphLayout.size())

    SetupParagraph(paragraph) ;
    SetupPage() ;
    
    // set default tab stops (overriden by .tb command)
    vector<COORD_T> tabstops ;
    tabstops.push_back(0) ;             // Wordstar sets a tab stop at the left margin
    COORD_T tabsize = static_cast<unsigned long>(TWIPSPERINCH / 2) ;           // default tabs are 1/2" == 720 twips
    for(short loop = 0; loop < 12; loop++)
    {
        tabstops.push_back(tabsize) ;
        tabsize += static_cast<unsigned long>(TWIPSPERINCH / 2) ;
    }
    tabstops.push_back(mRightMargin) ;

    mEditor->SetTabs(tabstops) ;

    
// this is where we figure out what rect to draw in (box layout)
    FindPageBox(mPageNumber, mBoxTop, mBoxLeft, mBoxBottom, mBoxRight) ;

    // make a local copy of the paragraph, and get its grapheme count and boundaries
    vector<POSITION_T> paragraphoffsets ;
    string paragraphtext = mDocument->GetParagraphText(paragraph) ;         // the the full paragraph, including MARKER_CHAR
    size_t paragraphlength = mDocument->GetParagraphGraphemeOffsets(paragraph, paragraphoffsets);

    POSITION_T start, end ;
    mParagraphStart = 0 ;
    mParagraphEnd = paragraphlength ;

    start = 0 ;
    end = mParagraphEnd ;

    // figure out if this is a comment or command paragraph
    CheckCommentorCommand(paragraphtext, paragraphoffsets) ;
    
#ifdef LAYOUT_TIMER
    prelimt += sw.nsecsElapsed() ;
    
    sw.start() ;
#endif
    //
    // Step 1: Apply any previous dot commands
    //
    bool oldpagebreak = mPageBreak ;

#ifdef CACHE_DOT
    COORD_T pageextend = 0 ;
    bool newpage = false ;
    if(paragraph != 0)
    {
        mParagraphMargin = mParagraphLayout[paragraph - 1].dots.paragraphgmargin ;
        mPageOffsetEven = mParagraphLayout[paragraph - 1].dots.pageoffseteven ;
        mPageOffsetOdd = mParagraphLayout[paragraph - 1].dots.pageoffsetodd ;
        mPaperWidth = mParagraphLayout[paragraph - 1].dots.paperwidth ;
        mPaperHeight = mParagraphLayout[paragraph - 1].dots.paperheight ;
        mLeftMargin = mParagraphLayout[paragraph - 1].dots.leftmargin ;
        mRightMargin = mParagraphLayout[paragraph - 1].dots.rightmargin ;
        mTopMargin = mParagraphLayout[paragraph - 1].dots.topmargin ;
        mBottomMargin = mParagraphLayout[paragraph - 1].dots.bottommargin ;
        mHeaderMargin = mParagraphLayout[paragraph - 1].dots.headermargin ;
        mFooterMargin = mParagraphLayout[paragraph - 1].dots.footermargin ;
        mCurrentPage = mParagraphLayout[paragraph - 1].dots.currentpage ;

        mSpaceAfter = mParagraphLayout[paragraph - 1].dots.spaceafter ;
        mSpaceBefore = mParagraphLayout[paragraph - 1].dots.spacebefore ;

        mValidLeftMargin = mParagraphLayout[paragraph - 1].dots.validleftmargin ;
        mValidRightMargin = mParagraphLayout[paragraph - 1].dots.validrightmargin ;
        mValidParagraphMargin = mParagraphLayout[paragraph - 1].dots.validparagraphmargin ;

//        mHeaders = mParagraphLayout[paragraph - 1].dots.headers ;
//        mFooters = mParagraphLayout[paragraph - 1].dots.footers ;
//        mHeadersEven = mParagraphLayout[paragraph - 1].dots.headersEven ;
//        mHeadersOdd = mParagraphLayout[paragraph - 1].dots.headersOdd ;
//        mFootersEven = mParagraphLayout[paragraph - 1].dots.footersEven ;
//        mFootersOdd = mParagraphLayout[paragraph - 1].dots.footersOdd ;

        pageextend = mParagraphLayout[paragraph - 1].dots.pageextend ;

        mLineSpace = mParagraphLayout[paragraph - 1].modifiers.linespace ;
// new
        mEditor->SetTabs(mParagraphLayout[paragraph - 1].dots.tabs) ;

    }
#else
    COORD_T pageextend = PreviousDotCommands(paragraph) ;
#endif

    mPageBreak = oldpagebreak ;                                     // if the dot commands gave us a page break, we don't care here
    mHeaderValue = 0 ;                                              // we also don't care about headers and footers
    mFooterValue = 0 ;

#ifdef LAYOUT_TIMER
    pdt += sw.nsecsElapsed() ;
    
    measuretimer.start() ;
#endif

    //
    // Step 2: Measure all text in paragraph, including visible markers, etc
    //
    vector<COORD_T> widths ;
#ifdef NDEBUG
    try
#endif
    {
        mTempSegmentList.clear() ;
        mTempLineList.clear() ;

        widths = MeasureParagraphText(paragraphtext, paragraphoffsets) ;
    }
#ifdef NDEBUG
    catch (...)
    {
        mParagraphLayout.clear() ;          // free some memory, just in case
        mEditor->EmergencySaveFile() ;
    }
#endif
#ifdef LAYOUT_TIMER
    mt += measuretimer.nsecsElapsed() ;
    
    sw.start() ;
#endif

    //
    // Step 3: Word wrap our paragraph
    //
#ifdef NDEBUG
    try
#endif
    {
//        WordWrapParagraph(pageextend, mPageNumber, paragraph, widths, mBoxLeft, start);            // creates line layout struct
        WordWrapParagraph(pageextend, paragraph);            // creates line layout struct
    }
#ifdef NDEBUG
    catch(...)
    {
        mParagraphLayout.clear() ;          // free some memory, just in case
        mEditor->EmergencySaveFile() ;
    }
#endif

#ifdef LAYOUT_TIMER
    wwt += sw.nsecsElapsed() ;
    
    sw.start() ;
#endif

    //
    // Step 4: Create the paragraph layout struct for this paragraph
    //
    bool same = false ;
#ifdef NDEBUG
    try
#endif
    {
        same = CreateParagraphs(paragraph) ;  // creates paragraph layout struct
    }
#ifdef NDEBUG
    catch(...)
    {
        mParagraphLayout.clear() ;          // free some memory, just in case
        mEditor->EmergencySaveFile() ;
    }
#endif

#ifdef LAYOUT_TIMER
    pt += sw.nsecsElapsed() ;
    
    sw.start() ;
#endif

    //
    // and finally, remember the state of our modifiers and dot commands
    //
    mParagraphLayout[paragraph].modifiers.bold = mIsBold ;
    mParagraphLayout[paragraph].modifiers.underline = mIsUnderline ;
    mParagraphLayout[paragraph].modifiers.italics = mIsItalics ;
    mParagraphLayout[paragraph].modifiers.strikethrough = mIsStrikethrough ;
    mParagraphLayout[paragraph].modifiers.superscript = mIsSuperscript ;
    mParagraphLayout[paragraph].modifiers.subscript = mIsSubscript ;
    mParagraphLayout[paragraph].modifiers.right = mRightLine ;
    mParagraphLayout[paragraph].modifiers.center = mCenterLine ;
    mParagraphLayout[paragraph].modifiers.justify = mJustifyLine ;
    mParagraphLayout[paragraph].modifiers.left = mLeftLine ;
    mParagraphLayout[paragraph].modifiers.linespace = mLineSpace ;

    mParagraphLayout[paragraph].modifiers.font = mCurrentFont ;
    mParagraphLayout[paragraph].modifiers.textcolor = mCurrentColor ;
    mParagraphLayout[paragraph].isKnownDot = mIsKnownDotCommand ;
    mParagraphLayout[paragraph].isCommand = mIsCommand ;
    mParagraphLayout[paragraph].isComment = mIsComment ;
    if(mIsComment)
    {
        mParagraphLayout[paragraph].isKnownDot = true ;
    }
    mParagraphLayout[paragraph].isSpecialComment = mIsSpecialComment ;

#ifdef CACHE_DOT
    if(mIsCommand || mIsComment) //  || mIsSpecialComment)
    {
        for(auto & line : mParagraphLayout[paragraph].lines)
        {
            pageextend += line.lineheight ;
        }
    }

    mParagraphLayout[paragraph].dots.paragraphgmargin = mParagraphMargin ;
    mParagraphLayout[paragraph].dots.pageoffseteven = mPageOffsetEven ;
    mParagraphLayout[paragraph].dots.pageoffsetodd = mPageOffsetOdd ;
    mParagraphLayout[paragraph].dots.paperwidth = mPaperWidth ;
    mParagraphLayout[paragraph].dots.paperheight = mPaperHeight ;
    mParagraphLayout[paragraph].dots.leftmargin = mLeftMargin ;
    mParagraphLayout[paragraph].dots.rightmargin = mRightMargin ;
    mParagraphLayout[paragraph].dots.topmargin = mTopMargin ;
    mParagraphLayout[paragraph].dots.bottommargin = mBottomMargin ;
    mParagraphLayout[paragraph].dots.headermargin = mHeaderMargin ;
    mParagraphLayout[paragraph].dots.footermargin = mFooterMargin ;
    mParagraphLayout[paragraph].dots.pageextend = pageextend ;

    mParagraphLayout[paragraph].dots.spaceafter = mSpaceAfter ;
    mParagraphLayout[paragraph].dots.spacebefore = mSpaceBefore ;

    mParagraphLayout[paragraph].dots.currentpage = mCurrentPage ;

    mParagraphLayout[paragraph].dots.validleftmargin = mValidLeftMargin ;
    mParagraphLayout[paragraph].dots.validrightmargin = mValidRightMargin ;
    mParagraphLayout[paragraph].dots.validparagraphmargin = mValidParagraphMargin ;

//    mParagraphLayout[paragraph].dots.headers = mHeaders ;
//    mParagraphLayout[paragraph].dots.footers = mFooters ;
//    mParagraphLayout[paragraph].dots.headersEven = mHeadersEven ;
//    mParagraphLayout[paragraph].dots.headersOdd = mHeadersOdd ;
//    mParagraphLayout[paragraph].dots.footersEven = mFootersEven ;
//    mParagraphLayout[paragraph].dots.footersOdd = mFootersOdd ;
#endif

    mIsCommand = false ;
    mIsComment = false ;
    mIsSpecialComment = 'x' ;
    mHeaderValue = 0 ;
    mFooterValue = 0 ;

    
    // if this is our last page, remember our current paper size for this page
    if(mEditor->GetIsHelp() == false)
    {
        if(paragraph + 1 == paracount)
        {
            SavePageInfo(mPageNumber) ;
        }
    }
    
#ifdef LAYOUT_TIMER
    postt += sw.nsecsElapsed() ;
    
    sw.start() ;
#endif
    return same;
}





void cLayout::FindPageBox(PAGE_T page, COORD_T &box_top, COORD_T &box_left, COORD_T &box_bottom, COORD_T &box_right)
{
    UNUSED_ARGUMENT(page) ;
    
    if(mEditor->GetIsHelp() == true)
    {
        box_top = 0 ;
        box_left = 0 ;
        box_bottom = 655350 ;        // arbitrary large number
        box_right = 655350 ;         // arbitrary large number
    }
    else            // Do the magic here
    {
        // get page number, find out what box we are in on the page, and return it.
        
        // fake out a full page for now (8.5 x 11)
        box_top = 0 ;
        box_left = 0 ;
        box_bottom = mPaperHeight - mTopMargin- mBottomMargin ; // * TWIPSPERINCH ;
        box_right = mRightMargin ;
    }
}




/////////////////////////////////////////////////////////////////////////////
///
/// @param  line    [IN/OUT] - the line we are searching on, return the line the paragraph starts at
///
/// @return PARAGRAPH_T
///
/// @brief Get the paragraph number from the line number
///
/////////////////////////////////////////////////////////////////////////////
PARAGRAPH_T cLayout::GetParagraphFromLine(LINE_T &line)
{
    size_t len = mParagraphLayout.size() ;
    LINE_T countlines = 0 ;
    size_t ploop = 0 ;
    
    // go through our layout parargraph by paragraph and count the lines
    for(ploop = 0 ; ploop < len ; ploop++)
    {
        countlines += mParagraphLayout[ploop].lines.size() ;
        if(countlines > line)
        {
            line = countlines - static_cast<LINE_T>(mParagraphLayout[ploop].lines.size()) ;  // the first line in the paragraph
            break ;
        }
    }
    
    if ((ploop == len) && (len != 0))
    {
        ploop--;
        line = countlines - static_cast<LINE_T>(mParagraphLayout[ploop].lines.size()) ;  // the first line in the paragraph
    }

    return ploop ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @param  line    [IN] - the position we are searching on
///
/// @return LINE_T
///
/// @brief Get the line number from the position
///
/////////////////////////////////////////////////////////////////////////////
LINE_T cLayout::GetLineFromPosition(POSITION_T pos)
{
    LINE_T retval = 0 ;
    
    if(mParagraphLayout.size() != 0)
    {
        POSITION_T start, end ;
        PARAGRAPH_T para = mDocument->GetParagraphFromPosition(pos) ;
        mDocument->GetParagraphStartandEnd(para, start, end) ;
        
        // not in formatted text yet
        if(para >= static_cast<PARAGRAPH_T>(mParagraphLayout.size()))
        {
            para = mParagraphLayout.size() - 1 ;
            pos = end - 1 ;
        }
        
        // count lines in previous paragraphs
        for(PARAGRAPH_T ploop = 0; ploop < para; ploop++)
        {
            retval += mParagraphLayout[ploop].lines.size() ;
        }
        
        bool breakit = false ;
        size_t numlines = mParagraphLayout[para].lines.size() ;
        for(size_t lloop = 0; lloop < numlines; lloop++)
        {
            size_t numsegments = mParagraphLayout[para].lines[lloop].segments.size() ;
            for(size_t sloop = 0; sloop < numsegments; sloop++)
            {
                POSITION_T numglyphs = mParagraphLayout[para].lines[lloop].segments[sloop].glyph.size() ;
                if(start + numglyphs > pos)
                {
                    retval += lloop ;
                    breakit = true ;
                    break ;
                }
                start += numglyphs ;
            }
            
            if(breakit)
            {
                break ;
            }
        }
    }
    return retval ;
}




/////////////////////////////////////////////////////////////////////////////
///
/// @return PAGE_T
///
/// @brief Get the number of pages
///
/////////////////////////////////////////////////////////////////////////////
PAGE_T cLayout::GetNumberofPages(void)
{
    PAGE_T retval = 0 ;
    if(!mParagraphLayout.empty())
    {
        PARAGRAPH_T pnum = mParagraphLayout.size() ;
        if(!mParagraphLayout[pnum - 1].lines.empty())
        {
           retval =  mParagraphLayout[pnum - 1].lines[mParagraphLayout[mParagraphLayout.size() - 1].lines.size() - 1].pagenumber ;
        }
    }
    
    return retval ;
}

/////////////////////////////////////////////////////////////////////////////
///
/// @return PARAGRAPH_T
///
/// @brief  returns the number of paragraphs in the layout, which may be different than the
///         number of paragraphs in the document du to hiding of dot commands and comments
///
/////////////////////////////////////////////////////////////////////////////
PARAGRAPH_T cLayout::GetNumberofParagraphs(void)
{
    return mParagraphLayout.size() ;
}



LINE_T cLayout::GetNumberofLines(void)
{
    LINE_T retval = 0 ;
    
    PARAGRAPH_T numpara = mParagraphLayout.size() ;
    for(PARAGRAPH_T ploop = 0; ploop < numpara; ploop++)
    {
        retval += mParagraphLayout[ploop].lines.size() ;
    }
    
    return retval ;
}


LINE_T cLayout::GetNumberofLinesinParagraph(PARAGRAPH_T para)
{
    LINE_T retval = 0 ;

    if(para < static_cast<PARAGRAPH_T>(mParagraphLayout.size()))
    {
        retval = static_cast<LINE_T>(mParagraphLayout[static_cast<size_t>(para)].lines.size()) ;
    }

    return retval ;
}




POSITION_T cLayout::GetLineStartPosition(LINE_T line)
{
    LINE_T oldline = line ;
    PARAGRAPH_T para = GetParagraphFromLine(line) ;
    line = oldline ;
    
    POSITION_T start, end ;
    mDocument->GetParagraphStartandEnd(para, start, end) ;
    POSITION_T startofline = start ;
    LINE_T parastartline = GetLineFromPosition(start) ;

    if(parastartline != line)              // if we are on the first line, just return the current startofline 
    {
        if(mEditor->GetShowControls() == SHOW_ALL)
        {
            // go through lines in para
            for(auto currentline : mParagraphLayout[para].lines)
            {
                for(auto currentsegment : currentline.segments)
                {
                    startofline += currentsegment.glyph.size() ;
                }
                parastartline++ ;
                if(parastartline == line)
                {
                    break ;
                }
            }
        }
        else
        {
            // go through lines in para and GetChar to get proper offsets
            for(auto currentline : mParagraphLayout[para].lines)
            {
                for(auto currentsegment : currentline.segments)
                {
                    for(auto currentglyph : currentsegment.glyph)
                    {
                        UNUSED_ARGUMENT(currentglyph) ;
                        mDocument->GetChar(startofline) ;
                        startofline++ ;
                    }
                }
                parastartline++ ;
                if(parastartline == line)
                {
                    break ;
                }
            }
        }
    }
    return startofline ;    
}




POSITION_T cLayout::GetLineEndPosition(LINE_T line)
{
    LINE_T oldline = line ;
    PARAGRAPH_T para = GetParagraphFromLine(line) ;
    line = oldline ;
    
    POSITION_T start, end ;
    mDocument->GetParagraphStartandEnd(para, start, end) ;
    POSITION_T endofline = start ;    
    LINE_T parastartline = GetLineFromPosition(start) ;

    if(mEditor->GetShowControls() == SHOW_ALL)
    {
        // go through lines in para
        for(auto currentline : mParagraphLayout[para].lines)
        {
            for(auto currentsegment : currentline.segments)
            {
                endofline += currentsegment.glyph.size() ;
            }
            if(parastartline == line)
            {
                break ;
            }
            parastartline++ ;
        }
    }
    else
    {
        // go through lines in para
        for(auto currentline : mParagraphLayout[para].lines)
        {
            for(auto currentsegment : currentline.segments)
            {
                for(auto currentglyph : currentsegment.glyph)
                {
                    UNUSED_ARGUMENT(currentglyph) ;
                    mDocument->GetChar(endofline) ;
                    endofline++ ;
                }
            }
            if(parastartline == line)
            {
                break ;
            }
            parastartline++ ;
        }
    }
    return endofline - 1 ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @param   line          [in] - the line we are looking for
///
/// @return  sPageInfo
///
/// @brief
/// returns the page infromation for the line
///
/////////////////////////////////////////////////////////////////////////////
sPageInfo *cLayout::GetPageInfoFromLine(LINE_T line)
{
    LINE_T oldline = line ;
    PARAGRAPH_T para = GetParagraphFromLine(line) ;
    line = oldline ;

    POSITION_T start, end ;
    mDocument->GetParagraphStartandEnd(para, start, end) ;
    LINE_T parastartline = GetLineFromPosition(start) ;

    // go through lines in para
    for(auto currentline : mParagraphLayout[para].lines)
    {
        if(parastartline == line)
        {
            return &mPageInfo[ static_cast<size_t>(currentline.pagenumber) - 1 ] ;
        }
    }
    
    
    return &mPageInfo[mPageInfo.size() - 1] ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param   page          [in] - the page we are looking for
///
/// @return  sPageInfo
///
/// @brief
/// returns the page infromation for the specific page
///
/////////////////////////////////////////////////////////////////////////////
sPageInfo *cLayout::GetPageInfo(PAGE_T page)
{
    if(page <= static_cast<PAGE_T>(mPageInfo.size()))
    {
        page-- ;
        
        return &mPageInfo[static_cast<size_t>(page)] ;
    }    
    
    return &mPageInfo[mPageInfo.size() - 1] ;
}






/////////////////////////////////////////////////////////////////////////////
///
/// @param  ptext    [IN] - the text we are working on
/// @param  poffsets [IN] - the grapheme offsets
///
/// @return vector<COORD_T> - the array of grapheme positions
///
/// @brief Do the layout of a single paragraph. Create all the segments it 
///        takes to display the paragaphs text 
///
/////////////////////////////////////////////////////////////////////////////
vector<COORD_T> cLayout::MeasureParagraphText(string &ptext, vector<POSITION_T> &poffsets)
{
    QFont font ;                   // the font we are using
    string measure ;              // the string we are measuring
    vector<COORD_T> positions ;     // the position of each character

    mLayoutHeight = 0 ;
    mLastGlyphWidth = 0 ;
    
    if(mIsCommand)                    // this is a command
    {
        // dot commands are ascii, so this is easy
        PrepareDotCommand(ptext);

        // if this is a header or footer, remove the leading dot command and a single space (as per wordstar)
        HandleHeadersandFooters(ptext, poffsets) ;
	}

    POSITION_T start = mParagraphStart ;
    POSITION_T end = mParagraphEnd ;
    
    // if this is our first paragraph, use the base font
    if(mCurrentParagraph == 0)
    {
        font = mEditor->GetFont();
    }
    else    // get the results from the last paragraph
    {
        font = mCurrentFont ;
    }
        
    // save our font going in...
    QFont currentfont = font ;

    POSITION_T realparagraphstart, notneeded ;

    // get paragraph start and end in the buffer
    mDocument->GetParagraphStartandEnd(mCurrentParagraph, realparagraphstart, notneeded) ;

    // now we'll go through the paragraph a grapheme at a time
    for(POSITION_T bufferpos = start; bufferpos < end; bufferpos++)
    {
        POSITION_T oldpos = bufferpos ;
        POSITION_T realbufferpos = realparagraphstart + bufferpos ;

        string ch = GetGrapheme(bufferpos, ptext, poffsets) ;
//        string ch = mDocument->GetChar(realbufferpos) ;
        CHAR_T attribch = 0 ;
                
        string fontstr ;                                    // in case of a font change, the name of the new font
        double fontchgwidth = 0 ;                           // in case of a font change, the width change


        // if we are not showing control codes, our bufferpos will be higher by the ammount of skipped codes
        // so we'll check our attributes to see if we still need to do a change
        if(bufferpos != oldpos)
        {
            for(POSITION_T loop = oldpos; loop < bufferpos; loop++)
            {
                attribch = CheckForChanges(loop + realparagraphstart, measure, positions, font, fontstr, fontchgwidth) ;
            }
            bufferpos = oldpos ;
        }
        else
        {
            attribch = STYLE_END_OF_STYLES ;
            // only do a lookup if we have a marker character
            if(ch[0] == MARKER_CHAR && ch.length() == 1)
            {
                attribch = CheckForChanges(bufferpos + realparagraphstart, measure, positions, font, fontstr, fontchgwidth) ;
                ch = attribch ;
            }
        }
                
        // see if we are at the start of a block, without an endblock marked
        if(realbufferpos == mDocument->mStartBlock && realbufferpos != 0 && mDocument->mBlockSet == false)
        {
            measure += '<' ;
        }
        
        // see if we have any marked text we need to highlight
        if(realbufferpos >= mDocument->mStartBlock && realbufferpos < mDocument->mEndBlock  && mDocument->mBlockSet == true)
        {
            if(realbufferpos == mDocument->mStartBlock)
            {
                MeasureText(measure, positions, font) ;
            }
            mInMarkedBlock = true ;
        }
        else
        {
            if(realbufferpos == mDocument->mEndBlock && realbufferpos != 0)
            {
                MeasureText(measure, positions, font) ;
            }
            mInMarkedBlock = false ;
        }

        if(realbufferpos >= mEditor->mStartSearchBlock && realbufferpos < mEditor->mEndSearchBlock  && mEditor->mSearchBlockSet == true)
        {
            if(realbufferpos == mEditor->mStartSearchBlock)
            {
                MeasureText(measure, positions, font) ;
            }
            mInMarkedSearchBlock = true ;
        }
        else
        {
            if(realbufferpos == mEditor->mEndSearchBlock && realbufferpos != 0)
            {
                MeasureText(measure, positions, font) ;
            }
            mInMarkedSearchBlock = false ;
        }

        // see if we have a saved position we need to deal with
        int sploop ;
        for(sploop = 0; sploop < 10; sploop++)
        {
            if(realbufferpos == mDocument->mSavePosition[sploop] && bufferpos != 0)
            {
                measure += sploop + '0' ;
                break ;
            }
        }

        
        if(mEditor->GetShowControls() != SHOW_ALL)
        {
            if(attribch >= STYLE_END_OF_STYLES || attribch == STYLE_TAB)
            {
                measure += ch ;
            }
        }
        else
        {
            measure += ch ;
        }
    }

    // just in case anything is left
    MeasureText(measure, positions, font) ;

    if(mIsCommand || mIsComment)
    {
        font = currentfont ;
    }
    mCurrentFont = font ;

    return positions ;
}

// assumes the dot command is in ascii, which should be right
void cLayout::PrepareDotCommand(string &ptext)
{
#ifdef DETAIL_LAYOUT_TIMER
    QElapsedTimer sw ;

    sw.start() ;
#endif

    string text ;

    // convert it all to upper case
    for(size_t loop = 0; loop < ptext.size(); loop++)
    {
        text.push_back(static_cast<char>(toupper(ptext[loop]))) ;
    }

    string command = text.substr(1, 2) ; //    mid(1, 2);                      // dot commands are two characters long max
    PerformDotCommand(command, text);

#ifdef DETAIL_LAYOUT_TIMER
    prepdct += sw.nsecsElapsed() ;
#endif
}



void cLayout::HandleHeadersandFooters(string &ptext, vector<POSITION_T> &poffsets)
{
    if((mHeaderValue != 0) || (mFooterValue != 0))
    {
        if(mForPrint)
        {
            mParagraphStart += 3 ;          // get past the command, but leave the odd or even indicator if there is one
        }

        // header/footer on both pages value 1 - MAX_HEADER_FOOTER (1-5)
        // header/footer on en pages value MAX_HEADER_FOOTER + 1 - MAX_HEADER_FOOTER * 2 (6-10)
        // header/footer on both pages value MAX_HEADER_FOOTER * 2 + 1 - MAX_HEADER_FOOTER * 2 + MAX_HEADER_FOOTER (11-15)
        string ch = GetGrapheme(mParagraphStart, ptext, poffsets) ;
        if(mHeaderValue != 0 && ch.length() == 1)
        {
            switch(ch[0])
            {
                case 'e' :
                case 'E' :
                    mHeaderValue += MAX_HEADER_FOOTER ;
                    break ;

                case 'o' :
                case 'O' :
                    mHeaderValue += (MAX_HEADER_FOOTER * 2) ;
                    break ;

                default :
                    break ;
            }
        }
        if(mFooterValue != 0 && ch.length() == 1)
        {
            switch(ch[0])
            {
                case 'e' :
                case 'E' :
                    mFooterValue += MAX_HEADER_FOOTER ;
                    break ;

                case 'o' :
                case 'O' :
                    mFooterValue += (MAX_HEADER_FOOTER * 2) ;
                    break ;

                default :
                    break ;
            }
        }

        if(mForPrint)
        {
            mParagraphStart++ ;     // get past odd/even indicator
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
///
/// @param  paragraph   [IN] - the paragraph we are working on
///
/// @return COORD_T  how much longer a page should be due to comments or commands
///
/// @brief Do all dot commands up to the paragraph passed in
///
/// We go through the document a paragraph at a time, looking for '..'
/// as the first two characters. If we find them, we do what the dot 
/// command says to do.
///
/// @todo This could probably be optimized.
///
/////////////////////////////////////////////////////////////////////////////
#ifndef CACHE_DOT
COORD_T cLayout::PreviousDotCommands(PARAGRAPH_T paragraph)
{
#ifdef DETAIL_LAYOUT_TIMER
    QElapsedTimer sw ;

    sw.start() ;
#endif
   
    PAGE_T page = 1 ;                       // keep track of pages so we know if we need to extend a page length
    COORD_T pageextend = 0.0 ;              // a command or comment extends the page length

    for(PARAGRAPH_T loop = 0; loop < paragraph; loop++)
    {
        if (mParagraphLayout[loop].isCommand == true)
        {
            string command = mDocument->GetParagraphText(loop) ;
            PrepareDotCommand(command);
            mHeaderValue = 0 ;
            mFooterValue = 0 ;

            // now calcualte how ,much extra the comment or command takes
            for(auto & line : mParagraphLayout[loop].lines)
            {
                pageextend += line.lineheight ;
            }
        }
        else
        {
            // check if we start a new page in this paragraph
            for(auto & line : mParagraphLayout[loop].lines)
            {
                if(line.pagenumber != page)
                {
                    pageextend = 0 ;
                    page = line.pagenumber ;
                }
            }
        }
    }
    
#ifdef DETAIL_LAYOUT_TIMER
    pdct += sw.nsecsElapsed() ;
#endif

    return pageextend ;
}
#endif



/////////////////////////////////////////////////////////////////////////////
///
/// @param   command       [in/out] - the dot command to perform
/// @param   text          [in/out] - the complete command line
///
/// @return  nothing
///
/// @brief
/// performs the work for the dot command
///
/////////////////////////////////////////////////////////////////////////////
bool cLayout::PerformDotCommand(string &command, string &text)
{
#ifdef DETAIL_LAYOUT_TIMER
    QElapsedTimer sw ;

    sw.start() ;
#endif

    bool complete = false ;
    bool incdec = false ;

    string str;           // temp string

    if (command.length() != 0)
    {
        // break our search up with switch statements
        char check = command[0] ;
        switch (check)
        {
            case 'P':
                // page break .pa 
                if (command == "PA")
                {
                    mPageBreak = true;
                    complete = true ;
                }
                
                // paragraph margin  .pm
                else if(command == "PM")
                {
                    double value = mDocument->GetValue(text.substr(3, text.length() - 3), incdec) ;
                    char type = mDocument->GetType(text.substr(3, text.length() - 3)) ;

                    double newpm;
                    newpm = mDocument->ConvertToTwips(value, type);

                    if(incdec)
                    {
                        mParagraphMargin += newpm;
                    }
                    else
                    {
                        mParagraphMargin = newpm ;
                    }
                    
                    if(type == 'x')
                    {
                        mValidParagraphMargin = false ;
                    }
                    else
                    {
                        mValidParagraphMargin = true ;
                    }
                    complete = true;
                }
                
                // page offset .po .poe .poo
                else if(command == "PO")
                {
                    str = text.substr(3, text.length() - 3) ;
                    
                    if(str[0] == 'E')                  // .poe
                    {
                        str = text.substr(4, text.length() - 4) ;


                        double value = mDocument->GetValue(str, incdec);
                        char type = mDocument->GetType(str);
                        
                        if(type != 'x')
                        {
                            double newpoe ;
                            newpoe = mDocument->ConvertToTwips(value, type) ;
                            
                            if(incdec)
                            {
                                mPageOffsetEven += newpoe;
                            }
                            else
                            {
                                mPageOffsetEven = newpoe;
                            }
                            complete = true ;
                        }
                    }
                    else if(str[0] == 'O')              // .poo
                    {
                        str = text.substr(4, text.length() - 4) ;

                        double value = mDocument->GetValue(str, incdec);
                        char type = mDocument->GetType(str);
                        
                        if(type != 'x')
                        {
                            double newpoo ;
                            newpoo = mDocument->ConvertToTwips(value, type) ;
                            
                            if(incdec)
                            {
                                mPageOffsetOdd += newpoo;
                            }
                            else
                            {
                                mPageOffsetOdd = newpoo;
                            }
                            complete = true ;
                        }
                    }
                    else                                // .po
                    {
                        double value = mDocument->GetValue(str, incdec);
                        char type = mDocument->GetType(str);
                        
                        if(type != 'x')
                        {
                            double newpo ;
                            newpo = mDocument->ConvertToTwips(value, type) ;
                            
                            if(incdec)
                            {
                                mPageOffsetOdd += newpo ;
                                mPageOffsetEven += newpo ;
                            }
                            else
                            {
                                mPageOffsetOdd = newpo ;
                                mPageOffsetEven = newpo ;
                            }
                            complete = true ;
                        }
                    }
                }

                // paper size .pt (not a Wordstar command)
                else if(command == "PT")   // .pt
                {

                    if(mCurrentPage.set == false)
                    {
                        str = text.substr(3, text.length() - 3) ;
                        
                        unsigned long index ;
                        index = stoul(str) ;
                        
                        QPageSize psize(static_cast<QPageSize::PageSizeId>(index)) ;

                        QSizeF tsize = psize.size(QPageSize::Millimeter) * TWIPSPERMM ;
                        mPaperWidth = tsize.width() ;
                        mPaperHeight = tsize.height() ;

                        mCurrentPage.paperwidth = mPaperWidth ;
                        mCurrentPage.paperheight = mPaperHeight ;
                        mCurrentPage.papertype = static_cast<QPageSize::PageSizeId>(index) ;
                        mCurrentPage.set = true ;
                        complete = true ;
                    }

                }

                else if(command == "PS")   // .psb .psa
                {
                    str = text.substr(3, text.length() - 3) ;

                    // space before a paragraph (not a Wordstar command)
                    if(str[0] == 'A')                   // .psa
                    {
                        str = text.substr(4, text.length() - 4) ;

                        double value = mDocument->GetValue(str, incdec);
                        char type = mDocument->GetType(str);

                        mSpaceAfter = mDocument->ConvertToTwips(value, type) ;
                        complete = true ;
                    }

                    // space after a paragraph (not a Wordstar command)
                    else if(str[0] == 'B')              // .psb
                    {
                        str = text.substr(4, text.length() - 4) ;

                        double value = mDocument->GetValue(str, incdec);
                        char type = mDocument->GetType(str);

                        mSpaceBefore = mDocument->ConvertToTwips(value, type) ;
                        complete = true ;
                    }
                }

                break;

            case 'L':
                // line spacing .ls
                if (command == "LS")
                {
                    double tempf = mDocument->GetValue(text.substr(3, text.length() - 3), incdec);
//                    str = text.substr(3, 10) ;
//                    double tempf;
//                    tempf = stod(str) ;
                    if (tempf > 0.25)
                    {
                        mLineSpace = tempf;
                        complete = true;
                    }
                }

                // left margin  .lm
                else if (command == "LM")
                {
                    double value = mDocument->GetValue(text.substr(3, text.length() - 3), incdec);
                    char type = mDocument->GetType(text.substr(4, text.length() - 4));

                    double newlm;
                    newlm = mDocument->ConvertToTwips(value, type);

                    if(incdec)
                    {
                        mLeftMargin += newlm;
                    }
                    else
                    {
                        mLeftMargin = newlm ;
                    }
                    mCurrentPage.leftmargin = mLeftMargin ;

                    if(type == 'x')
                    {
                        // reset tabs to default
                        vector<COORD_T>newtab ;
                        COORD_T tabstop = static_cast<unsigned long>(TWIPSPERINCH / 2) ;           // default tabs are 1/2" == 720 twips
                        newtab.push_back(0) ;                            // Wordstar sets a tab stop at the left margin.
                        for(short loop = 0; loop < 12; loop++)
                        {
                            newtab.push_back(tabstop) ;
                            tabstop += static_cast<unsigned long>(TWIPSPERINCH / 2) ;
                        }
                        newtab.push_back(mRightMargin) ;
                        mEditor->SetTabs(newtab) ;
                        
                        mValidLeftMargin = false ;
                    }
                    else
                    {
                        // Wordstar has a 'hidden' tab at the left margin
                        vector<COORD_T> tab = mEditor->GetTabs() ;
                        vector<COORD_T>newtab ;
                        newtab.push_back(mLeftMargin) ;
                        for(size_t loop = 1; loop < tab.size(); loop++)
                        {
                            if(tab[loop] > mLeftMargin)
                            {
                                newtab.push_back(tab[loop]) ;
                            }
                        }
                        newtab.push_back(mRightMargin) ;
                        mEditor->SetTabs(newtab) ;

                        mValidLeftMargin = true ;
                    }

                    complete = true;
                }
                break;

            case 'R':
                // right margin  .rm
                if (command == "RM")
                {
                    double value = mDocument->GetValue(text.substr(3, text.length() - 3), incdec);
                    char type = mDocument->GetType(text.substr(3, text.length() - 3)) ;

                    if(type != 'x')
                    {
                        double newrm;
                        newrm = mDocument->ConvertToTwips(value, type);
                        
                        if(incdec)
                        {
                            mRightMargin += newrm;
                        }
                        else
                        {
                            mRightMargin = newrm ;
                        }
                        mCurrentPage.rightmargin = mPaperWidth - mRightMargin - (mLeftMargin + mPageOffsetOdd) ;
                        
                        FindPageBox(mPageNumber, mBoxTop, mBoxLeft, mBoxBottom, mBoxRight) ;    /// @TODO - when we start using boxes
                        
                        if(type == 0)
                        {
                            mValidRightMargin = false ;
                        }
                        else
                        {
                            mValidRightMargin = true ;
                        }
                        complete = true;

                    }
                }
                break;

            case 'M':
                // top margin .mt
                if (command == "MT")
                {
                    double value = mDocument->GetValue(text.substr(3, text.length() - 3), incdec);
                    char type = mDocument->GetType(text.substr(3, text.length() - 3));

                    if(type != 'x')
                    {
                        double newmt;
                        newmt = mDocument->ConvertToTwips(value, type);

                        if(incdec)
                        {
                            mTopMargin += newmt ;
                       }
                        else
                        {
                            mTopMargin = newmt;
                        }
                        mCurrentPage.topmargin = mTopMargin ;
                                                
                        complete = true;
                    }
                }

                // bottom margin .mb
                else if (command == "MB")
                {
                    double value = mDocument->GetValue(text.substr(3, text.length() - 3), incdec);
                    char type = mDocument->GetType(text.substr(3, text.length() - 3));

                    if(type != 'x')
                    {
                        double newmb;
                        newmb = mDocument->ConvertToTwips(value, type);

                        if(incdec)
                        {
                            mBottomMargin += newmb ;
                        }
                        else
                        {
                            mBottomMargin = newmb;
                        }
                        mCurrentPage.bottommargin = mBottomMargin ; // mPaperHeight - mBottomMargin - mTopMargin ;
                        
                        complete = true;
                    }
                }
                break;

            case 'O':
                // justification  .oj
                if (command == "OJ")
                {
                    str = text.substr(3, text.length() - 3);

                    size_t found = str.find("OFF") ;
                    if (found != string::npos)
                    {
                        mCenterLine = false;
                        mRightLine = false;
                        mLeftLine = true;
                        mJustifyLine = false ;
                        complete = true;
                    }
                    found = str.find("ON") ;
                    if (found != string::npos)
                    {
                        mCenterLine = false;
                        mRightLine = false;
                        mLeftLine = false;
                        mJustifyLine = true ;
                        complete = true;
                    }
                    found = str.find("C") ;
                    if (found != string::npos)
                    {
                        mCenterLine = true;
                        mRightLine = false;
                        mLeftLine = false;
                        mJustifyLine = false ;
                        complete = true;
                    }
                    found = str.find("R") ;
                    if (found != string::npos)
                    {
                        mCenterLine = false;
                        mRightLine = true;
                        mLeftLine = false;
                        mJustifyLine = false ;

                        complete = true;
                    }
                }

                // centering (again) .oc
                else if (command == "OC")
                {
                    str = text.substr(3, text.length() - 3) ;

                    size_t found = str.find("OFF") ;
                    if (found != string::npos)
                    {
                        mCenterLine = false;
                        mRightLine = false;
                        mLeftLine = true;
                        mJustifyLine = false ;
                        complete = true;
                    }
                    else
                    {
                        mCenterLine = true;
                        mRightLine = false;
                        mLeftLine = false;
                        mJustifyLine = false ;
                        complete = true;
                    }
                }
                break;
                
            case 'T' :
                // set tabs .tb
                if(command == "TB")
                {
                    ParseTabs(text.substr(3, text.length() - 3)) ;
                    complete = true ;
                }
                break ;
                
            case 'H' :
                // headers .he .h1
                if(command == "HE" || command == "H1")
                {
                    mHeaderValue = 1 ;
                    complete = true ;
                }
                else if(command == "H2")
                {
                    mHeaderValue = 2 ;
                    complete = true ;
                }
                else if(command == "H3")
                {
                    mHeaderValue = 3 ;
                    complete = true ;
                }
                else if(command == "H4")
                {
                    mHeaderValue = 4 ;
                    complete = true ;
                }
                else if(command == "H5")
                {
                    mHeaderValue = 5 ;
                    complete = true ;
                }
                else if(command == "HM")        // .hm
                {
                    double value = mDocument->GetValue(text.substr(3, text.length() - 3), incdec);
                    char type = mDocument->GetType(text.substr(3, text.length() - 3));

                    if(type != 'x')
                    {
                        double newhm ;
                        newhm = mDocument->ConvertToTwips(value, type);

                        if(incdec)
                        {
                            mHeaderMargin += newhm ;
                        }
                        else
                        {
                            mHeaderMargin = newhm;
                        }
                        mCurrentPage.headermargin = mHeaderMargin ;
                        
                        complete = true;
                    }
                }
                break ;

            case 'F' :
                // footers .fo .f1
                if(command == "FO" || command == "F1")
                {
                    mFooterValue = 1 ;
                    complete = true ;
                }
                else if(command == "F2")
                {
                    mFooterValue = 2 ;
                    complete = true ;
                }
                else if(command == "F3")
                {
                    mFooterValue = 3 ;
                    complete = true ;
                }
                else if(command == "F4")
                {
                    mFooterValue = 4 ;
                    complete = true ;
                }
                else if(command == "F5")
                {
                    mFooterValue = 5 ;
                    complete = true ;
                }
                else if(command == "FM")            // .fm
                {
                    double value = mDocument->GetValue(text.substr(3, text.length() - 3), incdec);
                    char type = mDocument->GetType(text.substr(3, text.length() - 3));

                    double newfm ;
                    newfm = mDocument->ConvertToTwips(value, type);

                    if(incdec)
                    {
                        mFooterMargin += newfm ;
                    }
                    else
                    {
                        mFooterMargin = newfm;
                    }
                    
                    mCurrentPage.footermargin = mFooterMargin ;
                    
                    complete = true;
                }
                break ;
        }
    }
    
    mIsKnownDotCommand = complete ;
    
#ifdef DETAIL_LAYOUT_TIMER
    perfdct += sw.nsecsElapsed() ;
#endif

    return complete ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @param  paragraph   [IN] - the paragraph we are working on
///
/// @return nothing
///
/// @brief Do all dot commands up to the paragraph passed in
///
/////////////////////////////////////////////////////////////////////////////
CHAR_T cLayout::CheckForChanges(POSITION_T bufferpos, string &measure, vector<COORD_T> &positions, QFont &font, string &fontstr, double &fontchgwidth)
{
    eModifiers ch = STYLE_END_OF_STYLES ;
    ch = mDocument->GetControlChar(bufferpos) ; //  + start) ;

    switch(ch)
    {
        case STYLE_BOLD :
        case STYLE_ITALICS :
        case STYLE_UNDERLINE :
        case STYLE_SUBSCRIPT :
        case STYLE_SUPERSCRIPT :
        case STYLE_STRIKETHROUGH :
            // measure the text I already have
            MeasureText(measure, positions, font) ;
            if((mIsCommand == false) && (mIsComment == false))        // only apply attrib if in regular text
            {
                ApplyAttribute(font, ch) ;
            }
            break ;
            
        case STYLE_FONT1 :
            {
                // measure the text I already have
                MeasureText(measure, positions, font) ;

                // we don't check for isCommand or isCOmment here, since font gets applied in MeasureParagraphText
                sInternalFonts internalfont ;
                if(mDocument->GetFont(bufferpos /* + start */, internalfont) == true)
                {
                    QFont font1(internalfont.name.c_str(), static_cast<int>(internalfont.size)) ; // = internalfont.font ;
                    // if we have any attributes active, we need to apply them to the new font
                    // pass in true to clear any attributes first.
                    if(mIsBold)
                    {
                        ApplyAttribute(font1, STYLE_BOLD, true) ;
                    }
                    if(mIsItalics)
                    {
                        ApplyAttribute(font1, STYLE_ITALICS, true) ;
                    }
                    if(mIsUnderline)
                    {
                        ApplyAttribute(font1, STYLE_UNDERLINE, true) ;
                    }
                    if(mIsSubscript)
                    {
                        ApplyAttribute(font1, STYLE_SUBSCRIPT, true) ;
                    }
                    if(mIsSuperscript)
                    {
                        ApplyAttribute(font1, STYLE_SUPERSCRIPT, true) ;
                    }
                    if(mIsStrikethrough)
                    {
                        ApplyAttribute(font1, STYLE_STRIKETHROUGH, true) ;
                    }

                    font = font1 ;              // set the return font

                    // get the font change displayed, if we can
                    if(mEditor->GetShowControls() == SHOW_ALL)
                    {
//                        fontstr.sprintf("%s %d>", internalfont.font.family().toUtf8().constData(), internalfont.font.pointSize()) ;
//                        fontstr.sprintf("%s %d>", internalfont.name.c_str(), internalfont.size) ;
                        fontstr = string_sprintf("%s %.1f>", internalfont.name.c_str(), internalfont.size) ;
                        QFontMetricsF metrics(font) ;

                        MY_ASSERT(fontstr.length() != 0);

                        fontchgwidth = metrics.horizontalAdvance(QString::fromStdString(fontstr)) ;
                    }
                }
            }
            break ;
            
        case STYLE_TAB :  // tabs need to be computed during the wordwrap phase, since we don't know where we are until then.
            {
                sWSTab tab = mDocument->GetTab(bufferpos /* + start */) ;
                if(tab.type == TAB_CENTER)
                {
                    mCenterLine = true ;
                }
                else if(tab.type == TAB_RIGHT)
                {
                    mRightLine = true ;
                }
                
/*                     ch = ' ' ;      // so cursor shows on CR
                    inverted = false ;

//                    auto it = mEditor->mLayouts[mEditor->mUseLayout]->mTabType.find(loop) ;
                    TabPair comp ;
                    comp.first = loop ;
                    auto it = lower_bound(mEditor->mLayouts[mEditor->mUseLayout]->mTabType.begin(), mEditor->mLayouts[mEditor->mUseLayout]->mTabType.end(), comp, TabCompare) ;

                    if(it != mEditor->mLayouts[mEditor->mUseLayout]->mTabType.end())
                    {
                        tab = (*it).second ;
                        if(mEditor->mShowControl == true)
                        {
                            if(tab.type == static_cast<char>(tabtab))
                            {
                                ch = '>' ;
                                inverted = true ;
                            }
                            else
                            {
                                ch = tab.type ;
                                inverted = true ;
                            }
                        }
                    }
*/
            }
            break ;

            
        case STYLE_INTERNAL_COLOR :
            {
                // measure the text I already have
                MeasureText(measure, positions, font) ;
                
                if((mIsCommand == false) && (mIsComment == false))        // only apply attrib if in regular text
                {

                    sWSColor color ;
                    mDocument->GetColor(bufferpos /* + start*/, color) ;
                    QColor color1(gBaseWSColors[color.colornumber].red, gBaseWSColors[color.colornumber].green, gBaseWSColors[color.colornumber].blue) ;

                    mCurrentColor.setRgb(gBaseWSColors[color.colornumber].red, gBaseWSColors[color.colornumber].green, gBaseWSColors[color.colornumber].blue, gBaseWSColors[color.colornumber].alpha) ;
                    // get the color change displayed, if we can
                    if(mEditor->GetShowControls() == SHOW_ALL)
                    {
                        fontstr = string_sprintf("%s >", "color change") ;
                        QFontMetricsF metrics(font) ;
                        fontchgwidth = metrics.horizontalAdvance(QString::fromStdString(fontstr)) ;
                    }
                }
            }
            break ;
            
        default :
            break ;
    }

    return ch ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param   paragraph      [in] - paragraph we are working on
///
/// @return  nothing
///
/// @brief
/// Sets default value for the paragraph we are working on
/////////////////////////////////////////////////////////////////////////////
void cLayout::SetupParagraph(PARAGRAPH_T paragraph)
{

    if(paragraph == 0)
    {
        // reset to defaults
        mIsBold = false ;
        mIsUnderline = false ;
        mIsItalics = false ;
        mIsStrikethrough = false ;
        mIsSuperscript = false ;
        mIsSubscript = false ;
        mIsStrikethrough = false ;

        mRightLine = false ;
        mCenterLine = false ;
        mJustifyLine = false ;
        mLeftLine = true ;

        mParagraphMargin = 0.0 ;          // first line in(out) dent

        mHeaderMargin = TWIPSPERINCH * .33 ;           // .33"
        mFooterMargin = TWIPSPERINCH * .33 ;           // .33"

        mCurrentColor = QColor(0, 0, 0, 255) ;

        mCurrentFont = QFont("Courier", 10) ;

        mPageNumber = 1 ;
        mLineNumber = 1 ;
        mCumulativeLineHeight = 0.0 ;

        // at start, assume an 8.5 x 11 inch page with 1 inch margins
        mLineSpace = 1.0 ;
        mPageOffsetEven = 1 * TWIPSPERINCH ;        // 1" page offsetis
        mPageOffsetOdd = 1 * TWIPSPERINCH ;         // 1" page offset
        mLeftMargin = 0 ;                           // 0" left margin
        mRightMargin = 6.5 * TWIPSPERINCH ;         // 6.5" line length
        mTopMargin = 1 * TWIPSPERINCH ;             // 1" top margin
        mBottomMargin = 1 * TWIPSPERINCH ;          // 1" bottom margin
        mPaperWidth = 8.5 * TWIPSPERINCH ;          // initial width of paper
        mPaperHeight = 11 * TWIPSPERINCH ;          // initial paper height
        mValidParagraphMargin = false ;     // no paragraph margin set yet

    }
    else
    {
        mIsBold = mParagraphLayout[paragraph - 1].modifiers.bold ;
        mIsUnderline = mParagraphLayout[paragraph - 1].modifiers.underline ;
        mIsItalics = mParagraphLayout[paragraph - 1].modifiers.italics ;
        mIsStrikethrough = mParagraphLayout[paragraph - 1].modifiers.strikethrough ;
        mIsSuperscript = mParagraphLayout[paragraph - 1].modifiers.superscript ;
        mIsSubscript = mParagraphLayout[paragraph - 1].modifiers.subscript ;
        mRightLine = mParagraphLayout[paragraph - 1].modifiers.right ;
        mCenterLine = mParagraphLayout[paragraph - 1].modifiers.center ;
        mJustifyLine = mParagraphLayout[paragraph - 1].modifiers.justify ;
        mLeftLine = mParagraphLayout[paragraph - 1].modifiers.left ;
        mLineSpace = mParagraphLayout[paragraph - 1].modifiers.linespace ;

//        mRightMargin = mParagraphLayout[paragraph - 1].modifiers.rightmargin ;
//        mLeftMargin = mParagraphLayout[paragraph - 1].modifiers.leftmargin ;
//        mParagraphMargin = mParagraphLayout[paragraph - 1].modifiers.paragraphmargin ;

        mCurrentFont = mParagraphLayout[paragraph - 1].modifiers.font ;
        mCurrentColor = mParagraphLayout[paragraph - 1].modifiers.textcolor ;

        mPageNumber = mParagraphLayout[paragraph - 1].lines[0].pagenumber ;
        mLineNumber = mParagraphLayout[paragraph - 1].lines[mParagraphLayout[paragraph - 1].lines.size() - 1].linenumber; //  + 1;
        mCumulativeLineHeight = mParagraphLayout[paragraph - 1].lines[mParagraphLayout[paragraph - 1].lines.size() - 1].cumalativeheight ;

        if(mParagraphLayout[paragraph - 1].isCommand == false && mParagraphLayout[paragraph - 1].isComment == false)
        {
            mLineNumber++ ;
            mCumulativeLineHeight += mParagraphLayout[paragraph - 1].lines[mParagraphLayout[paragraph - 1].lines.size() - 1].lineheight ;
        }

    }

    // at start, assume an 8.5 x 11 inch page with 1 inch margins
    mLineSpace = 1.0 ;
    mPageOffsetEven = 1 * TWIPSPERINCH ;        // 1" page offsetis
    mPageOffsetOdd = 1 * TWIPSPERINCH ;         // 1" page offset
    mLeftMargin = 0 ;                           // 0" left margin
    mRightMargin = 6.5 * TWIPSPERINCH ;         // 6.5" line length
    mTopMargin = 1 * TWIPSPERINCH ;             // 1" top margin
    mBottomMargin = 1 * TWIPSPERINCH ;          // 1" bottom margin
    mPaperWidth = 8.5 * TWIPSPERINCH ;          // initial width of paper
    mPaperHeight = 11 * TWIPSPERINCH ;          // initial paper height
    mValidParagraphMargin = false ;     // no paragraph margin set yet

    mCurrentPage.paperwidth = mPaperWidth ;
    mCurrentPage.paperheight = mPaperHeight ;
    mCurrentPage.papertype = QPageSize::Letter ;
    mCurrentPage.set = false ;                  // made true by the first .pt command on a page (commented code so is per document)
    mCurrentPage.topmargin = mTopMargin ;
    mCurrentPage.bottommargin = mBottomMargin ; // mPaperHeight - mBottomMargin - mTopMargin ;
    mCurrentPage.leftmargin = mLeftMargin + mPageOffsetOdd ;
    mCurrentPage.rightmargin = mPaperWidth - mRightMargin - (mLeftMargin + mPageOffsetOdd) ;
    mCurrentPage.headermargin = mHeaderMargin ;
    mCurrentPage.footermargin = mFooterMargin ;

    if(paragraph == 1)
    {
        mHeaders.clear() ;
        mFooters.clear() ;
        mHeadersEven.clear() ;
        mFootersEven.clear() ;
        mHeadersOdd.clear() ;
        mFootersOdd.clear() ;

        for(unsigned long loop = 0; loop < MAX_HEADER_FOOTER; loop++)
        {
            mStoreHeader[loop].pagenumber = 0 ;
            mStoreFooter[loop].pagenumber = 0 ;
            mStoreHeaderEven[loop].pagenumber = 0 ;
            mStoreFooterEven[loop].pagenumber = 0 ;
            mStoreHeaderOdd[loop].pagenumber = 0 ;
            mStoreFooterOdd[loop].pagenumber = 0 ;
        }
    }

    mSpaceAfter = 0.0 ;
    mSpaceBefore = 0.0 ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @return  nothing
///
/// @brief
/// Sets default values for the page we are working on
/////////////////////////////////////////////////////////////////////////////
void cLayout::SetupPage(void)
{
    mCurrentPage.paperwidth = mPaperWidth ;
    mCurrentPage.paperheight = mPaperHeight ;
    mCurrentPage.papertype = QPageSize::Letter ;
    mCurrentPage.set = false ;                  // made true by the first .pt command on a page (commented code so is per document)
    mCurrentPage.topmargin = mTopMargin ;
    mCurrentPage.bottommargin = mBottomMargin ; // mPaperHeight - mBottomMargin - mTopMargin ;
    mCurrentPage.leftmargin = mLeftMargin + mPageOffsetOdd ;
    mCurrentPage.rightmargin = mPaperWidth - mRightMargin - (mLeftMargin + mPageOffsetOdd) ;
    mCurrentPage.headermargin = mHeaderMargin ;
    mCurrentPage.footermargin = mFooterMargin ;

    if(mEditor->GetIsHelp())
    {
        mPageOffsetEven = 0 ;
        mPageOffsetOdd = 0 ;
    }
}



void cLayout::CheckCommentorCommand(string &paragraphtext, vector<POSITION_T> &paragraphoffsets)
{
    // get a single grapheme from the paragraph
    string ch = GetGrapheme(mParagraphStart, paragraphtext, paragraphoffsets) ;

    mIsCommand = false;
    mIsComment = false;
    mIsSpecialComment = false ;
    mIsSpecialComment = 'x' ;
    if(ch[0] == '.' && ch.length() == 1)
    {
        mIsCommand = true ;
        mIsComment = false ;
        if(mParagraphEnd - mParagraphStart > 1)
        {
            POSITION_T next = mParagraphStart + 1 ;
            string checkchar = GetGrapheme(next, paragraphtext, paragraphoffsets) ;
//            string checkchar = mDocument->GetChar(next) ;
            if(checkchar[0] == '.' && checkchar.length() == 1)
            {
                mIsCommand = false ;
                mIsComment = true ;
                mIsKnownDotCommand = true ;
            }
            else if(checkchar[0] >= '0' && checkchar[0] <= '9' && checkchar.length() == 1)
            {
                mIsCommand = false ;
                mIsComment = true ;
                mIsKnownDotCommand = true ;
                char tchar = checkchar[0] ; // .toLatin1() ;
                mIsSpecialComment = tchar - '0' ;
            }
            else                        // support previous .ig command as well as ..
            {
                POSITION_T nextnext = next + 1 ;
                string checkg = GetGrapheme(nextnext, paragraphtext, paragraphoffsets) ;
                if((checkchar == "i" ||  checkchar == "I")
                    && (checkg == "g" ||  checkg == "G"))
                {
                    mIsCommand = false ;
                    mIsComment = true ;
                    mIsKnownDotCommand = true ;
                }
            }
        }
    }

}



/////////////////////////////////////////////////////////////////////////////
///
/// @param   measure        [in] - the string we need to measure
/// @param   positions      [in/out] - The existing positions array
/// @param   font           [in/out] - the current font (for display lists)
///
/// @return  nothing
///
/// @brief
/// measures the string passed in and appends their measurements to
/// the end of the positions array.  String should be all of one font
/// type and style.
///
/// This appends to the temporary segment list mTempSegmentList
/////////////////////////////////////////////////////////////////////////////
void cLayout::MeasureText(string &tomeasure, vector<COORD_T> &positions, QFont &font)
{
    if(tomeasure.length()== 0)
    {
        return ;
    }

#ifdef DETAIL_LAYOUT_TIMER
    QElapsedTimer sw;

    sw.start() ;
#endif

    QFont commfont ;    // this is a saved front if we are in a comment or command
    commfont = font ;

    /// @todo change this to use the cached version of offsets mDocument::GetParagraphGraphemeOffsets()
    vector<POSITION_T> moffsets ;
    POSITION_T mlen = mDocument->GraphemeCount(tomeasure, moffsets) ;
//    POSITION_T mlen = mDocument->GetParagraphGraphemeOffsets(mCurrentParagraph, moffsets) ;

    // if this is a command or comment, we will do all measurements off the basefont, and comfont will hold the proper font data
    if((mIsCommand == true || mIsComment == true) && mForPrint == false)
    {
        font = mEditor->GetFont() ;     // apply system font
    }

    // if this is not a help windows, we scale the font to window size
    QFont scaledfont = font;
    QFont scaledcommfont = commfont ;

    if (mEditor->GetIsHelp() == false)
    {
        scaledfont.setPointSizeF(font.pointSizeF() * mEditor->mFontScale);
        scaledcommfont.setPointSize(commfont.pointSizeF() * mEditor->mFontScale) ;
    }

    vector<COORD_T> tpositions ;

    sSegmentLayout layoutlist ;
    layoutlist.paragraph = mCurrentParagraph ;
    layoutlist.font = font ;
    layoutlist.tfont = scaledcommfont ;
    layoutlist.textcolor = mCurrentColor ;
    layoutlist.segmentheight = 0 ;


    QString ch;

    vector<double> ext ;
    QFontMetricsF metrics(scaledfont) ;

    // make sure we measure char width of markers and not weird width
    for(POSITION_T loop = 0; loop < mlen; loop++)
    {
        double fontoffset = 0 ;     // for leaving space so display code can add font text

        string grapheme = GetGrapheme(loop, tomeasure, moffsets) ;
        QString x = QString::fromUtf8(grapheme.c_str()) ;

        if(grapheme[0] == STYLE_FONT1 && grapheme.length() == 1)
        {
            POSITION_T bufferpos, toss ;
            mDocument->GetParagraphStartandEnd(mCurrentParagraph, bufferpos, toss) ;
            bufferpos += loop ;
            sInternalFonts internalfont ;
            if(mDocument->GetFont(bufferpos, internalfont) == true)
            {
                string fontstring ;
//                    fontstring.sprintf("%s %d>", internalfont.name.c_str(), internalfont.size) ;
                fontstring = string_sprintf("<%s %.1f>", internalfont.name.c_str(), internalfont.size) ;
#ifdef DETAIL_LAYOUT_TIMER
                sw.start();
#endif

//                    fontoffset += metrics.horizontalAdvance(QString::fromStdString(fontstring)) ;
                fontoffset = metrics.horizontalAdvance(QString::fromStdString(fontstring)) ;
#ifdef DETAIL_LAYOUT_TIMER
                percall += sw.nsecsElapsed();
                measurecount++ ;
#endif

                // save space for text

//                    tomeasure.insert(loop + 1, fontstring) ;
//                    mlen = mDocument->GraphemeCount(tomeasure, moffsets) ;
            }
        }

        if(grapheme[0] < STYLE_END_OF_STYLES && grapheme.length() == 1)
        {
            char temp = grapheme[0] ;
            temp += '@' ;
            x = temp ; //+= charoffset ;
        }


#ifdef DETAIL_LAYOUT_TIMER
        sw.start();
#endif
        COORD_T ha ;
        if(fontoffset != 0.0)
        {
            ha = fontoffset ;
        }
        else
        {
            ha = metrics.horizontalAdvance(x) ;
        }
#ifdef DETAIL_LAYOUT_TIMER
        percall += sw.nsecsElapsed();
        measurecount++ ;
#endif
        if(loop == 0)
        {
            ext.push_back(ha) ;
        }
        else
        {
            ext.push_back(ha + ext[loop - 1]) ;
        }
    }

    double width ;
    double offset = 0 ;
    if(!positions.empty())
    {
        offset = positions[positions.size() - 1] ;
    }

    for(POSITION_T loop = 0; loop < ext.size(); loop++)
    {
        tpositions.push_back(ext[loop]) ; // / mEditor->mFontScale) ;

        if(loop == 0)
        {
            width = tpositions[loop] ;
        }
        else
        {
            width = tpositions[loop] - tpositions[loop -1] ;
        }

        string grapheme = GetGrapheme(loop, tomeasure, moffsets) ;
        layoutlist.glyph.push_back(grapheme) ;
        layoutlist.position.push_back(ext[loop] + offset) ;
        if(mInMarkedBlock == true)
        {
            layoutlist.isBlock = true ;
        }
        else
        {
            layoutlist.isBlock = false ;
        }

        if(mInMarkedSearchBlock == true)
        {
            layoutlist.isSearch = true ;
        }
        else
        {
            layoutlist.isSearch = false ;
        }
    }

    for(double tposition : tpositions)
    {
        positions.push_back(tposition + offset) ;
    }

    mParagraphStart += mlen ;
    tomeasure.clear() ;                   // empty the string for the new one

    layoutlist.segmentheight = metrics.height() ;
//    layoutlist.font = font ;
//    if(mIsComment == false && mIsCommand == false)
//    {
//        layoutlist.tfont = font ;
//        layoutlist.tfont.setPointSizeF(font.pointSizeF() * mEditor->GetFontScale()) ;
//    }

    MY_ASSERT(layoutlist.glyph.size() == layoutlist.position.size())
    if(layoutlist.glyph.size() != 0)
    {
        mTempSegmentList.push_back(layoutlist) ;
    }
    mDrawx += mLastGlyphWidth ;

    MY_ASSERT(mlen == ext.size()) ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param   font               [in/out] - the font we are modifying
/// @param   attribute          [in] - the character we are triggering on
/// @param   assumeset          [in] - if true, ignore current status and just set the field (default false)
///
/// @return  CHAR_T         the charcater that replaces the atribute for display
///
/// @brief
/// Apply the font attribut modifier to the font passed in.
///
/// Wordstar use attributes as toggles, rather than having separate on/off
/// versions of it's attributes.  So, if bold is turned on, all bolds below
/// that will toggle their functions... what was bold will now be normal.
///
/////////////////////////////////////////////////////////////////////////////
CHAR_T cLayout::ApplyAttribute(QFont &font, eModifiers attribute, bool assumeset)
{
    static QFont oldfont ;         // used for super and sub script
    CHAR_T ch = 0 ;

    // if we assume the attribute is already set, we'll pre-set it to false, so the
    // switch below can do it job properly
    if(assumeset)
    {
        switch(attribute)
        {
            case STYLE_BOLD :
                mIsBold = false ;
                ch = 'B' ;
                break ;

            case STYLE_ITALICS :
                mIsItalics = false ;
                ch = 'Y' ;
                break ;

            case STYLE_UNDERLINE :
                mIsUnderline = false ;
                ch = 'S' ;
                break ;

            case STYLE_SUBSCRIPT :
                mIsSubscript = false ;
                ch = 'V' ;
                break ;

            case STYLE_SUPERSCRIPT :
                mIsSuperscript = false ;
                ch = 'T' ;
                break ;

            case STYLE_STRIKETHROUGH :
                mIsStrikethrough = false ;
                ch = 'X' ;
                break ;

            default :
                break ;
        }
    }

    // handle the attribute
    switch(attribute)
    {
    case STYLE_BOLD :
        if(mIsBold == false)
        {
            mIsBold = true ;
            font.setBold(true) ;
        }
        else
        {
            mIsBold = false ;
            font.setBold(false) ;
        }
        ch = 'B' ;
        break ;

    case STYLE_ITALICS :
         if(mIsItalics == false)
        {
            mIsItalics = true ;
            font.setItalic(true);
        }
        else
        {
            mIsItalics = false ;
            font.setItalic(false);
        }
        ch = 'Y' ;
        break ;

    case STYLE_UNDERLINE :
        if(mIsUnderline == false)
        {
            mIsUnderline = true ;
            font.setUnderline(true);
        }
        else
        {
            mIsUnderline = false ;
            font.setUnderline(false) ;
        }
        ch = 'S' ;
        break ;

    case STYLE_SUBSCRIPT :
        if(mIsSubscript == false)
        {
            mIsSubscript = true ;
            oldfont = font ;
            double x = font.pointSizeF() ;
            x *= 0.583 ;
            font.setPointSizeF(x) ;
        }
        else
        {
            mIsSubscript = false ;
            font = oldfont ;
        }
        ch = 'V' ;
        break ;

    case STYLE_SUPERSCRIPT :
        if(mIsSuperscript == false)
        {
            mIsSuperscript = true ;
            oldfont = font ;
            double x = font.pointSizeF() ;
            x *= 0.583 ;
            font.setPointSizeF(x) ;
        }
        else
        {
            mIsSuperscript = false ;
            font = oldfont ;
        }
        ch = 'T' ;
        break ;

    case STYLE_STRIKETHROUGH :
        if(mIsStrikethrough == false)
        {
            mIsStrikethrough = true ;
            font.setStrikeOut(true);
        }
        else
        {
            mIsStrikethrough = false ;
            font.setStrikeOut(false) ;
        }
        ch = 'X' ;
        break ;

    default :
        break ;
    }

//    font.QFontBase::SetStrikethrough(false) ;

    return ch ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param  pageextend  [IN/OUT] - page length extended size due to comments or commands
/// @param  paragraph   [IN] - the paragraph we are working on
///
/// @return nothing
///
/// @brief wordwrap our paragraph at the right spot(s)
///
/// Also takes care of what page the line is on. If this is a comment or
/// command paragraph ('..' or '.') then we extend our page length so the
/// comment or command does not increase page count. Tabs are expanded here.
/////////////////////////////////////////////////////////////////////////////
void cLayout::WordWrapParagraph(COORD_T &pageextend, PARAGRAPH_T paragraph)
{
    COORD_T lastsize = 0 ;                          // the last place we found a spot to wrap
    size_t wrappos = 0 ;                            // the position we can wrap at
    size_t wrapseg = 0 ;                            // the segment we wrap at
    COORD_T wrapsize = 0 ;                          // the size where we wrap at in TWIPS
    bool foundbreak = false ;                       // have we found a wrap point
    COORD_T margintest = mBoxRight ;                // our 'must wrap' point
    COORD_T pagetest = mBoxBottom  + pageextend + mTopMargin ;   // time to start a new page
    sLineLayout finalline ;                         // the final line ready to save
    string ch ;
    COORD_T pushfront = 0 ;                         // stores first offset for segment
    size_t oldseg = 0 ;                             // our last wrap segment
    COORD_T ypos = 0 ;                              // vertical position on page for line (includes margin)
    PAGE_T pagenumber = 1 ;                             // our current page number

    COORD_T tempy = 0 ;                             // our line height

    bool paCommand  = false ;                       // forced page break

    mTabOffset = 0 ;


    // on our first paragraph, make the first line at the top margin and start page number at one
    if(paragraph == 0)
    {
        ypos = mBoxTop + mTopMargin ;
        pagenumber = 1 ;
    }
    // go backwards through the paragraph, looking at the last line, and see if we can use its y position
    else
    {
        ssize_t ploop ;

        for(ploop = paragraph - 1; ploop >= 0; ploop--)
        {

            size_t index = mParagraphLayout[ploop].lines.size() - 1 ;
            if(mParagraphLayout[ploop].lines[index].y != 0.0)
            {
                ypos = mParagraphLayout[ploop].lines[index].y ;
                pagenumber = mParagraphLayout[ploop].lines[index].pagenumber ;
                paCommand =  mParagraphLayout[ploop].pagebreak ;
                if(paCommand)
                {
//                    ypos = mBoxTop + mTopMargin ;
                }
                break ;
            }
        }

        if(ploop < 0)           // no valid ypos
        {
//            ypos = mBoxTop + mTopMargin ;
            pagenumber = 1 ;
        }
    }
    mPageNumber = pagenumber ;

    if(mRightMargin != 0.0)    // .rm  .lm
    {
        // we refuse to have a width of less than 1/2 inch
        if(mRightMargin - mLeftMargin > TWIPSPERINCH / 2)
        {
            if(mBoxRight < mRightMargin)
            {
                margintest = mBoxRight ; // mRightMargin ;
            }
            else
            {
                margintest = mRightMargin ;
            }
        }
    }
    margintest -= mLeftMargin ;

    // if this is a command or comment, it doesn't get vertical adjustment
    if(mIsCommand == false && mIsComment == false)
    {
        ypos += mSpaceBefore ;
    }

    // go through all the segments we have stored for this paragarph
    COORD_T lastcharsize = 0 ;                          // used for tabs, so we tab from our last char position
    size_t numsegments = mTempSegmentList.size() ;
    sSegmentLayout *segment ;

    // handle a .pm
    bool firstline = true ;
    COORD_T firstlineoffset = mParagraphMargin ;    // .pm

    // if we are a comment or command, always left justify
    if(mIsCommand || mIsComment)
    {
        firstlineoffset = 0 ;
        mLeftMargin = 0 ;
    }

    // get our word start positions
    /// @todo if we are not showing markers, then this needs to strip them before finding wordstarts
    vector<POSITION_T> wordstarts ;
    string bob = mDocument->GetParagraphText(paragraph) ;
    size_t numwordbreaks = mDocument->GetWordPositions(paragraph, wordstarts) ;

    // make them relative to this paragraph
    POSITION_T subvalue, notneeded ;
    mDocument->GetParagraphStartandEnd(paragraph, subvalue, notneeded) ;
    for(size_t wsloop = 0; wsloop < wordstarts.size(); wsloop++)
    {
        wordstarts[wsloop] -= subvalue + 1 ;
    }
//    wordstarts.erase(wordstarts.begin()) ;        // erase the 0th position

    POSITION_T paragraphposition = -1 ;
    POSITION_T savedparapos = 0 ;

///////////////////////////////////////////////////////////////////////////////////////////////
    // go through each segment in the paragraph (segments created by font or style changes)
    for(size_t sloop = 0; sloop < numsegments; sloop++)
    {
        segment = &mTempSegmentList[sloop] ;                 // make a working copy

        // just in casse once of these creeps in
        if(segment->position.size() == 0 || segment->glyph.size() == 0)
        {
            printf("empty segment in word wrap\n") ;        // informational only
            continue ;
        }
                
        size_t numglyphs = segment->glyph.size() ;

        // go through the graphemes in segment and see where and when we wordwrap
        for(size_t gloop = 0; gloop < numglyphs; gloop++)
        {
            paragraphposition++ ;
            ch = segment->glyph[gloop] ;
//            mTempSegmentList[sloop].position[gloop] -= wrapsize ;           // make position correct for wrap
            segment->position[gloop] -= wrapsize ;
            
            if(ch[0] == STYLE_TAB && ch.length() == 1)
            {
                COORD_T tempsize = lastcharsize + mLeftMargin ; // segment.position[gloop] * fontscale ;
                COORD_T  tabsize = mEditor->FindNextTabStop(tempsize) ;
                tabsize -= mLeftMargin ;

                mTabOffset = tabsize - segment->position[gloop] ;
            }

            // offset the first line for a .pm command (we'll reuse mTabOffset for this)
            if(mValidParagraphMargin)
            {
                if(firstline && gloop == 0 && sloop == 0)   // only do this calculation once per paragraph
                {
                    COORD_T offset = (firstlineoffset - mLeftMargin);
                    mTabOffset += offset; // firstlineoffset ;     // offset for entire line
                    pushfront = offset ; // firstlineoffset ;       // start of line (first letter) offset
                }
            }

//            mTempSegmentList[sloop].position[gloop] += mTabOffset ;           // make position correct for wrap
            segment->position[gloop] += mTabOffset ;
            
            COORD_T size = (segment->position[gloop]);
            lastcharsize = size ;
/*
            // go through word start vector to see if this position is a wordbreak (utf8 word breaks)
            auto pit = find(wordstarts.begin(), wordstarts.end(), paragraphposition) ;
            if(pit != wordstarts.end())
            {
                if(ch[0] != MARKER_CHAR)
                {
                    lastsize = segment->position[gloop] ;
                    wrappos = gloop ;
                    wrapseg = sloop ;
                    savedparapos = paragraphposition ;
                    foundbreak = true ;
//                continue ;
                }
            }
*/
            // the UTF8 word break algorithm doesn break on all spaces or any tabs, so do that here
            // for example " - - - - - " will not use any of the spaces for a break
            if(ch.length() == 1)
            {
                // if we break on HARD_RETURN, we breal properly, but a blank line gets inserted and cursoring around fails.
                if((ch[0] == ' ') || (ch[0] == STYLE_TAB)) // || (ch[0] == HARD_RETURN))                           // if we have a space or a tab
                {
                    lastsize = segment->position[gloop] ;
                    wrappos = gloop ;
                    wrapseg = sloop ;
                    savedparapos = paragraphposition ;
                    foundbreak = true ;
//                    continue ;
                }
            }


            // do we need to wrap?
            if(mEditor->GetIsHelp() == false)
            {
                if(static_cast<COORD_T>(size) >= margintest - 1)
                {
                    // if this is our first wrap point, readjust for any first line in/outdents
                    if(firstline == true)
                    {
                        firstline = false ;
                    }

                    // force a word wrap
                    if(foundbreak == false)
                    {
                        lastsize = segment->position[gloop] ;
                        wrappos = gloop ;
                        wrapseg = sloop ;
                        savedparapos = paragraphposition ;
                        foundbreak = true ;
                    }

                    // do the wrap
//                    if(foundbreak == true)
                    {
                        // move all segments up to but not including wrapseg into finalline
                        for (size_t iloop = oldseg; iloop < wrapseg; iloop++)
                        {
                            mTempSegmentList[iloop].position.push_front(pushfront);
                            pushfront = mTempSegmentList[iloop].position[mTempSegmentList[iloop].position.size() - 1];
                            finalline.segments.emplace_back(mTempSegmentList[iloop]);
                        }

                        oldseg = wrapseg;

                        // move the data before the wrap point into it's own segment
                        sSegmentLayout tseg;
                        tseg.font = mTempSegmentList[wrapseg].font;
                        tseg.paragraph = mTempSegmentList[wrapseg].paragraph;
                        tseg.segmentheight = mTempSegmentList[wrapseg].segmentheight;
                        tseg.isBlock = mTempSegmentList[wrapseg].isBlock;
                        tseg.isSearch = mTempSegmentList[wrapseg].isSearch;

                        // if we didn't find a wrap position, then just cut the line here
                        if(wrapseg == 0 && wrappos == 0)
                        {
                            wrapseg = sloop ;
                            wrappos = gloop ;
                        }

                        for (size_t tsloop = 0; tsloop <= wrappos; tsloop++)
                        {
                    //        MY_ASSERT(mTempSegmentList[wrapseg].glyph.size() == mTempSegmentList[wrapseg].position.size())
                            tseg.position.emplace_back(mTempSegmentList[wrapseg].position[0]);   // position index is 0 because of the pop_front below
                            tseg.glyph.emplace_back((mTempSegmentList[wrapseg].glyph[0]));       // ditto

                                                                                                 // now remove the copied data from the current segment
                            mTempSegmentList[wrapseg].position.pop_front();
                            mTempSegmentList[wrapseg].glyph.pop_front();
                        }

                        // correct our current segment if it is the segment after our wrap
                        if (sloop > wrapseg)
                        {
                            // first correct any full segments we have left
                            for (size_t startseg = wrapseg; startseg < sloop; startseg++)
                            {
                                for (size_t correctloop = 0; correctloop < mTempSegmentList[startseg].position.size(); correctloop++)
                                {
                                    mTempSegmentList[startseg].position[correctloop] += wrapsize - mTabOffset;
                                }
                            }

                            // now correct our current segment
                            for (size_t correctloop = 0; correctloop <= gloop; correctloop++)
                            {
                                mTempSegmentList[sloop].position[correctloop] += wrapsize - mTabOffset;
                            }
                        }

                        // correct the position of the wrap point to current index for our wrapped segment
                        else if (wrapseg != sloop)
                        {
                            size_t cnt = gloop - wrappos;
                            if (cnt > mTempSegmentList[wrapseg].position.size())
                            {
                                cnt = mTempSegmentList[wrapseg].position.size();
                            }
                            for (size_t correctloop = wrappos; correctloop <= cnt; correctloop++)
                            {
                                mTempSegmentList[wrapseg].position[correctloop] += wrapsize - mTabOffset;
                            }
                        }

                        // correct the position of the wrap point to current index for our current segment
                        else
                        {
                            for (size_t correctloop = 0; correctloop < gloop - wrappos; correctloop++)
                            {
                                mTempSegmentList[sloop].position[correctloop] += wrapsize - mTabOffset;
                            }
                        }

                        // now that we are done with wrapsize, increment it for the next loop
                        wrapsize += lastsize;

                        // now save the segment in our final line and calculate x and y for the line
                        if (tseg.glyph.size() != 0)
                        {
                            finalline.segments.emplace_back(tseg);
                            /*
                            if(pagenumber % 2 == 0)
                            {
                            finalline.x =  mPageOffsetEven + mBoxLeft + mLeftMargin ;      // .lm .po .poe
                            }
                            else
                            {
                            finalline.x =  mPageOffsetOdd + mBoxLeft + mLeftMargin ;      // .lm .po .poo
                            }
                            */
                            finalline.x = mBoxLeft + mLeftMargin;                          // .lm
                            finalline.y = ypos;
                            finalline.segments[finalline.segments.size() - 1].position.push_front(pushfront);
                            pushfront = mTempSegmentList[sloop].position[mTempSegmentList[sloop].position.size() - 1];
                        }

                        for (auto& finalseg : finalline.segments)
                        {
                            if (finalseg.segmentheight > tempy)
                            {
                                tempy = (finalseg.segmentheight * mLineSpace);
                            }
                        }

                        mFinalParaLine = false;
                        bool newpage = SaveLine(paCommand, pagenumber, ypos, tempy, pagetest, finalline);
                        if (newpage)
                        {
                            pageextend = 0;
//                            ypos = mBoxTop + mTopMargin ;
                        }
                        // once we save our first line, we don't care if the previous was page break (and we shouldn't)
                        paCommand = false ;

                        // clear our temp line storage
                        finalline.segments.clear();
                        /*
                        if(pagenumber % 2 == 0)
                        {
                        finalline.x =  mPageOffsetEven + mBoxLeft + mLeftMargin ;      // .lm .po .poe
                        }
                        else
                        {
                        finalline.x =  mPageOffsetOdd + mBoxLeft + mLeftMargin ;      // .lm .po .poo
                        }
                        */
                        finalline.x = mBoxLeft + mLeftMargin;                          // .lm
                        finalline.y = ypos;

                        // reset our loops
                        sloop = wrapseg - 1;
                        paragraphposition = savedparapos ;
                        wrappos = 0;
                        wrapseg = 0;
                        pushfront = 0;
                        numsegments = mTempSegmentList.size();
                        // break from grapheme loop
                        break ;
                    }
                }
            }
        }       // end glyph loop

        // if we are done our segments, drain any that haven't been moved into finalline
        if(sloop + 1 == numsegments)
        {
            for(size_t iloop = oldseg; iloop < numsegments; iloop++)
            {
                mTempSegmentList[iloop].position.push_front(pushfront) ;
                pushfront = mTempSegmentList[iloop].position[mTempSegmentList[iloop].position.size() - 1] ;
                mTempSegmentList[iloop].position.pop_back() ;
                finalline.segments.emplace_back(mTempSegmentList[iloop]) ;
            }
            pushfront = 0 ;
        }

    }
/*              
    if(pagenumber % 2 == 0)
    {
        finalline.x =  mPageOffsetEven + mBoxLeft + mLeftMargin ;      // .lm po .poe
    }
    else
    {
        finalline.x =  mPageOffsetOdd + mBoxLeft + mLeftMargin ;      // .lm .po .poo
    }
*/
    finalline.x = mBoxLeft + mLeftMargin ;                          // .lm
    finalline.y = ypos ;
    
    for(auto &finalseg : finalline.segments)
    {
        if(finalseg.segmentheight > tempy)
        {
            tempy = (finalseg.segmentheight * mLineSpace);
        }
    }

    tempy += mSpaceAfter ;

    mFinalParaLine = true ;
    bool newpage = SaveLine(paCommand, pagenumber, ypos, tempy, pagetest, finalline);
    if(newpage)
    {
        pageextend = 0 ;
//        ypos = mBoxTop ; // + mTopMargin ;
    }

    // get rid of any first line adjustments (for one line paragraphs)
    if(firstline == true)
    {
        mLeftMargin -= mParagraphMargin ;
        margintest += mParagraphMargin ;
        firstline = false ;
    }
}



bool cLayout::SaveLine(bool paCommand, PAGE_T &pagenumber, COORD_T &ypos, COORD_T &tempy, COORD_T &pagetest, sLineLayout &finalline)
{
    bool newpage = false ;

    finalline.center = mCenterLine ;
    finalline.justify = mJustifyLine ;
    finalline.left = mLeftLine ;
    finalline.right = mRightLine ;
    finalline.linenumber = mLineNumber ;
    
    // if we are a comment or command, and we are not showing them
    if((mIsCommand || mIsComment) && mEditor->GetShowControls() == SHOW_NONE)
    {
        // if this is a header or footer, save the info for when we need it.
        if(mHeaderValue != 0)
        {
            if(pagenumber % 2 == 0)
            {
                finalline.x += mPageOffsetEven ;
            }
            else
            {
                finalline.x = mPageOffsetOdd ;
            }

            if(mHeaderValue <= MAX_HEADER_FOOTER)           // both pages
            {
                finalline.pagenumber = pagenumber ;
                mStoreHeader[static_cast<size_t>(mHeaderValue - 1)] = finalline ;
            }
            else if(mHeaderValue <= MAX_HEADER_FOOTER * 2)  // even pages
            {
                finalline.pagenumber = pagenumber ;
                mStoreHeaderEven[static_cast<size_t>(mHeaderValue - MAX_HEADER_FOOTER - 1)] = finalline ;
            }
            else
            {
                finalline.pagenumber = pagenumber ;
                mStoreHeaderOdd[static_cast<size_t>(mHeaderValue - (MAX_HEADER_FOOTER * 2) - 1)] = finalline ;
            }
        }
        if(mFooterValue != 0)
        {
            if(mFooterValue <= MAX_HEADER_FOOTER)           // both pages
            {
                finalline.pagenumber = pagenumber ;
                mStoreFooter[static_cast<size_t>(mFooterValue - 1)] = finalline ;
            }
            else if(mFooterValue <= MAX_HEADER_FOOTER * 2)  // even pages
            {
                finalline.pagenumber = pagenumber ;
                mStoreFooterEven[static_cast<size_t>(mFooterValue - MAX_HEADER_FOOTER - 1)] = finalline ;
            }
            else
            {
                finalline.pagenumber = pagenumber ;
                mStoreFooterOdd[static_cast<size_t>(mFooterValue - (MAX_HEADER_FOOTER * 2) - 1)] = finalline ;
            }
        }

        tempy = 0 ;
        mHeaderValue = 0 ;
        mFooterValue = 0 ;
    }

    finalline.lineheight = tempy ;
    ypos += tempy ;
    
    // if the previous paragraph was a .PA command or if our y position says we should be on a new page
    if ((paCommand == true) || (ypos >= pagetest))
    {
        if(mEditor->GetIsHelp() == false)
        {
            // remember our current paper size for this page
            SavePageInfo(pagenumber) ;
        }

        pagenumber++;
        mPageNumber = pagenumber ;
        if(mForPrint == false)
        {
            ypos = mBoxTop - 1  ; // mTopMargin ;  /// @TODO This is a fricken hack. I messed something up somewhere an dneed to fix it.
        }
        else
        {
            ypos = mBoxTop + mTopMargin ;
        }
        pagetest = mBoxBottom;
        newpage = true ;
        mLineNumber = 1 ;
        mCumulativeLineHeight = 0 ;
        
        InsertHeaderFooter(pagenumber, tempy) ;
    }

	finalline.y = ypos  ;
	finalline.pagenumber = pagenumber;
    finalline.linenumber = mLineNumber ;
    finalline.cumalativeheight = mCumulativeLineHeight ;

    // .po .poo .poe
    if(pagenumber % 2 == 0)
    {
        finalline.x += mPageOffsetEven ;
    }
    else
    {
        finalline.x += mPageOffsetOdd ;
    }

    if (!mIsComment && !mIsCommand)
    {
        // justification (center, right, justify
        if (mCenterLine || mRightLine || mJustifyLine)
        {
            JustifyLine(finalline);
        }
        
        if(mIsKnownDotCommand != false)
        {
            mLineNumber++ ;
            mCumulativeLineHeight += finalline.lineheight ;
        }
    }
    
	mTempLineList.emplace_back(finalline);
    
    return newpage ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @param   p          [in] - paragraph number
///
/// @return  bool       true - the paragraph is exactly like the one we have
///                     false - the paragraph is new or different 
///
/// @brief
/// creates a paragraph made up of lines and segments. Comments or commands
/// do not take up any vertical page space
///
/// multiple exit points to get out of nested loops
///
/////////////////////////////////////////////////////////////////////////////
bool cLayout::CreateParagraphs(PARAGRAPH_T p)
{
    bool retval = false ;
//    bool breakit = false ;
    sParagraphLayout paragraph ;
//    p -= 1 ;            // base zero
    
    paragraph.number = mCurrentParagraph ;
    paragraph.lines = mTempLineList ;
    paragraph.pagebreak = mPageBreak ;
    
    mPageBreak = false ;
    
    auto piter = mParagraphLayout.begin() ;
    piter += static_cast<long>(p) ;
    if(piter != mParagraphLayout.end())
    {
        sParagraphLayout *t = &mParagraphLayout[p] ;

        /// @todo check if paragraph layout is the same as what we have
        if(paragraph.number == t->number)        // is the paragraph number the same?
        {
            if(paragraph.lines.size() == t->lines.size())  // are the number of lines the same
            {
                // paragraph data is the same, now check the lines
                size_t len = paragraph.lines.size() ;
                for(size_t loop = 0; loop < len ; loop++)
                {
                    if(paragraph.lines[loop].linenumber == t->lines[loop].linenumber)       // is the line number the same
                    {
                        if(paragraph.lines[loop].x == t->lines[loop].x)  // is the x position the same
                        {
                            if(paragraph.lines[loop].y == t->lines[loop].y)  // is the y position the same
                            {
                                if(paragraph.lines[loop].pagenumber == t->lines[loop].pagenumber )  // is the pagenumber for this line the same
                                {
                                    if(paragraph.lines[loop].segments.size() == t->lines[loop].segments.size())  // are the number of segments the same
                                    {
                                        // line info is the same, so lets check the segments
                                        size_t slen = paragraph.lines[loop].segments.size() ;
                                        for(size_t sloop = 0; sloop < slen; sloop++)
                                        {
                                            if(paragraph.lines[loop].segments[sloop].font == t->lines[loop].segments[sloop].font)  // are the fonts the same
                                            {
                                                if(paragraph.lines[loop].segments[sloop].segmentheight == t->lines[loop].segments[sloop].segmentheight)  // are the lineheights the same
                                                {
                                                    if(paragraph.lines[loop].segments[sloop].position.size() == t->lines[loop].segments[sloop].position.size())
                                                    {
                                                        size_t gloop = 0 ;
                                                        for(gloop = 0; gloop < paragraph.lines[loop].segments[sloop].glyph.size(); gloop++)
                                                        {
                                                            if(paragraph.lines[loop].segments[sloop].position[gloop] != t->lines[loop].segments[sloop].position[gloop])
                                                            {
                                                                mParagraphLayout[p] = paragraph ;
                                                                return false ;
                                                            }
                                                            if(paragraph.lines[loop].segments[sloop].glyph[gloop] != t->lines[loop].segments[sloop].glyph[gloop])
                                                            {
                                                                mParagraphLayout[p] = paragraph ;
                                                                return false ;
                                                            }
                                                            if(paragraph.lines[loop].segments[sloop].isBlock != t->lines[loop].segments[sloop].isBlock)     /// @TODO - move out of loop
                                                            {
                                                                mParagraphLayout[p] = paragraph ;
                                                                return false ;
                                                            }
                                                            if(paragraph.lines[loop].segments[sloop].isSearch != t->lines[loop].segments[sloop].isSearch)     /// @TODO - move out of loop
                                                            {
                                                                mParagraphLayout[p] = paragraph ;
                                                                return false ;
                                                            }
                                                        }
                                                        if(gloop == paragraph.lines[loop].segments[sloop].position.size())
                                                        {
                                                            retval = true ;
                                                        }
                                                    }
                                                    else
                                                    {
                                                        mParagraphLayout[p] = paragraph ;
                                                        return false ;
                                                    }
                                                }
                                                else
                                                {
                                                    mParagraphLayout[p] = paragraph ;
                                                    return false ;
                                                }
                                            }
                                            else
                                            {
                                                mParagraphLayout[p] = paragraph ;
                                                return false ;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        mParagraphLayout[p] = paragraph ;
                                        return false ;
                                    }
                                }
                                else
                                {
                                    mParagraphLayout[p] = paragraph ;
                                    return false ;
                                }
                            }
                            else
                            {
                                mParagraphLayout[p] = paragraph ;
                                return false ;
                            }
                        }
                        else
                        {
                            mParagraphLayout[p] = paragraph ;
                            return false ;
                        }
                    }
                    else
                    {
                        mParagraphLayout[p] = paragraph ;
                        return false ;
                    }
                }
            }
        }

        // if they are different, replace this paragraph
        if(retval == false)     
        {
           mParagraphLayout[p] = paragraph ;
       }
    }
    else
    {
        mParagraphLayout.push_back(paragraph) ;
    }

    return retval ;
}



void cLayout::IsCommentorCommand(sSegmentLayout &segment, bool &comment, bool &command)
{
    command = false ;
    comment = false ;

    if(!segment.glyph.empty())       // the segment has to have data (sanity check)
    {
        if(segment.glyph[0][0] == '.')
        {
            command = true ;
            
            if(segment.glyph.size() > 1)
            {
                if(segment.glyph[0][1] == '.')
                {
                    comment = true ;
                    command = false ;
                }
            }
        }
    }
}


void cLayout::JustifyLine(sLineLayout &line)
{    
    COORD_T drawwidth = mBoxRight;
    COORD_T justifywidth = 0.0 ;

    size_t segments = line.segments.size() ;
    if(segments != 0)
    {
        if(!line.justify)
        {
            // this is pretty straight forward, modify coordinates to right justify or center line.
            size_t positions = line.segments[segments - 1].position.size() ;

            if(positions != 0)
            {
                COORD_T linewidth = line.segments[segments - 1].position[positions - 1] ;

                if(line.center)
                {
                    justifywidth = drawwidth - linewidth ;
                    justifywidth /= 2 ;
                }
                if(line.right)
                {
                    justifywidth = drawwidth - linewidth ;
                }

                for(size_t sloop = 0; sloop < segments; sloop++)
                {
                    for (COORD_T & gloop : line.segments[sloop].position)
                    {
                        gloop += justifywidth ;
                    }
                }
            }
        }
        else
        {
            // this is a bit of work. We add single twip offsets to separtor glyphs until the start position
            // of the last glyph plus its width is our draw page width. Brute force.
            bool done = false ;
            justifywidth = 1.0 ;

            if(mFinalParaLine)
            {
                return ;
            }

            // get the COORD_T of the last printable character (this includes last printable charcater)
            COORD_T offset = GetLastNonBreakOffset(line) ;
            offset = (drawwidth - offset) ; /// mEditor->mFontScale ; // number of twips I need to add

            int count = 0 ;
            int ioffset = static_cast<int>(offset) ;
            for(int loop = 0; loop < ioffset ; loop++)
            {
                // go through our segments
                for(size_t sloop = 0; sloop < segments; sloop++)
                {
                    size_t end = line.segments[sloop].glyph.size() ;

                    for (size_t gloop = 0; gloop < end ; gloop++)
                    {
                        if(line.segments[sloop].glyph[gloop] == " ")
                        {
                            count++ ;
                            size_t startglyph = gloop ;
                            for(size_t segloop = sloop; segloop < segments; segloop++)
                            {
                                size_t gsize = line.segments[segloop].glyph.size() ;
                                for (size_t glyphloop = startglyph; glyphloop < gsize ; glyphloop++)
                                {
                                    line.segments[segloop].position[glyphloop]++ ;
                                }
                                startglyph = 0 ;
                            }

                            if(count == ioffset)
                            {
                                loop = ioffset ;
                                break ;
                            }
                        }
                    }
                }
                done = true ;
            }
        }
    }
}




COORD_T cLayout::GetLastNonBreakOffset(sLineLayout &line)
{
    size_t segments = line.segments.size() ;
    size_t pos = line.segments[segments -1].glyph.size() ;
    COORD_T offset = line.segments[segments -1].position[pos - 1] ;

    if(pos != 1)        // don't bother if this is a one character line
    {
        for(size_t loop = pos - 1; loop >= 0; loop--)
        {
            if(line.segments[segments -1].glyph[loop] != " ")
            {
                if(loop != pos - 1)
                {
                    offset = line.segments[segments -1].position[loop + 1] ;
                }
                return offset ;
            }
        }
    }
    return offset ;
}




size_t cLayout::GetLastNonBreakPosition(sLineLayout &line)
{
    size_t segments = line.segments.size() ;
    size_t pos = line.segments[segments -1].glyph.size() ;

    for(size_t loop = pos - 1; loop >= 0; loop--)
    {
        if(line.segments[segments -1].glyph[loop] != " ")
        {
            return pos + 1 ;
        }
    }
    return pos ;
}




void cLayout::ParseTabs(string nums)
{
    vector<COORD_T> tabs ;
    bool incdec ;

    tabs.push_back(mLeftMargin) ;             // Wordstar sets a tab stop at the left margin

    do
    {
        string rest, item ;
        size_t spot = nums.find(",") ;
        if(spot == string::npos)
        {
            item = nums ;
            nums.clear();
            spot = 0 ;
        }
        else
        {
            item = nums.substr(0, spot) ;
//            item = nums.left(spot) ;
        }

        if(nums.length() != 0)
        {
            rest = nums.substr(nums.length() - spot - 1, nums.length()) ;
            nums = rest ;
        }

        double value = mDocument->GetValue(item, incdec);
        char type = mDocument->GetType(item);

        double newtab;
        newtab = mDocument->ConvertToTwips(value, type);
        tabs.push_back(newtab) ;
    } while(nums.length() != 0) ;
    
    
    mEditor->SetTabs(tabs) ;
}



void cLayout::InsertHeaderFooter(PAGE_T pagenumber, COORD_T &tempy)
{
    for(size_t loop = 0; loop < MAX_HEADER_FOOTER; loop++)
    {
        if(mStoreHeader[loop].pagenumber != 0)
        {
            mStoreHeader[loop].lineheight = tempy ;
            mStoreHeader[loop].pagenumber = pagenumber ;

            sLineLayout line = mStoreHeader[loop] ;             // make a copy, since we modify it
            line.x += mBoxLeft ;
            
            HeaderFooterReplace(line, pagenumber) ;
            
            if (line.center || line.right || line.justify)
            {
                JustifyLine(line);
            }
            sHeaderFooter header ;
            header.headernumber = static_cast<int>(loop + 1) ;
            header.page = pagenumber ;
            header.text = line ;
            mHeaders.emplace_back(header) ;
        }
        
        if(mStoreHeaderEven[loop].pagenumber != 0)
        {
            mStoreHeaderEven[loop].lineheight = tempy ;
            mStoreHeaderEven[loop].pagenumber = pagenumber ;

            sLineLayout line = mStoreHeaderEven[loop] ;             // make a copy, since we modify it
            line.x += mBoxLeft ;
            
            HeaderFooterReplace(line, pagenumber) ;
            
            if (line.center || line.right || line.justify)
            {
                JustifyLine(line);
            }
            sHeaderFooter header ;
            header.headernumber = static_cast<int>(loop + 1) ;
            header.page = pagenumber ;
            header.text = line ;
            mHeadersEven.emplace_back(header) ;
        }
        
        if(mStoreHeaderOdd[loop].pagenumber != 0)
        {
            mStoreHeaderOdd[loop].lineheight = tempy ;
            mStoreHeaderOdd[loop].pagenumber = pagenumber ;

            sLineLayout line = mStoreHeaderOdd[loop] ;             // make a copy, since we modify it
            
            HeaderFooterReplace(line, pagenumber) ;
            
            if (line.center || line.right || line.justify)
            {
                JustifyLine(line);
            }
            sHeaderFooter header ;
            header.headernumber = static_cast<int>(loop + 1) ;
            header.page = pagenumber ;
            header.text = line ;
            mHeadersOdd.emplace_back(header) ;
        }
        
        if(mStoreFooter[loop].pagenumber != 0)
        {
            mStoreFooter[loop].lineheight = tempy ;
            mStoreFooter[loop].pagenumber = pagenumber ;
            
            sLineLayout line = mStoreFooter[loop] ;             // make a copy, since we modify it
            
            HeaderFooterReplace(line, pagenumber) ;

            if (line.center || line.right || line.justify)
            {
                JustifyLine(line);
            }
            sHeaderFooter footer ;
            footer.headernumber = static_cast<int>(loop + 1) ;
            footer.page = pagenumber ;
            footer.text = line ;
            mFooters.emplace_back(footer) ;
        }
        
        if(mStoreFooterEven[loop].pagenumber != 0)
        {
            mStoreFooterEven[loop].lineheight = tempy ;
            mStoreFooterEven[loop].pagenumber = pagenumber ;
            
            sLineLayout line = mStoreFooterEven[loop] ;             // make a copy, since we modify it
            
            HeaderFooterReplace(line, pagenumber) ;

            if (line.center || line.right || line.justify)
            {
                JustifyLine(line);
            }
            sHeaderFooter footer ;
            footer.headernumber = static_cast<int>(loop + 1) ;
            footer.page = pagenumber ;
            footer.text = line ;
            mFootersEven.emplace_back(footer) ;
        }
        
        if(mStoreFooterOdd[loop].pagenumber != 0)
        {
            mStoreFooterOdd[loop].lineheight = tempy ;
            mStoreFooterOdd[loop].pagenumber = pagenumber ;
            
            sLineLayout line = mStoreFooterOdd[loop] ;             // make a copy, since we modify it
            
            HeaderFooterReplace(line, pagenumber) ;

            if (line.center || line.right || line.justify)
            {
                JustifyLine(line);
            }
            sHeaderFooter footer ;
            footer.headernumber = static_cast<int>(loop + 1) ;
            footer.page = pagenumber ;
            footer.text = line ;
            mFootersOdd.emplace_back(footer) ;
        }
    }
}



void cLayout::HeaderFooterReplace(sLineLayout &line, PAGE_T &pagenumber)
{
    for(size_t sloop = 0; sloop < line.segments.size(); sloop++)
    {
        for(size_t cloop = 0; cloop < line.segments[sloop].glyph.size(); cloop++)
        {
            if(line.segments[sloop].glyph[cloop] == "#")              // replace pagenumber
            {
                string text ;
                QFont font = line.segments[sloop].font ;
                font.setPointSizeF(font.pointSizeF() * mEditor->mFontScale) ;
                sSegmentLayout &segment = line.segments[sloop] ;
                QFontMetricsF metrics(font) ;
                
                text = string_sprintf("%d", pagenumber) ;

                // remove the '#' char and readjust positions
                segment.glyph.erase(segment.glyph.begin() + static_cast<ssize_t>(cloop)) ;
                
                // now adjust positions
                COORD_T width ;
                if(cloop != 0)
                {
                    width = segment.position[cloop] - segment.position[cloop - 1] ;
                }
                else
                {
                    width = segment.position[cloop] ;
                }
                                        
                for(size_t ploop = cloop; ploop < segment.position.size(); ploop++)
                {
                    segment.position[ploop] -= width ;
                }

                // remove the invalid position
                segment.position.erase(segment.position.begin() + static_cast<ssize_t>(cloop)) ;

//                sFontSizes size ;
                QString temp ;
                for(size_t mloop = 0; mloop < text.length(); mloop++)
                {
                    temp = text.at(mloop) ;
                    COORD_T width = metrics.horizontalAdvance(temp) ;
//                    mFontGC->GetTextExtent(temp, &size.width, &size.height, &size.descent, &size.externalLeading) ;
                    
                    // insert and update positions
                    for(size_t ploop = cloop ; ploop < segment.position.size(); ploop++)
                    {
                        segment.position[ploop] += width ;
                    }

                    // insert the glyph
                    string temp(1, text[mloop]) ;
                    segment.glyph.insert(segment.glyph.begin() + cloop + mloop, temp) ;
                    
                    // insert the position
                    if((cloop < segment.position.size()) && (cloop != 0))
                    {
                        segment.position.insert(segment.position.begin() + cloop, width + segment.position[cloop -1]) ;
                    }
                    else
                    {
                        if(segment.position.size() != 0)
                        {
                            segment.position.push_back(width + segment.position[segment.position.size() - 1]) ;
                        }
                        else
                        {
                            segment.position.push_back(width) ;
                        }
                    }
                }
            }
        }
    }    
}



void cLayout::SavePageInfo(PAGE_T pagenumber)
{
    if(mEditor->GetIsHelp() == false)
    {
        if(static_cast<PAGE_T>(mPageInfo.size()) <= pagenumber - 1)
        {
            mPageInfo.push_back(mCurrentPage) ;
        }
        else
        {
            mPageInfo[pagenumber -1] = mCurrentPage ;
        }

        // keep all the paper size info, but don't change the other data
        mCurrentPage.set = false ;
    }
}


/////////////////////////////////////////////////////////////////////////////
///
/// @param  index   [IN/OUT] - the position to read from
/// @param  text    [IN] - the string to pull the grapheme from
/// @param  offsets [IN] - the grapheme offsets associated with the string
///
/// @return string - the single grapheme
///
/// @brief
/// Get the grapheme at index. If index points to a MARKER_CHAR, and we
/// are not showing control characters, then skip and increment index.
///
/// This code is very similar to cDocument::GetChar(), but works on the local
/// text.
///
/////////////////////////////////////////////////////////////////////////////
string cLayout::GetGrapheme(ssize_t &index, string &text, vector<POSITION_T> &offsets)
{
    string grapheme ;

    // make sure we are in bounds
    if(index >= offsets.size())
    {
        return grapheme ;
    }

    // loop, incase we need to skip control characters
//    do
//    {
        size_t glength;
        if (index < offsets.size() - 1)
        {
            glength = offsets[index + 1] - offsets[index];
        }
        else
        {
            glength = text.size() - offsets[index] ;
        }

        for (POSITION_T loop = 0; loop < glength; loop++)
        {
            grapheme.push_back(text[offsets[index] + loop]);
        }
/*
        if(grapheme[0] = MARKER_CHAR && grapheme.length() == 1)
        {
            index++ ;
            grapheme.clear() ;
            // make sure we are still in bounds
            if(index >= offsets.size())
            {
                break ;
            }
            continue;
        }
        else
        {
            break ;
        }
*/
//break ;
/*
        // if this is a MARKER_CHAR then get the control code for this position
        char ch = grapheme[0];

        if(ch == MARKER_CHAR && grapheme.length() == 1)
        {
            // might be faster if we pass this in
            POSITION_T start, end ;
            mDocument->GetParagraphStartandEnd(mCurrentParagraph, start, end) ;

            char ch1 = mDocument->GetControlChar(index + start) ;

            // if we are showing control characters or if this is a tab, show the control character
            if(mEditor->GetShowControls() == SHOW_ALL || ch1 == STYLE_TAB)
            {
                string temp(1, ch1) ;
//                grapheme = temp ;
                break ;
            }
            else
            {
                grapheme.clear() ;
            }
        }
        else
        {
            break ;
        }

        // if this is a control char, and we are not showing them, then skip to next position
        index++ ;

        // make sure we are still in bounds
        if(index >= offsets.size())
        {
            grapheme.clear() ;
            break ;
        }
*/
//    } while(true) ;

    return grapheme ;
}
