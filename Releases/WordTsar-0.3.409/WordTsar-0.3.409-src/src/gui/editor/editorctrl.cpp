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

#ifdef _WIN32
#define _HAS_STD_BYTE 0  // see https://developercommunity.visualstudio.com/content/problem/93889/error-c2872-byte-ambiguous-symbol.html
#endif

#include <bitset>
#include <string>

#include <QPainter>
#include <QMessageBox>
#include <QCoreApplication>
#include <QApplication>
#include <QPaintEvent>
#include <QTimer>
#include <QCursor>

#include "editorctrl.h"
#include "src/files/wordstarfile.h"
#include "src/files/textfile.h"
#include "src/files/rtffile.h"
#include "src/files/docxfile.h"

#include "src/core/include/version.h"

#include "src/gui/print/printout.h"

#include "src/gui/spellcheck/clinuxspellcheck.h"

#include "src/gui/misc/overlaywidget.h"

#include "ui_preferences.h"
#include "ui_pagelayout.h"
#include "ui_find.h"
#include "ui_findreplace.h"

//#include "../../../third-party/coroutine/coroutine.h"

#include "../../../src/third-party/simpleini/SimpleIni.h"


const long CARAT_TIMER_DELAY = 450 ;

bool FuzzyCompare(double a, double b) ;         ///< in document.cpp

cEditorCtrl::cEditorCtrl(QWidget *parent, bool ishelp) : QWidget(parent)
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(false);
    setAttribute(Qt::WA_OpaquePaintEvent);
    mIsHelp = ishelp ;
    Init() ;
}


cEditorCtrl::~cEditorCtrl(void)
{
    for(size_t loop = 0; loop < mFileTypes.size(); loop++)
    {
        delete mFileTypes[loop] ;
    }
    delete mLayout ;
    delete mInput ;

    if(mPrintLayout != nullptr)
    {
        delete mPrintLayout ;
    }
}



QSize cEditorCtrl::minimumSizeHint() const
{
    return QSize(100, 100);
}



QSize cEditorCtrl::sizeHint() const
{
    return QSize(800, 600);
}

void cEditorCtrl::paintEvent(QPaintEvent *event)
{
    const QRectF &rect = event->rect() ;
    mPainterRect = rect ;

    QPainter painter(this) ;

    mIsReady = true ;


    if(mDocument.GetLoading() == true)
    {
        return ;
    }

    if(mIsHelp)
    {
        mFirstPaint = false ;

//        QColor grey(225, 225, 225) ;
//        QBrush brush(grey) ;
//        painter.setBackground(brush);
        painter.eraseRect(rect) ;
        PARAGRAPH_T paras = mDocument.GetNumberofParagraphs() ;
        for(PARAGRAPH_T i = 0 ; i < paras; i++)
        {
            mLayout->LayoutParagraph(i) ;
        }

        PaintHelp(painter) ;
    }
    else
    {
        if (mFirstPaint)
        {
            mDrawFullDisplay = true;
            mFirstPaint = false;
        }
        if(mDoDrawCaret == true)
        {
            DrawCaret(painter);
        }
        else
        {
            PaintEditor(painter) ;
        }
    }
}

void cEditorCtrl::keyPressEvent(QKeyEvent *event)
{

    bool done = false ;
    mSearchBlockSet = false ;
    mStartSearchBlock = 0 ;
    mEndSearchBlock = 0 ;
    mLayoutInt = true ;
    mSaveLayoutPara = mLayoutParagraph ;

    Qt::KeyboardModifiers modifier = event->modifiers() ;

#ifdef Q_OS_MACOS
//    Qt::KeyboardModifiers modifier = event->modifiers() ;
//
//    if(modifier == Qt::MetaModifier)
//    {
//        return ;
//    }
#endif

    POSITION_T caratposition = mDocument.GetPosition() ;
    LINE_T cline = mLayout->GetLineFromPosition(caratposition) ;
    if((cline < mFirstLine) || (cline > mFirstLine + mDisplayLines))
    {
        mFirstLine = cline ;
    }

    mLayoutParagraph = 0 ;
//    mWordTsar->UpdateStatus(this) ;       // really slow

    int keycode = event->key() ;

    // a first switch just to block special keys we do not use
    switch(keycode)
    {
        case Qt::Key_Cancel:
        case Qt::Key_Clear :
        case Qt::Key_Shift :
        case Qt::Key_Alt :
        case Qt::Key_Control :
        case Qt::Key_Menu :
        case Qt::Key_Pause :
        case Qt::Key_CapsLock :
        case Qt::Key_Select :
        case Qt::Key_Print :
        case Qt::Key_Execute :
        case Qt::Key_Insert :
        case Qt::Key_Help :
//        case Qt::Key_F1 :
        case Qt::Key_F2 :
        case Qt::Key_F3 :
        case Qt::Key_F4 :
        case Qt::Key_F5 :
        case Qt::Key_F6 :
        case Qt::Key_F7 :
        case Qt::Key_F8 :
        case Qt::Key_F9 :
        case Qt::Key_F10 :
//        case Qt::Key_F11 :
        case Qt::Key_F12 :
        case Qt::Key_NumLock :
        case Qt::Key_ScrollLock :
        {
            return;
        }

        default :
            break ;

    }

    // these are the keys we deal with
    switch(keycode)
    {
        case Qt::Key_Return :
            mDocument.Insert(HARD_RETURN) ;
            mHardReturn = true ;
            done = true ;
            break ;

        case Qt::Key_Up :
            MoveUp() ;
            done = true ;
            break ;

        case Qt::Key_Down :
            MoveDown() ;
            done = true ;
            break ;

        case Qt::Key_Left :
            if(event->modifiers() == Qt::ControlModifier)
            {
                WordLeft() ;
            }
            else
            {
                MoveLeft() ;
            }
            done = true ;
            break ;

        case Qt::Key_Right :
            if(event->modifiers() == Qt::ControlModifier)
            {
                WordRight() ;
            }
            else
            {
                MoveRight() ;
            }
            done = true ;
            break ;

        case Qt::Key_PageDown :
            PageDown() ;
            done = true ;
            break ;

        case Qt::Key_PageUp :
            PageUp() ;
            done = true ;
            break ;

        case Qt::Key_F1 :
            Preferences() ;
            done = true ;
            break ;

        case Qt::Key_F11 :
            if(mFullScreen == false)
            {
                mFullScreen = true ;
                mWordTsar->setWindowState(Qt::WindowFullScreen);
                QCoreApplication::processEvents() ;             // fixes full screen redraw
//                mFirstLine = mLayout->GetLineFromPosition(mDocument.GetPosition()) ;
            }
            else
            {
                mFullScreen = false ;
                mWordTsar->setWindowState(Qt::WindowNoState);
                QCoreApplication::processEvents() ;             // fixes full screen redraw
//                mFirstLine = mLayout->GetLineFromPosition(mDocument.GetPosition()) ;
            }

            done = true ;
            break ;

        case Qt::Key_Backspace :
            {
               POSITION_T pos = mDocument.GetPosition( );
                if(pos != 0)
                {
                    mDocument.SetPosition(pos - 1) ;
                    Delete(pos - 1, 1) ;
                }
                done = true ;
                break ;
            }

        case Qt::Key_Home :
            MoveCursorStartLine() ;
            done = true ;
            break ;

        case Qt::Key_End :
            MoveCursorEndLine() ;
            done = true ;
            break ;

        case Qt::Key_Delete :
            DeleteChar() ;
            done = true ;
            break ;

        case Qt::Key_Tab :
        {
            sWSTab tab ;
            tab.type = TAB_TAB ;
            mDocument.InsertTab(tab) ;
            done = true ;
            break ;
        }

        case Qt::Key_Z :
            if(modifier == Qt::AltModifier)
            {
                mDocument.Undo() ;
                done = true ;
                break ;
            }

    }

    if(!done)
    {
        QString text = event->text() ;

#ifdef Q_OS_MACOS
        if(modifier == Qt::ControlModifier)
        {
            int ctrlkeycode = keycode - 64 ;
            QString tempstr(ctrlkeycode) ;
            text = tempstr ;
        }
#endif
        // shift is current only used for ^shiftU (redo)
        bool shift ;
        if(modifier & Qt::ShiftModifier)
        {
            shift = true ;
        }
        else
        {
            shift = false ;
        }

        CHAR_T ch = text.at(0).unicode() ;
        bool handled = mInput->HandleKey(static_cast<char>(ch), shift) ;       // keyboard input for commands is only char
        if(handled == false)
        {
            mDocument.SaveUndo() ;
            if(mInsertMode == OVERWRITE_MODE)
            {
                DeleteChar() ;
            }

            mDocument.Insert(ch) ;
        }
        mHelpDisplay = mInput->GetHelpStatus() ;
    }

    RedrawFullDisplay() ;

    QTimer::singleShot(1, this, SLOT(OnIdle()));

}



/////////////////////////////////////////////////////////////////////////////
///
/// @param  event   [IN] - the size event
///
/// @return nothing
///
/// @brief
/// handle a resize of our display by forcing new word wrap calculations
///
/////////////////////////////////////////////////////////////////////////////
void cEditorCtrl::resizeEvent(QResizeEvent *event)
{
UNUSED_ARGUMENT(event) ;
    RedrawFullDisplay() ;
//    repaint() ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @param  event   [IN] - the mouse event
///
/// @return nothing
///
/// @brief
/// scroll the window in the dirction th wheel was scrolled
///
/////////////////////////////////////////////////////////////////////////////
void cEditorCtrl::wheelEvent(QWheelEvent *event)
{
    QPoint delta = event->angleDelta() ;
    int pos = (delta.y() / 120) * 3 ;       // move three lines for every mouse wheel

    if(pos < 0)
    {
        for(int loop = pos; loop < 0; loop++)
        {
            mFirstLine++ ;
            if(mFirstLine > mLayout->GetNumberofLines())
            {
                mFirstLine-- ;
            }
        }
    }
    else if(pos > 0)
    {
        for(int loop = 0; loop < pos; loop++)
        {
            mFirstLine-- ;
            if(mFirstLine < 0)
            {
                mFirstLine = 0 ;
            }
        }
    }

    update() ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param  event   [IN] - the mouse event
///
/// @return nothing
///
/// @brief
/// move the caret to where the user clicked.
///
/// If the mouse click is in the last 1/2 of a block, put the carat after the
/// character.  IF it's in the first half, put it before the character.
/////////////////////////////////////////////////////////////////////////////
void cEditorCtrl::mousePressEvent(QMouseEvent *event)
{
    mSearchBlockSet = false ;
    mStartSearchBlock = 0 ;
    mEndSearchBlock = 0 ;

    Qt::MouseButton button = event->button() ;
    if(button == Qt::LeftButton)
    {
        QPointF point = event->pos() ;
        point.setX(point.x() / mScale) ;
        point.setY(point.y() / mScale) ;

        POSITION_T pos = 0 ;
        for(auto mrect : mMouseRect)
        {
            if(mrect.coordinates.contains(point.x(), point.y()))
            {
                pos = mrect.position ;
                break ;
            }

            if(mrect.coordinates.y() > point.y())
            {
                pos = mrect.position - 1 ;
                break ;
            }

        }

        mDocument.SetPosition(pos) ;
        update() ;
    }
}





void cEditorCtrl::sliderChange(void)
{
    mFirstLine = mWordTsar->mScrollbar->value() ;
    if(mFirstLine == 1)
    {
        mFirstLine = 0 ;
    }

    RedrawFullDisplay() ;
}


void cEditorCtrl::Init(void)
{
    mProgress = nullptr ;
    mIsReady = false ;
	mFirstPaint = true;
    mRuler = nullptr ;
    mIsHelp = false ;
    mShowControl = SHOW_ALL ;
    mShowDot = true ;
    mAlwaysDot = true ;
    mAlwaysFlag = true ;
    mWordWrap = true ;
    mLayoutParagraph = 0 ;
    mSaveLayoutPara = 0 ;
    mScale = 1.0 ;
    mFirstLine = 0 ;
    mFontScale = FONTSCALE ;

QScreen *srn = QApplication::screens().at(0);
qreal dotsPerInch = (qreal)srn->logicalDotsPerInch();
mFontScale = TWIPSPERINCH / dotsPerInch ;

    mCurrentPage = 1 ;
    mFullScreen = false ;
    mHelpDisplay = HELP_MAIN ;
    mInsertMode = INSERT_MODE ;
    mWordTsar = nullptr ;
    mLayoutRest = false ;
    mDrawFullDisplay = true ;
    mHardReturn = false ;
    mFirstIdle = true ;
    mMeasure = MSR_CENTIMETERS ;
    mMeasurement = "0cm" ;
    mDoDrawCaret = false ;
    mHideCaret = false ;
    mPaperId = QPageSize::Letter ;
    mSearchBlockSet = false ;
    mStartSearchBlock = 0 ;
    mEndSearchBlock = 0 ;
    mPrintLayout = nullptr ;
    mLayoutInt = false;
    mInPrintPreview = false ;

    mFileDir = "./" ;
    mFileName = "Unknown.ws" ;
    mFileSet = false ;

    for(short loop = 0; loop < 10; loop++)
    {
        mDocument.mSavePosition[loop] = 0 ;
    }

    
    mInput = new cWordStarInput(this) ;

//    mDocument.SetEditor(this) ;
   
    mLayout = new cLayout(&mDocument, this) ;
    
    connect(&mStatusTimer, SIGNAL(timeout()), this, SLOT(OnStatusTimer())) ;
    mStatusTimer.setSingleShot(false);
    mStatusTimer.start(200) ;

    QTimer::singleShot(1, this, SLOT(OnIdle()));

    mCaratTimer = new QTimer() ;
    connect(mCaratTimer, SIGNAL(timeout()), this, SLOT(OnCaretTimer())) ;
    mCaratTimer->setSingleShot(true) ;

    if(mTabStops.empty() == false)
    {
        mTabStops.clear() ;
    }

    COORD_T tabstop = TWIPSPERINCH / 2.0 ;           // default tabs are 1/2" == 720 twips
    mTabStops.push_back(0) ;                            // Wordstar sets a tab stop at the left margin.
    for(short loop = 0; loop < 12; loop++)
    {
        mTabStops.push_back(tabstop) ;
        tabstop += TWIPSPERINCH / 2 ;
    }

    if(mFileTypes.empty())
    {
        // load our file handlers
        cFile *file ;


        try
        {
            file = new cWordstarFile(this) ;
        }
        catch(...)
        {
            QMessageBox msgBox(QMessageBox::Critical, "FATAL ERROR", "WordTsar Fatal Error: Cannot initialize Wordstar file methods", QMessageBox::Ok, this) ;
            msgBox.exec() ;
            QCoreApplication::quit() ;
        }
        mFileTypes.push_back(file) ;

        try
        {
            file = new cRTFFile(this) ;
        }
        catch(...)
        {
            QMessageBox msgBox(QMessageBox::Critical, "FATAL ERROR", "WordTsar Fatal Error: Cannot initialize RTF file methods", QMessageBox::Ok, this) ;
            msgBox.exec() ;
            QCoreApplication::quit() ;
        }
        mFileTypes.push_back(file) ;


        try
        {
            file = new cDOCXFile(this) ;
        }
        catch(...)
        {
            QMessageBox msgBox(QMessageBox::Critical, "FATAL ERROR", "WordTsar Fatal Error: Cannot initialize DOCX file methods", QMessageBox::Ok, this) ;
            msgBox.exec() ;
            QCoreApplication::quit() ;
        }
        mFileTypes.push_back(file) ;

        /*
        try
        {
            file = new cTextFile(this) ;            // save textfile for last, since it'll try to load anything
        }
        catch(...)
        {
            QMessageBox msgBox(QMessageBox::Critical, "FATAL ERROR", "WordTsar Fatal Error: Cannot initialize Text file methods", QMessageBox::Ok, this) ;
            msgBox.exec() ;
            QCoreApplication::quit() ;
        }
        mFileTypes.push_back(file) ;
*/
    }
        
    if(mIsHelp == false)
    {
        mBackgroundColour = QColor(245, 245, 245) ;
        mTextColour = (QColor(0, 0, 0)) ;
        mHighlightColour = (QColor(0, 150, 200, 127)) ;
        mDotColour = (QColor(100, 200, 200, 190)) ;
        mBlockColour = (QColor(50, 100, 200, 190)) ;
        mCommentColour = (QColor(255, 178, 102, 190)) ;
        mUnknownColour = (QColor(194, 70, 65, 190)) ;
    }
    mBaseFont = QFont("monospace", 10) ;
    mBaseFont.setPointSize(10);
    mBaseFont.setKerning(false);
}


void cEditorCtrl::CloseEvent(void)
{
    mInput->OnControlKChar('X') ;
}


void cEditorCtrl::SetRuler(cRulerCtrl *ruler)
{
    mRuler = ruler ;
    mRuler->SetTabStops(mTabStops) ;
    switch(mMeasure)
    {
        case MSR_INCHES :
            mRuler->SetRuler(8.5, MSR_INCHES) ;      // set to default
            break ;
        
        case MSR_MILLIMETERS :
            mRuler->SetRuler(215.9, MSR_MILLIMETERS) ;      // set to default
            break ;
        
        default :
            mRuler->SetRuler(21.59, MSR_CENTIMETERS) ;      // set to default
            break ;
    }
    mRuler->SetPageMargins(TWIPSPERINCH, TWIPSPERINCH * 7.5) ;                      // set to default
    mRuler->SetRightMargin((TWIPSPERINCH * 7.5) - TWIPSPERINCH) ;
    mRuler->SetPosition(0);
    mRuler->update() ;

    QCursor cursor ;
    cursor.setShape(Qt::IBeamCursor) ;
    setCursor(cursor) ;
}


void cEditorCtrl::SetFrame(cWordTsar *frame)
{
    mWordTsar = frame ;
    
    string title ;
    title = string_sprintf("%s - WordTsar %ld.%ld build %ld %s", mFileName.toUtf8().constData(), MAJOR, MINOR, BUILD, STATUS) ;
    mWordTsar->setWindowTitle(QString::fromStdString(title)) ;

    // now that we know our parent...
    connect(mWordTsar->mScrollbar, &QScrollBar::valueChanged, this, &cEditorCtrl::sliderChange) ;
}

/////////////////////////////////////////////////////////////////////////////
///
/// @return nothing
///
/// @brief
/// Set this editor as a help editor
///
/////////////////////////////////////////////////////////////////////////////
void cEditorCtrl::SetAsHelp(void)
{
    SetWordWrap(false) ;
    SetShowControls(SHOW_NONE) ;
    mCaratTimer->stop();
    
    mIsHelp = true ;
    mBaseFont = QFont("monospace", 10) ;
    mBaseFont.setPointSize(10);
    mBaseFont.setKerning(false);

    QCursor cursor ;
    cursor.setShape(Qt::ArrowCursor) ;
    setCursor(cursor) ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return bool
///
/// @brief
/// true if this is a help editor
///
/////////////////////////////////////////////////////////////////////////////
bool cEditorCtrl::GetIsHelp(void)
{
    return mIsHelp ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param  std::string name [in] name of the editor
///
/// @return nothing
///
/// @brief
/// Set this editor's name (helps in debugging)
///
/////////////////////////////////////////////////////////////////////////////
void cEditorCtrl::SetEditorName(std::string name)
{
    mEditorName = name ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return std::string  name of editor
///
/// @brief
/// Get this editor's name
///
/////////////////////////////////////////////////////////////////////////////
std::string cEditorCtrl::GetEditorName(void)
{
    return mEditorName ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return std::string  Title of Window
///
/// @brief
/// Set the window title
///
/////////////////////////////////////////////////////////////////////////////
void cEditorCtrl::SetTitle(std::string title)
{
    string title1 ;
    title1 = string_sprintf("%s - WordTsar %ld.%ld build %ld %s", title.c_str(), MAJOR, MINOR, BUILD, STATUS) ;
    mWordTsar->setWindowTitle(QString::fromStdString(title1)) ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return nothing
///
/// @brief
/// Set the base font for the editor
///
/////////////////////////////////////////////////////////////////////////////
void cEditorCtrl::SetFont(QFont font)
{
	mBaseFont = font;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return QFont
///
/// @brief
/// Get the base font of the editor
///
/////////////////////////////////////////////////////////////////////////////
QFont cEditorCtrl::GetFont(void)
{
	return mBaseFont;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param  loading   [IN] - true if loading file, else false
///
/// @return nothing
///
/// @brief
/// Set the word wrap flag
///
/////////////////////////////////////////////////////////////////////////////
void cEditorCtrl::SetWordWrap(bool wrap)
{
    mWordWrap = wrap ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @return bool
///
/// @brief
/// Get the word wrap flag: true if file wrapping, else flase
///
/////////////////////////////////////////////////////////////////////////////
bool cEditorCtrl::GetWordWrap(void)
{
    return mWordWrap ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @param  show        [IN] eShowControl
///
/// @return  nothing
///
/// @brief
/// set the flag that indicates whether we show the control characters
/// or not.
///
/////////////////////////////////////////////////////////////////////////////
void cEditorCtrl::SetShowControls(eShowControl show)
{
    mShowControl = show ;
    mDocument.SetShowControl(show) ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return  eShowControl
///
/// @brief
/// get the show control flag
///
/////////////////////////////////////////////////////////////////////////////
eShowControl cEditorCtrl::GetShowControls(void)
{
    return mShowControl ;
}




/////////////////////////////////////////////////////////////////////////////
///
/// @param  dot        [IN] bool
///
/// @return  nothing
///
/// @brief
/// set the flag that indicates whether we are printing or not.
///
/////////////////////////////////////////////////////////////////////////////
void cEditorCtrl::SetShowDot(bool dot)
{
    mShowDot = dot ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @return  bool
///
/// @brief
/// true if we are printing
///
/////////////////////////////////////////////////////////////////////////////
bool cEditorCtrl::GetShowDot(void)
{
    return mShowDot ;
}

/////////////////////////////////////////////////////////////////////////////
///
/// @param  colour        [IN] colour to set
///
/// @return  nothing
///
/// @brief
/// set the default background colour of the display
///
/////////////////////////////////////////////////////////////////////////////
void cEditorCtrl::SetBGroundColour(QColor colour)
{
    mBackgroundColour = colour ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return  QColor
///
/// @brief
/// get the default background colour of the display
///
/////////////////////////////////////////////////////////////////////////////
QColor cEditorCtrl::GetBGroundColour(void)
{
    return mBackgroundColour ;
}




/////////////////////////////////////////////////////////////////////////////
///
/// @param  colour        [IN] colour to set
///
/// @return  nothing
///
/// @brief
/// set the default text colour of the display
///
/////////////////////////////////////////////////////////////////////////////
void cEditorCtrl::SetTextColour(QColor colour)
{
    mTextColour = colour ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return  QColor
///
/// @brief
/// get the default text colour of the display
///
/////////////////////////////////////////////////////////////////////////////
QColor cEditorCtrl::GetTextColour(void)
{
    return mTextColour ;
}




/////////////////////////////////////////////////////////////////////////////
///
/// @param  colour        [IN] colour to set
///
/// @return  nothing
///
/// @brief
/// set the default high light colour of the display
///
/////////////////////////////////////////////////////////////////////////////
void cEditorCtrl::SetHighlightColour(QColor colour)
{
    QColor t(colour.red(), colour.green(), colour.blue(), 190) ;          // force alpha
    mHighlightColour = t ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return  QColor
///
/// @brief
/// get the default high light colour of the display
///
/////////////////////////////////////////////////////////////////////////////
QColor cEditorCtrl::GetHighlightColour(void)
{
    return mHighlightColour  ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param  colour        [IN] colour to set
///
/// @return  nothing
///
/// @brief
/// set the default high light colour of the display
///
/////////////////////////////////////////////////////////////////////////////
void cEditorCtrl::SetDotColour(QColor colour)
{
    QColor t(colour.red(), colour.green(), colour.blue(), 190) ;          // force alpha
    mDotColour = t ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return  QColor
///
/// @brief
/// get the default high light colour of the display
///
/////////////////////////////////////////////////////////////////////////////
QColor cEditorCtrl::GetDotColour(void)
{
    return mDotColour  ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param  colour        [IN] colour to set
///
/// @return  nothing
///
/// @brief
/// set the default block colour of the display
///
/////////////////////////////////////////////////////////////////////////////
void cEditorCtrl::SetBlockColour(QColor colour)
{
    QColor t(colour.red(), colour.green(), colour.blue(), 127) ;          // force alpha
    mBlockColour = t ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return  QColor
///
/// @brief
/// get the default block colour of the display
///
/////////////////////////////////////////////////////////////////////////////
QColor cEditorCtrl::GetBlockColour(void)
{
    return mBlockColour  ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param  colour        [IN] colour to set
///
/// @return  nothing
///
/// @brief
/// set the default comment colour of the display
///
/////////////////////////////////////////////////////////////////////////////
void cEditorCtrl::SetCommentColour(QColor colour)
{
    QColor t(colour.red(), colour.green(), colour.blue(), 190) ;          // force alpha
    mCommentColour = t ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return  QColor
///
/// @brief
/// get the default comment colour of the display
///
/////////////////////////////////////////////////////////////////////////////
QColor cEditorCtrl::GetCommentColour(void)
{
    return mCommentColour  ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param  colour        [IN] colour to set
///
/// @return  nothing
///
/// @brief
/// set the default unknown dot command colour of the display
///
/////////////////////////////////////////////////////////////////////////////
void cEditorCtrl::SetUnknownColour(QColor colour)
{
    QColor t(colour.red(), colour.green(), colour.blue(), 190) ;          // force alpha
    mUnknownColour = t ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return  QColor
///
/// @brief
/// get the default unknown dot command colour of the display
///
/////////////////////////////////////////////////////////////////////////////
QColor cEditorCtrl::GetUnknownColour(void)
{
    return mUnknownColour  ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param  index         [IN] index of colour to set (0 - 9)
/// @param  colour        [IN] colour to set
///
/// @return  nothing
///
/// @brief
/// set the default special comment colour of the display
///
/////////////////////////////////////////////////////////////////////////////
void cEditorCtrl::SetSpecialColour(int index, QColor colour)
{
    if(index < 0 || index > 9)
    {
        index = 0 ;
    }
    
    QColor t(colour.red(), colour.green(), colour.blue(), 190) ;          // force alpha
    mSpecialColour[index] = t ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @param  index         [IN] index value of colour to get(0 - 9)
/// @return  QColor
///
/// @brief
/// get the default special comment colour of the display
///
/////////////////////////////////////////////////////////////////////////////
QColor cEditorCtrl::GetSpecialColour(int index)
{
    if(index < 0 || index > 9)
    {
        index = 0 ;
    }

    return mSpecialColour[index]  ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param  measure        [IN] measure ment in string fromat (i.e. 0cm for centimeter)
///
/// @return  nothing
///
/// @brief
/// set the default measurement
///
/////////////////////////////////////////////////////////////////////////////
void cEditorCtrl::SetMeasurement(QString measure)
{
    mMeasurement = measure ;
    char c = mDocument.GetType(measure.toStdString()) ;
    switch(c)
    {
        case '\"' :
            mMeasure = MSR_INCHES ;
            break ;
            
        case 'C' :
            mMeasure = MSR_CENTIMETERS ;
            break ;
            
        case 'M' :
            mMeasure = MSR_MILLIMETERS ;
            break ;
            
        default :
            mMeasure = MSR_CENTIMETERS ;
            break ;
    }
    
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return  QString
///
/// @brief
/// get the default measurement style
///
/////////////////////////////////////////////////////////////////////////////
QString cEditorCtrl::GetMeasurement(void)
{
    return mMeasurement  ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param  text        [IN] wordstar formatted string to insert
///
/// @return  nothing
///
/// @brief
/// Insert the wordstar formatted (control characters) string into the
/// document
///
/////////////////////////////////////////////////////////////////////////////
void cEditorCtrl::InsertWordStarString(string &text)
{
    mDocument.SaveUndo() ;

    mDocument.SetLoading(true) ;

    cWordstarFile wsfile(this) ;
    size_t len = text.length() ;
    for(size_t loop = 0; loop < len; loop++)
    {
        unsigned char ch = static_cast<unsigned char>(text[loop]) ;
        wsfile.HandleChar(ch, loop) ;
    }

    mDocument.SetLoading(false) ;

    RedrawFullDisplay() ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @param  name - filename to load
///
/// @return bool - true on success, else false
///
/// @brief
/// Load a file.  If we have an known extension, we call a special loader
/////////////////////////////////////////////////////////////////////////////
bool cEditorCtrl::LoadFile(QString filename)
{
    mDocument.SetLoading(true) ;
    POSITION_T carat = mDocument.GetPosition() ;
    QString oldfilename = mFileName ;
    
    mDocument.SaveUndo() ;
    POSITION_T oldsize = mDocument.GetTextSize() ;

    bool retval = false ;

    // move file name and directory into holders
    QFileInfo info(filename) ;
    mFileDir = info.path() ;
    mFileDir += "/" ;
    mFileName = info.fileName() ;
    mFileSet = true ;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    string title ;
    title = string_sprintf("%s - WordTsar %ld.%ld build %ld %s", mFileName.toUtf8().constData(), MAJOR, MINOR, BUILD, STATUS) ;
    mWordTsar->setWindowTitle(QString::fromStdString(title)) ;

    for(auto & mFileType : mFileTypes)
    {
        if(mFileType->CanLoad() == true)
        {
            if(mFileType->CheckType(filename.toStdString()) == true)
            {
//                mFileType->SetWindow(this) ;
                SetStatus("Loading...") ;

                QProgressDialog progress("Loading file...", "Cancel", 0, 100, this) ;
                mProgress = &progress ;
                progress.show() ;

                retval = mFileType->LoadFile(filename.toStdString()) ;

                mProgress = nullptr ;

                break ;
            }
        }
    }

    
    mDocument.SetPosition(carat) ;
    
/// @todo remove
mFirstPaint = true ;

    // load file stats and internal values
    if(oldsize <= 1)
    {
        CSimpleIniA ini ;

        QFileInfo fname(filename);
        QString directory = fname.path();
        QString name = fname.fileName();
        QString internals = directory + "/.ws-" + name;

        if(ini.LoadFile(internals.toUtf8().constData()) >= 0)
        {
            POSITION_T pos = ini.GetLongValue("internal", "cursor", 0) ;
            mDocument.SetPosition(pos) ;
            

            mDocument.mSavePosition[0] = ini.GetLongValue("internal", "save1", 0) ;
            mDocument.mSavePosition[1] = ini.GetLongValue("internal", "save2", 0) ;
            mDocument.mSavePosition[2] = ini.GetLongValue("internal", "save3", 0) ;
            mDocument.mSavePosition[3] = ini.GetLongValue("internal", "save4", 0) ;
            mDocument.mSavePosition[4] = ini.GetLongValue("internal", "save5", 0) ;
            mDocument.mSavePosition[5] = ini.GetLongValue("internal", "save6", 0) ;
            mDocument.mSavePosition[6] = ini.GetLongValue("internal", "save7", 0) ;
            mDocument.mSavePosition[7] = ini.GetLongValue("internal", "save8", 0) ;
            mDocument.mSavePosition[8] = ini.GetLongValue("internal", "save9", 0) ;
            mDocument.mSavePosition[9] = ini.GetLongValue("internal", "save10", 0) ;
            
            mDocument.mStartBlock = ini.GetLongValue("internal", "blockstart", 0) ;
            mDocument.mEndBlock = ini.GetLongValue("internal", "blockend", 0) ;
            mDocument.mBlockSet = ini.GetBoolValue("internal", "blockactive", false) ;
            
        }
    }

    LayoutFullDocument(true) ;
    mDocument.SetLoading(false) ;

    QApplication::restoreOverrideCursor();

    mFirstLine = mLayout->GetLineFromPosition(mDocument.GetPosition()) ;

    RedrawFullDisplay() ;

//    mDocument.ClearUndoRedo() ;

    return retval ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param  name - filename to save
///
/// @return bool - true on success, else false
///
/// @brief
/// Save a file.  If we have an known extension, we call a special saver
/////////////////////////////////////////////////////////////////////////////
bool cEditorCtrl::SaveFile(QString filename)
{
    bool retval = false ;

    eShowControl wrap = GetShowControls();
    SetShowControls(SHOW_ALL) ;

    QApplication::setOverrideCursor(Qt::WaitCursor) ;
    POSITION_T size = mDocument.GetTextSize() ;

    for(size_t loop = 0; loop < mFileTypes.size(); loop++)
    {
        if(mFileTypes[loop]->CanSave() == true)
        {
            if(mFileTypes[loop]->CheckType(filename.toStdString()) == true)
            {
                retval = mFileTypes[loop]->SaveFile(filename.toStdString(), size) ;
                break ;
            }
        }
    }

    if(retval == false)
    {
        QApplication::restoreOverrideCursor() ;
        QMessageBox msgBox(QMessageBox::Critical, "Error", "File Save failed - invalid file extension", QMessageBox::Ok, this) ;
        msgBox.exec() ;
    }
    else
    {
        // save file stats and internal values
        CSimpleIniA ini ;

        QString directory, name ;
        QFileInfo info(filename) ;
        directory = info.path() ;
        name = info.fileName() ;
        QString internals = directory + "/.ws-" + name;

    //    string internals = internals1 ;

        ini.LoadFile(internals.toUtf8().constData()) ;

        POSITION_T pos = mDocument.GetPosition() ;
        ini.SetLongValue("internal", "cursor", pos) ;

        ini.SetLongValue("internal", "save1", mDocument.mSavePosition[0]) ;
        ini.SetLongValue("internal", "save2", mDocument.mSavePosition[1]) ;
        ini.SetLongValue("internal", "save3", mDocument.mSavePosition[2]) ;
        ini.SetLongValue("internal", "save4", mDocument.mSavePosition[3]) ;
        ini.SetLongValue("internal", "save5", mDocument.mSavePosition[4]) ;
        ini.SetLongValue("internal", "save6", mDocument.mSavePosition[5]) ;
        ini.SetLongValue("internal", "save7", mDocument.mSavePosition[6]) ;
        ini.SetLongValue("internal", "save8", mDocument.mSavePosition[7]) ;
        ini.SetLongValue("internal", "save9", mDocument.mSavePosition[8]) ;
        ini.SetLongValue("internal", "save10", mDocument.mSavePosition[9]) ;

        ini.SetLongValue("internal", "blockstart", mDocument.mStartBlock) ;
        ini.SetLongValue("internal", "blockend", mDocument.mEndBlock) ;
        ini.SetBoolValue("internal", "blockactive", mDocument.mBlockSet) ;

        ini.SaveFile(internals.toUtf8().constData()) ;

#ifdef Q_OS_WIN
        size_t csize = internals.length() + 1 ;
        wchar_t* fileLPCWSTR = new wchar_t[csize];
        mbstowcs(fileLPCWSTR, internals.toStdString().c_str(), csize);

        int attr = GetFileAttributes(fileLPCWSTR);
        if ((attr & FILE_ATTRIBUTE_HIDDEN) == 0)
        {
            SetFileAttributes(fileLPCWSTR, attr | FILE_ATTRIBUTE_HIDDEN);
        }

        delete fileLPCWSTR;
#endif

        // if this is not a backup file
        if(filename.contains("-bak") == false)
        {
    //        mDocument.ClearUndoRedo() ;
        }
    }

    QApplication::restoreOverrideCursor() ;
    SetShowControls(wrap) ;

    return retval ;
}


/////////////////////////////////////////////////////////////////////////////
///
///
/// @return nothing
///
/// @brief
/// trys to save the file with "-error" as part of the file name. Then exits.
/////////////////////////////////////////////////////////////////////////////
void cEditorCtrl::EmergencySaveFile(void)
{
    QMessageBox::critical(this,
                          "WordTsar Critical Error - MeasureParagraphText",
                          "Possible memory allocation or corruption error\n\nSaving your work with -error extension.\n\nIf this is a new file, the saved filename is:wordtsar-emergency-file-save",
                          QMessageBox::Ok) ;

    if(mFileName.length() == 0)
    {
        mFileName = "wordtsar-emergency-file-save.ws" ;
    }

    QString savefile ;
    QFileInfo filename(mFileName);

    savefile = mFileDir + filename.completeBaseName() + "-error." + filename.completeSuffix() ;
    SaveFile(savefile) ;

    exit(255) ;
}





void cEditorCtrl::OnIdle(void)
{
    // don't do layout while loading
    if(mDocument.GetLoading() == true)
    {
        return ;
    }

    // don't do if we are a help screen
    if(mIsHelp)
    {
        return ;
    }

    if(mDocument.GetTextSize() == 0)
    {
        return ;
    }

    bool ret = IdleLayout(mLayout, true) ;

    if(ret)
    {
        QTimer::singleShot(0, this, SLOT(OnIdle()));
    }

}


void cEditorCtrl::OnStatusTimer(void)
{
    static size_t savecounter = 0 ;
    static int wordcounter = 0 ;
    
    if(mIsHelp)
    {
        mStatusTimer.stop() ;
        return ;
    }

    if(mIsReady == false)
    {
        return ;
    }
        
    if(mWordTsar != nullptr)
    {
        mWordTsar->UpdateStatus(this) ;
    }
    
    wordcounter++ ;
    if(wordcounter == 25)       // we call this 5 times a second, so 5 seconds is 25 calls
    {
        if(mDocument.GetLoading() == false)
        {
            mWordCount = WordCount(0, 0) ;
        }
        wordcounter = 0 ;
    }
    
    savecounter++ ;
    if(savecounter == 300)      // we call this 5 times a second, so 1 minute is 300 calls
    {
        SetStatus("saving backup...") ;
        savecounter = 0 ;
//#ifndef NDEBUG
        if(mFileName.length() != 0)
        {
            QString savefile ;
            QFileInfo filename(mFileName);

            savefile = mFileDir + "/" + filename.completeBaseName() + "-bak." + filename.completeSuffix() ;
            SaveFile(savefile) ;

        }
//#endif
        SetStatus("") ;
    }
}



void cEditorCtrl::OnCaretTimer(void)
{
    mDoDrawCaret = true ;
    update() ;
}




bool cEditorCtrl::IdleLayout(cLayout *useLayout, bool fromIdle)
{
    bool retval = false ;           // false is do not request more events
    // no need to do layout if we reached the end of our document
    PARAGRAPH_T para = mDocument.GetNumberofParagraphs() ;

    if(mLayoutInt == true)
    {
//        mLayoutParagraph = mSaveLayoutPara ;
    }

    // if we are done, update less often
    if(mLayoutParagraph >= para)
    {
        SetStatus(" ") ;
        mLayoutRest = false ;
        return retval ;
    }

  
    // give an indication of how far along we are
    bool same = useLayout->LayoutParagraph(mLayoutParagraph) ;

    if(same == false || mLayoutRest == true)
    {
        int perc = static_cast<int>(mLayoutParagraph * 100 / para) ;
        QString str ;
        str.sprintf("Formatting %d %% ...", perc) ;
        SetStatus(str, true, perc);
        mLayoutParagraph++ ;
        mLayoutInt = false ;
    }
    else
    {
        mLayoutParagraph = para ;
    }
    
    return true ;
}




void cEditorCtrl::MoveDown(void)
{
    POSITION_T pos = mDocument.GetPosition() ;
    LINE_T currentline = mLayout->GetLineFromPosition(pos) ;
    LINE_T maxlines = mLayout->GetNumberofLines() ;
    LINE_T nextlineoffset = 1 ;
    
    // if we are not showing dot commands, then skip lines if they are
    if(GetShowDot() == false)
    {
        for(LINE_T loop = currentline + 1; loop < maxlines; loop++)
        {
            LINE_T line = loop ;
            PARAGRAPH_T para = mLayout->GetParagraphFromLine(line) ;
            if(mLayout->mParagraphLayout[static_cast<size_t>(para)].isCommand || mLayout->mParagraphLayout[static_cast<size_t>(para)].isComment)
            {
                nextlineoffset++ ;
            }
            else
            {
                break ;
            }
        }
    }
    
    if(currentline + nextlineoffset != maxlines)
    {
        POSITION_T curlinepos = mLayout->GetLineStartPosition(currentline) ;
        POSITION_T newlinepos = mLayout->GetLineStartPosition(currentline + nextlineoffset) ;
        POSITION_T endlinepos = mLayout->GetLineEndPosition(currentline + nextlineoffset) ;
        
        // if the new line is shorter than the old lines carat position
        if(endlinepos - newlinepos < pos - curlinepos)
        {
            pos = endlinepos ; // - 1 ;
        }
        else
        {
            pos = newlinepos + (pos - curlinepos) ;
        }

        if(pos == mDocument.GetTextSize())
        {
            pos-- ;
        }
        
        // this will get us past control characters if we are hiding them
        mDocument.GetChar(pos) ;
        mDocument.SetPosition((pos) );

        ScrollIntoView(Qt::Key_Down) ;
    }
}



void cEditorCtrl::MoveUp(void)
{
    POSITION_T pos = mDocument.GetPosition() ;
    LINE_T currentline = mLayout->GetLineFromPosition(pos) ;
    LINE_T nextlineoffset = 1 ;
    
    if(GetShowDot() == false)
    {
        for(LINE_T loop = currentline - 1; loop >= 0; loop--)
        {
            LINE_T line = loop ;
            PARAGRAPH_T para = mLayout->GetParagraphFromLine(line) ;
            if(mLayout->mParagraphLayout[static_cast<size_t>(para)].isCommand || mLayout->mParagraphLayout[static_cast<size_t>(para)].isComment)
            {
                nextlineoffset++ ;
            }
            else
            {
                break ;
            }
        }
    }

    if(currentline != 0)
    {
        POSITION_T curlinepos = mLayout->GetLineStartPosition(currentline) ;
        POSITION_T newlinepos = mLayout->GetLineStartPosition(currentline - nextlineoffset) ;
        POSITION_T endlinepos = mLayout->GetLineEndPosition(currentline - nextlineoffset) ;

        // if the new line is shorter than the old lines carat position
        if(endlinepos - newlinepos < pos - curlinepos)
        {
            pos = endlinepos ;
        }
        else
        {
            pos = newlinepos + (pos - curlinepos) ;
        }

        mDocument.SetPosition((pos) );
        ScrollIntoView(Qt::Key_Up) ;
    }
}


void cEditorCtrl::PageDown(void)
{
    for(int loop = 0; loop < mDisplayLines; loop++)
    {
        MoveDown() ;
    }
/*
    POSITION_T pos = mDocument.GetPosition() ;
    LINE_T currentline = mFirstLine ;
//    LINE_T maxlines = mLayout->GetNumberofLines() ;
    LINE_T nextlineoffset = mDisplayLines -1 ;
    
    if(GetShowDot() == false)
    {
        for(LINE_T loop = nextlineoffset; loop < mLayout->GetNumberofLines(); loop++)
        {
            LINE_T line ;
            PARAGRAPH_T para = mLayout->GetParagraphFromLine(line) ;
            if(mLayout->mParagraphLayout[static_cast<size_t>(para)].isCommand || mLayout->mParagraphLayout[static_cast<size_t>(para)].isComment)
            {
                nextlineoffset++ ;
            }
            else
            {
                break ;
            }
        }
    }

    if(mFirstLine + nextlineoffset < mLayout->GetNumberofLines())
    {
        mFirstLine += nextlineoffset ;
    }
    
    POSITION_T curlinepos = mLayout->GetLineStartPosition(currentline) ;
    POSITION_T newlinepos = mLayout->GetLineStartPosition(mFirstLine) ;
    POSITION_T endlinepos = mLayout->GetLineEndPosition(mFirstLine) ;
    
    // if the new line is shorter than the old lines carat position
    if(endlinepos - newlinepos < pos - curlinepos)
    {
        pos = endlinepos ;
    }
    else
    {
        pos = newlinepos + (pos - curlinepos) ;
    }
    mDocument.GetChar(pos) ;
    mDocument.SetPosition((pos) );
    ScrollIntoView(Qt::Key_Down) ;
*/
}


void cEditorCtrl::PageUp(void)
{
    POSITION_T pos = mDocument.GetPosition() ;
    LINE_T currentline = mFirstLine ;
    LINE_T nextlineoffset = mDisplayLines -1 ;
    
    if(GetShowDot() == false)
    {
        for(LINE_T loop = nextlineoffset; loop >= 0; loop--)
        {
            LINE_T line = loop ;
            PARAGRAPH_T para = mLayout->GetParagraphFromLine(line) ;
            if(mLayout->mParagraphLayout[static_cast<size_t>(para)].isCommand || mLayout->mParagraphLayout[static_cast<size_t>(para)].isComment)
            {
                nextlineoffset++ ;
            }
            else
            {
                break ;
            }
        }
    }

    if(mFirstLine - nextlineoffset >= 0)
    {
        mFirstLine -= nextlineoffset ;
    }
    else
    {
        mFirstLine = 0 ;
    }

    POSITION_T curlinepos = mLayout->GetLineStartPosition(currentline) ;
    POSITION_T newlinepos = mLayout->GetLineStartPosition(mFirstLine) ;
    POSITION_T endlinepos = mLayout->GetLineEndPosition(mFirstLine) ;

    // if the new line is shorter than the old lines carat position
    if(endlinepos - newlinepos < pos - curlinepos)
    {
        pos = endlinepos ;
    }
    else
    {
        pos = newlinepos + (pos - curlinepos) ;
    }
    
    mDocument.SetPosition((pos) );
    ScrollIntoView(Qt::Key_Up) ;
}


void cEditorCtrl::MoveLeft(void)
{
    POSITION_T pos = mDocument.GetPosition() ;
    if(pos == 0)
    {
        return ;
    }
    POSITION_T oldpos = 0;
    
    pos-- ;
    for(POSITION_T loop = pos; loop >= 0; loop--)
    {
        oldpos = loop ;
        mDocument.GetChar(oldpos) ;
        if(oldpos == loop)
        {
            if(GetShowDot() == false)
            {
                PARAGRAPH_T para = mDocument.GetParagraphFromPosition(oldpos) ;
                if(mLayout->mParagraphLayout[static_cast<size_t>(para)].isCommand == false && mLayout->mParagraphLayout[static_cast<size_t>(para)].isComment == false)
                {
                    break ;
                }
            }
            else
            {
                break ;
            }
        }
    }
    
    mDocument.SetPosition(oldpos) ;

    ScrollIntoView(Qt::Key_Up) ;
}



void cEditorCtrl::MoveRight(void)
{
    POSITION_T pos = mDocument.GetPosition() ;
    pos++ ;
    if(pos == mDocument.GetTextSize())
    {
        pos-- ;
    }

    POSITION_T oldpos = 0;
    for(POSITION_T loop = pos; loop < mDocument.GetTextSize(); loop++)
    {
        oldpos = loop ;
        mDocument.GetChar(oldpos) ;
        if(oldpos == loop)
        {
            if(GetShowDot() == false)
            {
                PARAGRAPH_T para = mDocument.GetParagraphFromPosition(oldpos) ;
                if(mLayout->mParagraphLayout[static_cast<size_t>(para)].isCommand == false && mLayout->mParagraphLayout[static_cast<size_t>(para)].isComment == false)
                {
                    break ;
                }
            }
            else
            {
                break ;
            }
        }
    }
    
    mDocument.SetPosition(oldpos) ;

    ScrollIntoView(Qt::Key_Down) ;
}


void cEditorCtrl::ScrollUp(void)
{
    mFirstLine-- ;
    if(mFirstLine < 0)
    {
        mFirstLine = 0 ;
    }
    else
    {
        MoveUp() ;
    }
}


void cEditorCtrl::ScrollDown(void)
{
    mFirstLine++ ;
    if(mFirstLine >= mLayout->GetNumberofLines())
    {
        mFirstLine-- ;
    }
    else
    {
        MoveDown() ;
    }
//    Refresh() ;
}


void cEditorCtrl::WordRight(void)
{
    POSITION_T currentpos = mDocument.GetPosition() ;

    POSITION_T loop = mDocument.GetNextWordPosition(currentpos) ;


    if(loop == mDocument.GetTextSize())
    {
        loop-- ;
    }

    mDocument.SetPosition(loop) ;

    ScrollIntoView(Qt::Key_Down) ;
}


void cEditorCtrl::WordLeft(void)
{
    POSITION_T currentpos = mDocument.GetPosition() ;

    // make sure we aren't at the start of the document
    if(currentpos != 0)
    {
        POSITION_T loop = mDocument.GetPrevWordPosition(currentpos) ;

        mDocument.SetPosition(loop) ;
        mDocument.GetChar(loop) ;       // places carat on actual charcater if !SHOW_ALL
        mDocument.SetPosition(loop) ;

        ScrollIntoView(Qt::Key_Up) ;
    }
}


void cEditorCtrl::DeleteChar(void)
{
    Delete(mDocument.GetPosition(), 1) ;
}



void cEditorCtrl::DeleteWordRight(void)
{
    POSITION_T pos = mDocument.GetPosition() ;

    POSITION_T next = mDocument.GetNextWordPosition(pos) ;

    // end of file test (if length to cut is <= 0 or next word position is at end of file)
    if((next - pos - 1 <= 0) || (next == mDocument.GetTextSize() -1))
    {
        next = mDocument.GetTextSize() ;
    }

    if(next - pos - 1 != 0)
    {
        mDocument.Delete(pos, next - pos - 1) ;
    }
}


void cEditorCtrl::DeleteLine(void)
{
    POSITION_T pos = mDocument.GetPosition() ;
    LINE_T line = mLayout->GetLineFromPosition(pos) ;
    POSITION_T start = mLayout->GetLineStartPosition(line) ;
    POSITION_T end = mLayout->GetLineEndPosition(line) ;
    mDocument.SetPosition(start) ;

    PARAGRAPH_T para = mLayout->GetParagraphFromLine(line) ;
    LINE_T numlines = mLayout->GetNumberofLinesinParagraph(para) ;
    POSITION_T size = mDocument.GetTextSize() ;

    // just make sure we don't delete the ^Z
    if(end != size - 1)
    {
        end++ ;
    }

    Delete(start, end - start) ;
}


void cEditorCtrl::LineBreak(void)
{
    mDocument.Insert(HARD_RETURN) ;
}


void cEditorCtrl::ToggleShowControl(void)
{
    switch(mShowControl)
    {
        case SHOW_ALL :
            mShowControl = SHOW_DOT ;
            if(mAlwaysDot == false)
            {
                SetShowDot(false) ;
            }
            break ;
            
        case SHOW_DOT :
//            mShowControl = SHOW_NONE ;
//            break ;
            
        case SHOW_NONE :
            mShowControl = SHOW_ALL ;
            SetShowDot(true) ;
            break ;
    }
}

void cEditorCtrl::NotImplemented(QString t)
{
    QString t1 ;
    t1.sprintf("Command %s not implemented (yet)", t.toUtf8().constData()) ;
    QMessageBox msgBox(QMessageBox::Information, "Information", t1, QMessageBox::Ok, this) ;
    msgBox.exec() ;
}



PARAGRAPH_T cEditorCtrl::GetCurrentParagraph(void)
{
    return mLayoutParagraph ;
}


double cEditorCtrl::GetFontScale(void)
{
    return mFontScale ;
}


COORD_T cEditorCtrl::FindNextTabStop(COORD_T size)
{
    COORD_T retval = 0 ; 
    
    size_t tabsize = mTabStops.size() ;
    size_t loop ;
    for(loop = 0; loop < tabsize; loop++)
    {
        if(mTabStops[loop] > size)
        {
            retval = mTabStops[loop] ;
            break ;
        }
    }
    
    if(loop == tabsize)
    {
        retval = mTabStops[tabsize - 1] ;
    }
    
    return retval ;
}



void cEditorCtrl::LayoutFullDocument(bool force)
{
#ifdef LAYOUT_TIMER
    QElapsedTimer sw ;
    
    printf("Performing Layout Timers\n") ;

    mLayout->prelimt = 0 ;
    mLayout->pdt = 0 ;
    mLayout->mt = 0 ;
    mLayout->wwt = 0 ;
    mLayout->pt = 0 ;
    mLayout->postt = 0 ;
#endif

#ifdef DETAIL_LAYOUT_TIMER
    printf("  and detailed timers\n") ;

    mLayout->pdct = 0 ;
    mLayout->prepdct = 0 ;
    mLayout->perfdct = 0 ;
    
    mLayout->measurecount = 0 ;
    mLayout->percall = 0;
#endif


    mLayoutParagraph = 0 ;
    if(force == false)
    {
        mLayoutRest = true ;

	}
    else
    {        
        PARAGRAPH_T para = mDocument.GetNumberofParagraphs() ;
        mLayoutParagraph = 0 ;

#ifdef LAYOUT_TIMER    
    sw.start() ;
#endif


        while ( mLayoutParagraph < para )
        {
            IdleLayout(mLayout) ;
            QApplication::processEvents() ;
        }
    }

#ifdef LAYOUT_TIMER
    printf("Time for full layout:          %10.4f ms\n\n", sw.nsecsElapsed() / 1000000.0) ;

    printf("Time in preliminary work:      %10.4f ms\n", mLayout->prelimt / 1000000.0) ;
    printf("Time in Previous Dot Commands: %10.4f ms\n", mLayout->pdt / 1000000.0) ;
    printf("Time in MeasureText:           %10.4f ms\n", mLayout->mt / 1000000.0) ;
    printf("Time in Wordwrap:              %10.4f ms\n", mLayout->wwt / 1000000.0) ;
    printf("Time in CreateParagraphs:      %10.4f ms\n", mLayout->pt / 1000000.0) ;
    printf("Time in post work:             %10.4f ms\n", mLayout->postt / 1000000.0) ;

#endif

#ifdef DETAIL_LAYOUT_TIMER
    printf("\n") ;
    printf("Time in Previous Dot Command:  %10.4f ms\n", mLayout->pdct / 1000000.0) ;
    printf("Time in Prepare Dot Command:   %10.4f ms\n", mLayout->prepdct / 1000000.0) ;
    printf("Time in Perform Dot COmmand:   %10.4f ms\n", mLayout->perfdct / 1000000.0) ;
    printf("Calls to GetTextExtents or GetPartTextExtents: %ld\n", mLayout->measurecount) ;
    printf("Time accum per call:           %10.4f ms\n", mLayout->percall / 1000000.0);
    printf("mFontScale %.2f\n", mFontScale) ;
#endif

    SetStatus(" ") ;
}


void cEditorCtrl::SetBeginBlock(void)
{
    mDocument.SetBeginBlock() ;
//    RedrawFullDisplay() ;
}



void cEditorCtrl::SetEndBlock(void)
{
    mDocument.SetEndBlock() ;
//    RedrawFullDisplay() ;
}



void cEditorCtrl::CopyBlock(void)
{
    if(mDocument.mBlockSet == true)
    {
        mDocument.SaveUndo() ;

        POSITION_T position = mDocument.GetPosition() ;

        Copy() ;
        mDocument.SetPosition(position) ;
        Paste() ;

        POSITION_T diff = mDocument.mEndBlock - mDocument.mStartBlock ;       // highlight our pasted text
        mDocument.mStartBlock = position  ;
        mDocument.mEndBlock = mDocument.mStartBlock + diff ;
        mDocument.SetPosition(mDocument.mEndBlock) ;

    }
}


void cEditorCtrl::MoveBlock(void)
{
    if(mDocument.mBlockSet == true)
    {
        mDocument.SaveUndo() ;

        POSITION_T position = mDocument.GetPosition() ;
        POSITION_T diff = mDocument.mEndBlock - mDocument.mStartBlock ;       // highlight our pasted text

        if((position < mDocument.mStartBlock) || (position > mDocument.mEndBlock))
        {

            Copy() ;
            mDocument.SetPosition(position) ;
            Paste() ;

            if(position > mDocument.mEndBlock)
            {
                Cut() ;

                mDocument.mStartBlock = position - diff ;
                mDocument.mEndBlock = mDocument.mStartBlock + diff ;

                mDocument.SetPosition(mDocument.mEndBlock ) ;
            }
            else
            {
                Cut() ;

                mDocument.mStartBlock = position ;
                mDocument.mEndBlock = mDocument.mStartBlock + diff ;

                mDocument.SetPosition(mDocument.mEndBlock) ;
            }
        }
    }
}



void cEditorCtrl::DeleteBlock(void)
{
    if(mDocument.mBlockSet == true)
    {
        mDocument.SaveUndo() ;

        mDocument.SetPosition(mDocument.mStartBlock) ;

        Cut() ;

        mDocument.mBlockSet = false ;
        mDocument.SetPosition(mDocument.mStartBlock) ;

        UnSetBlock() ;
    }

}


void cEditorCtrl::UpperCaseBlock(void)
{
    if(mDocument.mBlockSet == true)
    {
        mDocument.SaveUndo() ;

        POSITION_T start = mDocument.mStartBlock ;
        POSITION_T end = mDocument.mEndBlock ;
        POSITION_T position = mDocument.GetPosition() ;

        if(start > end || end > mDocument.GetTextSize())
        {
            // error
            return ;
        }

        eShowControl show = GetShowControls() ;
        SetShowControls(SHOW_ALL) ;

        string ch, str ;
        for(POSITION_T loop = start; loop < end; loop++)
        {
            ch = mDocument.GetChar(loop) ;
            str += ch ;
        }

        vector<utf8proc_int32_t> codepoints ;

        string ntext = mDocument.Normalize(str) ;

        mDocument.GetCodePoints(ntext, codepoints) ;
        DeleteBlock() ;
        mDocument.SetPosition(start) ;

        for(size_t loop = 0; loop < codepoints.size(); loop++)
        {
            mDocument.Insert(static_cast<CHAR_T>(utf8proc_toupper(codepoints[loop]))) ;
        }

        SetShowControls(show) ;

        mDocument.SetPosition(start) ;
        SetBeginBlock() ;
        mDocument.SetPosition(end) ;
        SetEndBlock() ;

        if(mDocument.mEndBlock < position)
        {
            ScrollIntoView(Qt::Key_Down) ;
        }
        else
        {
            ScrollIntoView(Qt::Key_Up) ;
        }
    }
}



void cEditorCtrl::LowerCaseBlock(void)
{
    if(mDocument.mBlockSet == true)
    {
        mDocument.SaveUndo() ;

        POSITION_T start = mDocument.mStartBlock ;
        POSITION_T end = mDocument.mEndBlock ;
        POSITION_T position = mDocument.GetPosition() ;

        if(start > end || end > mDocument.GetTextSize())
        {
            // error
            return ;
        }

        eShowControl show = GetShowControls() ;
        SetShowControls(SHOW_ALL) ;

        string ch, str ;
        for(POSITION_T loop = start; loop < end; loop++)
        {
            ch = mDocument.GetChar(loop) ;
            str += ch ;
        }

        vector<utf8proc_int32_t> codepoints ;

        string ntext = mDocument.Normalize(str) ;

        mDocument.GetCodePoints(ntext, codepoints) ;
        DeleteBlock() ;
        mDocument.SetPosition(start) ;

        for(size_t loop = 0; loop < codepoints.size(); loop++)
        {
            mDocument.Insert(static_cast<CHAR_T>(utf8proc_tolower(codepoints[loop]))) ;
        }

        SetShowControls(show) ;

        mDocument.SetPosition(start) ;
        SetBeginBlock() ;
        mDocument.SetPosition(end) ;
        SetEndBlock() ;

        if(mDocument.mEndBlock < position)
        {
            ScrollIntoView(Qt::Key_Down) ;
        }
        else
        {
            ScrollIntoView(Qt::Key_Up) ;
        }
    }
}


// not working
void cEditorCtrl::SentenceBlock(void)
{
    if(mDocument.mBlockSet == true)
    {
        mDocument.SaveUndo() ;

        POSITION_T start = mDocument.mStartBlock ;
        POSITION_T end = mDocument.mEndBlock ;
        POSITION_T position = mDocument.GetPosition() ;

        eShowControl show = GetShowControls() ;
        SetShowControls(SHOW_ALL) ;

        string ch, str ;
        for(POSITION_T loop = mDocument.mStartBlock; loop < mDocument.mEndBlock; loop++)
        {
            ch = mDocument.GetChar(loop) ;
            str += ch ;
        }

        vector<utf8proc_int32_t> codepoints ;

        string ntext = mDocument.Normalize(str) ;

        mDocument.GetCodePoints(ntext, codepoints) ;
        DeleteBlock() ;
        mDocument.SetPosition(start) ;

        for(size_t loop = 0; loop < codepoints.size(); loop++)
        {
            utf8proc_category_t category = utf8proc_category(codepoints[loop]) ;
            if(category == UTF8PROC_CATEGORY_PE)
            {
                for(size_t loop2 = loop + 1; loop2 < codepoints.size(); loop2++)
                {
                    loop++ ;
                    category = utf8proc_category(codepoints[loop2]) ;
                    if(category == UTF8PROC_CATEGORY_ZS)
                    {
                        mDocument.Insert(static_cast<CHAR_T>(utf8proc_tolower(codepoints[loop2]))) ;
                        continue ;
                    }

                    if(category != UTF8PROC_CATEGORY_ZS)
                    {
                        mDocument.Insert(static_cast<CHAR_T>(utf8proc_toupper(codepoints[loop2]))) ;
                    }
                }
            }
            else
            {
                mDocument.Insert(static_cast<CHAR_T>(utf8proc_tolower(codepoints[loop]))) ;
            }
        }

        SetShowControls(show) ;

        mDocument.SetPosition(start) ;
        SetBeginBlock() ;
        mDocument.SetPosition(end) ;
        SetEndBlock() ;

        if(mDocument.mEndBlock < position)
        {
            ScrollIntoView(Qt::Key_Down) ;
        }
        else
        {
            ScrollIntoView(Qt::Key_Up) ;
        }
    }
}





void cEditorCtrl::UnSetBlock(void)
{
    mDocument.mStartBlock = 0 ;
    mDocument.mEndBlock = 0 ;
    mDocument.mBlockSet = false ;
}


void cEditorCtrl::ToggleHideBlock(void)
{
    if(mDocument.mBlockSet == true)
    {
        mDocument.mBlockSet = false ;
        mDocument.mOldStartBlock = mDocument.mStartBlock ;
        mDocument.mOldEndBlock = mDocument.mEndBlock ;

        mDocument.mStartBlock = 0 ;
        mDocument.mEndBlock = 0 ;
    }
    else
    {
//        if(mOldEndBlock > mOldStartBlock)
        {
            mDocument.mStartBlock = mDocument.mOldStartBlock ;
            mDocument.mEndBlock = mDocument.mOldEndBlock ;

            mDocument.mBlockSet = true ;
        }
    }
}


void cEditorCtrl::ClipboardPaste(void)
{
    const QClipboard *clipboard = QApplication::clipboard() ;
    const QMimeData *mimeData = clipboard->mimeData();

    QStringList format = mimeData->formats() ;
    for (int i = 0; i < format.size(); ++i)
    {
        printf("%s\n", format.at(i).toLocal8Bit().constData()) ;
    }

    // are we pasting RTF data?
//    if(format.contains("text/rtf"))
//    {
//        QByteArray d = mimeData->data("text/rtf") ;
//        string str = d.constData() ;
//        cRTFFile rtf(this) ;
//        rtf.LoadString(str) ;
//    }
//    else if(mimeData->hasText())
    if(mimeData->hasText())
    {
        mDocument.SaveUndo() ;

        QString str = clipboard->text() ;

        string utf8 = str.toUtf8().toStdString() ;

        mDocument.Insert(utf8) ;
/*
        utf8 = mDocument.Normalize(utf8) ;
        vector<utf8proc_int32_t> offsets ;
        mDocument.GetCodePoints(utf8, offsets) ;
        for(size_t loop = 0; loop < offsets.size(); loop++)
        {
            CHAR_T ch = static_cast<CHAR_T>(offsets[loop]) ;
            if(ch == 10)         // line feed
            {
                ch = HARD_RETURN ;
            }
            mDocument.Insert(ch) ;
        }
*/
    }
    else
    {
        QMessageBox msgBox(QMessageBox::Critical, "Error", "Unknown data format - cannot paste", QMessageBox::Ok, this) ;
        msgBox.exec() ;
    }
}



void cEditorCtrl::ClipboardCopy(void)
{
    if(mDocument.mBlockSet)
    {
        string str ;
        for(POSITION_T loop = mDocument.mStartBlock; loop < mDocument.mEndBlock; loop++)
        {
            string t = mDocument.GetChar(loop) ;
            str.append(t) ;
        }

        QClipboard *clipboard = QApplication::clipboard() ;
        clipboard->setText(QString::fromUtf8(str.c_str()), QClipboard::Clipboard) ; //   fromStdString(str)) ;
    }
}


void cEditorCtrl::SavePosition(int offset)
{
    POSITION_T position = mDocument.GetPosition() ;
    if(mDocument.mSavePosition[offset] == position)
    {
        mDocument.mSavePosition[offset] = 0 ;
    }
    else
    {
        mDocument.mSavePosition[offset] = position ;
    }
}


void cEditorCtrl::GotoSavePosition(int offset)
{
    POSITION_T position = mDocument.GetPosition() ;
    
    if(mDocument.mSavePosition[offset] != 0)
    {
        mDocument.SetPosition(mDocument.mSavePosition[offset]) ;
        if(mDocument.mSavePosition[offset] < position)
        {
            ScrollIntoView(Qt::Key_Up) ;
        }
        else
        {
            ScrollIntoView(Qt::Key_Down) ;
        }
    }
}


void cEditorCtrl::WordCountBlock(void)
{
    long words = 0 ;

    if(mDocument.mBlockSet == true)
    {
        words = WordCount(mDocument.mStartBlock, mDocument.mEndBlock) ;
    }
    else
    {
        words = WordCount(0, 0) ;
    }

    QString tmp ;
    tmp.sprintf("       \n\n       Word Count: %ld       \n\n\n", words) ;
    QMessageBox msgBox(QMessageBox::Information, "Word Count", tmp, QMessageBox::Ok, this) ;
    msgBox.exec() ;
}




void cEditorCtrl::Replace(void)
{
    QDialog dialog(this) ;

    Ui::FindReplace findandreplace ;
    findandreplace.setupUi(&dialog) ;

    findandreplace.mFind->setFocus() ;
    findandreplace.mNextOccurance->setChecked(true) ;

    findandreplace.mMaintCase->setEnabled(false) ;
    findandreplace.mWholeWords->setEnabled(false) ;

    int ecode = dialog.exec() ;

    if(ecode != 0)
    {
        // get the info from the dialog box
        mSearchText = findandreplace.mFind->text().toStdString() ;
        mReplaceText = findandreplace.mReplace->text().toStdString() ;

        if(mSearchText.length() != 0)
        {
            bool next, entire, rest ;

            next = findandreplace.mNextOccurance->isChecked() ;
            entire = findandreplace.mGlobal->isChecked() ;
            rest = findandreplace.mRestofFile->isChecked() ;

            bool dontask = findandreplace.mDontAsk->isChecked() ;
//            bool mcase = findandreplace.mMaintCase->isChecked() ;

//            bool wholeword = findandreplace.mWholeWords->isChecked() ;
            mCaseCmp = findandreplace.mIgnoreCase->isChecked() ;
            mSearchBackwards = findandreplace.mBackward->isChecked() ;
            mWildCard = findandreplace.mUseWildCard->isChecked() ;

            mReplaceSize = static_cast<POSITION_T>(mSearchText.length()) ;
            mReplaceAsk = !dontask ;
            
            // if we do the next one only
            if(next)
            {
                ReplaceAgain() ;
            }
            else if(entire)
            {
                if(mSearchBackwards == false)
                {
                    mDocument.SetPosition(0) ;
                }
                else
                {
                    mDocument.SetPosition(mDocument.GetTextSize() - 1) ;
                }
                
                bool retval = false ;
                ssize_t count = -1 ;
                while(retval == false)
                {
                    count++ ;
                    retval = ReplaceAgain() ;
                }
                
                QString t ;
                t.sprintf("Replaced %ld items", count) ;
                QMessageBox msgBox(QMessageBox::Information, "Replace", t, QMessageBox::Ok, this) ;
                msgBox.exec() ;
            }
            else if(rest)
            {
                bool retval = false ;
                ssize_t count = -1 ;
                while(retval == false)
                {
                    count++ ;
                    retval = ReplaceAgain() ;
                }
                
                QString t ;
                t.sprintf("Replaced %ld items", count) ;
                QMessageBox msgBox(QMessageBox::Information, "Replace", t, QMessageBox::Ok, this) ;
                msgBox.exec() ;
            }
        }
    }
}


bool cEditorCtrl::ReplaceAgain(bool noquery)
{
    UNUSED_ARGUMENT(noquery) ;
    
    bool retval = false ;
    POSITION_T fpos ;
    
    if(mSearchBackwards == false)
    {
        if(mWholeFile)
        {
            mDocument.SetPosition(0) ;
        }
        
        fpos = mDocument.FindNext(mSearchText, mDocument.GetPosition() + 1, mWildCard, mCaseCmp) ;
        
        if(fpos == mDocument.GetTextSize())
        {
            if(mReplaceAsk == true)
            {
                QString temp ;
                temp.sprintf("At end of document.") ;
                QMessageBox msgBox(QMessageBox::Information, "Not Found", temp, QMessageBox::Ok, this) ;
                msgBox.exec() ;
            }
            retval = true ;
        }
        else
        {
            mDocument.SetPosition(fpos) ;
            ScrollIntoView(Qt::Key_Down) ;
            
            bool rep = true ;
            if(mReplaceAsk == true)
            {
                QMessageBox msgBox(QMessageBox::Question, "Replace", "Replace?", QMessageBox::Yes | QMessageBox::No, this) ;
                msgBox.setDefaultButton(QMessageBox::Yes) ;
                int answer = msgBox.exec() ;
                if(answer == QMessageBox::No)
                {
                    rep = false ;
                }
            }

            if(rep == true)
            {
                mDocument.SaveUndo() ;

                Delete(mDocument.GetPosition(), mReplaceSize) ;
                mDocument.Insert(mReplaceText.c_str()) ;
            }

        }
    }
    else
    {
        if(mWholeFile)
        {
            mDocument.SetPosition(mDocument.GetTextSize() - 2) ;
        }

        fpos = mDocument.FindPrev(mSearchText, mDocument.GetPosition(), mWildCard, mCaseCmp) ;

        if(fpos == 0)
        {
            if(mReplaceAsk == true)
            {
                QString temp ;
                temp.sprintf("At start of document.") ;
                QMessageBox msgBox(QMessageBox::Information, "Not Found", temp, QMessageBox::Ok, this) ;
                msgBox.exec() ;
            }
            retval = true ;
        }
        else
        {
            mDocument.SetPosition(fpos) ;
            ScrollIntoView(Qt::Key_Up) ;

            bool rep = true ;
            if(mReplaceAsk == true)
            {
                QMessageBox msgBox(QMessageBox::Question, "Replace", "Replace?", QMessageBox::Yes | QMessageBox::No, this) ;
                msgBox.setDefaultButton(QMessageBox::Yes) ;
                int answer = msgBox.exec() ;
                if(answer == QMessageBox::No)
                {
                    rep = false ;
                }
            }

            if(rep == true)
            {
                Delete(mDocument.GetPosition(), mReplaceSize) ;
                mDocument.Insert(mReplaceText.c_str()) ;
            }
        }
    }

    return retval ;
}


void cEditorCtrl::Find(void)
{
    QDialog dialog(this) ;

    Ui::Find find ;
    find.setupUi(&dialog) ;

    find.mFind->setFocus() ;
    find.mNextOccurance->setChecked(true) ;

    int ecode = dialog.exec() ;

    if(ecode != 0)
    {
        // get the info from the dialog box
        QString text = find.mFind->text() ;
        if(text.length() != 0)
        {

            mWholeFile = find.mGlobal->isChecked() ;
            mCaseCmp = find.mIgnoreCase->isChecked() ;
            mSearchBackwards = find.mBackward->isChecked() ;
            mWildCard = find.mUseWildCard->isChecked() ;
            mWholeWord = find.mWholeWord->isChecked() ;

            mSearchText = text.toStdString() ;
            FindAgain() ;
        }
    }
}



void cEditorCtrl::FindAgain(void)
{
    if(mSearchBackwards == false)
    {
        POSITION_T fpos = mDocument.FindNext(mSearchText, mDocument.GetPosition() + 1, mWildCard, mCaseCmp, mWholeWord) ;

        if(fpos == mDocument.GetTextSize())
        {
            QString temp ;
            temp.sprintf("Search word: %s  not found.", mSearchText.c_str()) ;
            QMessageBox msgBox(QMessageBox::Information, "Not found", temp, QMessageBox::Ok, this) ;
            msgBox.exec() ;
        }
        else
        {
            mDocument.SetPosition(fpos) ;
            mStartSearchBlock = fpos ;
            mEndSearchBlock = fpos + static_cast<POSITION_T>(mSearchText.length()) ;
            mSearchBlockSet = true ;
            ScrollIntoView(Qt::Key_Down) ;
            ScrollIntoView(Qt::Key_Up) ;            // some subtle bug here, need to scroll into view twice on first search
        }
    }
    else
    {
        POSITION_T fpos = mDocument.FindPrev(mSearchText, mDocument.GetPosition(), mWildCard, mCaseCmp, mWholeWord) ;

        if(fpos == 0)
        {
            QString temp ;
            temp.sprintf("Search word: %s  not found.", mSearchText.c_str()) ;
            QMessageBox msgBox(QMessageBox::Information, "Not found", temp, QMessageBox::Ok, this) ;
            msgBox.exec() ;
        }
        else
        {
            mDocument.SetPosition(fpos) ;
            mStartSearchBlock = fpos ;
            mEndSearchBlock = fpos + static_cast<POSITION_T>(mSearchText.length()) ;
            mSearchBlockSet = true ;
            ScrollIntoView(Qt::Key_Up) ;
            ScrollIntoView(Qt::Key_Down) ;            // some subtle bug here, need to scroll into view twice on first search
        }

    }
}



void cEditorCtrl::SelectFont(void)
{
    bool ok ;
    QFont font = QFontDialog::getFont(&ok, this) ;

    if(ok)
    {
        sInternalFonts intfont ;
//        intfont.font = font ;
        intfont.name = font.family().toStdString() ;
        intfont.haveWSFont = false ;

        std::bitset<16> style ;

        QFontInfo info(font) ;
        bool fp = info.fixedPitch() ;

        if(fp)
        {
            style.set(11, 0) ;
            style.set(10, 0) ;
            style.set(15, 0) ;                  // proportional or not
        }
        else
        {
            style.set(11, 0) ;
            style.set(10, 1) ;
            style.set(15, 1) ;                  // proportional or not
        }
///* so, styleHint doesn't work at all. Ugh.
        // set the font family
        switch(info.styleHint())  // (font.styleHint())
        {
            case QFont::SansSerif :
                style.set(11, 0) ;
                style.set(10, 0) ;
                style.set(15, 1) ;                  // proportional or not
                break ;

            case QFont::Serif :
                style.set(11, 0) ;
                style.set(10, 1) ;
                style.set(15, 1) ;                  // proportional or not
                break ;

            case QFont::Cursive :
                style.set(11, 1) ;
                style.set(10, 0) ;
                style.set(15, 1) ;                  // proportional or not
                break ;

            case QFont::TypeWriter :
                style.set(11, 0) ;
                style.set(10, 0) ;
                style.set(15, 0) ;                  // proportional or not
                break ;

            default :
                style.set(11, 1) ;
                style.set(10, 1) ;
                style.set(15, 1) ;                  // proportional or not
                break ;
        }
//*/
        // set the font encode
//        switch(encoding)
//        {
//            case QFontENCODING_CP437 :
                style.set(13, 0) ;
                style.set(12, 0) ;
//                break ;
/*
            case QFontENCODING_CP850 :
                style.set(13, 0) ;
                style.set(12, 1) ;

            default :
                style.set(13, 0) ;
                style.set(12, 0) ;
                break ;
        }
*/
        // set the font typestyle.set
        // @todo FONTS
        style.set(8, 1) ;
        style.set(7, 1) ;
        style.set(6, 1) ;
        style.set(5, 1) ;
        style.set(4, 1) ;
        style.set(3, 1) ;
        style.set(2, 1) ;
        style.set(1, 1) ;
        style.set(0, 1) ;

        intfont.wsfont.style = static_cast<unsigned short>(style.to_ulong()) ;
        intfont.wsfont.height = static_cast<unsigned short>(font.pointSize()) * 20 ;

        intfont.size = font.pointSizeF();
        intfont.fontname = font.family().toStdString();

        mDocument.InsertFont(intfont) ;

//        RedrawFullDisplay() ;
    }
}


void cEditorCtrl::SelectColor(void)
{
    QColor tcolor = QColorDialog::getColor(mTextColour, this, "Please Choose the text color") ;
    if(tcolor != mTextColour)
    {
        sColorTable color ;
        color.red = tcolor.red() ;
        color.green = tcolor.green() ;
        color.blue = tcolor.blue() ;

        mDocument.InsertColor(color) ;
    }
}


void cEditorCtrl::MoveCursorTopLeft(void)
{
    POSITION_T position = mLayout->GetLineStartPosition(mFirstLine) ;
    mDocument.SetPosition(position) ;
    ScrollIntoView(Qt::Key_Up) ;
}


void cEditorCtrl::MoveCursorBottomRight(void)
{
    POSITION_T position = mLayout->GetLineStartPosition(mFirstLine + mDisplayLines + 1) - 1 ;
    mDocument.SetPosition(position) ;
    ScrollIntoView(Qt::Key_Down) ;
}


void cEditorCtrl::MoveCursorTopofFile(void)
{
    mDocument.SetPosition(0) ;
    ScrollIntoView(Qt::Key_Up) ;
}

void cEditorCtrl::MoveCursorEndofFile(void)
{
    POSITION_T position = mDocument.GetTextSize() - 2 ;  // subtract extra for our non displayed ^Z
    mDocument.SetPosition(position) ;
    ScrollIntoView(Qt::Key_Down) ;
}


void cEditorCtrl::MoveCursorStartBlock(void)
{
    if(mDocument.mBlockSet)
    {
        Qt::Key dir = Qt::Key_Down ;
        if(mDocument.GetPosition() > mDocument.mStartBlock)
        {
            dir = Qt::Key_Up ;
        }
        POSITION_T position = mDocument.mStartBlock ;
        mDocument.SetPosition(position) ;
        ScrollIntoView(dir) ;
    }
}


void cEditorCtrl::MoveCursorEndBlock(void)
{
    if(mDocument.mBlockSet)
    {
        Qt::Key dir = Qt::Key_Down ;
        if(mDocument.GetPosition() > mDocument.mStartBlock)
        {
            dir = Qt::Key_Up ;
        }
        POSITION_T position = mDocument.mEndBlock ;
        mDocument.SetPosition(position) ;
        ScrollIntoView(dir) ;
    }
}


void cEditorCtrl::MoveCursorStartLine(void)
{
    LINE_T line = mLayout->GetLineFromPosition(mDocument.GetPosition()) ;
    mDocument.SetPosition(mLayout->GetLineStartPosition(line)) ;
}




void cEditorCtrl::MoveCursorEndLine(void)
{
    LINE_T line = mLayout->GetLineFromPosition(mDocument.GetPosition()) ;
    mDocument.SetPosition(mLayout->GetLineEndPosition(line)) ;
}



vector<COORD_T> cEditorCtrl::GetTabs(void)
{
    return mTabStops ;
}


void cEditorCtrl::SetTabs(vector<COORD_T> &tabs)
{
    mTabStops = tabs ;
//    mRuler->SetTabStops(mTabStops) ;
}

void cEditorCtrl::About(void)
{
    QMessageBox about(this) ;
    about.about(this, "WordTsar", FULLVERSION_STRING) ;
}


void cEditorCtrl::PrintPreview(void)
{
    if(mPrintLayout == nullptr)
    {
        mPrintLayout = new cLayout(&mDocument, this, true) ;            // true does header/footer things
    }

    mInPrintPreview = true ;

    // layout the document for printing
    eShowControl oldsc = GetShowControls();
    SetShowControls(SHOW_NONE);
    PARAGRAPH_T para = mDocument.GetNumberofParagraphs();
    for (PARAGRAPH_T loop = 0; loop < para; loop++)
    {
        mLayoutParagraph = loop;
        IdleLayout(mPrintLayout);
    }

    SetStatus(" ") ;



    cPrintout print(this) ;
    print.PrintPreview();

    SetShowControls(oldsc);

    mInPrintPreview = false ;
}



void cEditorCtrl::SpellCheckDocument(void)
{
    // @TODO
//    cHunspellSpellCheck spell(this) ;
//    spell.CheckDocument() ;
}



void cEditorCtrl::SpellCheckWord(void)
{
//    cHunspellSpellCheck spell(this) ;
//    spell.CheckWord()
}


void cEditorCtrl::PageLayout(void) 
{
    QDialog dialog(this) ;

    Ui::PageLayout pagelayout ;
    pagelayout.setupUi(&dialog) ;

    double value ;
    QString text ;

    // populate page size combo list
    for(int loop = 1; loop < 200; loop++)
    {
        QPageSize psize(static_cast<QPageSize::PageSizeId>(loop)) ;
        if(psize.isValid() == false)
        {
            continue ;
        }

        QString name = psize.name() ;

        QString full ;
        QSizeF size ;

        if(mMeasure == MSR_CENTIMETERS || mMeasure == MSR_MILLIMETERS)
        {
            size = psize.size(QPageSize::Millimeter) ;
            full.sprintf("%s, %d x %d mm", name.toUtf8().constData(), static_cast<int>(size.width()), static_cast<int>(size.height())) ;
        }
        else
        {
            size = psize.size(QPageSize::Inch) ;
            full.sprintf("%s, %.2f x %.2f in", name.toUtf8().constData(), size.width(), size.height()) ;
        }

        pagelayout.mPaperSize->addItem(full, loop) ;
    }

    pagelayout.mPaperSize->setCurrentIndex(mPaperId - 1) ;

// @TODO page orientation set
    pagelayout.mOrientation->setEnabled(false) ;
    
    value = mLayout->mPageOffsetOdd ;
    switch(mMeasure)
    {
        case MSR_INCHES :
            text.sprintf("%0.2f\"", value / TWIPSPERINCH) ;
            break ;
        
        case MSR_MILLIMETERS :
            text.sprintf("%0.2f\"", value / TWIPSPERMM) ;
            break ;

        default :
            text.sprintf("%0.2f\"", value / TWIPSPERCM) ;
            break ;
    }
    pagelayout.mOddPageOffset->setText(text) ;

    value = mLayout->mPageOffsetEven ;
    switch(mMeasure)
    {
        case MSR_INCHES :
            text.sprintf("%0.2f\"", value / TWIPSPERINCH) ;
            break ;
        
        case MSR_MILLIMETERS :
            text.sprintf("%0.2f\"", value / TWIPSPERMM) ;
            break ;

        default :
            text.sprintf("%0.2f\"", value / TWIPSPERCM) ;
            break ;
    }
    pagelayout.mEvenPageOffset->setText(text) ;

    value = mLayout->mTopMargin ;
    switch(mMeasure)
    {
        case MSR_INCHES :
            text.sprintf("%0.2f\"", value / TWIPSPERINCH) ;
            break ;
        
        case MSR_MILLIMETERS :
            text.sprintf("%0.2f\"", value / TWIPSPERMM) ;
            break ;

        default :
            text.sprintf("%0.2f\"", value / TWIPSPERCM) ;
            break ;
    }
    pagelayout.mTopMargin->setText(text) ;

    value = mLayout->mBottomMargin ;
    switch(mMeasure)
    {
        case MSR_INCHES :
            text.sprintf("%0.2f\"", value / TWIPSPERINCH) ;
            break ;
        
        case MSR_MILLIMETERS :
            text.sprintf("%0.2f\"", value / TWIPSPERMM) ;
            break ;

        default :
            text.sprintf("%0.2f\"", value / TWIPSPERCM) ;
            break ;
    }
    pagelayout.mBottomMargin->setText(text) ;

    value = mLayout->mRightMargin ;
    switch(mMeasure)
    {
        case MSR_INCHES :
            text.sprintf("%0.2f\"", value / TWIPSPERINCH) ;
            break ;
        
        case MSR_MILLIMETERS :
            text.sprintf("%0.2f\"", value / TWIPSPERMM) ;
            break ;

        default :
            text.sprintf("%0.2f\"", value / TWIPSPERCM) ;
            break ;
    }
    pagelayout.mRightMargin->setText(text) ;

    value = mLayout->mHeaderMargin ;
    switch(mMeasure)
    {
        case MSR_INCHES :
            text.sprintf("%0.2f\"", value / TWIPSPERINCH) ;
            break ;
        
        case MSR_MILLIMETERS :
            text.sprintf("%0.2f\"", value / TWIPSPERMM) ;
            break ;

        default :
            text.sprintf("%0.2f\"", value / TWIPSPERCM) ;
            break ;
    }
    pagelayout.mHeader->setText(text) ;

    value = mLayout->mFooterMargin ;
    switch(mMeasure)
    {
        case MSR_INCHES :
            text.sprintf("%0.2f\"", value / TWIPSPERINCH) ;
            break ;
        
        case MSR_MILLIMETERS :
            text.sprintf("%0.2f\"", value / TWIPSPERMM) ;
            break ;

        default :
            text.sprintf("%0.2f\"", value / TWIPSPERCM) ;
            break ;
    }
    pagelayout.mFooter->setText(text) ;


    int ecode = dialog.exec() ;

    
    if(ecode != 0)
    {
        // insert anything we need at the beginning of our current paragraph
        PARAGRAPH_T para = mDocument.GetParagraphFromPosition(mDocument.GetPosition()) ;
        POSITION_T start, end ;
        mDocument.GetParagraphStartandEnd(para, start, end) ;
        mDocument.SetPosition(start) ;

        int newtype = pagelayout.mPaperSize->currentIndex() ;
        int newpaper = pagelayout.mPaperSize->currentData().toInt() ;
        if(newtype != mPaperId)
        {
            mPaperId = static_cast<QPageSize::PageSizeId>(newpaper) ;

            QString t ;
            t.sprintf(".pt %d (%s)", newpaper, pagelayout.mPaperSize->currentText().toUtf8().constData()) ;
            mDocument.Insert(t.toUtf8().constData()) ;
            mDocument.Insert(HARD_RETURN) ;
        }

        bool incdec ;
        QString str = pagelayout.mOddPageOffset->text() ;
        double num = mDocument.GetValue(str.toStdString(), incdec);
        char type = mDocument.GetType(str.toStdString());
        value = mDocument.ConvertToTwips(num, type);
        if(FuzzyCompare(value, mLayout->mPageOffsetOdd)) //  value != mLayout->mPageOffsetOdd)
        {
            QString t ;
            switch(mMeasure)
            {
                case MSR_INCHES :
                    t.sprintf(".poo %0.2f\"", value / TWIPSPERINCH) ;
                    break ;
                
                case MSR_MILLIMETERS :
                    t.sprintf(".poo %0.2fmm", value / TWIPSPERMM) ;
                    break ;

                default :
                    t.sprintf(".poo %0.2fcm", value / TWIPSPERCM) ;
                    break ;
            }
            mDocument.Insert(t.toUtf8().constData()) ;
            mDocument.Insert(HARD_RETURN) ;
        }

        str = pagelayout.mEvenPageOffset->text() ;
        num = mDocument.GetValue(str.toStdString(), incdec);
        type = mDocument.GetType(str.toStdString());
        value = mDocument.ConvertToTwips(num, type);
        if(value != mLayout->mPageOffsetEven)
        {
            QString t ;
            switch(mMeasure)
            {
                case MSR_INCHES :
                    t.sprintf(".poe %0.2f\"", value / TWIPSPERINCH) ;
                    break ;
                
                case MSR_MILLIMETERS :
                    t.sprintf(".poe %0.2fmm", value / TWIPSPERMM) ;
                    break ;

                default :
                    t.sprintf(".poe %0.2fcm", value / TWIPSPERCM) ;
                    break ;
            }
            mDocument.Insert(t.toUtf8().constData()) ;
            mDocument.Insert(HARD_RETURN) ;
        }

        str = pagelayout.mTopMargin->text() ;
        num = mDocument.GetValue(str.toStdString(), incdec);
        type = mDocument.GetType(str.toStdString());
        value = mDocument.ConvertToTwips(num, type);
        if(value != mLayout->mTopMargin)
        {
            QString t ;
            switch(mMeasure)
            {
                case MSR_INCHES :
                    t.sprintf(".mt %0.2f\"", value / TWIPSPERINCH) ;
                    break ;
                
                case MSR_MILLIMETERS :
                    t.sprintf(".mt %0.2fmm", value / TWIPSPERMM) ;
                    break ;

                default :
                    t.sprintf(".mt %0.2fcm", value / TWIPSPERCM) ;
                    break ;
            }
            mDocument.Insert(t.toUtf8().constData()) ;
            mDocument.Insert(HARD_RETURN) ;
        }

        str = pagelayout.mBottomMargin->text() ;
        num = mDocument.GetValue(str.toStdString(), incdec);
        type = mDocument.GetType(str.toStdString());
        value = mDocument.ConvertToTwips(num, type);
        if(value != mLayout->mBottomMargin)
        {
            QString t ;
            switch(mMeasure)
            {
                case MSR_INCHES :
                    t.sprintf(".mb %0.2f\"", value / TWIPSPERINCH) ;
                    break ;
                
                case MSR_MILLIMETERS :
                    t.sprintf(".mb %0.2fmm", value / TWIPSPERMM) ;
                    break ;

                default :
                    t.sprintf(".mb %0.2fcm", value / TWIPSPERCM) ;
                    break ;
            }
            mDocument.Insert(t.toUtf8().constData()) ;
            mDocument.Insert(HARD_RETURN) ;
        }

        str = pagelayout.mRightMargin->text() ;
        num = mDocument.GetValue(str.toStdString(), incdec);
        type = mDocument.GetType(str.toStdString());
        value = mDocument.ConvertToTwips(num, type);
        if(value != mLayout->mRightMargin)
        {
            QString t ;
            switch(mMeasure)
            {
                case MSR_INCHES :
                    t.sprintf(".rm %0.2f\"", value / TWIPSPERINCH) ;
                    break ;
                
                case MSR_MILLIMETERS :
                    t.sprintf(".rm %0.2fmm", value / TWIPSPERMM) ;
                    break ;

                default :
                    t.sprintf(".rm %0.2fcm", value / TWIPSPERCM) ;
                    break ;
            }
            mDocument.Insert(t.toUtf8().constData()) ;
            mDocument.Insert(HARD_RETURN) ;
        }

        str = pagelayout.mHeader->text() ;
        num = mDocument.GetValue(str.toStdString(), incdec);
        type = mDocument.GetType(str.toStdString());
        value = mDocument.ConvertToTwips(num, type);
        if(value != mLayout->mHeaderMargin)
        {
            QString t ;
            switch(mMeasure)
            {
                case MSR_INCHES :
                    t.sprintf(".hm %0.2f\"", value / TWIPSPERINCH) ;
                    break ;
                
                case MSR_MILLIMETERS :
                    t.sprintf(".hm %0.2fmm", value / TWIPSPERMM) ;
                    break ;

                default :
                    t.sprintf(".hm %0.2fcm", value / TWIPSPERCM) ;
                    break ;
            }
            mDocument.Insert(t.toUtf8().constData()) ;
            mDocument.Insert(HARD_RETURN) ;
        }

        str = pagelayout.mFooter->text() ;
        num = mDocument.GetValue(str.toStdString(), incdec);
        type = mDocument.GetType(str.toStdString());
        value = mDocument.ConvertToTwips(num, type);
        if(value != mLayout->mFooterMargin)
        {
            QString t ;
            switch(mMeasure)
            {
                case MSR_INCHES :
                    t.sprintf(".fm %0.2f\"", value / TWIPSPERINCH) ;
                    break ;
                
                case MSR_MILLIMETERS :
                    t.sprintf(".fm %0.2fmm", value / TWIPSPERMM) ;
                    break ;

                default :
                    t.sprintf(".fm %0.2fcm", value / TWIPSPERCM) ;
                    break ;
            }
            mDocument.Insert(t.toUtf8().constData()) ;
            mDocument.Insert(HARD_RETURN) ;
        }
        
        QString t ;
        if(pagelayout.mLandscape->isChecked())
        {
            t.sprintf(".pr or=p") ;
            mDocument.Insert(t.toUtf8().constData()) ;
            mDocument.Insert(HARD_RETURN) ;
        }
        else
        {
            t.sprintf(".pr or=l") ;
            mDocument.Insert(t.toUtf8().constData()) ;
            mDocument.Insert(HARD_RETURN) ;
        }
    }

}


void cEditorCtrl::Preferences(void)
{
    QDialog preferences(this) ;

    Ui::Preferences ui ;
    ui.setupUi(&preferences) ;

    ui.mBackgroundColor->setColor(GetBGroundColour()) ;
    ui.mTextColor->setColor(GetTextColour()) ;
    ui.mControlColor->setColor(GetHighlightColour()) ;
    ui.mBlockColor->setColor(GetBlockColour()) ;
    ui.mDotColor->setColor(GetDotColour()) ;
    ui.mCommentColor->setColor(GetCommentColour()) ;
    ui.mUnknownColor->setColor(GetUnknownColour()) ;

    ui.mShortName->setText(mShortName) ;
    ui.mLongName->setText(mLongName) ;

    if(mAlwaysDot == true)
    {
        ui.mFlagAwlaysOn->setChecked(true) ;
    }
    else
    {
        ui.mFlagWithTags->setChecked(true) ;
    }

    if(mAlwaysFlag == true)
    {
        ui.mFlagColumn->setChecked(true) ;
    }
    else
    {
        ui.mFlagColumn->setChecked(true) ;
    }

    // disable control we don't do yet
    ui.mSoftSpaceDots->setEnabled(false) ;

    switch(mMeasure)
    {
        case MSR_INCHES :
            ui.mInches->setChecked(true) ;
            break ;

        case MSR_CENTIMETERS :
            ui.mCM->setChecked(true) ;
            break ;

        default :
            ui.mMM->setChecked(true) ;
            break ;
    }

    ui.mScrollBar->setChecked(mDispScrollBar) ;
    ui.mStyleBar->setChecked(mDispStyleBar) ;
    ui.mStatusLine->setChecked(mDispStatusBar) ;
    ui.mRulerLine->setChecked(mDispRuler) ;

    ui.mTabWidget->setCurrentWidget(ui.mFirstTab) ;

    int ecode = preferences.exec() ;
    
    if(ecode != 0)
    {
        SetBGroundColour(ui.mBackgroundColor->color()) ;
        SetTextColour(ui.mTextColor->color()) ;
        SetHighlightColour(ui.mControlColor->color()) ;
        SetBlockColour(ui.mBlockColor->color()) ;
        SetDotColour(ui.mDotColor->color()) ;
        SetCommentColour(ui.mCommentColor->color()) ;
        SetUnknownColour(ui.mUnknownColor->color()) ;
        
        mShortName = ui.mShortName->text() ;
        mLongName = ui.mLongName->text() ;
        
        if(ui.mInches->isChecked())
        {
            mMeasure = MSR_INCHES ;
            mMeasurement = "0i" ;
        }
        else if(ui.mCM->isChecked())
        {
            mMeasure = MSR_CENTIMETERS ;
            mMeasurement = "0cm" ;
        }
        else
        {
            mMeasure = MSR_MILLIMETERS ;
            mMeasurement = "0mm" ;
        }

        
        if(ui.mDotCommands->isChecked())
        {
            mAlwaysDot = true ;
            if(GetShowControls() == SHOW_ALL)
            {
                SetShowDot(true) ;
            }
        }
        else
        {
            mAlwaysDot = false ;
            if(GetShowControls() == SHOW_DOT)
            {
                SetShowDot(false) ;
            }
        }
        
        if(ui.mFlagAwlaysOn->isChecked())
        {
            mAlwaysFlag = true ;
        }
        else
        {
            mAlwaysFlag = false ;
        }
        
        mDispScrollBar = ui.mScrollBar->isChecked() ;
        mDispStyleBar = ui.mStyleBar->isChecked() ;
        mDispStatusBar = ui.mStatusLine->isChecked() ;
        mDispRuler = ui.mRulerLine->isChecked() ;
                
        if(mDispScrollBar)
        {
            mWordTsar->mScrollbar->show() ;
        }
        else
        {
            mWordTsar->mScrollbar->hide() ;
        }
        
        if(mDispStyleBar)
        {
            mWordTsar->mStatusTop->show() ;
        }
        else
        {
            mWordTsar->mStatusTop->hide() ;
        }

        if(mDispRuler)
        {
            mWordTsar->mRuler->show() ;
        }
        else
        {
            mWordTsar->mRuler->hide() ;
        }

        if(mDispStatusBar)
        {
            mWordTsar->mStatusBottom->show() ;
        }
        else
        {
            mWordTsar->mStatusBottom->hide() ;
        }

        update() ;
    }
}

void cEditorCtrl::GetStatus(sStatus &status)
{
    status.filename = mFileName ;
    status.column = 0 ; //mRenderers[mUseLayout]->mCursorX ;
    status.line = mCurrentLine ;
    status.page = mCurrentPage ; // mRenderers[mUseLayout]->mPageNumber ;
    status.pagecount = mLayout->GetNumberofPages() ; // mRenderers[mUseLayout]->GetNumberOfPages() ; // mPages.size() ;
    status.mode = mInsertMode ;
    status.wordcount = mWordCount ;
    status.charcount = mDocument.GetTextSize( );
    status.showcontrol = mShowControl ;
    status.saving = false ;
    status.help = mHelpDisplay ;
//    status.attrib = mRenderers[mUseLayout]->mInAttrib ;
    status.bold = mStatusBold ;
    status.italic = mStatusItalic ;
    status.underline = mStatusUnderline ;
    status.just = mStatusJust ;
    status.style = mStatusStyle ;
    status.font = mStatusFont ;
    status.height = mCurrentHeight ;
    status.width = 0 ;

//    .sprintf(status.filled, "%.2f%%", ((double)mDocument.GetTextSize() / ((double)SSIZE_MAX - 2.0)) * 100.0) ;
    sprintf(status.filled, " ") ;
}


void cEditorCtrl::Undo(void)
{
    mDocument.Undo() ;
}


void cEditorCtrl::Redo(void)
{
    mDocument.Redo() ;
}


void cEditorCtrl::FileIOProgress(int percent)
{
    if(mProgress != nullptr)
    {
        mProgress->setValue(percent) ;
    }
    QApplication::processEvents() ;
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
/// @param  position   [IN] - the current cursor position in the editor
/// @param  length     [IN] - the length of text we are deleting
///
/// @return bool - true on success
///
/// @brief
/// delete text from the editor
///
/// @todo figure out to do this for all charcaters, not one at a time.
/////////////////////////////////////////////////////////////////////////////
bool cEditorCtrl::Delete(POSITION_T position, POSITION_T length)
{
    bool retval = false ;

    eShowControl showcontrol = GetShowControls() ;
    SetShowControls(SHOW_ALL) ;

    mDocument.SaveUndo() ;


    retval = mDocument.Delete(position, length) ;

    SetShowControls(showcontrol) ;

    return retval ;
}




/////////////////////////////////////////////////////////////////////////////
///
/// @param  QString    String to display in slot 1 of status bar
///
/// @return nothing
///
/// @brief
/// Display the status text only if mWordTsar is set
///
/////////////////////////////////////////////////////////////////////////////
void cEditorCtrl::SetStatus(QString text, bool progress, int percent)
{
    if(mWordTsar != nullptr)
    {
        mWordTsar->SetStatus(text, progress, percent) ;
    }
}



/////////////////////////////////////////////////////////////////////////////
///
/// @return nothing
///
/// @brief
/// Tell the paint routine to redraw the entire display
///
/////////////////////////////////////////////////////////////////////////////
void cEditorCtrl::RedrawFullDisplay(void)
{
    mCaratTimer->stop() ;
    mDrawFullDisplay = true ;
    mDoDrawCaret = false ;
    update() ;
//    QCoreApplication::processEvents() ;
//    repaint() ;
}




/////////////////////////////////////////////////////////////////////////////
///
/// @return nothing
///
/// @brief
/// Draw the help displays. Very simple
///
/////////////////////////////////////////////////////////////////////////////
void cEditorCtrl::PaintHelp(QPainter &paint)
{
    double maxlineheight ;

//    paint.setPen(Qt::black) ;
    paint.setFont(mBaseFont) ;

    QBrush background(mBackgroundColour) ;
    paint.setBackground(background);
    paint.setPen(mTextColour) ;
    paint.eraseRect(mPainterRect) ;

    double width, height ;

    width = paint.device()->width() ;
    height= paint.device()->height() ;

    size_t numlines = mLayout->mParagraphLayout.size() ;
    COORD_T myy = 0 ; // mLayout->mDisplayList[0].y;  // hack

    // go through each of the paragraphs
    for(size_t ploop = 0; ploop < numlines; ploop++)
    {
        sParagraphLayout *paragraph = &mLayout->mParagraphLayout[ploop] ;
        maxlineheight = 0 ;

        // go through each of the lines (this is help, there should only be one per paragraph)
        for(sLineLayout line : paragraph->lines)
        {
            COORD_T x = line.x ;
//                COORD_T y = line.y ;

            // go through each of the segments in the line
            for(sSegmentLayout segment : line.segments)
            {
                // draw the segment a glyph at a time
                paint.setPen(mTextColour);
                paint.setFont(segment.font) ;
                for(size_t loop1= 0; loop1<  segment.glyph.size(); loop1 ++)
                {
                    QString str = QString::fromStdString(segment.glyph[loop1]) ;
                    QPointF point(x + segment.position[loop1], myy + segment.segmentheight) ;
                    paint.drawText(point , str);

                    if(segment.segmentheight > maxlineheight)
                    {
                        maxlineheight = segment.segmentheight ;
                    }
//wxLogMessage("loop %d, x %d y %d - %c\n", loop, mLayout->mDisplayList[loop].x + mLayout->mDisplayList[loop].position[loop1], myy, (char)mLayout->mDisplayList[loop].glyph[loop1] ) ;
                }
            }
        }


        myy += maxlineheight ;
    }

    // fit the window size to the text size
    COORD_T theight = myy + 7.0 ;
    if(height != theight)
    {
        setMaximumHeight(static_cast<int>(theight)) ;
        setMinimumHeight(static_cast<int>(theight)) ;
    }

//wxLogMessage("paint time %ldms in all", sw.Time());
}



/////////////////////////////////////////////////////////////////////////////
///
/// @return nothing
///
/// @brief
/// Create the display list to draw.
///
/// @todo Optimized to only draw what has changed.
///
/////////////////////////////////////////////////////////////////////////////
void cEditorCtrl::CreateDisplayList(QPainter *paint)
{
    COORD_T dispoffset = 0; 
    COORD_T lastoffset = 0 ;
    COORD_T suboffset = 0 ;
    COORD_T firstlineoffset = 0 ;
    PAGE_T lastpage  = 0 ;
    sDisplayList newdisplay ;
    sParagraphLayout newparagraph ;
    
    double maxlineheight = 0 ; 
    QFont font ;
    bool doexit = false ;
    
    POSITION_T caratposition = mDocument.GetPosition() ;
    PARAGRAPH_T currentparagraph = mDocument.GetParagraphFromPosition(caratposition) ;
    
    mDisplayLines = 0 ;
    mCaretPos.setWidth(0) ;
    mCaretPos.setHeight(0) ;
    
    mMouseRect.clear() ;
    mBlockCoords.clear() ;
    mSearchCoords.clear() ;
    
#ifdef __WXGTK__
    // GTK3 clears the background occasionally, so we have to draw the whole screen everytime
//    mDrawFullDisplay = true ;
#endif

//    wxGraphicsContext *gc = wxGraphicsContext::Create( dc1 );
//    if (gc)
    {
        double width, height ;
        width = mPainterRect.width() ;
        height = mPainterRect.height() ;

        mLayout->LayoutParagraph(0) ;           // make sure the paragraph we want to display is formatted

        LINE_T line = mLayout->GetLineFromPosition(mDocument.GetPosition()) ;          // get our current line number
        mPageInfo = mLayout->GetPageInfoFromLine(line) ;                     // get out paperwidth on the current line so we scale the display properly

        mScale = (width - 60) / mPageInfo->paperwidth ;
        paint->scale(mScale, mScale) ;

        // update the ruler for any new measurements
        switch(mMeasure)
        {
            case MSR_INCHES :
                mRuler->SetRuler(mPageInfo->paperwidth / TWIPSPERINCH, MSR_INCHES) ;
                break ;
                
            case MSR_MILLIMETERS :
                mRuler->SetRuler(mPageInfo->paperwidth / TWIPSPERMM, MSR_MILLIMETERS) ;
                break ;
                
            default :
                mRuler->SetRuler(mPageInfo->paperwidth / TWIPSPERCM, MSR_CENTIMETERS) ;
                break ;
        }

        // set a base font and color
        paint->setPen( Qt::black );

        font = mBaseFont ;
        double fsize = font.pointSizeF() ;
        font.setPointSizeF(fsize * mFontScale);
        paint->setFont(font) ;

		PARAGRAPH_T numparas =  mDocument.GetNumberofParagraphs() ;  //static_cast<PARAGRAPH_T>(mLayout->mParagraphLayout.size()) ;

        // Layout any visible paragraphs
        LINE_T parastartline = mFirstLine ;
        PARAGRAPH_T para = mLayout->GetParagraphFromLine(parastartline) ;      // parastartline contains the first line of the paragraph

        COORD_T lineheight = 0 ;

        POSITION_T caratadjust = 0 ;

        // go through each of the paragraphs
        PARAGRAPH_T ploop ;
//        bool same = false ;
        int paracounter = -1 ;
        COORD_T subdotline = 0 ;
        
        POSITION_T currentposition = mLayout->GetLineStartPosition(mFirstLine) ; //  paraposition ;         // for mouse mapping
        POSITION_T orgstart, orgend ;
        for(ploop = para; ploop < numparas; ploop++)
        {            
            vector<QRectF> rects;
            paracounter++ ;

            // get the buffer position of the start of this paragraph (for carat)
            POSITION_T paraposition, end ;
            mDocument.GetParagraphStartandEnd(ploop, paraposition, end) ;
            orgstart = paraposition ;
            orgend = end ;
            
            newparagraph.lines.clear() ;
            newparagraph.number = 0 ;

            mLayout->LayoutParagraph(ploop) ;           // make sure the paragraph we want to display is formatted
            
            sDispInfo dispinfo ;
            
            sParagraphLayout *paragraph = &mLayout->mParagraphLayout[static_cast<size_t>(ploop)] ;
            if(paragraph == nullptr)
            {
                continue ;
            }
            
            // if we are not showing dot lines, correct the display line of subsequent paragraphs
            // in reality, we should do this in layout, but it's not there and retro-ing it in would hurt. Alot.
            if(GetShowDot() == false)
            {
                if(caratposition < orgstart || caratposition >= orgend)   // if our carat is on the line with the dot command, keep it
                {
                    if(paragraph->isCommand || paragraph->isComment)
                    {
                        subdotline += paragraph->lines[0].lineheight ;
    //                    continue ;
                    }
                }
            }

            maxlineheight = 0 ;
            if(paragraph->isCommand)
            {
                lineheight = 0 ;
            }

            if(paragraph->lines[0].segments.size() != 0)
            {
                dispinfo.isCommand = paragraph->isCommand ;
                dispinfo.isComment = paragraph->isComment ;
                dispinfo.isKnownDot = paragraph->isKnownDot ;
                
                dispinfo.height = paragraph->modifiers.linespace ; // 0 ;


//                POSITION_T currentposition = mLayout->GetLineStartPosition(mFirstLine) ; //  paraposition ;         // for mouse mapping
                for(sLineLayout line : paragraph->lines)
                {
                    if(line.segments.size() == 0)
                    {
                        continue ;
                    }
                    if(parastartline >= mFirstLine)                 // if this line is in our display
                    {
                        if(parastartline == mFirstLine)
                        {
                            lastpage = line.pagenumber ;
                            suboffset = line.y ;
                            firstlineoffset = line.lineheight ;
                        }

                        if(line.pagenumber != lastpage)
                        {
                            dispoffset = lastoffset + lineheight ;
                            sLineLayout templine ;
                            templine.pagenumber = line.pagenumber ;
                            templine.y = line.y + dispoffset - suboffset ;
                            templine.x = line.x ;
                            templine.lineheight = line.lineheight ;
                            newparagraph.lines.emplace_back(templine) ;
                            
                            dispoffset = lastoffset + lineheight * (paragraph->modifiers.linespace + 1) ;
                            subdotline = 0 ;
    //                        mFirstLine++ ;
                        }
                        
                        COORD_T x = line.x ;
                        COORD_T y = line.y + dispoffset - subdotline ;
                        line.y = y ;                            // set screen y position
                        line.y -= suboffset ;
                
                        // go through each of the segments in the line
                        DisplayListSegments(line, font, paint, x, paraposition, caratposition, newdisplay, currentposition, caratadjust, lineheight, rects, paragraph);

                        maxlineheight += lineheight ;
                        
                        lastoffset = y ;
                        lastpage = line.pagenumber ;
                                            
                        newparagraph.lines.emplace_back(line) ;
                        newparagraph.number = line.segments[0].paragraph ;


                        // so, the math....
                        // y - suboffset takes care of paper page breaks
                        // + firstlineoffset makes sure we count the height of the firstline
                        // + line.lineheight makes sure we don't go too far if a partial last line is available
                        if(line.y + firstlineoffset  >= height / mScale)
                        {
                            doexit = true ;
                            break ;
                        }

                        // the break if above let's us display partial lines at bottom of wndow, but they shouldn't be included in the line count
                        if(line.y + firstlineoffset + line.lineheight <= height / mScale)
                        {
                            mDisplayLines++ ;
                            
                        }
                    }
                    // adjust our paraposition to the right spot
                    else
                    {
                        for(auto & segment : line.segments)
                        {
                            paraposition += segment.glyph.size() ;
                            // we have to use GetChar in order to compensate for position of mShowContol is false
    //                        for(auto &glyph : segment.glyph)
    //                        {
    //                            mDocument.GetChar(paraposition) ;
    //                            paraposition++ ;
    //                        }
                        }
                    }
                    parastartline++ ;
                }

                // save our paragraph ;
                dispinfo.controls = rects;
                dispinfo.paragraph = newparagraph ;
                dispinfo.rightmargin = mLayout->mRightMargin ;
                if(dispinfo.isCommand || dispinfo.isComment)
                {
                    dispinfo.height *= maxlineheight ;      // height contains the linespacing for the paragraph (set above), so this works to set background drawing
                }
                else
                {
                    dispinfo.height = maxlineheight ;
                }

                // if we are not showing dot lines
                if(GetShowDot() == false)
                {
                    if(paragraph->isCommand == false && paragraph->isComment == false)
                    {
                        newdisplay.display.emplace_back(dispinfo) ;
                    }
                    else if(caratposition >= orgstart && caratposition < orgend)  // if our carat is on the line with the dot command, keep it
                    {
                        newdisplay.display.emplace_back(dispinfo) ;
                    }
                }
                else
                {
                    newdisplay.display.emplace_back(dispinfo) ;
                }
                
                if(doexit)
                {
                    break ;
                }
            }
        }
        if(ploop == numparas)
        {
            mLayoutParagraph = ploop ; // + 1;
        }
        else
        {
            mLayoutParagraph = ploop + 1 ;
        }
        
        // this is where we can optimize the display
        PARAGRAPH_T block1 = 0 ;
        PARAGRAPH_T block2 = 0 ;
/*
        // redraw paragraph if part of block
        if(mDocument.mBlockSet)
        {
            block1 = mDocument.GetParagraphFromPosition(mDocument.mStartBlock) ;
            block2 = mDocument.GetParagraphFromPosition(mDocument.mEndBlock) ;
        }

        PARAGRAPH_T searchblock1 = 0 ;
        PARAGRAPH_T searchblock2 = 0 ;

        // redraw paragraph if part of block
        if(mSearchBlockSet)
        {
            searchblock1 = mDocument.GetParagraphFromPosition(mStartSearchBlock) ;
            searchblock2 = mDocument.GetParagraphFromPosition(mEndSearchBlock) ;
        }
*/
        if(mDrawFullDisplay == false)
        {
            sDisplayList optimized;
            for (size_t loop = 0; loop < newdisplay.display.size(); loop++)
            {
                if (loop < mFullDisplay.display.size())
                {
                    if (CompareDisplay(newdisplay.display[loop], mFullDisplay.display[loop]) == false)
                    {
                        optimized.display.emplace_back(newdisplay.display[loop]);
                    }
                    // for non working wxCarat in wxGTK
                    else if(newdisplay.display[loop].paragraph.number == currentparagraph || newdisplay.display[loop].paragraph.number == mLastParagraph)
                    {
                        optimized.display.emplace_back(newdisplay.display[loop]);
                    }
                    if(newdisplay.display[loop].paragraph.number >= block1 && newdisplay.display[loop].paragraph.number <= block2)
                    {
                        optimized.display.emplace_back(newdisplay.display[loop]);
                    }
                }
                else
                {
                    optimized.display.emplace_back(newdisplay.display[loop]);
                }
            }
            mCurrentDisplay = optimized ;
            mFullDisplay = newdisplay;
        }
        else
        {
            mCurrentDisplay = newdisplay ;
            mFullDisplay = newdisplay ;
        }
//        delete gc ;
    }   
//    else
//    {
//        wxMessageBox("Fatal Error: Could not get GraphicsContext", "Fatal Error") ;
//    }

    mLastFirstLine = mFirstLine ;
    mLastParagraph = currentparagraph ;
}


void cEditorCtrl::DisplayListSegments(sLineLayout &line, QFont &font, QPainter *paint, COORD_T x, POSITION_T &paraposition,
                                        POSITION_T caratposition, sDisplayList &newdisplay, POSITION_T &currentposition, 
                                        POSITION_T caratadjust, COORD_T &lineheight,vector<QRectF> &rects, sParagraphLayout *paragraph)
{
    UNUSED_ARGUMENT(caratadjust) ;
//    double fontscale = mFontScale ;
    
    bool blockstartdone = false ;
    bool savedloopdone[10] = {false, false, false, false, false, false, false, false, false, false } ;
    for (size_t sloop = 0; sloop < line.segments.size(); sloop++)
    {
        sSegmentLayout segment = line.segments[sloop];

        if(segment.glyph.empty())
        {
            continue ;          ///< @TODO find out why this is happening
        }

        // draw the segment a glyph at a time
        font = segment.font;
        double fsize = font.pointSizeF() ;
        font.setPointSizeF(fsize * mFontScale);
        paint->setFont(font);

        segment.font = font;       // set the screen font
        
        for (size_t loop1 = 0; loop1 < segment.glyph.size(); loop1++)
        {
            mDocument.GetChar(paraposition) ;          // we get the char every loop, since it knows about showcontrolchars or not, and adjust our position properly
         
            segment.position[loop1] = x + (segment.position[loop1]);

            // handle control characters
            if(mShowControl == SHOW_ALL)
            {
                paraposition = DisplayListContolChars(segment, loop1, x, paraposition, caratposition, paint, line, newdisplay, rects, paragraph->isCommand | paragraph->isComment);
            }

            // if layout added a charcater for block start, highlight it (B)
            if(paraposition == mDocument.mStartBlock && paraposition != 0 && mDocument.mBlockSet == false)
            {
                if(blockstartdone == false )
                {
//                    segment.glyph[loop1] = 'B' ;
                    // now get the coordinates to draw the background
                    double width, height;
                    QString ch ;
                    ch = QString::fromStdString(segment.glyph[loop1]) ;
//                    dc.GetTextExtent(ch, &width, &height);

                    QSizeF size ;

                    QFontMetricsF metrics(font) ;
                    size = metrics.size(Qt::TextSingleLine, ch) ;
                    size.setWidth(metrics.horizontalAdvance(ch)) ;

//                    sFontSizes sizes ;
//                    mLayout->mFontCache.GetGlyphSize(font, ch, sizes) ;
                    width = size.width() ;
                    height = size.height() ;


                    QRectF rect;
                    rect.setX(segment.position[loop1]);
                    rect.setY(line.y) ;
                    rect.setWidth(width) ;
                    rect.setHeight(height) ;

                    rects.emplace_back(rect);
                    paraposition-- ;
                    blockstartdone = true ;
                }
            }
            
            // if layout added a charcater for a saved position
            for(int sploop = 0; sploop < 10; sploop++)
            {
                if(paraposition == mDocument.mSavePosition[sploop] && paraposition != 0)
                {
                    if(savedloopdone[sploop] == false)
                    {
                        // get the coordinates to draw the background
                        COORD_T width, height;
                        QString ch;
                        ch = QString::fromStdString(segment.glyph[loop1]) ;
//                        dc.GetTextExtent(ch, &width, &height);
                        QSizeF size ;

                        QFontMetricsF metrics(font) ;
                        size = metrics.size(Qt::TextSingleLine, ch) ;
                        size.setWidth(metrics.horizontalAdvance(ch)) ;

                        width = size.width() ;
                        height = size.height() ;

                        QRectF rect;
                        rect.setX(segment.position[loop1]);
                        rect.setY(line.y) ;
                        rect.setWidth(width) ;
                        rect.setHeight(height) ;

                        rects.emplace_back(rect);
//                        newdisplay.controls.push_back(rect);
                        paraposition-- ;
                        savedloopdone[sploop] = true ;
                    }
                }
            }

            // build mouse clicking vector
            sMouseRect mrect;
            mrect.position = currentposition;
            mrect.coordinates.setX(segment.position[loop1]) ;
            mrect.coordinates.setY(line.y); // +segment.segmentheight);

            COORD_T width, height;
            QString ch;
            if (segment.glyph[loop1][0] == HARD_RETURN)
            {
                ch = " ";
            }
            else
            {
                ch = QString::fromStdString(segment.glyph[loop1]) ;
            }
            
            if (loop1 < segment.glyph.size() - 1)
            {
                width = ((x + (segment.position[loop1 + 1] )) - segment.position[loop1]);
            }
            else
            {
                width = 128;
            }
            height = segment.segmentheight;
            mrect.coordinates.setWidth(width) ;
            mrect.coordinates.setHeight(height) ;

            // if we are not showing dot lines
            if(GetShowDot() == false)
            {
                if(paragraph->isCommand == false && paragraph->isComment == false)
                {
                    mMouseRect.push_back(mrect);
                }
            }
            else
            {
                mMouseRect.push_back(mrect);
            }
            
            // this is our cursor position
            if (paraposition /*- caratadjust */ == caratposition)
            {
                // use ints for carat rect to stop artifacts
                mCaretPos.setX(static_cast<int>(mrect.coordinates.x())) ;
                mCaretPos.setY(static_cast<int>(mrect.coordinates.y())) ;
                mCaretPos.setWidth(static_cast<int>(mrect.coordinates.width())) ;
                mCaretPos.setHeight(static_cast<int>(mrect.coordinates.height())) ;

                if (mInsertMode == INSERT_MODE)
                {
#ifdef Q_OS_MACOS
                    mCaretPos.setWidth( 2.5 * mFontScale) ;
#else
                    mCaretPos.setWidth( 3.5 * mFontScale) ;
#endif
                }
                mCurrentPage = line.pagenumber;
                mCurrentLine = line.linenumber ;
                mCurrentHeight = line.cumalativeheight + mTopMargin  ;
                
                sPageInfo *page = mLayout->GetPageInfo(line.pagenumber) ;
                mTopMargin = page->topmargin ;
                
                // now update status infromation
                QFontInfo f(font) ;
                mStatusFont.sprintf("%s %.1f", f.family().toUtf8().constData(), f.pointSizeF() / mFontScale);
//                mStatusFont.Printf("%s %d", font.GetFaceName(), static_cast<int>(static_cast<double>(font.GetPointSize())  / mFontScale + .05)) ;
                
                if(font.weight() != QFont::Bold)
                {
                    mStatusBold = false ;
                }
                else
                {
                    mStatusBold = true ;
                }

                if(font.italic() != true)
                {
                    mStatusItalic = false ;
                }
                else
                {
                    mStatusItalic = true ;
                }

                mStatusUnderline = font.underline() ;

                if((paragraph->modifiers.center == true)) //  || (mTabCenterLine == true))
                {
                    mStatusJust = JUST_CENTER ;
                }
                else if ((paragraph->modifiers.right == true)) // || (mTabRightLine == true))
                {
                    mStatusJust = JUST_RIGHT ;
                }
                else if((paragraph->modifiers.justify == true)) //  || (mTabJustifyLine == true))
                {
                    mStatusJust = JUST_JUST ;
                }
                else
                {
                    mStatusJust = JUST_LEFT ;
                }

                mStatusStyle = "Body Text" ;
                
                // Update the ruler
                mRuler->SetTabStops(mTabStops) ;
                mRuler->SetParagraph(mLayout->mParagraphMargin) ;
                mRuler->SetRightMargin(mLayout->mRightMargin) ;
                if(line.pagenumber % 2 == 0)
                {
                    mRuler->SetPageMargins(mLayout->mPageOffsetEven, mLayout->mPageOffsetEven + mLayout->mRightMargin) ;
                }
                else
                {
                    mRuler->SetPageMargins(mLayout->mPageOffsetOdd, mLayout->mPageOffsetOdd + mLayout->mRightMargin) ;
                }
                
                
//                This really is a lot of work, since it forces a ruler redraw every character entered
                mRuler->SetPosition( mrect.coordinates.x() - 1440) ;
                mRuler->update() ;

            }
            

            //                            mDocument.GetChar(paraposition) ;
            paraposition++;
            currentposition++;
        }

        // the rect list for a block
        if(segment.isBlock)
        {
            QRectF rect;
            rect.setX(segment.position[0]) ;
            rect.setY(line.y) ;

            COORD_T width = segment.position[segment.glyph.size() - 1] - rect.x() ;  // width of entire segment (not including last letter)

            // measure last character
            QFontMetricsF metrics(font) ;
            width += metrics.horizontalAdvance(QString::fromStdString(segment.glyph[segment.glyph.size() - 1])) ;

            rect.setWidth(width) ;
            rect.setHeight(segment.segmentheight) ;

            mBlockCoords.push_back(rect) ;
        }

        // the rect list for a search block
        if(segment.isSearch)
        {
            QRectF rect;
            rect.setX(segment.position[0]) ;
            rect.setY(line.y) ;

            COORD_T width = segment.position[segment.glyph.size() - 1] - rect.x();  // width of entire segment (not including last letter)

            // measure last character
            QSizeF size ;
            QFontMetricsF metrics(font) ;
            width += metrics.horizontalAdvance(QString::fromStdString(segment.glyph[segment.glyph.size() - 1])) ;

            rect.setWidth(width) ;
            rect.setHeight(segment.segmentheight) ;

            mSearchCoords.push_back(rect) ;
        }

        if (segment.segmentheight > lineheight)
        {
            lineheight = segment.segmentheight ;
        }

        line.segments[sloop] = segment;

    }
}


POSITION_T & cEditorCtrl::DisplayListContolChars(sSegmentLayout &segment, size_t loop1, COORD_T x, POSITION_T & paraposition, POSITION_T caratposition, QPainter *paint, sLineLayout &line, sDisplayList &newdisplay, vector<QRectF> &rects, bool dot)
{
    UNUSED_ARGUMENT(paint) ;
    UNUSED_ARGUMENT(newdisplay) ;
    UNUSED_ARGUMENT(x) ;
    UNUSED_ARGUMENT(caratposition) ;

//    double fontscale = mFontScale ;
    
    if (segment.glyph[loop1][0] < STYLE_END_OF_STYLES && segment.glyph[loop1].length() == 1)
    {
        if (segment.glyph[loop1][0] != HARD_RETURN)
        {
            COORD_T fontwidth = 0;

            // convert the character to something printable
            if (segment.glyph[loop1][0] == STYLE_TAB)
            {
                if (mShowControl == SHOW_ALL)
                {
                    segment.glyph[loop1] = '>';
                }
                else
                {
                    segment.glyph[loop1] = ' ';
                }
            }
            else if (segment.glyph[loop1][0] == STYLE_FONT1)
            {
                string fontstring ;
                if(!dot)
                {
                    fontstring = string_sprintf("<%s %.1f>",  segment.font.family().toStdString().c_str(), (segment.font.pointSizeF() / mFontScale)) ;
                }
                else
                {
                    fontstring = string_sprintf("<%s %.1f>",  segment.tfont.family().toStdString().c_str(), (segment.tfont.pointSizeF() / mFontScale)) ;
                }

                segment.glyph[loop1] = fontstring ;
#ifdef NOPE
                segment.glyph[loop1] = '<';

                // since this is a font, the background extends more than one charcater... up to the closing >
                for (size_t floop = loop1; floop < segment.glyph.size(); floop++)
                {
                    if (segment.glyph[floop][0] == '>')
                    {
                        QChar ch = '>';
                        QSizeF size ;

//                        QFontMetricsF metrics(segment.font) ;
//                        size = metrics.size(Qt::TextSingleLine, ch) ;
//                        size.setWidth(metrics.horizontalAdvance(ch)) ;

                        int width = size.width() ;

                        COORD_T cpos = x + (segment.position[floop]) + width;
                        /*
                        if (floop + 1 < segment.glyph.size() - 1)
                        {
                            cpos = x + (segment.position[floop + 1] * fontscale);
                        }
                        else
                        {
                            cpos = x + (segment.position[floop] * fontscale);
                        }
*/
                        fontwidth = cpos - segment.position[loop1];
                        if (paraposition < caratposition)
                        {
                            // adjust our parapoistion so the carat is in the right spot
                            paraposition -= (floop - loop1);
                            break;
                        }
                    }
                }
#endif
            }
            else
            {
                segment.glyph[loop1][0] += '@';  // get it into ascii range (uppercase)
            }

            // now get the coordinates to draw the background
//            dc1.SetFont((segment.font));
//            dc1.GetTextExtent(ch, &width, &height);
            COORD_T width, height;
            QString ch;
            ch = QString::fromStdString(segment.glyph[loop1]) ;

            QSizeF size ;
            QFontMetricsF metrics(segment.font) ;

            size = metrics.size(Qt::TextSingleLine, ch) ;
            size.setWidth(metrics.horizontalAdvance(ch)) ;
            QRectF crect = metrics.boundingRect(ch);
            size.setWidth(crect.width());
            size.setWidth(metrics.width(ch));
            width = size.width() ;
            height = size.height() ;
//            fontwidth += width * mFontScale ;
            fontwidth += width ;

            QRectF rect;
            rect.setX(segment.position[loop1]);
            rect.setY(line.y);
            rect.setWidth(width);
            if (fontwidth != 0)
            {
                rect.setWidth(fontwidth) ;
            }
            rect.setHeight(height) ;
//            rect.setHeight(height * mFontScale) ;

            rects.emplace_back(rect);
//            newdisplay.controls.push_back(rect);
        }
    }
    
    return paraposition;
}


bool cEditorCtrl::CompareDisplay(sDispInfo &one, sDispInfo &two)
{
    bool retval = false;
    bool breakit = false;

    do          // a do...while(false) loop, so we can breakout at any time
    {
        if (one.height != two.height)
        {
            break;
        }
        if (one.isCommand != two.isCommand)
        {
            break;
        }
        if (one.isComment != two.isComment)
        {
            break;
        }
        if (one.paragraph.pagebreak != two.paragraph.pagebreak)
        {
            break;
        }
        if (one.paragraph.number != two.paragraph.number)
        {
            break;
        }
        if (one.paragraph.lines.size() != two.paragraph.lines.size())
        {
            break;
        }
        for (size_t lloop = 0; lloop < one.paragraph.lines.size(); lloop++)
        {
            if (one.paragraph.lines[lloop].pagenumber != two.paragraph.lines[lloop].pagenumber)
            {
                breakit = true;
                break;
            }
            if (one.paragraph.lines[lloop].x != two.paragraph.lines[lloop].x)
            {
                breakit = true;
                break;
            }
            if (one.paragraph.lines[lloop].y != two.paragraph.lines[lloop].y)
            {
                breakit = true;
                break;
            }
            if (one.paragraph.lines[lloop].lineheight != two.paragraph.lines[lloop].lineheight)
            {
                breakit = true;
                break;
            }
            if (one.paragraph.lines[lloop].segments.size() != two.paragraph.lines[lloop].segments.size())
            {
                breakit = true;
                break;
            }
            for (size_t sloop = 0; sloop < one.paragraph.lines[lloop].segments.size(); sloop++)
            {
                if (one.paragraph.lines[lloop].segments[sloop].paragraph != two.paragraph.lines[lloop].segments[sloop].paragraph)
                {
                    breakit = true;
                    break;
                }
                if (one.paragraph.lines[lloop].segments[sloop].segmentheight != two.paragraph.lines[lloop].segments[sloop].segmentheight)
                {
                    breakit = true;
                    break;
                }
                if (one.paragraph.lines[lloop].segments[sloop].glyph != two.paragraph.lines[lloop].segments[sloop].glyph)
                {
                    breakit = true;
                    break;
                }
                if (one.paragraph.lines[lloop].segments[sloop].position != two.paragraph.lines[lloop].segments[sloop].position)
                {
                    breakit = true;
                    break;
                }
                if (one.paragraph.lines[lloop].segments[sloop].isBlock != two.paragraph.lines[lloop].segments[sloop].isBlock)
                {
                    breakit = true;
                    break;
                }
                if (one.paragraph.lines[lloop].segments[sloop].font != two.paragraph.lines[lloop].segments[sloop].font)
                {
                    breakit = true;
                    break;
                }
            }

            if (breakit == true)
            {
                break;
            }
        }

        if (breakit == true)
        {
            break;
        }

        retval = true;

    } while (false);

    return retval;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @param  dc1    [in]  paint dc
///
/// @return nothing
///
/// @brief
/// Draws the editor window. 
///
/////////////////////////////////////////////////////////////////////////////
void cEditorCtrl::PaintEditor(QPainter &dc1)
{
    if(mDocument.GetTextSize() == 0)
    {
        return ;
    }

    if(mInPrintPreview == true)
    {
        return ;            // this is a hack. There's some weird stuff going on in print preview that cause this to crash.
                            // it may have soemthing to do with show/no show control chars. This hack works, since we don't really
                            // need to update the editor window when in print preview mode.
    }

    mCaratTimer->stop() ;
#ifdef PAINT_TIMER
    wxStopWatch sw ;
#endif

#ifdef NOOPTIMIZEDISPLAY
    mDrawFullDisplay = true;
#endif
    
    QBrush background(mBackgroundColour) ;
    dc1.setBackground(background);

    
    if (mDrawFullDisplay)
    {
        dc1.eraseRect(mPainterRect) ;
    }

    PAGE_T lastpagenumber = 0 ;

    {
        CreateDisplayList(&dc1);

        mDrawFullDisplay = false ;
        
        
        if (!mCurrentDisplay.display.empty())
        {
            lastpagenumber = mCurrentDisplay.display[0].paragraph.lines[0].pagenumber;
        }

        dc1.setPen(Qt::blue);
        dc1.setBrush(Qt::blue);

        int width, height ;
        width = mPainterRect.width() ;
        height = mPainterRect.height() ;

//gc->Clip(0, 0, width, 500) ;

        size_t len = mCurrentDisplay.display.size() ;
        for(size_t loop = 0; loop < len; loop++)
        {
            size_t linelen = mCurrentDisplay.display[loop].paragraph.lines.size() ;
            
            // clear the paragraph
            dc1.setBrush(mBackgroundColour);
            dc1.setPen(mBackgroundColour);
//            COORD_T clearx, cleary, clearw, clearh;
//            for (size_t lloop = 0; lloop < linelen; lloop++)
//            {
//                clearx = 0;
//                cleary = mCurrentDisplay.display[loop].paragraph.lines[lloop].y;
//                clearw = width / mScale;
//                clearh = mCurrentDisplay.display[loop].paragraph.lines[lloop].lineheight;
//                dc1.drawRect(clearx, cleary, clearw, clearh);
//            }

            dc1.setPen(mTextColour);

            // lay down some backgrounds before we draw the text
            if (mCurrentDisplay.display[loop].isCommand)
            {
                QBrush brush(mDotColour);

                if(mCurrentDisplay.display[loop].isKnownDot == false)
                {
                    brush.setColor(mUnknownColour) ;
                }
                dc1.setBrush(brush);

                COORD_T x = mCurrentDisplay.display[loop].paragraph.lines[0].x;
                COORD_T y = mCurrentDisplay.display[loop].paragraph.lines[0].y;
                COORD_T x1 = mCurrentDisplay.display[loop].rightmargin ; // mLayout->mRightMargin ; // 6.5 * TWIPSPERINCH;
                COORD_T y1 = mCurrentDisplay.display[loop].height;

                dc1.drawRoundedRect(x, y, x1, y1 - 10, 30.0, 30.0);
            }
            else if (mCurrentDisplay.display[loop].isComment)
            {
                QBrush brush(mCommentColour);
                dc1.setBrush(brush);

                COORD_T x = mCurrentDisplay.display[loop].paragraph.lines[0].x;
                COORD_T y = mCurrentDisplay.display[loop].paragraph.lines[0].y;
                COORD_T x1 = mCurrentDisplay.display[loop].rightmargin ; // mLayout->mRightMargin ; // 6.5 * TWIPSPERINCH;
                COORD_T y1 = mCurrentDisplay.display[loop].height;

                dc1.drawRoundedRect(x, y, x1, y1 - 10, 30.0, 30.0);
            } 

            // draw control char backgrounds
//            if (mShowControl == SHOW_ALL)
            {
                QBrush brush(mHighlightColour);
                dc1.setBrush(brush);
                for (auto rect : mFullDisplay.display[loop].controls)
                {
                    dc1.drawRoundedRect(rect.x(), rect.y(), rect.width(), rect.height(), 30.0, 30.0);
                }
            }

            // draw a line at a time
            // we assume display list optimization has occurred, and we will only draw what is in the list

            QRectF chardrawrect ;
            chardrawrect.setWidth(1000000) ;
            chardrawrect.setHeight(100000) ;

            for(size_t lloop = 0; lloop < linelen; lloop++)
            {
                size_t seglen = mCurrentDisplay.display[loop].paragraph.lines[lloop].segments.size() ;

                if(mCurrentDisplay.display[loop].paragraph.lines[lloop].pagenumber != lastpagenumber)
                {
                    chardrawrect.setX(mPageInfo->paperwidth + 50) ;
                    // @TODO correct Y coordinate to put 'P' in center of line on all linespacings
                    chardrawrect.setY(mCurrentDisplay.display[loop].paragraph.lines[lloop].y + (mCurrentDisplay.display[loop].paragraph.lines[lloop].lineheight / 2)) ;
                    dc1.drawText(chardrawrect, "P") ;

                    dc1.drawLine(0, mCurrentDisplay.display[loop].paragraph.lines[lloop].y + (mCurrentDisplay.display[loop].paragraph.lines[lloop].lineheight  / 2),
                                  mPageInfo->paperwidth, mCurrentDisplay.display[loop].paragraph.lines[lloop].y + (mCurrentDisplay.display[loop].paragraph.lines[lloop].lineheight  / 2));

                    lastpagenumber = mCurrentDisplay.display[loop].paragraph.lines[lloop].pagenumber ;
                }
                else if(mCurrentDisplay.display[loop].isCommand || mCurrentDisplay.display[loop].isComment)
                {
                    if((mShowControl == false && mAlwaysFlag == false) || (mAlwaysFlag == true))
                    {
                        if(mCurrentDisplay.display[loop].isKnownDot)
                        {
                            chardrawrect.setX(mPageInfo->paperwidth + 50) ;
                            chardrawrect.setY(mCurrentDisplay.display[loop].paragraph.lines[lloop].y) ;
                            dc1.drawText(chardrawrect, ".") ;
                        }
                        else
                        {
                            chardrawrect.setX(mPageInfo->paperwidth + 50) ;
                            chardrawrect.setY(mCurrentDisplay.display[loop].paragraph.lines[lloop].y) ;
                            dc1.drawText(chardrawrect, "?") ;
                        }
                    }
                }
                else if(lloop == linelen -1)
                {
                    if((mShowControl == false && mAlwaysFlag == false) || (mAlwaysFlag == true))
                    {
                        chardrawrect.setX(mPageInfo->paperwidth + 50) ;
                        chardrawrect.setY(mCurrentDisplay.display[loop].paragraph.lines[lloop].y) ;
                        dc1.drawText(chardrawrect, "<") ;
                    }
                }

                // draw the text in this segment
                for(size_t sloop = 0; sloop < seglen; sloop++)
                {
                    sParagraphLayout *para = &mCurrentDisplay.display[loop].paragraph ;

                    QColor tcolor(para->lines[lloop].segments[sloop].textcolor) ;           // get rid of alpha value  @TODO

                    // change text colour to specified, unless it's black, then change to default text colour
                    if(tcolor.red() == 0 && tcolor.green() == 0 && tcolor.blue() == 0)
                    {
                        tcolor = mTextColour ;
                    }
                    
                    dc1.setPen(mTextColour);
                    dc1.setFont(para->lines[lloop].segments[sloop].font) ;
                    size_t glyphlen = para->lines[lloop].segments[sloop].glyph.size() ;
                    QString tmp ;
                    for(size_t gloop = 0 ; gloop < glyphlen; gloop++)
                    {
                        QString str = QString::fromStdString(para->lines[lloop].segments[sloop].glyph[gloop]) ;
                        chardrawrect.setX(para->lines[lloop].segments[sloop].position[gloop]) ;
                        chardrawrect.setY(para->lines[lloop].y) ;

                        dc1.drawText(chardrawrect, str) ;
//                        dc1.drawText(para.lines[lloop].segments[sloop].position[gloop],  para.lines[lloop].y + (para.lines[lloop].segments[sloop].segmentheight * mFontScale), str) ;
//                        dc1.drawText(mCurrentDisplay.display[loop].paragraph.lines[lloop].segments[sloop].position[gloop],  mCurrentDisplay.display[loop].paragraph.lines[lloop].y, str) ;
                        tmp += str ;
                    }
                }
            }
        }

        // any marked blocks
        if(mBlockCoords.size() != 0)
        {
            QPen bpen(mHighlightColour) ;
            QBrush brush(mHighlightColour) ;
            dc1.setBrush(brush);
            dc1.setPen(bpen);
//            QRectF newrect = mBlockCoords[0] ;
            for(auto rect: mBlockCoords)
            {
                dc1.drawRect(rect);
//                if(newrect.y() != rect.y())
//                {
//                    dc1.drawRect(newrect);
//                    newrect = rect ;
//                }
//                else
//                {
//                    newrect.setWidth(rect.width() + newrect.width()) ;
//                }
            }
//            dc1.drawRect(newrect);
        }

        // any search blocks
        if(mSearchCoords.size() != 0)
        {
            QPen bpen(mBlockColour) ;
            QBrush brush(mBlockColour) ;
            dc1.setBrush(brush);
            dc1.setPen(bpen);
 //           QRectF newrect = mSearchCoords[0] ;
            for(auto rect: mSearchCoords)
            {
                dc1.drawRect(rect);
//               if(newrect.y() != rect.y())
//                {
//                    dc1.drawRect(newrect);
//                    newrect = rect ;
//                }
//                else
//                {
//                    newrect.setWidth(rect.width() + newrect.width()) ;
//                }
            }
//            dc1.drawRect(newrect);
        }

        // draw the vertical line at the left end of the page that separates WordStar info
        QPointF points[2] ;
        points[0].setX(mPageInfo->paperwidth) ; // 8.5 * TWIPSPERINCH ;
        points[0].setY(0) ;
        points[1].setX(points[0].x()) ;
        points[1].setY(static_cast<double>(height) / mScale) ;

//        dc1.drawLine(8.5 * TWIPSPERINCH, 0, 8.5 * TWIPSPERINCH, height / mScale );
        dc1.drawLine(points[0], points[1]) ;

        // draw our caret by forcing the timer
        QPainter::CompositionMode oldmode = dc1.compositionMode() ;
        dc1.setCompositionMode(QPainter::RasterOp_NotDestination) ;

        QColor c(Qt::black); // = dc1.GetPen().GetColour() ;
        QBrush b(c);
        QPen p(c);
        dc1.setBrush(b);
        dc1.setPen(p);

        dc1.fillRect(mCaretPos.x(), mCaretPos.y(), mCaretPos.width(), mCaretPos.height(), b);

        dc1.setCompositionMode(oldmode) ;
        mDrawnCaret = true;

        // setAttribute(Qt::WA_OpaquePaintEvent); does seem to work under OSX, so the background gets erased on paint. So, no flashing cursor for OSX
#ifndef Q_OS_MACOS
        mCaratTimer->start(CARAT_TIMER_DELAY);
#endif

        // set the scroll bar
        LINE_T numlines = mLayout->GetNumberofLines() ;
        mWordTsar->mScrollbar->setRange(1, numlines) ;
        mWordTsar->mScrollbar->setValue(mFirstLine) ;
        
    }
    
#ifdef PAINT_TIMER
    wxLogMessage("Action to end of paint: %d ms", mSW.Time()) ;
    wxLogMessage("  time in paint itself %d ms", sw.Time()) ;
#endif
}


void cEditorCtrl::ScrollIntoView(Qt::Key key)
{
    POSITION_T pos = mDocument.GetPosition() ;
    LINE_T current = mLayout->GetLineFromPosition(pos) ;
    
    switch(key)
    {
        case Qt::Key_Up :
            if(current < mFirstLine)
            {
                mFirstLine = current ;
            }
            break ;
            
        case Qt::Key_Down :
            if(current == mFirstLine + mDisplayLines)
            {
                mFirstLine++ ;
            }
            else if(current > mFirstLine + mDisplayLines)
            {
                mFirstLine += current - mFirstLine + mDisplayLines ; //  - (mDisplayLines + 2) ;
            }

            if(mFirstLine >= mLayout->GetNumberofLines())
            {
                mFirstLine = mLayout->GetNumberofLines() - mDisplayLines ;
            }
        break ;
            
        default :
            break ;
    }
}



void cEditorCtrl::Copy(void)
{
	//    if((mSelectStart != 0) && (mSelectEnd != 0))
    if(mDocument.mStartBlock != mDocument.mEndBlock)
    {
        mCopyBuffer.clear() ;
        for(POSITION_T loop = mDocument.mStartBlock; loop < mDocument.mEndBlock; loop++)
        {
            string ch = mDocument.GetChar(loop) ;
            mCopyBuffer += ch ;
        }
    }
}



void cEditorCtrl::Paste(void)
{
    if(mCopyBuffer.length() != 0)
    {
        mDocument.SetLoading(true) ;          // stops word count (etc) from working while we do this

        mDocument.SaveUndo() ;

        mDocument.Insert(mCopyBuffer) ;

        mDocument.SetLoading(false) ;
    }
}



void cEditorCtrl::Cut(void)
{
    if(mDocument.mBlockSet)
    {
        POSITION_T current = mDocument.GetPosition() ;
        POSITION_T cutsize = mDocument.mEndBlock - mDocument.mStartBlock ;

        mDocument.SetPosition(mDocument.mStartBlock) ;
        Delete(mDocument.mStartBlock, cutsize) ;

        if(current > mDocument.mStartBlock)
        {
            current -= cutsize ;
        }
        mDocument.SetPosition(current) ;
    }
}

long cEditorCtrl::WordCount(POSITION_T start, POSITION_T end)
{
    QString text ;
//    auto text = make_shared<QString> ;

//                    QString replace = wxT("\\.()[]{},;:?+-*=/&~|\"\'<>!\r\n\t") ;        // break apostrophes into two words
    QString replace("\\.()[]{},;:?+-*=/&~|\"<>!\r\n\t") ;           // do not break apostrophes (closer to MS Word wordcount)
    replace += static_cast<QChar>(HARD_RETURN) ;
    QString newchar = " " ;


    if(end == 0)        // if we do the whole text
    {
        ssize_t paras = mDocument.GetNumberofParagraphs() ;
        for(ssize_t loop = 0; loop < paras; loop++)
        {
            QString text1 = QString::fromStdString(mDocument.GetParagraphText(loop)) ;
            if (text1.length() != 0)
            {
                if (text1[0] == '.')
                {
                    continue;
                }
                text += text1;
            }
        }
    }
    else                // we do a block
    {
        text = QString::fromStdString(mDocument.GetBlockText(start, end)) ;
    }

    // we have the text, now get rid of oddball characters
    for(int loop = 0; loop < replace.length(); loop++)
    {
        QString oldchar = (QChar)replace.at(loop) ;
        text.replace(oldchar, newchar) ;
    }

    // brute force
    bool space = false ;
    long wordcount = 1 ;
    QChar currentchar ;
//                    QString word ;

    for(auto i : text)
    {

        currentchar = i;
        if(currentchar == ' ' && space == false)
        {
            space = true ;
            ++wordcount ;
        }
        else if(currentchar != ' ')
        {
            space = false ;
        }
    }
    return wordcount ;   
}

/*

// brute force search
void cEditorCtrl::Search(QString text, vector<size_t> &pos, bool whole, bool ignore1, bool wild)
{
UNUSED_ARGUMENT(whole) ;
    eShowControl show = GetShowControls() ;
    SetShowControls(SHOW_ALL) ;
    
    POSITION_T length = text.length()  - 1 ;

// size_t comps = 0 ;
 size_t fcomps = 0 ;

    QString cmp ;

    SetStatus("Searching...") ;

//    for(size_t loop = 0; loop < mBuffer->GetTextSize() - length; ++loop)
    for(POSITION_T loop = 0; loop < mDocument.GetTextSize() - length; ++loop)
    {
        cmp.clear() ;
        for(POSITION_T iloop = loop; iloop <= loop + length; ++iloop)
        {
//            QChar t = mBuffer->GetChar(iloop) ;
            QChar t = mDocument.GetChar(iloop) ;
            cmp.append(t) ;
        }
        if(wild == false)
        {
            ++fcomps ;
            if(text.IsSameAs(cmp, !ignore1) == true)
            {
                pos.push_back(loop) ;
            }
        }
        else
        {
            bool match = true ;
            QString one, two ;
            if(ignore1 == true)
            {
                one = text.Upper() ;
                two = cmp.Upper() ;
            }
            else
            {
                one = text ;
                two = cmp ;
            }
            for(POSITION_T iloop = 0; iloop <= length; iloop++)
            {
                if(one[iloop] != '?')
                {
                    if(one[iloop] != two[iloop])
                    {
                        match = false ;
                        break ;
                    }
                }
            }
            if(match == true)
            {
                pos.push_back(loop) ;
            }
        }
    }
//    mBuffer->SetShowControl(show) ;
    SetShowControls(show) ;

    QString temp ;
    temp.Printf("Found %d occurances", pos.size()) ;
    SetStatus(temp) ;
}
*/




void cEditorCtrl::HideCaret(void)

{
    mHideCaret = true;
}


void cEditorCtrl::ShowCaret(void)
{
    mHideCaret = false;
}


void cEditorCtrl::DrawCaret(QPainter &paint)
{
    if(mIsReady == false)
    {
        mCaratTimer->start(CARAT_TIMER_DELAY);
        return ;
    }

    if (mHideCaret == true && mDrawnCaret == true)      // don't show caret
    {
        return;
    }
    else if (mDrawnCaret == true)                       // erase the carat
    {
        if (hasFocus())
        {
            paint.scale(mScale, mScale) ;
            QPainter::CompositionMode oldmode = paint.compositionMode() ;
            paint.setCompositionMode(QPainter::RasterOp_NotDestination) ;

            QColor c(Qt::black); // = dc1.GetPen().GetColour() ;
            QBrush b(c);
            QPen p(c);
            paint.setBrush(b);
            paint.setPen(p);

//            paint.fillRect(mCaretPos.x(), mCaretPos.y(), mCaretPos.width(), mCaretPos.height(), b);
            paint.fillRect(mCaretPos, b) ;

            paint.setCompositionMode(oldmode) ;
            mDrawnCaret = false;
        }
    }
    else                                                // draw the caret
    {
        paint.scale(mScale, mScale) ;
        QPainter::CompositionMode oldmode = paint.compositionMode() ;
        paint.setCompositionMode(QPainter::RasterOp_NotDestination) ;
//        paint.setCompositionMode(QPainter::CompositionMode_Xor) ;

        QColor c(Qt::black); // = dc1.GetPen().GetColour() ;
        QBrush b(c);
        QPen p(c);
        paint.setBrush(b);
        paint.setPen(p);

//        paint.fillRect(mCaretPos.x(), mCaretPos.y(), mCaretPos.width(), mCaretPos.height(), b);
        paint.fillRect(mCaretPos, b) ;

        paint.setCompositionMode(oldmode) ;
        mDrawnCaret = true;
    }
    mDoDrawCaret = false ;
    mCaratTimer->start(CARAT_TIMER_DELAY);
}

