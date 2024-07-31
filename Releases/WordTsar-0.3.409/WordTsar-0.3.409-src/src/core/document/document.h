#ifndef DOCUMENT_H
#define DOCUMENT_H

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


#include <string>
#include <vector>
#include <deque>
#include <algorithm>


#include "utf8proc-2.5.0/utf8proc.h"

#include "src/core/include/config.h"
//#include "../include/enums.h"
#include "doctstructs.h"
#include "math.h"
//#include "../files/rtf/read/rtfparser.h"

struct sColorTable ;
struct sFontTable ;

// using namespace std ;  REMOVED BECAUSE VISUAL STUDIO 2019 IS BORKED

using PairTable = std::pair<POSITION_T, eType> ;
bool TableCompare(const PairTable &first, const PairTable &second) ;

using TabPair = std::pair<POSITION_T, sWSTab > ;
bool TabCompare(const TabPair &firstElem, const TabPair &secondElem) ;

using FormatPair = std::pair<POSITION_T, eModifiers > ;
bool FormatCompare(const FormatPair &first, const FormatPair &second) ;

using ColorPair = std::pair<POSITION_T, sWSColor> ;
bool ColorCompare(const ColorPair &first, const ColorPair &second) ;

using FontPair = std::pair<POSITION_T, sInternalFonts> ;
bool FontCompare(const FontPair &first, const FontPair &second) ;

using IndexPair = std::pair<POSITION_T, std::string> ;
bool IndexCompare(const IndexPair &first, const IndexPair &second) ;

using FootnotePair = std::pair<POSITION_T, sNote> ;
bool FootnoteCompare(const FootnotePair &first, const FootnotePair &second) ;

using EndnotePair = std::pair<POSITION_T, sNote> ;
bool EndnoteCompare(const EndnotePair &first, const EndnotePair &second) ;

struct sParagraphData ;
bool ParagraphCompare(const sParagraphData &first, const sParagraphData &second) ;

// declare a memory policy for using a tracing garbage collector


/// describes a complete paragraph
struct sParagraphData
{
    POSITION_T index ;                                          ///< where paragraph starts in the buffer
//    std::string buffer ;
    std::vector<char> buffer ;                           ///< the buffer for this paragraph
//    sModifierParagraph paragraph ;                              ///< end of paragraph attributes
    std::vector<PairTable> pairs ;                       ///< the pairings table
    std::vector<FormatPair> format ;                     ///< format modifiers
    std::vector<FontPair> font ;                         ///< font modifiers
    std::vector<TabPair> tab ;                           ///< tab positions and type
    std::vector<ColorPair> color ;                       ///< color modifiers
    std::vector<FootnotePair> footnote ;                 ///< footnotes
    std::vector<EndnotePair> endnote ;                   ///< endnotes

    std::vector<POSITION_T> offsets;                     ///< cache our grapheme boundarys (potential large speedup)
};


struct sColorTable
{
    int red, green, blue ;
} ;


class cDocument
{
public:
    cDocument();
    virtual ~cDocument();
    
    void Clear(void) ;

    bool Insert(CHAR_T ch) ;
    bool Insert(const std::string &text) ;
    bool Delete(POSITION_T position, POSITION_T length) ;
    std::string GetChar(POSITION_T &position) ;

    void BeginBold(void) ;
    void EndBold(void) ;
    void BeginItalics(void) ;
    void EndItalics(void) ;
    void BeginUnderline(void) ;
    void EndUnderline(void) ;
    void BeginStrikeThrough(void) ;
    void EndStrikeThrough(void) ;
    void BeginSuperscript(void) ;
    void EndSuperscript(void) ;
    void BeginSubscript(void) ;
    void EndSubscript(void) ;
    void BeginIndex(void) ;
    void EndIndex(void) ;

    void BeginCenter(void) ;
    void BeginLeft(void) ;
    void BeginRight(void) ;
    void BeginJustify(void) ;

    bool InsertTab(sWSTab &tab) ;
    bool InsertColor(sWSColor &color) ;                     // inserts a wordstar color
    bool InsertColor(sColorTable &color) ;                  // inserts an RGB color (not WS standard)
    bool InsertFont(sInternalFonts &font) ;                 // used by Wordstar file loader
    bool InsertFootnote(sNote &note) ;                      // insert a footnote
    bool InsertEndnote(sNote &note) ;                       // insert an endnote

    void SetLoading(bool loading) ;
    bool GetLoading(void) ;

    POSITION_T GetTextSize(void) ;

    POSITION_T GetPosition(void) ;
    void SetPosition(POSITION_T pos) ;

    PARAGRAPH_T GetNumberofParagraphs() ;
    void GetParagraphStartandEnd(const PARAGRAPH_T para, POSITION_T &start, POSITION_T &end) ;
    PARAGRAPH_T GetParagraphFromPosition(POSITION_T position) ;


    sWSTab GetTab(POSITION_T position) ;
    bool GetColor(POSITION_T position, sWSColor &color) ;
    bool GetFont(POSITION_T position, sInternalFonts &intfont) ;
        
    CHAR_T GetRawBufferChar(POSITION_T &position) ;         // Testing only
    eModifiers GetControlChar(POSITION_T) ;
    
    std::string GetParagraphText(PARAGRAPH_T para, bool show = false) ;
    size_t GetParagraphGraphemeOffsets(PARAGRAPH_T para, std::vector<PARAGRAPH_T> &offsets);
    std::string GetBlockText(POSITION_T start, POSITION_T end) ;

    POSITION_T FindNext(const std::string &needle, const POSITION_T &start, bool wildcard = false, bool casecmp = false, bool wholeword = false) ;
    POSITION_T FindPrev(const std::string &needle, const POSITION_T &start, bool wildcard = false, bool casecmp = false, bool wholeword = false) ;
    
    void GetFontList(std::vector<sInternalFonts> &fontlist) ;

    COORD_T ConvertToTwips(double value, char type) ;
    double GetValue(std::string txt, bool &incdec) ;
    char GetType(std::string text) ;

    void SetShowControl(eShowControl show) ;

    void SetBeginBlock(void) ;
    void SetEndBlock(void) ;

    void SaveUndo(void) ;
    bool Undo(void) ;
    bool Redo(void) ;

    size_t GraphemeCount(std::string& text, std::vector<POSITION_T>& offsets);
    size_t GraphemeCount(std::vector<char> &text, std::vector<POSITION_T>& offsets);
    size_t GetCodePoints(const std::string &text, std::vector<utf8proc_int32_t> &codepoints);

    size_t GetWordPositions(PARAGRAPH_T para, std::vector<POSITION_T> &wordstarts) ;
    POSITION_T GetNextWordPosition(POSITION_T pos) ;
    POSITION_T GetPrevWordPosition(POSITION_T pos) ;

    std::string Normalize(const std::string &str) ;
    std::string LowerCase(const std::string &str) ;

private :
    bool SetControlChar(CHAR_T ch) ;


    void DeleteTab(POSITION_T position) ;
    void DeleteColor(POSITION_T position) ;
    void DeleteFont(POSITION_T position) ;
    
    void InsertParagraph(POSITION_T position, POSITION_T offset, PARAGRAPH_T paragraph) ;
    void DeleteParagraph(POSITION_T position) ;
    

    void DeleteControlChar(POSITION_T position) ;

    void IncrementAttributes(POSITION_T position, POSITION_T length = 1, bool changeparaindex = true) ;
    void DecrementAttributes(POSITION_T, POSITION_T length = 1, bool changeparaindex = true) ;
    
    void IncrementOffsets(POSITION_T position, CHAR_T ch) ;
    
    size_t NaiveSearch(std::string& haystack, std::string needle, bool wildcard, bool wholeword);
    size_t NaiveBackwardsSearch(std::string& haystack, std::string needle, bool wildcard, bool wholeword);

    void ConvertToHSL(int red, int green, int blue, double &h, double &s, double &l) ;
    double CalculateDistance(double &h, double &s, double &l, double &h1, double &s1, double &l1) ;

    void ConvertCodepointstoParagraph(PARAGRAPH_T para, std::vector<utf8proc_int32_t> &cp) ;

    void SaveOffsets(PARAGRAPH_T paragraph, POSITION_T position, std::vector<POSITION_T>& offsets);

public:
    bool mChanged ;                                     ///< flag for 'has the file changed'

    POSITION_T mStartBlock ;                            ///< position where a selected block starts
    POSITION_T mEndBlock ;                              ///< position where a selected block ends
    POSITION_T mOldStartBlock ;                         ///< old position where a selected block starts
    POSITION_T mOldEndBlock ;                           ///< old position where a selected block ends
    bool mBlockSet ;                                    ///< true if a block is marked, else false

    POSITION_T mSavePosition[10] ;                      ///< saved positions


private:
    POSITION_T mCurrentPosition ;                       ///< the current carat postion

    std::vector<sParagraphData> mParagraphData ;             ///< the documents paragraphs
    std::deque<std::vector<sParagraphData>> mUndo ;               ///< undo stack
    std::deque<POSITION_T> mUndoCarat ;                      ///< the carat position for undo stack
    std::deque<std::vector<sParagraphData>> mRedo ;               ///< redo stack
    std::deque<POSITION_T> mRedoCarat ;                      ///< the carat position for redo stack
    sParagraphData mTempParagraph ;                     ///< speed up MULTI_BUFFER since I don't have to create a cBuffer everytime

    std::u32string mCodePoints ;                             ///< buffer to hold current paragraph codepoints
    PARAGRAPH_T mCPParagraph ;                          ///< the paragraph for the mCodePoints array

    bool mRedrawFullDisplay ;                           ///< set to true if the editor should redraw everything
    bool mIsLoading ;                                   ///< true if loading file, else false

    POSITION_T mTextSize ;                              ///< size of text in buffer
    bool mRecalcTextSize ;                              ///< flag for getting new text size or not
    
    POSITION_T mLastParagraphFromPosition ;
    PARAGRAPH_T mLastParagraphFromPositionResult ;
    PARAGRAPH_T mLastNumParagraph ;
    
    cMath mMath ;

    eShowControl mShowControl ;

//    ctpl::thread_pool mThreadPool  ;
    int mMaxThreads ;
//    TP::ThreadPool mThreadPool ;
};

#endif // DOCUMENT_H
