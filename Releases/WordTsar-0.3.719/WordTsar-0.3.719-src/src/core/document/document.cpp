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

#include <math.h>
#include <string.h>

#include <string>
#include <algorithm>
#include <thread>
#include <regex>


#include "cpp-unicodelib/unicodelib.h"                  // we use this for word and sentence boundaries - ugh rewrite to fit into utf8proc

#include "document.h"
#include "src/core/include/config.h"


//#include "src/files/rtf/read/rtfparser.h"           // just for sColorTable


//#include "../../third-party/parallel-for/parallel-for.h"

using namespace std ;

// base wordstar colors
sSeqNewColor gBaseWSColors[] =
{
    {   0,   0,   0, 255 },             // black
    {   0,   0, 170, 255 },             // blue
    {   0, 170,   0, 255 },             // green
    {   0, 170, 170, 255 },             // cyan
    { 170,   0,   0, 255 },             // red
    { 170,   0, 170, 255 },             // magenta
    { 170,  85,   0, 255 },             // brown
    { 170, 170, 170, 255 },             // light gray
    {  85,  85,  85, 255 },             // dark gray
    {  85,  85, 170, 255 },             // light blue
    {  85, 170,  85, 255 },             // light green
    {  85, 170, 170, 255 },             // light cyan
    { 170,  85,  85, 255 },             // light red
    { 170,  85, 170, 255 },             // light magenta
    { 170, 170,  85, 255 },             // yellow
    { 170, 170, 170, 255 },             // white
} ;


// comparator for sorted array of formatting structures
bool TableCompare(PairTable const & first, PairTable const & second)
{
    return first.first < second.first;
}


bool TabCompare(TabPair const & first, TabPair const & second)
{
    return first.first < second.first;
}


bool FormatCompare(const FormatPair &first, const FormatPair &second)
{
    return first.first < second.first;
}

bool ColorCompare(const ColorPair &first, const ColorPair &second)
{
    return first.first < second.first ;
}

bool FontCompare(const FontPair &first, const FontPair &second)
{
    return first.first < second.first ;
}

bool IndexCompare(const IndexPair &first, const IndexPair &second)
{
    return first.first < second.first ;
}

bool FootnoteCompare(const FootnotePair &first, const FootnotePair &second)
{
    return first.first < second.first ;
}

bool EndnoteCompare(const EndnotePair &first, const EndnotePair &second)
{
    return first.first < second.first ;
}

bool ParagraphCompare(const sParagraphData &first, const sParagraphData &second)
{
    return first.index < second.index ;
}

double myabs(const double &t)
{
    return t >= 0 ? t : -t;
}


bool FuzzyCompare(double a, double b)
{
    return (myabs(a - b) * 1000000000000. <= std::min(myabs(a), myabs(b)));
}


cDocument::cDocument()
{
    Clear();
    mChanged = false ;

    int numthreads = static_cast<int>(thread::hardware_concurrency()) ;
    if(numthreads == 0)
    {
        numthreads = 4 ;
    }
    mMaxThreads = numthreads ;
}

cDocument::~cDocument()
{
//dtor
}



/////////////////////////////////////////////////////////////////////////////
///
/// @return nothing
///
/// @brief
/// Clear the document completely.
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::Clear(void)
{
    // clear tabs, color, etc tables
    mParagraphData.clear();

    mRedrawFullDisplay = true;
    mIsLoading = false;
    mRecalcTextSize = true;

    sParagraphData pdata;
    pdata.index = 0;
    mParagraphData.push_back(pdata);

    mChanged = false;

    mLastParagraphFromPosition = 0;
    mLastParagraphFromPositionResult = 0;
    mLastNumParagraph = 0;

    SetPosition(0);

    Insert(STYLE_EOF) ;
//Insert(" ") ;
    SetPosition(0);

    mBlockSet = false ;
    mStartBlock = 0 ;
    mEndBlock = 0 ;

    mCPParagraph = 1024 ;           // random number just so it's no 0 to start (see GetParagraphCodepoints())


}


/////////////////////////////////////////////////////////////////////////////
///
/// @param  char   [IN] - CHAR_T charcater to insert into buffer
///
/// @return bool - true on success
///
/// @brief
/// This will insert charcaters into the text buffer and build all the
/// tables required for special characters
///
/////////////////////////////////////////////////////////////////////////////
bool cDocument::Insert(CHAR_T ch)
{
    bool retval = true ;                ///< @tofo add error checking

    mChanged = true ;

    // convert the CHAR_T codepoint into a utf8 array
    unsigned char utf8[4] ;
    memset(utf8, 0, 4) ;
    utf8proc_ssize_t ulen = utf8proc_encode_char(static_cast<utf8proc_int32_t>(ch), &utf8[0]) ;
    POSITION_T pos = GetPosition() ;
    POSITION_T undopos = pos;
    POSITION_T parapos = 0;
    PARAGRAPH_T currentparagraphnumber = GetParagraphFromPosition(pos) ;
    auto paraiter = mParagraphData.begin() + currentparagraphnumber ;

    MY_ASSERT(pos >= 0)
    MY_ASSERT(currentparagraphnumber >= 0)

    parapos = mCurrentPosition - paraiter->index;
    vector<POSITION_T> offsets;
//    size_t len = GraphemeCount(paraiter->buffer, offsets);
    size_t len = GetParagraphGraphemeOffsets(currentparagraphnumber, offsets);
    if(len == 0)
    {
        offsets.push_back(0);
    }

    IncrementAttributes(pos) ;

    // if this is a formatting character, deal with it
    // we insert MARKER_CHAR into the buffer, and read our control char tables to figure out what to do
    // because CP437 uses these values as well
    if((ch <= STYLE_END_OF_STYLES) && (ch != HARD_RETURN) && (ch != SPACE) && (ch != 0))
    {
        retval = SetControlChar(ch) ;
        ch = MARKER_CHAR;
        utf8[0] = MARKER_CHAR ;
    }

    // insert our utf8 array into the buffer
    POSITION_T index ;
    if(parapos <= static_cast<POSITION_T>(len))
    {
        index = offsets[static_cast<size_t>(parapos)] ;
    }
    else
    {
        index = static_cast<POSITION_T>(paraiter->buffer.size()) ;
    }

    if(index < CAST_POSITION_SIZE(paraiter->buffer.size()))
    {
        for(int loop = 0; loop < ulen; loop++)
        {
            auto bstart = paraiter->buffer.begin() ;
            paraiter->buffer.insert(bstart + index, utf8[loop]) ;
            index++ ;
        }
    }
    else
    {
        for(int loop = 0; loop < ulen; loop++)
        {
            paraiter->buffer.push_back(utf8[loop]) ;
            index++ ;
        }
    }

    // refresh our grapheme offsets
    SaveOffsets(currentparagraphnumber, offsets);

    // increment position count if we have a new grapheme bundle
    if (len < offsets.size())
    {
//        IncrementAttributes(pos) ;
        mCurrentPosition++;
    }
//    else if(len > offsets.size())
//    {
//        DecrementAttributes(pos) ;
//        mCurrentPosition-- ;
//    }

    // if this is a formatting character, deal with it
    // we insert MARKER_CHAR into the buffer, and read our control char tables to figure out what to do
    // because CP437 uses these values as well
    if((ch <= STYLE_END_OF_STYLES) && (ch != HARD_RETURN) && (ch != SPACE) && (ch != 0))
    {
        retval = SetControlChar(ch) ;
        ch = MARKER_CHAR;
        utf8[0] = MARKER_CHAR ;
    }

//    retval = true ;

    // deal with a hard return to create paragraphs
    if (ch == HARD_RETURN)
    {
        parapos = mCurrentPosition - paraiter->index;
//        size_t len = GraphemeCount(paraiter->buffer, offsets);

        InsertParagraph(mCurrentPosition, offsets[static_cast<size_t>(parapos)], currentparagraphnumber);
    }

//    SaveUndo(INSERT, undopos) ;

    mRecalcTextSize = true ;
    return retval ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @param  text   [IN] - stl string to insert into buffer
///
/// @return nothing
///
/// @brief
/// insert an stl string into the buffer. Normalizes (NFC) the string first.
///
/////////////////////////////////////////////////////////////////////////////
bool cDocument::Insert(const string &text)
{
    bool retval = true;

    vector<utf8proc_int32_t> codepoints ;

    string ntext = Normalize(text) ;

    GetCodePoints(ntext, codepoints) ;

    bool oldloading = mIsLoading ;
    mIsLoading = true ;         // used for undo

    for(size_t loop = 0; loop < codepoints.size() ; ++loop)
    {
        if(codepoints[loop] == 10)
        {
            retval = Insert(static_cast<CHAR_T>(HARD_RETURN)) ;
        }
        else
        {
            retval = Insert(static_cast<CHAR_T>(codepoints[loop])) ;
        }
        if (retval == false)
        {
            break;
        }
//        Insert(static_cast<CHAR_T>(text[loop])) ;
    }

    mIsLoading = oldloading ;

    return retval;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param  position   [IN] - the current cursor position in the editor
/// @param  length     [IN] - the length of text we are deleting
///
/// @return bool - true on success
///
/// @brief
/// delete text from the editor (grapheme based)
///
/// @todo This is exceedingly slow when deleting a lot of characters (in DecrementAttributes)
///       multiple paragraphs make it worse.
/////////////////////////////////////////////////////////////////////////////
bool cDocument::Delete(POSITION_T position, POSITION_T length)
{
    bool retval = false ;

    POSITION_T bsize = GetTextSize() ;
    // nothing to delete (can't delete STYLE_EOF)
    if (bsize <= 1)
    {
        return false;
    }

    if(position == bsize - 1)
    {
        return false ;
    }


    MY_ASSERT(length <= GetTextSize() - position)
    MY_ASSERT(position >= 0)
    MY_ASSERT(length > 0)

    mChanged = true ;
    
    mCurrentPosition = position ;

    for(POSITION_T ctr = position + length -1; ctr >= position; ctr--)
//    for (POSITION_T ctr = position; ctr < position + length; ctr++)
    {

        string grapheme = GetChar(position);
        if (grapheme[0] < STYLE_END_OF_STYLES && grapheme.length() == 1)
        {
            switch (grapheme[0])
            {
                case STYLE_TAB:
                    DeleteTab(position);
                    break;

                case STYLE_FONT1:
                    DeleteFont(position);
                    break;

                case STYLE_INTERNAL_COLOR:
                    DeleteColor(position);
                    break;

                case 13: // HARD_RETURN :
                    DeleteParagraph(position);
                    break;

                default:
                    DeleteControlChar(position);
                    break;
            }
        }

        PARAGRAPH_T currentparagraphnumber = GetParagraphFromPosition(position);
        auto paraiter = mParagraphData.begin() + currentparagraphnumber ;
        POSITION_T buffer_position = position - paraiter->index;        // makeposition relative to paragraph start

        vector<POSITION_T> offsets;
        size_t len = GetParagraphGraphemeOffsets(currentparagraphnumber, offsets); //  GraphemeCount(paraiter->buffer, offsets);
        if (len == 0)
        {
            offsets.push_back(0);
        }

        MY_ASSERT(buffer_position >= 0)
        MY_ASSERT(static_cast<size_t>(buffer_position) < offsets.size())

        auto bstart = paraiter->buffer.begin() + offsets[static_cast<size_t>(buffer_position)] ;
        paraiter->buffer.erase(bstart, bstart + grapheme.size()) ;

        // refresh our grapheme offsets
        SaveOffsets(currentparagraphnumber, offsets);
    }

    DecrementAttributes(position, length) ;
    mRecalcTextSize = true ;


    return true ; // retval ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param  position   [IN/OUT] - the position in the buffer to get grapheme from
///
/// @return string - the grapheme at position
///
/// @brief
/// if mShowControl is not SHOW_ALL, then the next showable character
/// is returned and position is incremented to the right value
///
/// @todo This issumes the end of a paragraph is not a control code
/////////////////////////////////////////////////////////////////////////////
string cDocument::GetChar(POSITION_T &position)
{
    string grapheme;
    POSITION_T textsize = GetTextSize() ;

    MY_ASSERT(position >= 0)
//    MY_ASSERT(position <= textsize)


    // if the position requested is not in our buffer
    if(position >= textsize)
    {
        return grapheme ;
    }

    // we use a do loop to get past control chars if we don't want them
    do
    {
        PARAGRAPH_T currentparagraphnumber = GetParagraphFromPosition(position) ;
        auto paraiter = mParagraphData.begin() + currentparagraphnumber ;
        POSITION_T newposition = position - paraiter->index ;        // make position relative to paragraph start

        vector<POSITION_T> offsets;
        size_t len = GetParagraphGraphemeOffsets(currentparagraphnumber, offsets); //  GraphemeCount(paraiter->buffer, offsets);

        // this should never happen, but let's test for it anyway. Safety first.
       if(newposition >= static_cast<POSITION_T>(offsets.size()))
        {
            break ;
        }


        MY_ASSERT(newposition >= 0)
        MY_ASSERT(newposition < static_cast<POSITION_T>(offsets.size()))
        MY_ASSERT(newposition + 1 <= static_cast<POSITION_T>(offsets.size()))

        POSITION_T index = offsets[static_cast<size_t>(newposition)];
        POSITION_T glength;
        if (static_cast<size_t>(newposition) < len - 1)
        {
            glength = offsets[static_cast<size_t>(newposition + 1)] - index;
        }
        else
        {
            glength = static_cast<POSITION_T>(paraiter->buffer.size()) - index ;
        }

        MY_ASSERT(index < static_cast<POSITION_T>(paraiter->buffer.size()))

        for (size_t loop = 0; loop < static_cast<size_t>(glength); loop++)
        {
            grapheme.push_back(paraiter->buffer[static_cast<size_t>(index) + loop]);
        }

        // if this is a MARKER_CHAR then get the control code for this position
        char ch = grapheme[0];

        if(ch == MARKER_CHAR)
        {
            char ch1 = GetControlChar(position) ;

            // if we are showing control characters or if this is a tab, show the control character
            if(mShowControl == SHOW_ALL || ch1 == STYLE_TAB)
            {
                grapheme[0] = ch1 ;
                break ;
            }
        }
        else
        {
            break ;
        }

        // if this is a control char, and we are not showing them, then skip to next position
        position++ ;
    } while(true) ;

    return grapheme ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return nothing
///
/// @brief
/// Toggle Boldface
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::BeginBold(void)
{
    Insert(STYLE_BOLD) ;
    mRedrawFullDisplay = true ;         // make whole screen redraw, since this will toggle any text after it
}



/////////////////////////////////////////////////////////////////////////////
///
/// @return nothing
///
/// @brief
/// Toggle Boldface
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::EndBold(void)
{
    BeginBold() ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return nothing
///
/// @brief
/// Toggle Italics
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::BeginItalics(void)
{
    Insert(STYLE_ITALICS) ;
    mRedrawFullDisplay = true ;         // make whole screen redraw, since this will toggle any text after it
}

/////////////////////////////////////////////////////////////////////////////
///
/// @return nothing
///
/// @brief
/// Toggle Italics
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::EndItalics(void)
{
    BeginItalics() ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return nothing
///
/// @brief
/// Toggle Underline
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::BeginUnderline(void)
{
    Insert(STYLE_UNDERLINE) ;
    mRedrawFullDisplay = true ;         // make whole screen redraw, since this will toggle any text after it
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return nothing
///
/// @brief
/// Toggle Underline
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::EndUnderline(void)
{
    BeginUnderline() ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return nothing
///
/// @brief
/// Toggle Strikethrough
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::BeginStrikeThrough(void)
{
    Insert(STYLE_STRIKETHROUGH) ;
    mRedrawFullDisplay = true ;         // make whole screen redraw, since this will toggle any text after it
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return nothing
///
/// @brief
/// Toggle Strikethrough
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::EndStrikeThrough(void)
{
    BeginStrikeThrough() ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return nothing
///
/// @brief
/// Toggle Superscript
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::BeginSuperscript(void)
{
    Insert(STYLE_SUPERSCRIPT) ;
    mRedrawFullDisplay = true ;         // make whole screen redraw, since this will toggle any text after it
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return nothing
///
/// @brief
/// Toggle Superscript
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::EndSuperscript(void)
{
    BeginSuperscript() ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return nothing
///
/// @brief
/// Toggle Subscript
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::BeginSubscript(void)
{
    Insert(STYLE_SUBSCRIPT) ;
    mRedrawFullDisplay = true ;         // make whole screen redraw, since this will toggle any text after it
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return nothing
///
/// @brief
/// Toggle Subscript
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::EndSubscript(void)
{
    BeginSubscript() ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return nothing
///
/// @brief
/// Toggle Index
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::BeginIndex(void)
{
    Insert(STYLE_INDEX) ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return nothing
///
/// @brief
/// Toggle Index
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::EndIndex(void)
{
    BeginIndex() ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return nothing
///
/// @brief
/// Turn on centering
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::BeginCenter(void)
{
    char out[100] ;
    snprintf(out, 99, ".ojc%c", HARD_RETURN) ;
    Insert(out) ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return nothing
///
/// @brief
/// Turn on left justification
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::BeginLeft(void)
{
    char out[100] ;
    snprintf(out, 99, ".oj off%c", HARD_RETURN) ;
    Insert(out) ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return nothing
///
/// @brief
/// Turn on right justification
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::BeginRight(void)
{
    char out[100] ;
    snprintf(out, 99, ".ojr%c", HARD_RETURN) ;
    Insert(out) ;
}

/////////////////////////////////////////////////////////////////////////////
///
/// @return nothing
///
/// @brief
/// Turn on full justification
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::BeginJustify(void)
{
    char out[100] ;
    snprintf(out, 99, ".oj on%c", HARD_RETURN) ;
    Insert(out) ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @return  nothing
///
/// @brief
/// Insert a hard return only if we are not at the start of a paragraph
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::MaybeInsertHardReturn(void)
{
    POSITION_T cpos = GetPosition() ;
    PARAGRAPH_T para = GetParagraphFromPosition(cpos) ;

    POSITION_T start, end ;
    GetParagraphStartandEnd(para, start, end) ;
    if(start != cpos)
    {
        Insert(HARD_RETURN) ;
    }
}



/////////////////////////////////////////////////////////////////////////////
///
/// @return  nothing
///
/// @brief
/// Insert a tab
///
/////////////////////////////////////////////////////////////////////////////
bool cDocument::InsertTab(sWSTab &tab)
{
    POSITION_T pos = GetPosition() ;
    PARAGRAPH_T currentparagraphnumber = GetParagraphFromPosition(pos) ;
    auto paraiter = mParagraphData.begin() + currentparagraphnumber ;
    pos -= paraiter->index ;        // makeposition relative to paragraph start

    PairTable t  = make_pair(pos, TYPE_TAB) ;
    TabPair t1 = make_pair(pos,  tab) ;

    Insert(MARKER_CHAR) ;

    auto iter = lower_bound(paraiter->pairs.begin(), paraiter->pairs.end(), t, TableCompare) ;
    auto iter1 = lower_bound(paraiter->tab.begin(), paraiter->tab.end(), t1, TabCompare) ;

    size_t tpos = static_cast<size_t>(distance(paraiter->tab.begin(), iter1)) ;

    try
    {
        paraiter->tab.insert(paraiter->tab.begin() + tpos, t1) ;
    }
    catch(...)
    {
        return false ;
    }

    try
    {
        tpos = static_cast<size_t>(distance(paraiter->pairs.begin(), iter)) ;
        paraiter->pairs.insert(paraiter->pairs.begin() + tpos, t) ;
    }
    catch(...)
    {
        return false ;
    }

    return true ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @return  bool
///
/// @brief
/// insert a color change
///
/////////////////////////////////////////////////////////////////////////////
bool cDocument::InsertColor(sWSColor &color)
{
    POSITION_T pos = GetPosition() ;
    PARAGRAPH_T currentparagraphnumber = GetParagraphFromPosition(pos) ;
    auto paraiter = mParagraphData.begin() + currentparagraphnumber ;
    pos -= paraiter->index ;        // makeposition relative to paragraph start

    PairTable t  = make_pair(pos, TYPE_COLOR) ;
    ColorPair t1 = make_pair(pos,  color) ;

    Insert(MARKER_CHAR) ;

    auto iter = lower_bound(paraiter->pairs.begin(), paraiter->pairs.end(), t, TableCompare) ;
    auto iter1 = lower_bound(paraiter->color.begin(), paraiter->color.end(), t1, ColorCompare) ;

    size_t tpos = static_cast<size_t>(distance(paraiter->color.begin(), iter1)) ;

    try
    {
        paraiter->color.insert(paraiter->color.begin() + tpos, t1) ;
    }
    catch(...)
    {
        return false ;
    }

    try
    {
        tpos = static_cast<size_t>(distance(paraiter->pairs.begin(), iter)) ;
        paraiter->pairs.insert(paraiter->pairs.begin() + tpos, t) ;
    }
    catch(...)
    {
        return false ;
    }
    
    return true ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @return  bool
///
/// @brief
/// deal with a color change in RGB format
///
/// Wordstar doesn't do RGB colors, so this is non-standard
///
/// @TODO - for now, finds the closest Wordstar color and uses that. Finds 
///         closest color by converting to HSL and find the closest
///         Euclidian distance.
/////////////////////////////////////////////////////////////////////////////
bool cDocument::InsertColor(sColorTable &color)
{
    double h, s, l ;
    double finaldistance = 2000 ;
    unsigned char finalindex = 0 ;
    
    ConvertToHSL(color.red, color.green, color.blue, h, s, l) ;
    
    for(unsigned char loop = 0; loop < MAXWSCOLORS; loop++)
    {
        double h1, s1, l1 ;
        ConvertToHSL(gBaseWSColors[loop].red, gBaseWSColors[loop].green, gBaseWSColors[loop].blue, h1, s1, l1) ;
        
        double distance = CalculateDistance(h, s, l, h1, s1, l1) ;
        if(distance < finaldistance)
        {
            finaldistance = distance ;
            finalindex = loop ;
        }
    }
    
    sWSColor newcolor ;
    newcolor.colornumber = finalindex ;
    return InsertColor(newcolor) ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return  nothing
///
/// @brief
/// insert a font change
///
/////////////////////////////////////////////////////////////////////////////
bool cDocument::InsertFont(sInternalFonts &font)
{
    POSITION_T pos = GetPosition() ;
    PARAGRAPH_T currentparagraphnumber = GetParagraphFromPosition(pos) ;
    auto paraiter = mParagraphData.begin() + currentparagraphnumber ;
    pos -= paraiter->index ;        // makeposition relative to paragraph start

    PairTable t  = make_pair(pos, TYPE_FONT) ;
    FontPair t1 = make_pair(pos,  font) ;

    Insert(MARKER_CHAR) ;

    auto iter = lower_bound(paraiter->pairs.begin(), paraiter->pairs.end(), t, TableCompare) ;
    auto iter1 = lower_bound(paraiter->font.begin(), paraiter->font.end(), t1, FontCompare) ;

    size_t tpos = static_cast<size_t>(distance(paraiter->font.begin(), iter1)) ;

    try
    {
        paraiter->font.insert(paraiter->font.begin() + tpos, t1) ;
    }
    catch(...)
    {
        return false ;
    }

    try
    {
        tpos = static_cast<size_t>(distance(paraiter->pairs.begin(), iter)) ;
        paraiter->pairs.insert(paraiter->pairs.begin() + tpos, t) ;
    }
    catch(...)
    {
        return false ;
    }

    return true ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @return  nothing
///
/// @brief
/// insert a footnote
///
/////////////////////////////////////////////////////////////////////////////
bool cDocument::InsertFootnote(sNote &note)
{
    POSITION_T pos = GetPosition() ;
    PARAGRAPH_T currentparagraphnumber = GetParagraphFromPosition(pos) ;
    auto paraiter = mParagraphData.begin() + currentparagraphnumber ;
    pos -= paraiter->index ;        // makeposition relative to paragraph start

    PairTable t  = make_pair(pos, TYPE_FOOTNOTE) ;
    FootnotePair t1 = make_pair(pos,  note) ;

    Insert(MARKER_CHAR) ;

    auto iter = lower_bound(paraiter->pairs.begin(), paraiter->pairs.end(), t, TableCompare) ;
    auto iter1 = lower_bound(paraiter->footnote.begin(), paraiter->footnote.end(), t1, FootnoteCompare) ;

    size_t tpos = static_cast<size_t>(distance(paraiter->footnote.begin(), iter1)) ;

    try
    {
        paraiter->footnote.insert(paraiter->footnote.begin() + tpos, t1) ;
    }
    catch(...)
    {
        return false ;
    }

    try
    {
        tpos = static_cast<size_t>(distance(paraiter->pairs.begin(), iter)) ;
        paraiter->pairs.insert(paraiter->pairs.begin() + tpos, t) ;
    }
    catch(...)
    {
        return false ;
    }

    return true ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @return  nothing
///
/// @brief
/// insert a end note
///
/////////////////////////////////////////////////////////////////////////////
bool cDocument::InsertEndnote(sNote &note)
{
    POSITION_T pos = GetPosition() ;
    PARAGRAPH_T currentparagraphnumber = GetParagraphFromPosition(pos) ;
    auto paraiter = mParagraphData.begin() + currentparagraphnumber ;
    pos -= paraiter->index ;        // makeposition relative to paragraph start

    PairTable t  = make_pair(pos, TYPE_ENDNOTE) ;
    EndnotePair t1 = make_pair(pos,  note) ;

    Insert(MARKER_CHAR) ;

    auto iter = lower_bound(paraiter->pairs.begin(), paraiter->pairs.end(), t, TableCompare) ;
    auto iter1 = lower_bound(paraiter->endnote.begin(), paraiter->endnote.end(), t1, EndnoteCompare) ;

    size_t tpos = static_cast<size_t>(distance(paraiter->endnote.begin(), iter1)) ;

    try
    {
        paraiter->endnote.insert(paraiter->endnote.begin() + tpos, t1) ;
    }
    catch(...)
    {
        return false ;
    }

    try
    {
        tpos = static_cast<size_t>(distance(paraiter->pairs.begin(), iter)) ;
        paraiter->pairs.insert(paraiter->pairs.begin() + tpos, t) ;
    }
    catch(...)
    {
        return false ;
    }

    return true ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param  loading   [IN] - true if loading file, else false
///
/// @return nothing
///
/// @brief
/// Set the loading flag
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::SetLoading(bool loading)
{
    mIsLoading = loading ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return bool
///
/// @brief
/// Get the loading flag: true if file loading, else flase
///
/////////////////////////////////////////////////////////////////////////////
bool cDocument::GetLoading(void)
{
    return mIsLoading ;
}




/////////////////////////////////////////////////////////////////////////////
///
/// @return POSITION_T - number of graphemes in document (including formatting chars)
///
/// @brief
/// Get the number of graphemes in the document
///
/////////////////////////////////////////////////////////////////////////////
POSITION_T cDocument::GetTextSize(void)
{
//    if(mRecalcTextSize == true)
    {
        mTextSize = 0 ;
        PARAGRAPH_T size = static_cast<PARAGRAPH_T>(mParagraphData.size() - 1) ;

        MY_ASSERT(size >= 0)
        MY_ASSERT(size < GetNumberofParagraphs())

        mTextSize = mParagraphData[size].index + mParagraphData[size].offsets.size() ;
    }

    return mTextSize ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @return POSITION_T - the position in the buffer
///
/// @brief
/// Get the position of the carat in the buffer
///
/////////////////////////////////////////////////////////////////////////////
POSITION_T cDocument::GetPosition(void)
{
    POSITION_T position = mCurrentPosition ;

    return position ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param position   [IN] the position to set
///
/// @return nothing
///
/// @brief
/// set the current position in the buffer
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::SetPosition(POSITION_T position)
{
    if(position >= GetTextSize() && position != 0)
    {
        position = GetTextSize() - 1 ;
    }

    mPreviousPosition = mCurrentPosition ;
    mCurrentPosition = position ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @return nothing
///
/// @brief
/// set the current position to the previous position
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::GotoPreviousPosition(void)
{
    SetPosition(mPreviousPosition) ;
}




/////////////////////////////////////////////////////////////////////////////
///
/// @return number of paragraph
///
/// @brief
/// Get the number of paragraphs in the document
///
/////////////////////////////////////////////////////////////////////////////
PARAGRAPH_T cDocument::GetNumberofParagraphs(void)
{
    return static_cast<PARAGRAPH_T>(mParagraphData.size()) ;
}




/////////////////////////////////////////////////////////////////////////////
///
/// @param paragraph   [IN] the paragraph to check
/// @param start       [IN/OUT] the start position
/// @param end         [IN/OUT] the end position
///
/// @return nothing
///
/// @brief
/// returns start bufferpos and end bufferpos of paragraph 
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::GetParagraphStartandEnd(const PARAGRAPH_T paragraph, POSITION_T &start, POSITION_T &end)
{
    MY_ASSERT(paragraph >= 0)
    MY_ASSERT(paragraph < static_cast<POSITION_T>(mParagraphData.size()))
    auto iter = mParagraphData.begin() + paragraph ;
    start = iter->index ;

    vector<POSITION_T> offsets ;
    size_t numgraphemes = iter->offsets.size(); // GraphemeCount(iter->buffer, offsets);
    end =  iter->index + static_cast<POSITION_T>(numgraphemes) - 1 ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param  position    [IN] - the position in the buffer
///
/// @return PARAGRAPH_T
///
/// @brief Get the paragraph number from the character position
///
/////////////////////////////////////////////////////////////////////////////
PARAGRAPH_T cDocument::GetParagraphFromPosition(POSITION_T position)
{
#ifdef DEBUG
    POSITION_T tsize = GetTextSize() ;
#endif
    MY_ASSERT(position >= 0)
    MY_ASSERT(position <= tsize)

    mTempParagraph.index = position ;

    auto iter = lower_bound(mParagraphData.begin(), mParagraphData.end(), mTempParagraph, ParagraphCompare) ;
    // handle things if we are at end of line
	if (iter != mParagraphData.end())
	{
		if (iter->index > position)
        {
		    if (iter != mParagraphData.begin())
			{
				iter--;
			}
		}
	}

    PARAGRAPH_T pos = static_cast<PARAGRAPH_T>(std::distance(mParagraphData.begin(), iter)) ;
    if(static_cast<size_t>(pos) >= mParagraphData.size())
    {
        pos = static_cast<POSITION_T>(mParagraphData.size()) - 1 ;
    }
    
    return pos ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param position   [IN] the position to check tab type for
///
/// @return sWSTab
///
/// @brief
/// We have a tab in the buffer, check what type it is
///
/////////////////////////////////////////////////////////////////////////////
sWSTab cDocument::GetTab(POSITION_T position)
{
    MY_ASSERT(position >= 0)
    MY_ASSERT(position <= GetTextSize())

    TabPair comp ;

    PARAGRAPH_T currentparagraphnumber = GetParagraphFromPosition(position) ;
    auto paraiter = mParagraphData.begin() + currentparagraphnumber ;
    position -= paraiter->index ;
    comp.first = position ;
    auto  iter1 = lower_bound(paraiter->tab.begin(), paraiter->tab.end(), comp, TabCompare) ;

    return static_cast<sWSTab>(iter1->second) ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param position   [IN] the position to check color for
/// @param color      [OUT] the found color
///
/// @return bool - true if color found at position
///
/// @brief
/// We have a color in the buffer, check what it is
///
/////////////////////////////////////////////////////////////////////////////
bool cDocument::GetColor(POSITION_T position, sWSColor &color)
{
    MY_ASSERT(position >= 0)
    MY_ASSERT(position <= GetTextSize())

    ColorPair comp ;
    bool retval = false ;

    PARAGRAPH_T currentparagraphnumber = GetParagraphFromPosition(position) ;
    auto paraiter = mParagraphData.begin() + currentparagraphnumber ;

    position -= paraiter->index ;
    comp.first = position ;
    auto  iter1 = lower_bound(paraiter->color.begin(), paraiter->color.end(), comp, ColorCompare) ;
    if(iter1 != paraiter->color.end())
    {
        color = iter1->second ;
        retval = true ;
    }

    return retval ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param position   [IN] the position to check font for
/// @param intfont    [OUT] sInternalFonts data
///
/// @return bool - true on success else false
///
/// @brief
/// We have a font in the buffer, check what it is
///
/////////////////////////////////////////////////////////////////////////////
bool cDocument::GetFont(POSITION_T position, sInternalFonts &intfont)
{
    MY_ASSERT(position >= 0)
    MY_ASSERT(position <= GetTextSize())

    FontPair comp ;
    bool retval = false ;

    PARAGRAPH_T currentparagraphnumber = GetParagraphFromPosition(position) ;
    auto paraiter = mParagraphData.begin() + currentparagraphnumber ;
    position -= paraiter->index ;
    comp.first = position ;
    auto  iter1 = lower_bound(paraiter->font.begin(), paraiter->font.end(), comp, FontCompare) ;

    if(iter1 != paraiter->font.end())
    {
        intfont =  iter1->second ;
        retval = true ;
    }

    return retval ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param  fontlist   [IN/OUT] - All the fonts in the document 
///
/// @return nothing
///
/// @brief
/// fill fontlist with alll te font used in the document, including duplicates
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::GetFontList(vector<sInternalFonts> &fontlist)
{
    for(auto &paraiter : mParagraphData)
    {
        for(auto &fontiter : paraiter.font)
        {
            fontlist.push_back(fontiter.second) ;
        }
    }
}


/////////////////////////////////////////////////////////////////////////////
///
/// @param   value          [in] - the value we are converting
/// @param   type           [in] - the type we are converting from
///
/// @return  double - twips value
///
/// @brief
/// convert known data values into twip (rows, cm, mm, points, inches)
///
/////////////////////////////////////////////////////////////////////////////
COORD_T cDocument::ConvertToTwips(double value, char type)
{
    COORD_T retval = 0 ;

    switch(type)
    {
        case 'R' :              // rows (we assume 12pt font)
            retval = static_cast<COORD_T>(value * 240) ;    // 240 twips per 12 point font
            break ; 
            
        case 'C' :              // centimeters
            retval = static_cast<COORD_T>((value * 10) * TWIPSPERMM) ;
            break ;

        case 'M' :              // millimeters
            retval = static_cast<COORD_T>(value * TWIPSPERMM) ;
            break ;
            
        case 'P' :              // points
            retval = static_cast<COORD_T>(value * POINTSTOTWIPS) ;
            break ;

        case '\"' :               // inches
        case 'I' :
        default :
            retval = static_cast<COORD_T>(value * TWIPSPERINCH) ;
            break ;
    }

    return retval ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param   txt          [in] - the string we are converting to a double
/// @param   incdec       [out] - true if this is an increment or decrement, else false
///
/// @return  double
///
/// @brief
/// Converts a string to a double, removing the type indicator (M, ", C, etc)
///
/////////////////////////////////////////////////////////////////////////////
double cDocument::GetValue(string txt, bool &incdec)
{
    double value = 0.0 ;
    incdec = false ;

    if(txt.empty())
    {
        return value ;
    }

    // now we replace any trailing non-numeric characters from the string
    unsigned long loop ;
    for(loop = txt.length() - 1; loop > 0; loop--)
    {
        if(isdigit(txt.at(loop))) // .unicode()))
        {
            break ;
        }
         txt[loop] = ' ' ;
    }


    if(txt[0] == '+')
    {
        incdec = true ;
    }
    else if(txt[0] == '-')
    {
        incdec = true ;
    }

    string expression_str = txt.c_str() ;

    value = mMath.DoMath(expression_str) ;
    
    return value ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @param   txxt          [in] - return the type of the passed in value
///
/// @return  char
///
/// @brief
/// finds out the type of a value and returns it (M, ", C, etc)
///
/// M - millimeter, C - centimeter, " and I is inches  X - no type specified
/////////////////////////////////////////////////////////////////////////////
char cDocument::GetType(string text)
{
    char retval = 'x' ;         // @todo Wordstar defaults to rows, which WordTsar doesn't implement

    if(text.find('\"') != string::npos)              // inches
    {
        retval = '\"' ;
    }
    else if(text.find('I') != string::npos)          // inches
    {
        retval = '\"' ;
    }
    else if(text.find('C') != string::npos)          // centimeters
    {
        retval = 'C' ;
    }
    else if(text.find('M') != string::npos)          // millimeters
    {
        retval = 'M' ;
    }
    else if(text.find('P') != string::npos)          // points
    {
        retval = 'P' ;
    }
    else if(text.find('R') != string::npos)         // it's rows
    {
        retval = 'R' ;
    }

    return retval ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @param   show          [in] - eSHowCOntrol type
///
/// @return  nothing
///
/// @brief
/// Set whether we show control codes and/or dot commands
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::SetShowControl(eShowControl show)
{
    mShowControl = show ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @return nothing
///
/// @brief
/// Sets the current position as a block start
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::SetBeginBlock(void)
{
    // if a block is set, save the info
    if(mBlockSet)
    {
        mOldStartBlock = mStartBlock ;
        mOldEndBlock = mEndBlock ;
        mOldBlockSet = true ;
    }

    mStartBlock = GetPosition() ;

    if(mEndBlock > mStartBlock)
    {
        mBlockSet = true ;
    }
    else
    {
        mBlockSet = false ;
    }
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return nothing
///
/// @brief
/// Sets the current position as a block end
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::SetEndBlock(void)
{
    // if a block is set, save the info
    if(mBlockSet)
    {
        mOldStartBlock = mStartBlock ;
        mOldEndBlock = mEndBlock ;
        mOldBlockSet = true ;
    }

    mEndBlock = GetPosition() ;

    if(mEndBlock > mStartBlock)
    {
        mBlockSet = true ;
    }
    else
    {
        mBlockSet = false ;
    }
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return nothing
///
/// @brief
/// Sets the current position as a block end
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::SetPreviousBlock(void)
{
    if(mOldBlockSet)
    {
        POSITION_T start, end ;

        start = mStartBlock ;
        end = mEndBlock ;

        mStartBlock = mOldStartBlock ;
        mEndBlock = mOldEndBlock ;

        mOldStartBlock = start ;
        mOldEndBlock = end ;
    }
}



/////////////////////////////////////////////////////////////////////////////
///
/// @return nothing
///
/// @brief
/// Saves current state for Undo
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::SaveUndo(char type, POSITION_T position)
{
    auto size = mUndo.size() ;
    if(size > MAX_UNDO_REDO)
    {
        mUndo.pop_front() ;
    }

    sUndoRedo t ;
    t.position = position ;
    t.type = type ;

    mUndo.push_back(t) ;

    mRedo.clear() ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return bool - true if undo performed, else false if nothing to undo
///
/// @brief
/// return state to last undo position
///
/////////////////////////////////////////////////////////////////////////////
bool cDocument::Undo(void)
{

    if(mUndo.empty() == true)
    {
        return false ;
    }

    struct sUndoRedo t = mUndo[mUndo.size() - 1] ;
    SetPosition(t.position) ;
    switch(t.type)
    {
        case INSERT:
            Delete(t.position, 1) ;
            break ;

        case DELETE:
            Insert(t.ch) ;
        break ;
    }

/*
    mRedo.push_back(mUndo.back()) ;                     // save for our redo
    mRedoCarat.push_back(mUndoCarat.back()) ;

    mParagraphData = mUndo.back() ;                     // perform the undo
    mUndo.pop_back() ;                                  // get the undo off the queue
    mCurrentPosition = mUndoCarat.back() ;
    mUndoCarat.pop_back() ;
*/
    return true ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @return bool - true if true performed, else false if nothing to undo
///
/// @brief
/// return state to last undo position
///
/////////////////////////////////////////////////////////////////////////////
bool cDocument::Redo(void)
{
/*
    if(mRedo.empty() == true)
    {
        return false ;
    }

    mUndo.push_back(mRedo.back()) ;                     // save for our undo
    mUndoCarat.push_back(mRedoCarat.back()) ;                     // save for our undo

    mParagraphData = mRedo.back() ;                     // perform the redo
    mRedo.pop_back() ;                                  // get the redo off our queue
    mCurrentPosition = mRedoCarat.back() ;                     // perform the redo
    mRedoCarat.pop_back() ;                                  // get the redo off our queue
*/
    return true ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @param  text     [IN]  the string we are finding grapheme boundaries for
///  @param offsets  [OUT] array of grapheme start positions
///
/// @return size_t - the number of graphemes in the string
///
/// @brief
/// Get the count and grapheme boundaries in the string
///
/////////////////////////////////////////////////////////////////////////////
size_t cDocument::GraphemeCount(string &text, vector<POSITION_T> &offsets)
{
    offsets.clear() ;

    utf8proc_int32_t firstch = 0x00AD ;   // soft hyphen is never a grapheme break ;

    // if our paragraph is empty
    if(text.length() == 0)
    {
        offsets.emplace_back(0);
        return 0 ;
    }

    ssize_t size = 0;               // the size of our code point
    size_t index = 0;               // our current index into text
    utf8proc_int32_t codepoint;     // the code point we read
    utf8proc_int32_t state = 0 ;

    // now count our grapheme breaks

    long length = text.length() ;

    do
    {
        size = utf8proc_iterate(reinterpret_cast<const utf8proc_uint8_t*> (&text[index]), static_cast<long>(length - index), &codepoint);

        if (size > 0)
        {
            if (utf8proc_grapheme_break_stateful(firstch, codepoint, &state))
            {
                offsets.push_back(static_cast<POSITION_T>(index)) ;
            }
            index += static_cast<size_t>(size) ;
            firstch = codepoint;
        }
    } while (size > 0);

    return offsets.size() ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @param  text     [IN]  the buffer we are finding grapheme boundaries for
///  @param offsets  [OUT] array of grapheme start positions
///
/// @return size_t - the number of graphemes in the buffer
///
/// @brief
/// Get the count and grapheme boundaries in the buffer
///
/////////////////////////////////////////////////////////////////////////////
size_t cDocument::GraphemeCount(vector<char>& text, vector<POSITION_T>& offsets)
{
    offsets.clear();

    utf8proc_int32_t firstch = 0x00AD;   // soft hyphen is never a grapheme break ;

    // if our paragraph is empty
    if (text.size() == 0)
    {
        offsets.emplace_back(0);
        return 0;
    }

    ssize_t size = 0;               // the size of our code point
    size_t index = 0;               // our current index into text
    utf8proc_int32_t codepoint;     // the code point we read
    utf8proc_int32_t state = 0 ;

    // now count our grapheme breaks
//    do
    for(index = 0; index < text.size(); index++)
    {
        size = utf8proc_iterate(reinterpret_cast<const utf8proc_uint8_t*> (&text[index]), static_cast<long>(text.size() - index), &codepoint);

        if (size > 0)
        {
            if (utf8proc_grapheme_break_stateful(firstch, codepoint, &state))
            {
                offsets.push_back(index) ;
            }
//            index += static_cast<size_t>(size) ;
            firstch = codepoint;
        }
    }
//    } while (size > 0);

    return offsets.size();
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param  text     [IN]  the buffer we are getting codepoints for
///  @param codepoints  [OUT] array of codepoints
///
/// @return size_t - the number of codepoints in the string
///
/// @brief
/// Get the count and codepoints in the string
///
/////////////////////////////////////////////////////////////////////////////
size_t cDocument::GetCodePoints(const string& text, vector<utf8proc_int32_t>& codepoints)
{
    codepoints.resize(text.length() + 1) ;
    long size = static_cast<long>(text.length()) ;

    ssize_t res = utf8proc_decompose(reinterpret_cast<const utf8proc_uint8_t * >(text.c_str()), size, codepoints.data(), size + 1, static_cast<utf8proc_option_t>(0)) ;

    codepoints.resize(static_cast<size_t>(res));

    return codepoints.size();
}

/////////////////////////////////////////////////////////////////////////////
///
/// @param  para    [IN] the paragraph we are looking at
/// @param wordstarts [OUT] the position in the buffer of each word
///
/// @return size_t - number of words (including end of paragraph)
///
/// @brief
/// Get the number of words in a paragraph. Adds an extra marker for the end
/// of paragraph
///
/// This uses the cpp_unicode library, so we convert to utf32 to find the word
/// boundary. Finding word boundaries needs to be written for utf8proc
/////////////////////////////////////////////////////////////////////////////
size_t cDocument::GetWordPositions(PARAGRAPH_T para, vector<POSITION_T> &wordstarts)
{
    MY_ASSERT(para >= 0)
    MY_ASSERT(para < mParagraphData.size())

    string text = GetParagraphText(para) ;

    vector<utf8proc_int32_t> utf32 ;
    size_t utf32index = 0 ;

    utf8proc_ssize_t size = utf8proc_decompose(reinterpret_cast<utf8proc_uint8_t *>(&text[0]), static_cast<long>(text.length()), &utf32[utf32index], static_cast<long>(utf32.size()), UTF8PROC_NULLTERM) ;
    if(size >= 0)
    {
        utf32.resize(static_cast<size_t>(size)) ;
        size = utf8proc_decompose(reinterpret_cast<utf8proc_uint8_t *>(&text[0]), static_cast<long>(text.length()), &utf32[utf32index], static_cast<long>(utf32.size()), UTF8PROC_NULLTERM) ;

        size_t i = 0 ;
        size_t l = static_cast<size_t>(size) ;

        while (i < l)
        {
            while (i < l && !unicode::is_cased(utf32[i]))
            {
                i++;
            }

            if (i == l)
            {
                break;
            }

            wordstarts.push_back(i + mParagraphData[para].index) ;
            i++;

            if (i == l)
            {
              break;
            }

            while (i < l && !unicode::is_word_boundary(reinterpret_cast<const char32_t *>(&utf32[0]), l, i))
            {
                i++;
            }
        }

        wordstarts.push_back(size - 1 + mParagraphData[para].index) ;
    }

    return wordstarts.size() ;
}

/////////////////////////////////////////////////////////////////////////////
///
/// @param  pos     [IN]  the buffer we start looking for word breaks
///
/// @return POSITION_T - position of the first grapheme of the next word
///
/// @brief
/// Find the start of the next word in the paragraph.
///
/// This uses the cpp_unicode library, so we convert to utf32 to find the word
/// boundary. Finding word boundaries needs to be written for utf8proc
/////////////////////////////////////////////////////////////////////////////
POSITION_T cDocument::GetNextWordPosition(POSITION_T pos)
{
    pos++ ;

    MY_ASSERT(pos >= 0)
//    MY_ASSERT(pos < GetTextSize())

    PARAGRAPH_T para = GetParagraphFromPosition(pos) ;
    vector<POSITION_T> wordstarts ;
    POSITION_T retpos = pos ;

    GetWordPositions(para, wordstarts) ;

    for(size_t loop = 0; loop < wordstarts.size(); loop++)
    {
        if(wordstarts[loop] > pos)
        {
            retpos = wordstarts[loop] ;
            break ;
        }
    }

    return retpos ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @param  pos     [IN]  the buffer we start looking for word breaks
///
/// @return POSITION_T - position of the first grapheme of the prev word
///
/// @brief
/// Find the start of the prev word in the paragraph.
///
/// This uses the cpp_unicode library, so we convert to utf32 to find the word
/// boundary. Finding word boundaries needs to be written for utf8proc
/////////////////////////////////////////////////////////////////////////////
POSITION_T cDocument::GetPrevWordPosition(POSITION_T pos)
{
    pos-- ;

    MY_ASSERT(pos >= 0)
    MY_ASSERT(pos < GetTextSize())

    PARAGRAPH_T para = GetParagraphFromPosition(pos) ;
    vector<POSITION_T> wordstarts ;
    POSITION_T retpos = pos ;

    GetWordPositions(para, wordstarts) ;

//    for(size_t loop = wordstarts.size() - 1 ; loop >= 0; loop--)
//    {
//        if(wordstarts[loop] < pos)
//        {
//            retpos = wordstarts[loop] ;
//            break ;
//        }
//    }
    for(auto iter = wordstarts.rbegin() ; iter != wordstarts.rend(); iter++)
    {
        if(*iter < pos)
        {
            retpos = *iter ;
            break ;
        }
    }

    return retpos ;
}




/////////////////////////////////////////////////////////////////////////////
///
/// @return POSITION_T - the position of the next font tag, or the current position if none found
///
/// @brief
/// Find the next font tag in the document
///
/////////////////////////////////////////////////////////////////////////////
POSITION_T cDocument::GetNextFontTagPosition(void)
{

    PARAGRAPH_T numpara = GetNumberofParagraphs() ;
    POSITION_T pos = GetPosition() ;
    PARAGRAPH_T para = GetParagraphFromPosition(pos) ;

    // go through paragraph from current to end of list
    for(PARAGRAPH_T ploop = para; ploop < numpara; ploop++)
    {
        size_t numfonts = mParagraphData[ploop].font.size() ;
        if(numfonts != 0)
        {
            for(size_t floop = 0; floop < numfonts; floop++)
            {
                POSITION_T fpos = mParagraphData[ploop].font[floop].first ;
                if(fpos + mParagraphData[ploop].index > pos)
                {
                    pos = fpos + mParagraphData[ploop].index ;
                    ploop = numpara ;       // get out of outer loop
                    break ;
                }
            }
        }
    }

    return pos ;
}




/////////////////////////////////////////////////////////////////////////////
///
/// @param  str     [IN]  the string to normalize
///
/// @return string - the normalized string
///
/// @brief
/// NFC normalize the string
///
/////////////////////////////////////////////////////////////////////////////
string cDocument::Normalize(const string &str)
{
    // normalize the string
    utf8proc_uint8_t *normalized = utf8proc_NFC(reinterpret_cast<const utf8proc_uint8_t * >(str.c_str())) ;


    // put the normalized text into a string
    string ntext ;
    size_t x = 0 ;
    while(normalized[x] != 0)
    {
        ntext.push_back(static_cast<char>(normalized[x])) ;
        x++ ;
    }

    return ntext ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @param  str     [IN]  the string to lowercase
///
/// @return string - the lowercased string
///
/// @brief
/// lowercase the string
///
/////////////////////////////////////////////////////////////////////////////
string cDocument::LowerCase(const string &str)
{
    vector<utf8proc_int32_t> codepoints ;
    string outstr ;
    unsigned char utf8[4] ;

    size_t len = GetCodePoints(str, codepoints) ;
    for(size_t loop = 0; loop < len; loop++)
    {
        // lowercase codepoint
        codepoints[loop] = utf8proc_tolower(codepoints[loop]) ;

        // convert code point to array of char
        memset(utf8, 0, 4) ;
        utf8proc_ssize_t ulen = utf8proc_encode_char(static_cast<utf8proc_int32_t>(codepoints[loop]), &utf8[0]) ;

        string x ;
        for(ssize_t ctr = 0; ctr < ulen; ctr++)
        {
            x = static_cast<char>(utf8[static_cast<size_t>(ctr)]) ;
            outstr.append(x) ;
        }
    }

    return outstr ;
}



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
// PRIVATE
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
///
/// @param  ch   [IN] - the control charcater to insert
///
/// @return bool - true on success
///
/// @brief
/// Insert the control into the right table and update the main lookup
/// table. ONLY DEAL WITH bold, italic, etc)
///
/// @todo return code
/////////////////////////////////////////////////////////////////////////////
bool cDocument::SetControlChar(CHAR_T ch)
{
    POSITION_T pos = GetPosition() ;
    PARAGRAPH_T currentparagraphnumber = GetParagraphFromPosition(pos) ;
    auto paraiter = mParagraphData.begin() + currentparagraphnumber ;
    pos -= paraiter->index ;        // makeposition relative to paragraph start

    switch(ch)
    {
        case STYLE_TAB :                    // these styles get taken care of in their respective methods
        case STYLE_INTERNAL_COLOR :
        case STYLE_FONT1 :
            break ;


        default :
            // first create the pairs for the main lookup table and the format modifier (sorted)
            PairTable t = make_pair(pos, TYPE_FORMAT) ;
            FormatPair t1 = make_pair(pos,  static_cast<eModifiers>(ch)) ;

            auto iter = lower_bound(paraiter->pairs.begin(), paraiter->pairs.end(), t, TableCompare) ;
            auto iter1 = lower_bound(paraiter->format.begin(), paraiter->format.end(), t1, FormatCompare) ;

            try
            {
                PARAGRAPH_T index = static_cast<PARAGRAPH_T>(distance(paraiter->format.begin(), iter1)) ;
                paraiter->format.insert(paraiter->format.begin() + index, t1) ;
            }
            catch(...)
            {
                return false ;
            }

            try
            {
                PARAGRAPH_T index = static_cast<PARAGRAPH_T>(distance(paraiter->pairs.begin(), iter)) ;
                paraiter->pairs.insert(paraiter->pairs.begin() + index, t) ;
            }
            catch(...)
            {
                return false ;
            }

            break ;
    }

    return true ;
}




/////////////////////////////////////////////////////////////////////////////
///
/// @param  position   [IN] - the position to get character from
///
/// @return ch - the character
///
/// @brief
/// Get the control from the right table
///
/////////////////////////////////////////////////////////////////////////////
eModifiers cDocument::GetControlChar(POSITION_T position)
{
    PARAGRAPH_T currentparagraphnumber = GetParagraphFromPosition(position) ;
    auto paraiter = mParagraphData.begin() + currentparagraphnumber ;
    auto iter = paraiter->pairs.begin() ;
    eModifiers ch = STYLE_END_OF_STYLES ;

    // we'll make sure its in our pairs table. If it is, we assume its in the other table(s)
    PairTable pcomp ;
    position -= paraiter->index ;
    pcomp.first = position ;

    iter = lower_bound(paraiter->pairs.begin(), paraiter->pairs.end(), pcomp, TableCompare) ;
    if(iter != paraiter->pairs.end() && !(position < iter->first))
    {
        switch(iter->second)
        {
            case TYPE_FORMAT :
                {
                    FormatPair comp ;
                    comp.first = position ;
                    auto  iter1 = lower_bound(paraiter->format.begin(), paraiter->format.end(), comp, FormatCompare) ;
                    ch = iter1->second ;
                }
                break ;

            case TYPE_TAB :
                ch = STYLE_TAB ;
                break ;

            case TYPE_COLOR :
                ch = STYLE_INTERNAL_COLOR ;
                break ;

            case TYPE_FONT :
                ch = STYLE_FONT1 ;
                break ;
                
            case TYPE_INDEX :
                ch = STYLE_INDEX ;
                break ;

            // non standard styles
            case TYPE_FOOTNOTE :
                ch = STYLE_FOOTNOTE ;
                break ;

            case TYPE_ENDNOTE :
                ch = STYLE_ENDNOTE ;
                break ;

        }
    }

    return ch ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param  para   [IN] - the paragraph to get
///
/// @return string - the paragraph in UTF8
///
/// @brief
/// dump the entire paragraph into a string. MARKER_CHAR is not converted
/// to it's STYLE_* value. If viewing with no control chars, then we skip
/// them here as well.
///
/////////////////////////////////////////////////////////////////////////////
string cDocument::GetParagraphText(PARAGRAPH_T para)
{
    string text ;

    MY_ASSERT(para >= 0)

    if(static_cast<size_t>(para) < mParagraphData.size())
    {
        for(char c: mParagraphData[para].buffer)
        {
            text += c ;
        }
    }
    return text ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param  para   [IN] - the paragraph to get
/// @param  offsets [OUT] - the offset positions
///
/// @return size_t - number of offsets
///
/// @brief
/// Copy the paragraph offsets (cached) to the passed in vector
///
/////////////////////////////////////////////////////////////////////////////
size_t cDocument::GetParagraphGraphemeOffsets(PARAGRAPH_T para, vector<POSITION_T> &offsets)
{   
/*
    offsets.clear();
    offsets.reserve(mParagraphData[static_cast<size_t>(para)].buffer.size());        // preallocate max number of offsets
    for (size_t loop = 0; loop < mParagraphData[static_cast<size_t>(para)].offsets.size()  ; loop++)
    {
        offsets.emplace_back(mParagraphData[static_cast<size_t>(para)].offsets[loop]);
    }
*/
    offsets = mParagraphData[static_cast<size_t>(para)].offsets;
    return offsets.size();
}


/////////////////////////////////////////////////////////////////////////////
///
/// @param  start   [IN] - the start of the block
/// @param  end     [IN] - the end position of the block
///
/// @return string - the block in UTF8
///
/// @brief
/// dump the marked block into a string
///
/////////////////////////////////////////////////////////////////////////////
string cDocument::GetBlockText(POSITION_T start, POSITION_T end)
{
    string ret ;

    for(POSITION_T loop = start; loop < end; loop++)
    {
        ret += GetChar(loop) ;
    }

    return ret ;
}





/////////////////////////////////////////////////////////////////////////////
///
/// @param  needle      [IN} - the string we are looking for
/// @param  start       [IN] - the position in the buffer to start the search
/// @param  wildcard    [IN] - allow wildcard search (brute force, ascii only)
/// @param  casecmp     [IN] - ignore case (brute force, ascii only)
///
/// @return POSITION_T
///
/// @brief search the buffer starting at position start for the string in
///        needle
///
/////////////////////////////////////////////////////////////////////////////
POSITION_T cDocument::FindNext(const std::string &needle, const POSITION_T &start, bool wildcard, bool casecmp, bool wholeword)
{
    string finalneedle ;
    POSITION_T offset = 0 ;

    /// normalize needle
    finalneedle = Normalize(needle) ;

    /// if no case comp
    ///    lowercase needle
    if(casecmp)
    {
        finalneedle = LowerCase(finalneedle) ;
    }

    /// get size of needle
    size_t needlesize = finalneedle.length() ;

    if(needlesize > 0)
    {
        /// for each paragraph starting at postion
        PARAGRAPH_T startpara = GetParagraphFromPosition(start) ;
        for(PARAGRAPH_T ploop = startpara; ploop < GetNumberofParagraphs(); ploop++)
        {
            ///    get para text
            string haystack = GetParagraphText(ploop)  ;

            ///    if para start < position
            ///       truncate and set offset size (in graphemes)
            if(ploop == startpara)
            {
                vector<POSITION_T> graphemeindex ;
                GraphemeCount(haystack, graphemeindex) ;

                offset = start - mParagraphData[static_cast<size_t>(ploop)].index ;
                haystack = haystack.substr(static_cast<size_t>(graphemeindex[static_cast<size_t>(offset)]), haystack.length() - static_cast<size_t>(graphemeindex[static_cast<size_t>(offset)])) ;
            }
            else
            {
                offset = 0 ;
            }

            ///    if no case comp
            ///       lowercase text
            if(casecmp)
            {
                haystack = LowerCase(haystack) ;
            }

            /// normalize text
            haystack = Normalize(haystack) ;
            /// get text size
            size_t textsize = haystack.length() ;
            POSITION_T pos = static_cast<POSITION_T>(textsize - 1) ;


            /// if text size > needle size and needle size > 0
            if(textsize > needlesize)
            {
                if(!wholeword && !wildcard)
                {
                    size_t j = haystack.find(finalneedle) ;

                    if(j != string::npos)
                    {
                        vector<POSITION_T> offsets ;
                        GraphemeCount(haystack, offsets) ;
                        for(size_t ctr = 0; ctr < offsets.size(); ctr++)
                        {
                            if(offsets[ctr] >= static_cast<POSITION_T>(j))
                            {
                                pos = static_cast<POSITION_T>(offsets[ctr]) ;
                                return pos + static_cast<POSITION_T>(offset + mParagraphData[static_cast<size_t>(ploop)].index) ;
                            }
                        }
                    }
                }
                // honestly, on my dev box and doing a naive search in a 110K word novel, this is plenty fast
                else         // naive search
                {
                    POSITION_T rpos = static_cast<POSITION_T>(NaiveSearch(haystack, finalneedle, wildcard, wholeword)) ;
                    if(static_cast<size_t>(rpos) != string::npos)
                    {
                        return rpos + offset + mParagraphData[static_cast<size_t>(ploop)].index ;
                    }
                }
            }
        }
    }

    return GetTextSize() ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param  needle      [IN} - the string we are looking for
/// @param  start       [IN] - the position in the buffer to start the search
/// @param  wildcard    [IN] - allow wildcard search (brute force, ascii only)
/// @param  casecmp     [IN] - ignore case (brute force, ascii only)
/// @param  wholeword   [IN] - find only whole words
///
/// @return POSITION_T
///
/// @brief search the buffer backwards starting at position start for the string in
///        needle
///
/////////////////////////////////////////////////////////////////////////////
POSITION_T cDocument::FindPrev(const std::string &needle, const POSITION_T &start, bool wildcard, bool casecmp, bool wholeword)
{
    string finalneedle;
    POSITION_T offset = 0;

    /// normalize needle
    finalneedle = Normalize(needle);

    /// if no case comp
    ///    lowercase needle
    if (casecmp)
    {
        finalneedle = LowerCase(finalneedle);
    }

    /// get size of needle
    size_t needlesize = finalneedle.length();

    if (needlesize > 0)
    {
        /// for each paragraph starting at postion
        PARAGRAPH_T startpara = GetParagraphFromPosition(start);
        for (PARAGRAPH_T ploop = startpara; ploop >= 0 ; ploop--)
        {
            ///    get para text
            string haystack = GetParagraphText(ploop);

            ///    if para start < position
            ///       truncate and set offset size (in graphemes)
            if (ploop == startpara)
            {
                vector<POSITION_T> graphemeindex;
                GraphemeCount(haystack, graphemeindex);

//                offset = start - mParagraphData[static_cast<size_t>(ploop)].index;
//                haystack = haystack.substr(static_cast<size_t>(graphemeindex[static_cast<size_t>(offset)]), haystack.length() - static_cast<size_t>(graphemeindex[static_cast<size_t>(offset)]));
                haystack = haystack.substr(0, start - 1);
            }
            else
            {
                offset = 0;
            }

            /// get text size
            size_t textsize = haystack.length();
            POSITION_T pos = static_cast<POSITION_T>(textsize - 1);


            /// if text size > needle size and needle size > 0
            if (textsize >= needlesize)
            {
                if (!wholeword && !wildcard)
                {
                    size_t j = haystack.rfind(finalneedle);

                    if (j != string::npos)
                    {
                        vector<POSITION_T> offsets;
                        GraphemeCount(haystack, offsets);
                        for (size_t ctr = 0; ctr < offsets.size(); ctr++)
                        {
                            if (offsets[ctr] >= static_cast<POSITION_T>(j))
                            {
                                pos = static_cast<POSITION_T>(offsets[ctr]);
                                return pos + static_cast<POSITION_T>(offset + mParagraphData[static_cast<size_t>(ploop)].index);
                            }
                        }
                    }
                }
                // honestly, on my dev box and doing a naive search in a 110K word novel, this is plenty fast
                // modern computers a fast
                else         // naive search
                {
                    POSITION_T rpos = static_cast<POSITION_T>(NaiveBackwardsSearch(haystack, finalneedle, wildcard, wholeword));
                    if (static_cast<size_t>(rpos) != string::npos)
                    {
                        return rpos + offset + mParagraphData[static_cast<size_t>(ploop)].index ;
                    }
                }
            }
        }
    }

    return start;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @param  position    [IN] - the position in the buffer
///
/// @return nothing
///
/// @brief Delete a tab entry from the tabs table
///
/// This should never be called directly, only from Delete()
/////////////////////////////////////////////////////////////////////////////
void cDocument::DeleteTab(POSITION_T position)
{
    MY_ASSERT(position >= 0)
    MY_ASSERT(position <= GetTextSize())

    // we'll make sure its in our pairs table. If it is, we assume its in the other table(s)
    PairTable pcomp ;
    PARAGRAPH_T currentparagraphnumber = GetParagraphFromPosition(position) ;
    auto paraiter = mParagraphData.begin() + currentparagraphnumber ;
    position -= paraiter->index ;
    pcomp.first = position ;

    auto iter = lower_bound(paraiter->pairs.begin(), paraiter->pairs.end(), pcomp, TableCompare) ;
    if(iter != paraiter->pairs.end())
    {
        pcomp.second = iter->second ;

        if(iter->first == position)
        {
            if(iter->second == TYPE_TAB)
            {
                TabPair t1 ;
                t1.first = position ;

                auto iter1 = lower_bound(paraiter->tab.begin(), paraiter->tab.end(), t1, TabCompare) ;
                if(iter1->first == position)
                {
//                    PARAGRAPH_T index = static_cast<PARAGRAPH_T>(distance(paraiter->tab.begin(), iter1)) ;
                    paraiter->tab.erase(iter1) ;
//                    index = static_cast<PARAGRAPH_T>(distance(paraiter->pairs.begin(), iter)) ;
                    paraiter->pairs.erase(iter) ;
                }
            }
        }
    }
}




/////////////////////////////////////////////////////////////////////////////
///
/// @param  position    [IN] - the position in the buffer
///
/// @return nothing
///
/// @brief Delete a font entry from the fonts table
///
/// This should never be called directly, only from Delete()
/////////////////////////////////////////////////////////////////////////////
void cDocument::DeleteFont(POSITION_T position)
{
    MY_ASSERT(position >= 0)
    MY_ASSERT(position <= GetTextSize())

    // we'll make sure its in our pairs table. If it is, we assume its in the other table(s)
    PairTable pcomp ;
    PARAGRAPH_T currentparagraphnumber = GetParagraphFromPosition(position) ;
    auto paraiter = mParagraphData.begin() + currentparagraphnumber ;
    position -= paraiter->index ;
    pcomp.first = position ;

    auto iter = lower_bound(paraiter->pairs.begin(), paraiter->pairs.end(), pcomp, TableCompare) ;
    if(iter != paraiter->pairs.end())
    {
        pcomp.second = iter->second ;

        if(iter->first == position)
        {
            if(iter->second == TYPE_FONT)
            {
                FontPair t1 ;
                t1.first = position ;

                auto iter1 = lower_bound(paraiter->font.begin(), paraiter->font.end(), t1, FontCompare) ;
                if(iter1->first == position)
                {
//                    PARAGRAPH_T index = static_cast<PARAGRAPH_T>(distance(paraiter->font.begin(), iter1)) ;
                    paraiter->font.erase(iter1) ;
//                  index = static_cast<PARAGRAPH_T>(distance(paraiter->pairs.begin(), iter)) ;
                    paraiter->pairs.erase(iter) ;
                }
            }
        }
    }
}




/////////////////////////////////////////////////////////////////////////////
///
/// @param  position    [IN] - the position in the buffer
///
/// @return nothing
///
/// @brief Delete a color entry from the colorss table
///
/// This should never be called directly, only from Delete()
/////////////////////////////////////////////////////////////////////////////
void cDocument::DeleteColor(POSITION_T position)
{
    MY_ASSERT(position >= 0)
    MY_ASSERT(position <= GetTextSize())

    // we'll make sure its in our pairs table. If it is, we assume its in the other table(s)
    PairTable pcomp ;
    PARAGRAPH_T currentparagraphnumber = GetParagraphFromPosition(position) ;
    auto paraiter = mParagraphData.begin() + currentparagraphnumber ;
    position -= paraiter->index ;
    pcomp.first = position ;

    auto iter = lower_bound(paraiter->pairs.begin(), paraiter->pairs.end(), pcomp, TableCompare) ;
    if(iter != paraiter->pairs.end())
    {
        pcomp.second = iter->second ;

        if(iter->first == position)
        {
            if(iter->second == TYPE_COLOR)
            {
                ColorPair t1 ;
                t1.first = position ;

                auto iter1 = lower_bound(paraiter->color.begin(), paraiter->color.end(), t1, ColorCompare) ;
                if(iter1->first == position)
                {
//                  PARAGRAPH_T index = static_cast<PARAGRAPH_T>(distance(paraiter->color.begin(), iter1)) ;
                    paraiter->color.erase(iter1) ;
//                    index = static_cast<PARAGRAPH_T>(distance(paraiter->pairs.begin(), iter)) ;
                    paraiter->pairs.erase(iter) ;
                }
            }
        }
    }
}




/////////////////////////////////////////////////////////////////////////////
///
/// @param  position    [IN] - the position in the buffer
///
/// @return nothing
///
/// @brief Delete a control char entry from the control chars table
///
/// This should never be called directly, only from Delete()
/////////////////////////////////////////////////////////////////////////////
void cDocument::DeleteControlChar(POSITION_T position)
{
    MY_ASSERT(position  >= 0)
    MY_ASSERT(position <= GetTextSize())

    // we'll make sure its in our pairs table. If it is, we assume its in the other table(s)
    PairTable pcomp ;
    PARAGRAPH_T currentparagraphnumber = GetParagraphFromPosition(position) ;
    auto paraiter = mParagraphData.begin() + currentparagraphnumber ;
    position -= paraiter->index ;
    pcomp.first = position ;

    auto iter = lower_bound(paraiter->pairs.begin(), paraiter->pairs.end(), pcomp, TableCompare) ;
    if(iter != paraiter->pairs.end())
    {
        pcomp.second = iter->second ;

        if(iter->first == position)
        {
            if(iter->second == TYPE_FORMAT)
            {
                FormatPair t1 ;
                t1.first = position ;

                auto iter1 = lower_bound(paraiter->format.begin(), paraiter->format.end(), t1, FormatCompare) ;
                if(iter1->first == position)
                {
//                    PARAGRAPH_T index = static_cast<PARAGRAPH_T>(distance(paraiter->format.begin(), iter1)) ;
                    paraiter->format.erase(iter1) ;
//                    index = static_cast<PARAGRAPH_T>(distance(paraiter->pairs.begin(), iter)) ;
                    paraiter->pairs.erase(iter) ;
                }
            }
        }
    }
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param  position    [IN] - the position in the buffer
///
/// @return nothing
///
/// @brief Delete a paragraph from the document and merge para data
///
/// This should never be called directly, only from Delete()
/////////////////////////////////////////////////////////////////////////////
void cDocument::DeleteParagraph(POSITION_T position)
{
    MY_ASSERT(position >= 0)
    MY_ASSERT(position <= GetTextSize())

    PARAGRAPH_T currentparagraphnumber = GetParagraphFromPosition(position) ;
    auto paraiter = mParagraphData.begin() + currentparagraphnumber ;
    auto paraiternext = paraiter + 1 ; //  + currentparagraphnumber ;
//    std::advance(iter, currentparagraphnumber) ;

    if(paraiter != mParagraphData.end())
    {
        if(paraiternext != mParagraphData.end())
        {
            // change the offsets in the deleting paragraph
            POSITION_T start, end ;
            start = 0;
            end = static_cast<POSITION_T>(paraiter->buffer.size());

            POSITION_T length = end - start ;

            // increment attributes of next paragraph
            IncrementAttributes(paraiternext->index, length, false);

            // append everything from next paragraph to current paragragh ;
            paraiter->color.insert(paraiter->color.end(), paraiternext->color.begin(), paraiternext->color.end()) ;
            paraiter->font.insert(paraiter->font.end(), paraiternext->font.begin(), paraiternext->font.end()) ;
            paraiter->format.insert(paraiter->format.end(), paraiternext->format.begin(), paraiternext->format.end()) ;
            paraiter->pairs.insert(paraiter->pairs.end(), paraiternext->pairs.begin(), paraiternext->pairs.end()) ;
            paraiter->tab.insert(paraiter->tab.end(), paraiternext->tab.begin(), paraiternext->tab.end()) ;

            string nextbuffer = GetParagraphText(currentparagraphnumber + 1);
            for (size_t loop = 0; loop < nextbuffer.size(); loop++)
            {
                paraiter->buffer.push_back(nextbuffer[loop]);
            }

            mParagraphData.erase(paraiternext) ;

            vector<POSITION_T> offsets;
            SaveOffsets(currentparagraphnumber, offsets);
        }
    }
}


void cDocument::InsertParagraph(POSITION_T position, POSITION_T offset, PARAGRAPH_T paragraph)
{
    MY_ASSERT(position >= 0)
    MY_ASSERT(position <= GetTextSize())
    MY_ASSERT(paragraph >= 0)
    MY_ASSERT(paragraph < GetNumberofParagraphs())

    sParagraphData data ;
//    PARAGRAPH_T para = paragraph ; // GetParagraphFromPosition(position) ;
    PARAGRAPH_T para = GetParagraphFromPosition(position) ;
    PARAGRAPH_T currentparagraphnumber = para ;
    auto paraiter = mParagraphData.begin() + currentparagraphnumber ;

    data.index = position ; // + 1 ;                  // the index of this paragraph in the buffer

    POSITION_T start, end ;                             // start and end + 1 of paragraph
    start = 0;
    end = static_cast<POSITION_T>(mParagraphData[static_cast<size_t>(para)].buffer.size()) ;
//    position -= paraiter->index;
    para++ ;

    // if we are in the middle of a paragraph
    POSITION_T textsize = GetTextSize() ;
    if((position != textsize) && (offset != start) && (offset != end - 1)) //  && (position != start) && (position != end - 2)) // end -2 because char has alreday been added and end is always a < check
    {
// this needs to be grapheme aware
        POSITION_T length = position - paraiter->index ;

        // move format data to new paragraph and delete from old
        ColorPair c1 ;
        c1.first = position - mParagraphData[currentparagraphnumber].index ;
        auto citer = lower_bound(mParagraphData[currentparagraphnumber].color.begin(), mParagraphData[currentparagraphnumber].color.end(), c1, ColorCompare) ;
        data.color.insert(data.color.begin(), citer, mParagraphData[currentparagraphnumber].color.end()) ;
        mParagraphData[currentparagraphnumber].color.erase(citer, mParagraphData[currentparagraphnumber].color.end()) ;


        FontPair f1 ;
        f1.first = position  - mParagraphData[currentparagraphnumber].index ;
        auto fiter = lower_bound(mParagraphData[currentparagraphnumber].font.begin(), mParagraphData[currentparagraphnumber].font.end(), f1, FontCompare) ;
        data.font.insert(data.font.begin(), fiter, mParagraphData[currentparagraphnumber].font.end()) ;
        mParagraphData[currentparagraphnumber].font.erase(fiter, mParagraphData[currentparagraphnumber].font.end()) ;

        FormatPair f2 ;
        f2.first = position  - mParagraphData[currentparagraphnumber].index ;
        auto fiter2 = lower_bound(mParagraphData[currentparagraphnumber].format.begin(), mParagraphData[currentparagraphnumber].format.end(), f2, FormatCompare) ;
        data.format.insert(data.format.begin(), fiter2, mParagraphData[currentparagraphnumber].format.end()) ;
        mParagraphData[currentparagraphnumber].format.erase(fiter2, mParagraphData[currentparagraphnumber].format.end()) ;

        TabPair t1 ;
        t1.first = position  - mParagraphData[currentparagraphnumber].index ;
        auto titer = lower_bound(mParagraphData[currentparagraphnumber].tab.begin(), mParagraphData[currentparagraphnumber].tab.end(), t1, TabCompare) ;
        data.tab.insert(data.tab.begin(), titer, mParagraphData[currentparagraphnumber].tab.end()) ;
        mParagraphData[currentparagraphnumber].tab.erase(titer, mParagraphData[currentparagraphnumber].tab.end()) ;

        PairTable p1 ;
        p1.first = position  - mParagraphData[currentparagraphnumber].index ;
        auto piter = lower_bound(mParagraphData[currentparagraphnumber].pairs.begin(), mParagraphData[currentparagraphnumber].pairs.end(), p1, TableCompare) ;
        data.pairs.insert(data.pairs.begin(), piter, mParagraphData[currentparagraphnumber].pairs.end()) ;
        mParagraphData[currentparagraphnumber].pairs.erase(piter, mParagraphData[currentparagraphnumber].pairs.end()) ;

        // the glyphs
        data.buffer = paraiter->buffer ;
        data.buffer.erase(data.buffer.begin(), data.buffer.begin() + offset) ;
//        data.buffer = paraiter->buffer.drop(static_cast<size_t>(offset)) ;
        paraiter->buffer.erase(paraiter->buffer.begin() + offset, paraiter->buffer.end()) ;
//        paraiter->buffer = paraiter->buffer.take(static_cast<size_t>(offset)) ;


        auto iter = mParagraphData.begin() ;
        std::advance(iter, para) ;

        mParagraphData.insert(iter, data) ;
        
        // now correct any formatting we may have to the right offset
        if(position != mParagraphData[static_cast<size_t>(paragraph)].index)
        {
            vector<POSITION_T> offsets;
            SaveOffsets(currentparagraphnumber, offsets);
            SaveOffsets(currentparagraphnumber + 1, offsets);

            currentparagraphnumber++ ;
            DecrementAttributes((mParagraphData.begin() + currentparagraphnumber)->index, length, false) ;
            currentparagraphnumber-- ;
        }        
    }

    // we are at the end of the document
    else if((position == textsize) || (para >= static_cast<PARAGRAPH_T>(mParagraphData.size())))
    {
        data.buffer = paraiter->buffer ;
        data.buffer.erase(data.buffer.begin(), data.buffer.begin() + offset) ;
//        data.buffer = paraiter->buffer.drop(static_cast<size_t>(offset)) ;
        paraiter->buffer.erase(paraiter->buffer.begin() + offset, paraiter->buffer.end()) ;
//        paraiter->buffer = paraiter->buffer.take(static_cast<size_t>(offset)) ;

        mParagraphData.push_back(data) ;
    }

    // we are at the start of a paragraph
    else if(position == static_cast<POSITION_T>(mParagraphData[static_cast<size_t>(para)].index))
    {
        auto iter = mParagraphData.begin() ;
        std::advance(iter, para) ;
        mParagraphData.insert(iter, data) ;

        // increment the index of all paragraphs following our insert
        iter = mParagraphData.begin() ;
        std::advance(iter, para + 2) ;
        for( ; iter != mParagraphData.end(); iter++)
        {
            iter->index++ ;
        }
    }

    // we are at the end of a line
    else
    {
        data.buffer = paraiter->buffer ;
        data.buffer.erase(data.buffer.begin(), data.buffer.begin() + offset) ;
//        data.buffer = paraiter->buffer.drop(static_cast<size_t>(offset)) ;
        paraiter->buffer.erase(paraiter->buffer.begin() + offset, paraiter->buffer.end()) ;
//        paraiter->buffer = paraiter->buffer.take(static_cast<size_t>(offset)) ;

        auto iter = mParagraphData.begin() ;
        std::advance(iter, para) ;
        mParagraphData.insert(iter, data) ;
    }

    vector<POSITION_T> offsets;
    SaveOffsets(currentparagraphnumber, offsets);
    SaveOffsets(currentparagraphnumber + 1, offsets);
}


size_t glength = 0 ;  /// needed for call to lambdas
/////////////////////////////////////////////////////////////////////////////
///
/// @param  position    [IN] - the position in the buffer
/// @param  length      [IN] - the length to decrement
/// @param  changeparaindex [IN] - defaults to true
///
/// @return nothing
///
/// @brief Increment the attributes (modifiers, tab, colors, etc) for the current paragraph
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::IncrementAttributes(POSITION_T position, POSITION_T length, bool changeparaindex)
{
    MY_ASSERT(position >= 0)
    MY_ASSERT(position <= GetTextSize())

    POSITION_T orgpos = position ;
    glength = static_cast<size_t>(length) ;
    PARAGRAPH_T currentparagraphnumber = GetParagraphFromPosition(position) ;
    auto paraiter = mParagraphData.begin() + currentparagraphnumber ;
    position -= paraiter->index ;
    
    TabPair comp ;
    comp.first = position ;
    auto iter = lower_bound(paraiter->tab.begin(), paraiter->tab.end(), comp, TabCompare) ;
    auto iterend = paraiter->tab.end() ;
    for_each(iter, iterend, [](TabPair &x){x.first += glength; return x;}) ;


    FormatPair comp1 ;
    comp1.first = position ;
    auto iter1 = lower_bound(paraiter->format.begin(), paraiter->format.end(), comp1, FormatCompare) ;
    auto iter1end = paraiter->format.end() ;
    for_each(iter1, iter1end, [](FormatPair &x){x.first += glength; return x;}) ;

    FontPair comp2 ;
    comp2.first = position ;
    auto iter2 = lower_bound(paraiter->font.begin(), paraiter->font.end(), comp2, FontCompare) ;
    auto iter2end = paraiter->font.end() ;
    for_each(iter2, iter2end, [](FontPair &x){x.first += glength; return x;}) ;


    // Wordstar style colors
    ColorPair comp3 ;
    comp3.first = position ;
    auto iter3 = lower_bound(paraiter->color.begin(), paraiter->color.end(), comp3, ColorCompare) ;
    auto iter3end = paraiter->color.end() ;
    for_each(iter3, iter3end, [](ColorPair &x){x.first += glength; return x;}) ;

    // Footnotes
    FootnotePair comp4 ;
    comp4.first = position ;
    auto iter4 = lower_bound(paraiter->footnote.begin(), paraiter->footnote.end(), comp4, FootnoteCompare) ;
    auto iter4end = paraiter->footnote.end() ;
    for_each(iter4, iter4end, [](FootnotePair &x){x.first += glength; return x;}) ;

    // Endnotes
    EndnotePair comp5 ;
    comp5.first = position ;
    auto iter5 = lower_bound(paraiter->endnote.begin(), paraiter->endnote.end(), comp5, EndnoteCompare) ;
    auto iter5end = paraiter->endnote.end() ;
    for_each(iter5, iter5end, [](EndnotePair &x){x.first += glength; return x;}) ;

    PairTable comp10 ;
    comp10.first = position ;
    auto iter10 = lower_bound(paraiter->pairs.begin(), paraiter->pairs.end(), comp10, TableCompare) ;
    auto iter10end = paraiter->pairs.end() ;
    for_each(iter10, iter10end, [](PairTable &x){x.first += glength; return x;}) ;

    // increment paragraph index value
    if(changeparaindex)
    {
        mTempParagraph.index =  GetPosition() ;
        auto iter5 = upper_bound(mParagraphData.begin(), mParagraphData.end(), mTempParagraph, ParagraphCompare) ;
        for(;iter5 != mParagraphData.end(); iter5++)
        {
            if(iter5->index != 0)                   // never increment the first index position
            {
                iter5->index += length ;
            }
        }
    }
    
    if(mEndBlock > mStartBlock)
    {
        if(orgpos >= mStartBlock && orgpos <= mEndBlock)
        {
            mEndBlock++ ;
        }
        if(orgpos < mStartBlock)
        {
            mStartBlock++ ;
            mEndBlock++ ;
        }
    }

    if(mOldEndBlock > mOldStartBlock)
    {
        if(orgpos >= mOldStartBlock && orgpos <= mOldEndBlock)
        {
            mOldEndBlock++ ;
        }
        if(orgpos < mOldStartBlock)
        {
            mOldStartBlock++ ;
            mOldEndBlock++ ;
        }
    }

    // and finally, any saved positions
    for(int loop = 0; loop < 10; loop++)
    {
        if(mSavePosition[loop] != 0)
        {
            if(mSavePosition[loop] > orgpos)
                mSavePosition[loop]++ ;
            {
            }
        }
    }
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param  position    [IN] - the position in the buffer
/// @param  length      [IN] - the length to decrement
/// @param  changeparaindex [IN] - defaults to true
///
/// @return nothing
///
/// @brief Decrement the attributes (modifiers, tab, colors, etc) for the document
///        if changeparaindex is false, we are calling this function because of a
///        paragraph being split into two, and we don't change any of the lower
///        paragraphs index values.
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::DecrementAttributes(POSITION_T position, POSITION_T length, bool changeparaindex)
{
    MY_ASSERT(position >= 0)
    MY_ASSERT(position <= GetTextSize())

    glength = static_cast<size_t>(length) ;
    TabPair comp ;
    POSITION_T orgpos = position ;
    PARAGRAPH_T currentparagraphnumber = GetParagraphFromPosition(position) ;
    auto paraiter = mParagraphData.begin() + currentparagraphnumber ;
    position -= paraiter->index ;
        
    comp.first = position ;
    auto iter = lower_bound(paraiter->tab.begin(), paraiter->tab.end(), comp, TabCompare) ;
    auto iterend = paraiter->tab.end() ;
    for_each(iter, iterend, [](TabPair &x){x.first -= glength; return x;}) ;

    FormatPair comp1 ;
    comp1.first = position ;
    auto iter1 = lower_bound(paraiter->format.begin(), paraiter->format.end(), comp1, FormatCompare) ;
    auto iter1end = paraiter->format.end() ;
    for_each(iter1, iter1end, [](FormatPair &x){x.first -= glength; return x;}) ;

    FontPair comp2 ;
    comp2.first = position ;
    auto iter2 = lower_bound(paraiter->font.begin(), paraiter->font.end(), comp2, FontCompare) ;
    auto iter2end = paraiter->font.end() ;
    for_each(iter2, iter2end, [](FontPair &x){x.first -= glength; return x;}) ;

    // Wordstar style colors
    ColorPair comp3 ;
    comp3.first = position ;
    auto iter3 = lower_bound(paraiter->color.begin(), paraiter->color.end(), comp3, ColorCompare) ;
    auto iter3end = paraiter->color.end() ;
    for_each(iter3, iter3end, [](ColorPair &x){x.first -= glength; return x;}) ;

    PairTable comp4 ;
    comp4.first = position ;
    auto iter4 = lower_bound(paraiter->pairs.begin(), paraiter->pairs.end(), comp4, TableCompare) ;
    auto iter4end = paraiter->pairs.end() ;
    for_each(iter4, iter4end, [](PairTable &x){x.first -= glength; return x;}) ;

    // decrement paragraph index value
    if(changeparaindex)
    {
        sParagraphData para ;
        para.index = GetPosition() ;
        auto iter5 = lower_bound(mParagraphData.begin(), mParagraphData.end(), para, ParagraphCompare) ;

        for(;iter5 != mParagraphData.end(); iter5++)
        {
            if(iter5->index != 0)                   // never decrement the first position
            {
                if(iter5->index != para.index)
                {
                    iter5->index -= length ;
                }
            }
        }

        // don't decrement the first paragraph
//        if (iter5->index == 0)
//        {
//            iter5++;
//        }

//        auto decrementindex = [](int i, vector<sParagraphData>::iterator start, vector<sParagraphData>::iterator end)
//        {
//            for(auto loop = start; loop < end; loop++)
//            {
//                loop->index -= glength ;
//            }
//        } ;

//        decrementindex(iter5, iterend) ;
//        }
/*        else
        {

            size_t loopsize = plength / mMaxThreads ;
//printf(" %d threads of %d size  (num paragraphs to do is %d)\n", mMaxThreads, loopsize, plength) ;

            // throw the threads into the pools queue
            for(auto startthread = 1; startthread < mMaxThreads; startthread++)
            {
//printf("thread %d  start %d  end %d\n", startthread, start, start + loopsize) ;
                mThreadPool.push(decrementindex, mParagraphData, start, start + loopsize) ;
                start += loopsize ;
            }
//printf("thread 8  start %d  end %d\n", start, end) ;
            mThreadPool.push(decrementindex, mParagraphData, start, end) ;

            // wait until all threads are done
            while(mThreadPool.n_idle() < mMaxThreads)
            {
//                std::this_thread::sleep_for(std::chrono::milliseconds(1)) ;
                std::this_thread::yield() ;
            }
        }
*/
    }
    
    if(mEndBlock > mStartBlock)
    {
        if(orgpos >= mStartBlock && orgpos <= mEndBlock)
        {
            mEndBlock-- ;
        }
        if(orgpos < mStartBlock)
        {
            mStartBlock-- ;
            mEndBlock-- ;
        }
    }

    // and finally, any saved positions
    for(int loop = 0; loop < 10; ++loop)
    {
        if(mSavePosition[loop] != 0)
        {
            if(mSavePosition[loop] > orgpos)
            {
                mSavePosition[loop]-- ;
            }
        }
    }
}



size_t cDocument::NaiveSearch(string &haystack, string needle, bool wildcard, bool wholeword)
{
    vector<POSITION_T> haystackoffsets ;
    size_t hcount = GraphemeCount(haystack, haystackoffsets) ;

    // if we are looking for a whole word, add the buffers
    if(wholeword)
    {
        if(needle.length() + 2 < haystack.length())
        {
            needle.insert(0, "?") ;
            needle.append("?") ;
//            wildcard = true ;
        }
    }

    vector<POSITION_T> needleoffsets ;
    size_t ncount = GraphemeCount(needle, needleoffsets) ;

    for(size_t gloop = 0; gloop < hcount - ncount; gloop++)
    {
        size_t nloop ;
        for(nloop = 0; nloop < ncount; nloop++)
        {
            string hgrapheme, ngrapheme ;

            // build haystack grapheme
            POSITION_T end ;
            if(nloop < ncount - 1)
            {
                end = needleoffsets[nloop + 1] ;
            }
            else
            {
                end = static_cast<POSITION_T>(needle.size()) ;
            }

            for(POSITION_T loop = needleoffsets[nloop]; loop < end; loop++)
            {
                ngrapheme += needle[static_cast<size_t>(loop)] ;
            }

            // build needle grapheme
            end = haystackoffsets[gloop + 1 + nloop] ; // - haystackoffsets[gloop] ;

            for(POSITION_T loop = haystackoffsets[gloop + nloop]; loop < end; loop++)
            {
                hgrapheme += haystack[static_cast<size_t>(loop)] ;
            }

            if(ngrapheme != "?")
            {
                if(hgrapheme != ngrapheme)
                {
                    break ;
                }
            }

            if(nloop == ncount - 1)
            {
                if(!wholeword)
                {
                    return gloop ;
                }
                else
                {
                    // build our found string, plus one grapheme out each side
                    size_t fend  = needle.length();
                    string found ;
                    for(size_t loop = gloop; loop < gloop + fend; loop++)
                    {
                        found += haystack[loop] ;
                    }

                    vector<utf8proc_int32_t> codepoints ;
                    size_t cplen = GetCodePoints(found, codepoints) ;

                    size_t loop ;
                    for(loop = 0 ; loop < cplen; loop++)
                    {
                        utf8proc_category_t category ;

                        if(loop == 0)
                        {
                            category = utf8proc_category(codepoints[loop]) ;
                            if(category != UTF8PROC_CATEGORY_ZS && category != UTF8PROC_CATEGORY_ZL && category != UTF8PROC_CATEGORY_ZP)
                            {
                                break ;
                            }
                        }
                        else if(loop == cplen - 1)
                        {
                            category = utf8proc_category(codepoints[loop]) ;
                            if(category != UTF8PROC_CATEGORY_ZS && category != UTF8PROC_CATEGORY_ZL && category != UTF8PROC_CATEGORY_ZP && codepoints[loop] != HARD_RETURN)
                            {
                                break ;
                            }
                        }
                        else
                        {
                            category = utf8proc_category(codepoints[loop]) ;
                            if(category == UTF8PROC_CATEGORY_ZS || category == UTF8PROC_CATEGORY_ZL || category == UTF8PROC_CATEGORY_ZP)
                            {
                                break ;
                            }
                        }
                    }
/*
                    // check for break in middle
                    size_t loop ;

                    // make suer there are no breas midword (wild card)
                    for(loop = gloop; loop < gloop + needle.length(); loop++)
                    {
                        utf8proc_category_t category = utf8proc_category(codepoints[loop]) ;
                        if(category == UTF8PROC_CATEGORY_ZS || category == UTF8PROC_CATEGORY_ZL || category == UTF8PROC_CATEGORY_ZP)
                        {
                            break ;
                        }
                    }
*/
                    if(wholeword)
                    {
                        gloop++ ;
                    }

                    if(loop == cplen)
                    {
                        return gloop ;
                    }

                    break ;
                }
            }
        }
    }
    return string::npos ;
}



size_t cDocument::NaiveBackwardsSearch(string& haystack, string needle, bool wildcard, bool wholeword)
{
    vector<POSITION_T> haystackoffsets;
    size_t hcount = GraphemeCount(haystack, haystackoffsets);

    // if we are looking for a whole word, add the buffers
    if (wholeword)
    {
        if (needle.length() + 2 < haystack.length())
        {
            needle.insert(0, "?");
            needle.append("?");
            wildcard = true;
        }
    }

    vector<POSITION_T> needleoffsets;
    size_t ncount = GraphemeCount(needle, needleoffsets);

//    for (size_t gloop = 0; gloop < hcount - ncount; gloop++)
    for (size_t gloop = hcount  - 1; gloop >= ncount; gloop--)
    {
        size_t nloop;
        for (nloop = ncount - 1; nloop >= 0; nloop--)
        {
            string hgrapheme, ngrapheme;

            // build needle grapheme
            POSITION_T end;
            if (nloop < ncount - 1)
            {
                end = needleoffsets[nloop + 1];
            }
            else
            {
                end = static_cast<POSITION_T>(needle.size());
            }

            for (POSITION_T loop = needleoffsets[nloop]; loop < end; loop++)
            {
                ngrapheme += needle[static_cast<size_t>(loop)];
            }

            // build haystack grapheme
//            end = haystackoffsets[gloop - nloop]; // - haystackoffsets[gloop] ;

            for (POSITION_T loop = haystackoffsets[gloop - nloop]; loop < haystackoffsets[gloop]; loop++)
            {
                hgrapheme += haystack[static_cast<size_t>(loop)];
            }

            if (ngrapheme != "?")
            {
                if (hgrapheme != ngrapheme)
                {
                    break;
                }
            }

            if (nloop == ncount - 1)
            {
                if (!wholeword)
                {
                    return gloop - nloop ;
                }
                else
                {
                    // build our found string, plus one grapheme out each side
                    size_t fend = needle.length();
                    string found;
                    for (size_t loop = gloop - nloop; loop < gloop - nloop + fend; loop++)
                    {
                        found += haystack[loop];
                    }

                    vector<utf8proc_int32_t> codepoints;
                    size_t cplen = GetCodePoints(found, codepoints);

                    size_t loop;
                    for (loop = 0; loop < cplen; loop++)
                    {
                        utf8proc_category_t category;

                        if (loop == 0)
                        {
                            category = utf8proc_category(codepoints[loop]);
                            if (category != UTF8PROC_CATEGORY_ZS && category != UTF8PROC_CATEGORY_ZL && category != UTF8PROC_CATEGORY_ZP)
                            {
                                break;
                            }
                        }
                        else if (loop == cplen - 1)
                        {
                            category = utf8proc_category(codepoints[loop]);
                            if (category != UTF8PROC_CATEGORY_ZS && category != UTF8PROC_CATEGORY_ZL && category != UTF8PROC_CATEGORY_ZP && codepoints[loop] != HARD_RETURN)
                            {
                                break;
                            }
                        }
                        else
                        {
                            category = utf8proc_category(codepoints[loop]);
                            if (category == UTF8PROC_CATEGORY_ZS || category == UTF8PROC_CATEGORY_ZL || category == UTF8PROC_CATEGORY_ZP)
                            {
                                break;
                            }
                        }
                    }
                    /*
                                        // check for break in middle
                                        size_t loop ;

                                        // make suer there are no breas midword (wild card)
                                        for(loop = gloop; loop < gloop + needle.length(); loop++)
                                        {
                                            utf8proc_category_t category = utf8proc_category(codepoints[loop]) ;
                                            if(category == UTF8PROC_CATEGORY_ZS || category == UTF8PROC_CATEGORY_ZL || category == UTF8PROC_CATEGORY_ZP)
                                            {
                                                break ;
                                            }
                                        }
                    */
                    if (wholeword)
                    {
                        gloop++;
                    }
                    if (wildcard)
                    {
                        gloop--;
                    }

                    if (loop == cplen)
                    {
                        return gloop - nloop ;
                    }

                    break;
                }
            }
        }
    }
    return string::npos;
}



double Min( double fR, double fG, double fB )
{
    double fMin = fR;
    if (fG < fMin)
    {
        fMin = fG;
    }
    if (fB < fMin)
    {
        fMin = fB;
    }
    return fMin;
}


double Max( double fR, double fG, double fB)
{
    double fMax = fR;
    if (fG > fMax)
    {
        fMax = fG;
    }
    if (fB > fMax)
    {
        fMax = fB;
    }
    return fMax;
}

/////////////////////////////////////////////////////////////////////////////
///
/// @param  red         [IN] - the red value
/// @param  green       [IN] - the green value
/// @param  blue        [IN] = the blue value
/// @param  h           [OUT]
/// @param  s           [OUT]
/// @param  l           [OUT]
///
/// @return nothing
///
/// @brief Convert RGB to HSL
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::ConvertToHSL(int red, int green, int blue, double &h, double &s, double &l)
{
    double r = red / 255.0 ;
    double g = green / 255.0 ;
    double b = blue / 255.0 ;
    
    double min = Min(r, g, b) ;
    double max = Max(r, g, b) ;
    
    l = 50 * (min + max) ;
    
    if(FuzzyCompare(min, max))   // min == max)
    {
        s = 0 ;
        h = 0 ;
    }
    else 
    {
        s = 0 ;
        h = 0 ;
        if(l < 50.0)
        {
            s = 100.0 * (max - min) / (max + min) ;
        }
        else
        {
            s = 100.0 * (max - min) / (2.0 - max - min) ;
        }
        
        if(FuzzyCompare(max, r))  // max == r)
        {
            h = 60.0 * (g - b) / (max - min) ;
        }
        if(FuzzyCompare(max, g))   // max == g)
        {
            h = 60.0 * (b - r) / (max - min) + 120 ;
        }
        if(FuzzyCompare(max, b))   //max == b)
        {
            h = 60.0 * (r - g) / (max - min) + 240 ;
        }

        if(h < 0)
        {
            h += 360 ;
        }
    }
}



double cDocument::CalculateDistance(double &h, double &s, double &l, double &h1, double &s1, double &l1) 
{
    return pow(h - h1, 2) +
            pow(s - s1, 2) +
            pow(l -l1, 2);
}





/////////////////////////////////////////////////////////////////////////////
///
/// @param  paragraph   [IN] - the paragraph to work on
/// @param  offset      [OUT] - the grapheme offsets of the paragraph
///
/// @return nothing
/// @brief get the grapheme offsets of the paragraph and save them for later
///
/////////////////////////////////////////////////////////////////////////////
void cDocument::SaveOffsets(PARAGRAPH_T paragraph, vector<POSITION_T>& offsets)
{
    /// @todo we actually do the entire paragraph. No optimizations yet.
    GraphemeCount(mParagraphData[paragraph].buffer, offsets);

    mParagraphData[paragraph].offsets = offsets ;
}
