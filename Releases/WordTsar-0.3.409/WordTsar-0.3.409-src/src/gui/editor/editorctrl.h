
#ifndef EDITORCTRL_H
#define EDITORCTRL_H

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

#include <QWidget>
#include <QScrollBar>
#include <QTimer>
#include <QProgressDialog>

#include <string>
#include <vector>


#include "src/wordtsar.h"

#include "src/gui/ruler/rulerctrl.h"
#include "src/gui/layout/layout.h"
#include "src/gui/misc/overlaywidget.h"
#include "src/core/document/document.h"
#include "src/files/file.h"
#include "src/input/wordtsarinput.h"

class cWordTsar ;
class cWordStarInput ;
class cFile ;

const COORD_T PAGE_LINE_HEIGHT = 200.0 ;                /// how much space to leave on screen for page line drawing



// use to comunicate data to the status basr
struct sStatus
{
    QString filename ;
    long column ;
    long line ;
    long page ;
    long pagecount ;
    bool mode ;
    long wordcount ;
    long charcount ;
    bool showcontrol ;
    bool saving ;
    eHelpDisplay help ;
    bool attrib ;
    bool bold ;
    bool italic ;
    bool underline ;
    eJustification just ;
    QString style ;
    QString font ;
    size_t height ;
    size_t width ;
    char filled[10] ;
};



// this is what is displayed on screen. This uses the same structs as the
// layouts, but everything here is in screen coordinates rather than page/printer 
// coordinates
struct sDispInfo
{
    bool isComment ;
    bool isCommand ;
    bool isKnownDot ;
    COORD_T height ;
    COORD_T rightmargin ;
    sParagraphLayout paragraph ; 
    std::vector<QRectF> controls;
} ;


struct sDisplayList
{
//    std::vector<QRect> controls ;
    std::vector <sDispInfo> display ;
} ;

struct sMouseRect
{
    POSITION_T position ;
    QRectF coordinates ;
} ;



class cEditorCtrl : public QWidget
{
    Q_OBJECT

public:
    explicit cEditorCtrl(QWidget *parent = nullptr, bool ishelp = false);
    ~cEditorCtrl(void) ;

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    void Init(void) ;

    void CloseEvent(void) ;

    // Getters and Setters
    void SetRuler(cRulerCtrl *ruler) ;
    void SetFrame(cWordTsar *frame) ;

    void SetAsHelp(void) ;
    bool GetIsHelp(void) ;
    
    void SetEditorName(std::string name) ;
    std::string GetEditorName(void) ;
    void SetTitle(std::string title) ;

	void SetFont(QFont font);
	QFont GetFont(void);

    void SetWordWrap(bool wrap) ;
    bool GetWordWrap(void) ;

    void SetShowControls(eShowControl show) ;
    eShowControl GetShowControls(void) ;
    
    void SetShowDot(bool dot) ;
    bool GetShowDot(void) ;

    void SetBGroundColour(QColor colour) ;
    QColor GetBGroundColour(void) ;

    void SetTextColour(QColor colour) ;
    QColor GetTextColour(void) ;

    void SetHighlightColour(QColor colour) ;
    QColor GetHighlightColour(void) ;

    void SetDotColour(QColor colour) ;
    QColor GetDotColour(void) ;

    void SetBlockColour(QColor colour) ;
    QColor GetBlockColour(void) ;

    void SetCommentColour(QColor colour) ;
    QColor GetCommentColour(void) ;

    void SetUnknownColour(QColor colour) ;
    QColor GetUnknownColour(void) ;
    
    void SetSpecialColour(int index, QColor colour) ;
    QColor GetSpecialColour(int index) ;
        
    void SetSpecialColour0(QColor colour) ;
    QColor GetSpecialColour0(void) ;
    
    void SetMeasurement(QString measure) ;
    QString GetMeasurement(void) ;

    void InsertWordStarString(std::string &text) ;

    bool LoadFile(QString name) ;
    bool SaveFile(QString name) ;
    [[ noreturn ]] void EmergencySaveFile(void) ;

    bool IdleLayout(cLayout *useLayout, bool fromidle = false) ;
    
    void MoveDown(void) ;
    void MoveUp(void) ;
    void PageDown(void) ;
    void PageUp(void) ;
    void MoveLeft(void) ;
    void MoveRight(void) ;
    void ScrollUp(void) ;
    void ScrollDown(void) ;
    void WordLeft(void) ;
    void WordRight(void) ;
    
    void DeleteChar(void) ;
    void DeleteWordRight(void) ;
    void DeleteLine(void) ;
    
    void LineBreak(void) ;
    
    void ToggleShowControl(void) ;
    
    void NotImplemented(QString t) ;
    
    PARAGRAPH_T GetCurrentParagraph(void) ; 
    
    double GetFontScale(void) ;
    
    void GetStatus(sStatus &status) ;
    
    COORD_T FindNextTabStop(COORD_T size) ;
    
    void LayoutFullDocument(bool force = false); 
    
    void SetBeginBlock(void) ;
    void SetEndBlock(void) ;
    void CopyBlock(void) ;
    void MoveBlock(void) ;
    void DeleteBlock(void) ;
    void UpperCaseBlock(void) ;
    void LowerCaseBlock(void) ;
    void SentenceBlock(void) ;

    void UnSetBlock(void) ;
    void ToggleHideBlock(void) ;
    void ClipboardPaste(void) ;
    void ClipboardCopy(void) ;

    void SavePosition(int offset) ;
    void GotoSavePosition(int offset) ;
    
    void WordCountBlock(void) ;

    void Replace(void) ;
    void Find(void) ;
    void FindAgain(void) ;
    bool ReplaceAgain(bool noquery = false) ;
    
    void SelectFont(void) ;
    void SelectColor(void) ;

    void MoveCursorTopLeft(void) ;
    void MoveCursorBottomRight(void) ;
    void MoveCursorTopofFile(void) ;
    void MoveCursorEndofFile(void) ;
    void MoveCursorStartBlock(void) ;
    void MoveCursorEndBlock(void) ;
    void MoveCursorStartLine(void) ;
    void MoveCursorEndLine(void) ;
    
    std::vector<COORD_T> GetTabs(void) ;
    void SetTabs(std::vector<COORD_T> &tabs) ;
    
    void About(void) ;
    
    void PrintPreview(void) ;
    
    void SpellCheckDocument(void) ;
    void SpellCheckWord(void) ;
    
    void PageLayout(void) ;
    void Preferences(void) ;

    void Undo(void) ;
    void Redo(void) ;

    void FileIOProgress(int percent) ;

private:
    bool Delete(POSITION_T pos, POSITION_T length) ;

    void SetStatus(QString text, bool progress = false, int percent = 0) ;
    
    void RedrawFullDisplay(void) ;

    void PaintHelp(QPainter &painter) ;
    void CreateDisplayList(QPainter *gc) ;
    void DisplayListSegments(sLineLayout &line, QFont &font,  QPainter *gc, COORD_T x, POSITION_T &paraposition, POSITION_T caratposition, sDisplayList &newdisplay, POSITION_T &currentposition, POSITION_T caratadjust, COORD_T &lineheight, std::vector<QRectF> &rects, sParagraphLayout *paragraph);
    POSITION_T & DisplayListContolChars(sSegmentLayout &segment, size_t loop1, COORD_T x, POSITION_T & paraposition, POSITION_T caratposition, QPainter *gc, sLineLayout &line, sDisplayList &newdisplay, std::vector<QRectF> &rects, bool dot);
    bool CompareDisplay(sDispInfo &one, sDispInfo &two);

    void PaintEditor(QPainter &painter);
    void ScrollIntoView(Qt::Key key) ;

    void Copy(void) ;
    void Paste(void) ;
    void Cut(void) ;

    long WordCount(POSITION_T start, POSITION_T end) ;
    void Search(QString text, std::vector<size_t> &pos, bool whole, bool ignore, bool wild) ;

    void DrawCaret(QPainter &paint) ;
    void HideCaret(void);
    void ShowCaret(void);

    QTimer mStatusTimer ;
    QTimer *mCaratTimer ;

signals :

public slots :
        void OnStatusTimer(void) ;                          ///< handle status timer event
        void OnCaretTimer(void);                            ///< handle caret timer event
        void OnIdle(void) ;                   ///< handle idle event (layout)

protected :
        void paintEvent(QPaintEvent *event) override;
        void keyPressEvent(QKeyEvent *event) override ;
        void resizeEvent(QResizeEvent *event) override ;
        void wheelEvent(QWheelEvent *event) override ;
        void mousePressEvent(QMouseEvent *event) override ;

        void sliderChange(void) ;

public:
    cRulerCtrl *mRuler ;                                ///< the editors ruler

    QString mFileDir ;
    QString mFileName ;
    bool mFileSet ;                                     ///< true if a valid file name is set, else false
    
    double mScale ;
    double mFontScale ;

    cDocument mDocument ;                               ///< the document itslef (buffer, meta, etc)
    cLayout *mLayout ;                                  ///< the document layout for the screen
    cLayout *mPrintLayout ;                             ///< the document layout for printer
    PARAGRAPH_T mLayoutParagraph ;                      ///< the paragraph we are doing layout on

    eHelpDisplay mHelpDisplay ;                         ///< show (or not) the help display
    bool mInsertMode ;                                  ///< insert mode (true) or overwrite (false)

    POSITION_T mStartSearchBlock ;                                ///< position where a searched block starts
    POSITION_T mEndSearchBlock ;                                  ///< position where a searched block ends
    bool mSearchBlockSet ;                                    ///< true if a block is marked, else false

    QString mShortName ;                       ///< users short name
    QString mLongName ;                        ///< users long name

    eMeasurement mMeasure ;                             ///< the measurement type in eMeasurement values (ties in with mMeasurement)
    
    bool mDispScrollBar ;
    bool mDispStyleBar ;
    bool mDispStatusBar ;
    bool mDispRuler ;
    bool mAlwaysDot ;                                   ///< flag to see if we turen off dots when not showing control codes
    bool mAlwaysFlag ;                                  ///< flag to see if we turn off indicator when not showing control codes

    std::vector<cFile *> mFileTypes ;                        ///< array of file types we can load

    QPageSize::PageSizeId mPaperId ;                    ///< our current paper type
    
private:
    cWordTsar *mWordTsar ;                               ///< pointer to parent

    bool mFirstPaint;									///< flag to clear the DC on the first paint
    std::vector<COORD_T> mTabStops ;                         ///< tab positions in twips
    std::string mEditorName ;                           ///< the name of the editor

    QColor mBackgroundColour ;
    QColor mTextColour ;
    QColor mHighlightColour ;
    QColor mDotColour ;
    QColor mBlockColour ;
    QColor mCommentColour ;
    QColor mUnknownColour ;

    QColor mSpecialColour[10] ;                       ///< colour used for special comment blocks .0 - .9

    QString mMeasurement ;                             ///< 0i or 0cm or 0mm etc (ties in with mMeasure)

    QRectF mPainterRect ;

    QImage memdc ;

    QProgressDialog *mProgress ;                        ///< used for file IO progress meters
    LINE_T mFirstLine ;                                 ///< first line being displayed
    LINE_T mLastFirstLine ;                             ///< what was the first line the last time we went through
    LINE_T mDisplayLines ;                              ///< how many lines are in our window
    LINE_T mCurrentLine ;                               ///< the line number on our page
    PAGE_T mCurrentPage ;                               ///< the page we are on
    size_t mCurrentHeight ;                             ///< vertical position on page in twips
    size_t mTopMargin ;                                 ///< current page top margin
    
//#ifdef __WXGTK__
    PARAGRAPH_T mLastParagraph ;                        ///< to cleanup wxGTK carat drawing (temp)
//#endif
    QRect mCaretPos ;                                   ///< where to draw the carat (not fouble to stop artifacts)
    bool mDoDrawCaret ;
    bool mLayoutRest ;                                  ///< if true, layout rest of document
    bool mLayoutInt ;                                   ///< if true, layout was interrupted by user input
    bool mDrawFullDisplay ;                             ///< if true, redraw full display
    bool mHardReturn ;                                  ///< true if hard return done (display optimization)

    long mSaveLayoutPara ;                              ///< the paragraph we are on before interuppted
    long mWordCount ;                                   ///< word count of document
    
    sPageInfo *mPageInfo ;                              ///< the paginfo for the current caret location
    
    cWordStarInput *mInput ;

    bool mIsHelp ;                                      ///< true if editor is help display, else false
    bool mWordWrap ;                                    ///< true if word wrapping, else false
    eShowControl mShowControl ;                         ///< true if showing controls, else false
    bool mShowDot ;                                     ///< true if doing layout for printing, else false
    bool mInPrintPreview ;                              ///< true if in print preview (no editor displat updates), else false

    bool mFullScreen ;                                  ///< true if in full screen mode, else false

	QFont mBaseFont;
        
    sDisplayList mCurrentDisplay ;                      ///< this is what is on our screen (optimized)
    sDisplayList mFullDisplay;                          ///< this is what a full screen looks like
    std::vector<sMouseRect> mMouseRect ;                     ///< used to map mouse clicks to positions
    std::vector<QRectF> mBlockCoords ;                       ///< used to hold rects for a marked area
    std::vector<QRectF> mSearchCoords ;                      ///< used to hold rects for a searched text

    std::string mCopyBuffer ;                              ///< the buffer used for copy/paste

    bool mDoSearch ;                                    ///< true doing search - false doing replace
//    std::vector<size_t> mLastSearch ;                        ///< last search positions
    bool mSearchBackwards ;                             ///< are we backwards?
    bool mWholeWord ;                                   ///< find whole words only
    bool mWildCard ;                                    ///< do we have wildcards
    bool mCaseCmp ;                                     ///< ingnore case
    bool mWholeFile ;                                   ///< search the whole file
    std::string mSearchText ;                           ///< the text we are searching for

    bool mReplaceAsk ;                                  ///< ask on replace?
    POSITION_T mReplaceSize ;                           ///< size of text to cut
    std::string mReplaceText ;                             ///< text to replace with

    bool mHideCaret;                                    ///< whether to display caret or not
    bool mDrawnCaret;                                   ///< true if caret is displayed, else false
//    wxBitmap mBitmapCaret;                              ///< the data under the carat
    
    bool mStatusBold ;                          ///< set by layout engine for status control indicators
    bool mStatusItalic ;                        ///< set by layout engine for status control indicators
    bool mStatusUnderline ;                     ///< set by layout engine for status control indicators
    eJustification mStatusJust ;                ///< set by layout engine for status control indicators
    QString mStatusStyle ;                     ///< set by layout engine for status control indicators
    QString mStatusFont ;                      ///< set by layout engine for status control indicators

    bool mFirstIdle ;                           ///< force a screen redraw on first idle event
//    wxStopWatch mSW ;
    
    bool mIsReady ;                             ///< is the window ready to draw on
    
//    QColorData mColorData ;                   ///< color dialog data


};



#endif // CEDITORCTRL_H
