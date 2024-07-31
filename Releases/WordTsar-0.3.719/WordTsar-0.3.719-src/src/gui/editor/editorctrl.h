
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

#include "src/core/editor/editorbase.h"

#include "src/gui/ruler/rulerctrl.h"
#include "src/gui/layout/layout.h"
#include "src/gui/misc/overlaywidget.h"
#include "src/files/file.h"
#include "src/input/wordtsarinput.h"

class cWordTsar ;
class cWordStarInput ;
class cFile ;

const COORD_T PAGE_LINE_HEIGHT = 200.0 ;                /// how much space to leave on screen for page line drawing


enum ePreferenceTabs
{
    PT_COLOR = 1,
    PT_USER,
    PT_SCREEN
};

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



class cEditorCtrl : public QWidget, public cEditorBase
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
    void DeleteLineLeft(void) ;
    void DeleteLineRight(void) ;
    void DeleteToChar(void) ;
    
    void LineBreak(void) ;
    
    void ToggleShowControl(void) ;
    
    void NotImplemented(QString t) ;
    void InvalidCommand(std::string t) ;

    PARAGRAPH_T GetCurrentParagraph(void) ; 
    
    double GetFontScale(void) ;
    
    void GetStatus(sStatus &status) ;
    
    COORD_T FindNextTabStop(COORD_T size) ;
    
    void LayoutFullDocument(bool force = false); 
    
    void SetBeginBlock(void) ;
    void SetEndBlock(void) ;
    void SetPreviousBlock(void) ;
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

    void GotoCharacter(void) ;
    void GotoCharacterBackward(void) ;
    void GotoPage(void) ;
    void GotoFontTag(void) ;
    void GotoPreviousPosition(void) ;
    void GotoLastFindandReplace(void) ;

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
    void Preferences(int tab = 0) ;

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

    
    double mScale ;
    double mFontScale ;

    cLayout *mLayout ;                                  ///< the document layout for the screen
    cLayout *mPrintLayout ;                             ///< the document layout for printer
    PARAGRAPH_T mLayoutParagraph ;                      ///< the paragraph we are doing layout on

    cWordStarInput *mInput ;

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
    
    QRect mCaretPos ;                                   ///< where to draw the carat (not fouble to stop artifacts)
    
    sPageInfo *mPageInfo ;                              ///< the paginfo for the current caret location
    
	QFont mBaseFont;
        
    sDisplayList mCurrentDisplay ;                      ///< this is what is on our screen (optimized)
    sDisplayList mFullDisplay;                          ///< this is what a full screen looks like
    std::vector<sMouseRect> mMouseRect ;                     ///< used to map mouse clicks to positions
    std::vector<QRectF> mBlockCoords ;                       ///< used to hold rects for a marked area
    std::vector<QRectF> mSearchCoords ;                      ///< used to hold rects for a searched text

    
    QString mStatusStyle ;                     ///< set by layout engine for status control indicators
    QString mStatusFont ;                      ///< set by layout engine for status control indicators

    
};



#endif // CEDITORCTRL_H
