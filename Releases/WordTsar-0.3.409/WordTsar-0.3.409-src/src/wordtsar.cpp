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
#include <memory>

#include <QtGlobal>
#include <QtWidgets>
#include <QVBoxLayout>
#include <QFont>

#include "src/wordtsar.h"
#include "src/gui/editor/editorctrl.h"
#include "third-party/simpleini/SimpleIni.h"

using namespace std ;


const char HELPDELAY = 4 ;          // wait x * 200 ms before showing help


cWordTsar::cWordTsar(int argc, char *argv[], QWidget *parent)
    : QMainWindow(parent)
{
#ifdef Q_OS_MACOS
    QCoreApplication::setAttribute(Qt::AA_MacDontSwapCtrlAndMeta) ;
#endif

    // create the base widget and set it's layout
    if (objectName().isEmpty())
    {
        setObjectName(QStringLiteral("MainWindow"));
    }
    resize(400, 300);
    mBaseWidget = new QWidget(this) ;
    mBaseWidget->setObjectName(QStringLiteral("mBaseWidget"));
    mBaseLayout = new QVBoxLayout(mBaseWidget);
    mBaseLayout->setSpacing(0);
    mBaseLayout->setObjectName(QStringLiteral("mBaseLayout"));
//    mBaseLayout->setContentsMargins(2, 0, 2, 0);
    mBaseLayout->setContentsMargins(0, 0, 0, 0);

    mLayout = new QVBoxLayout();
    mLayout->setSpacing(0);
    mLayout->setObjectName(QStringLiteral("mLayout"));

    CreateMenus() ;

    mLayout->addWidget(menuBar) ;


    // create the top status bar
    mStatusTop = new QStatusBar(this) ;
    mStatusTop->setObjectName("mStatusTop") ;
    mStatusTop->setSizeGripEnabled(false);
    mStatusTop->setMaximumSize(16777215, 20);                   // @TODO replace with https://stackoverflow.com/questions/22508296/how-to-get-the-size-height-of-a-label-after-the-word-wrap

    mStatStyle = new QLabel("Body Text", this) ;
    mStatusTop->addPermanentWidget(mStatStyle, 1) ;

    mStatFont = new QLabel("Courier 10 Pitch 12", this) ;
    mStatusTop->addPermanentWidget(mStatFont, 1) ;

    mStatBold = new QLabel(" B ", this) ;
    mStatusTop->addPermanentWidget(mStatBold);

    mStatItalic = new QLabel(" I ", this) ;
    mStatusTop->addPermanentWidget(mStatItalic);

    mStatUnderline = new QLabel( "U ", this) ;
    mStatusTop->addPermanentWidget(mStatUnderline);

    mStatChange = new QLabel(" * ", this) ;
    mStatusTop->addPermanentWidget(mStatChange);

    mStatLeft = new QLabel(" L ", this) ;
    mStatusTop->addPermanentWidget(mStatLeft);

    mStatCenter = new QLabel(" C ", this) ;
    mStatusTop->addPermanentWidget(mStatCenter);

    mStatRight = new QLabel(" R ", this) ;
    mStatusTop->addPermanentWidget(mStatRight);

    mStatJustify = new QLabel(" J ", this) ;
    mStatusTop->addPermanentWidget(mStatJustify);


    mLayout->addWidget(mStatusTop, 1) ;

    // create the help controls
    mHelpCtrl = new cEditorCtrl(mBaseWidget, true);
    mHelpCtrl->setObjectName(QStringLiteral("mHelpCtrl"));
    mHelpCtrl->setMaximumSize(QSize(16777215, 100));
    mLayout->addWidget(mHelpCtrl);

    mHelpKCtrl = new cEditorCtrl(mBaseWidget);
    mHelpKCtrl->setObjectName(QStringLiteral("mHelpKCtrl"));
    mHelpKCtrl->setMaximumSize(QSize(16777215, 100));
    mLayout->addWidget(mHelpKCtrl);
    mHelpKCtrl->hide();

    mHelpOCtrl = new cEditorCtrl(mBaseWidget);
    mHelpOCtrl->setObjectName(QStringLiteral("mHelpOCtrl"));
    mHelpOCtrl->setMaximumSize(QSize(16777215, 100));
    mLayout->addWidget(mHelpOCtrl);
    mHelpOCtrl->hide();

    mHelpPCtrl = new cEditorCtrl(mBaseWidget);
    mHelpPCtrl->setObjectName(QStringLiteral("mHelpPCtrl"));
    mHelpPCtrl->setMaximumSize(QSize(16777215, 100));
    mLayout->addWidget(mHelpPCtrl);
    mHelpPCtrl->hide();

    mHelpQCtrl = new cEditorCtrl(mBaseWidget);
    mHelpQCtrl->setObjectName(QStringLiteral("mHelpQCtrl"));
    mHelpQCtrl->setMaximumSize(QSize(16777215, 100));
    mLayout->addWidget(mHelpQCtrl);
    mHelpQCtrl->hide();

    // these strings are stored in internal buffer format for easy displaying
    string helpj("                        ----- E D I T   M E N U -----\n" \
    "  CURSOR      SCROLL        DELETE      OTHER                 MENUS\n" \
    " ^E up       ^W up         ^G char    ^J help                ^O onscreen format\n" \
    " ^X down     ^Z down       ^T word    ^I tab                 ^K block & save\n" \
    " ^S left     ^R up screen  ^Y line    ^V turn insert off     ^M macros\n" \
    " ^D right    ^C down      Del char    ^L find/replace again  ^P print controls\n" \
    " ^A word left   screen     ^U unerase                        ^Q quick functions\n" \
    " ^F word right                        F1 Setup\n" \
    "                                     F11 Full Screen") ;
    mHelpCtrl->SetAsHelp() ;
    mHelpCtrl->InsertWordStarString(helpj) ;

    string helpk("                  ----- B L O C K   &   S A V E   M E N U -----\n" \
   "    SAVE                     BLOCK                       WINDOW\n" \
   "  D save                   B begin block               A copy between\n" \
   "  T save as                K end block                 G move between\n" \
   "  S save and resume        C copy                      \n" \
   "  X save and exit          V move                        CASE\n" \
   "  Q abandon changes        Y delete                    \" upper\n" \
   "    FILE                   W write to disk             \' lower\n" \
   "  O copy                   M math                      . sentence\n" \
   "  E rename                 Z sort                      \n" \
   "  J delete                 ? word count                  CURSOR\n" \
   "  P print                  H turn disp on/off        0-9 set marker\n" \
   "  \\ fax                    U mark previous block       \n" \
   "  L change drive/dir       < unmark block                SYSTEM CLIPBOARD\n" \
   "  R insert a file          N turn column mode on       [ copy from clipboard\n" \
   "  F run command            I turn column replace on    ] copy to clipboard\n") ;
   mHelpKCtrl->SetAsHelp() ;
   mHelpKCtrl->InsertWordStarString(helpk) ;

   string helpp("              ----- P R I N T   C O N T R O L S   M E N U -----\n" \
   "            BEGIN & END                                OTHER \n" \
   "    B bold         X strike out         H overprint char   O binding space\n" \
   "    S underline    D double strike    RET overprint line   C print pause\n" \
   "    V subscript    Y italics            F phantom space    I 8-column tab\n" \
   "    T superscript  K indexing           G phantom rubout   . dot leader\n" \
   "                                        * graphics tag     0 extended characters\n" \
   "               STYLE                    & start Inset\n" \
   "    =/+ select font N Normal Font\n" \
   "    - select color  A alternate font    Q W E R ! custom    ? select printer") ;
   mHelpPCtrl->SetAsHelp() ;
   mHelpPCtrl->InsertWordStarString(helpp) ;


   string helpq("                      ----- Q U I C K   M E N U -----\n" \
   "            CURSOR              FIND            OTHER             SPELL\n" \
   " E upper left   P previous   F find text     U align rest doc  L check document\n" \
   " X lower right  V prev find  A find/replace  M math  Q repeat  N check word\n" \
   " S begin line   B beg block  G char forward  J thesaurus       O enter word\n" \
   " D end line     K end block  H char back                         DELETE\n" \
   " R beg doc    0-9 marker     I page/line       SCROLL        Del line to left\n" \
   " C end doc                   = next font     W up, repeat      Y line to right\n" \
   "                             < next style    Z dn, repeat      T to character\n") ;
   mHelpQCtrl->SetAsHelp() ;
   mHelpQCtrl->InsertWordStarString(helpq) ;


   string helpo("           ----- O N S C R E E N   F O R M A T   M E N U -----\n" \
   "   MARGINS & TABS            TYPING                         DISPLAY\n" \
   " L left  R right     C center line                   P page preview\n" \
   " G temorary indent   ] right flush line              D turn command tags off\n" \
   " X release margin    V vertically center             B change screen settings\n" \
   " I set/clear tabs    E enter soft hyphen             K open or switch window\n" \
   " O ruler to text     H turn auto-hyphenation off     M size current window\n" \
   " U column layout     J turn justification on         ? status\n" \
   " Y page layout       A turn auto-align off           Z paragraph number\n" \
   " F paragraph styles  W turn word wrap off            # page numbering\n" \
   " S set line spacing RET turn Enter closes dialog off N notes\n") ;
   mHelpOCtrl->SetAsHelp() ;
   mHelpOCtrl->InsertWordStarString(helpo) ;

    // create the ruler control
    mRuler = new cRulerCtrl(this) ;
    mLayout->addWidget(mRuler);


    // create the main editor
    mBaseEditor = new QWidget(this) ;
    mEditorLayout = new QHBoxLayout(mBaseEditor) ;
    mEditorLayout->setSpacing(0);
    mEditorLayout->setObjectName(QStringLiteral("mEditorLayout"));
    mEditorLayout->setContentsMargins(0, 0, 0, 0);
    mScrollbar = new QScrollBar(mBaseEditor) ;
    mEditor = new cEditorCtrl(mBaseEditor) ;
    mEditorLayout->addWidget(mEditor) ;
    mEditorLayout->addWidget(mScrollbar) ;

//    mEditor = new cEditorCtrl(mBaseWidget);
    mEditor->setObjectName(QStringLiteral("mEditor"));
    mEditor->SetFrame(this) ;

#if defined(Q_OS_MACOS)
    QFont font("Menlo", 12) ;

#elif defined(Q_OS_WINDOWS)
    QFont font("Consolas", 12);

#else
    QFont font("Monospace", 12) ;
#endif

#ifndef Q_OS_MACOS
    font.setPointSize(10);
#endif

    mHelpCtrl->SetFont(font);
    mHelpKCtrl->SetFont(font);
    mHelpOCtrl->SetFont(font);
    mHelpPCtrl->SetFont(font);
    mHelpQCtrl->SetFont(font);

    font.setPointSize(10) ;
    mEditor->SetFont(font) ;

    mLayout->addWidget(mBaseEditor) ;
//    mLayout->addWidget(mEditor);

    mBaseLayout->addLayout(mLayout);

    setCentralWidget(mBaseWidget);

//    menuBar = new QMenuBar(this);
//    menuBar->setObjectName(QStringLiteral("menuBar"));
//    menuBar->setGeometry(QRect(0, 0, 400, 22));
//    setMenuBar(menuBar);

//    mainToolBar = new QToolBar(this);
//    mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
//    addToolBar(Qt::TopToolBarArea, mainToolBar);

    mStatusBottom = new QStatusBar(this);
    mStatusBottom->setObjectName(QStringLiteral("statusBar"));

    setStatusBar(mStatusBottom);

    mStatText = new QLabel(" ", this) ;
    mStatusBottom->addPermanentWidget(mStatText, 1);

    mBusyIndicator = new QMovie(":/gui/images/busy.gif") ;
    QSize size(24, 24) ;
    mBusyIndicator->setScaledSize(size) ;

    mBusy = new QLabel(this) ;
    mBusy->setMovie(mBusyIndicator) ;
    mStatusBottom->addPermanentWidget(mBusy) ;

//    mBusy1 = new QLabel(this) ;
//    mBusy1->setMovie(mBusyIndicator) ;
//    mStatusBottom->addPermanentWidget(mBusy1) ;

//    mBusy2 = new QLabel(this) ;
//    mBusy2->setMovie(mBusyIndicator) ;
//    mStatusBottom->addPermanentWidget(mBusy2) ;

    mStatInfo = new QLabel("Page 1 of 1   Line 0   V:0.00 cm   column 0   Insert   Words 1   Characters 1   Hide   00:00:00", this) ;
    mStatusBottom->addPermanentWidget(mStatInfo, 1);

    mEditor->SetRuler(mRuler);
    mEditor->setFocusPolicy(Qt::StrongFocus);

//    mEditor->mDocument.Insert("﻿काचं शक्नोम्यत्तुम् । नोपहिनस्ति माम् ॥ ABC ﻿काचं शक्नोम्यत्तुम् । नोपहिनस्ति माम् ॥ ABCj fd lkjfdsjl lks  ﻿काचं शक्नोम्यत्तुम् । नोपहिनस्ति माम् ॥ ABC ﻿काचं शक्नोम्यत्तुम् । नोपहिनस्ति माम् ॥ ABC ﻿काचं शक्नोम्यत्तुम् । नोपहिनस्ति माम् ॥ ABC ﻿काचं शक्नोम्यत्तुम् । नोपहिनस्ति माम् ॥ ABC ﻿काचं शक्नोम्यत्तुम् । नोपहिनस्ति माम् ॥ ABC ") ;
//    mEditor->mDocument.Insert("ABC ") ;


    setWindowIcon(QIcon(":/gui/images/icon64x64.png")) ;

    ReadConfig();

    // filename as argument
//    if(argc > 1)
//    {
//        QString arg(argv[1]) ;
//        mLoadFileName = arg ;
//
//        QApplication::processEvents() ;
//
//        mEditor->LoadFile(mLoadFileName) ;
//    }
}




cWordTsar::~cWordTsar()
{
    WriteConfig() ;

//    delete mPrintPreviewAction ;
//    delete mPrintAction ;
//    delete mSaveandCloseAction ;
//    delete mSaveAsAction ;
//    delete mSaveAction ;
//    delete mOpenAction ;
//    delete menubar ;
    delete mStatInfo ;
    delete mBusy ;
    delete mBusyIndicator ;
    delete mStatText ;
    delete mStatusBottom ;
    delete mEditor ;
    delete mScrollbar ;
    delete mEditorLayout ;
    delete mBaseEditor ;
    delete mRuler ;
    delete mHelpQCtrl ;
    delete mHelpPCtrl ;
    delete mHelpOCtrl ;
    delete mHelpKCtrl ;
    delete mHelpCtrl ;
    delete mStatJustify ;
    delete mStatRight ;
    delete mStatCenter ;
    delete mStatLeft ;
    delete mStatChange ;
    delete mStatUnderline ;
    delete mStatItalic ;
    delete mStatBold ;
    delete mStatFont ;
    delete mStatStyle ;
    delete mStatusTop ;
    delete mLayout ;
    delete mBaseLayout ;
    delete mBaseWidget ;

}


/////////////////////////////////////////////////////////////////////////////
///
/// @param  event   [IN] - the close event
///
/// @return nothing
///
/// @brief
/// handle a window close event
///
/////////////////////////////////////////////////////////////////////////////
void cWordTsar::closeEvent(QCloseEvent *event)
{
    mEditor->CloseEvent() ;       // let our keyboard handler deal with it.
}



void cWordTsar::LoadFile(QString name)
{
    mLoadFileName = name ;
    mEditor->LoadFile(name) ;
}



void cWordTsar::UpdateStatus(cEditorCtrl *editor)
{
    if(editor == NULL)
    {
        return ;
    }
    
    if(mStatusBottom == NULL)
    {
        return ;
    }

    static char counter = 0 ;
    static eHelpDisplay lasthelp = HELP_MAIN ;
    
    sStatus cstatus ;
    editor->GetStatus(cstatus) ;
    
    time_t now = time(NULL) ;
    tm *t2 = localtime(&now) ;
    
    QString status ;
    
    switch(mEditor->mMeasure)
    {
        case MSR_INCHES :
            status.sprintf("Page %ld of %ld    line %ld    V:%0.2f in    column %ld    %s    %ld %s    %ld %s    %s  %s   %2d:%02d:%02d",
                        cstatus.page, cstatus.pagecount, cstatus.line, cstatus.height / TWIPSPERINCH , cstatus.column, (cstatus.mode ? "Insert" : "Overwrite"),
                        cstatus.wordcount, (cstatus.wordcount == 1 ? "Word" : "Words"), cstatus.charcount, (cstatus.charcount == 1 ? "Character" : "Characters"), (cstatus.showcontrol ? "Show" : "Hide"),
                        (cstatus.saving ? "- Saving...    " : ""), t2->tm_hour, t2->tm_min, t2->tm_sec) ;
            break ;
        
        case MSR_MILLIMETERS :
            status.sprintf("Page %ld of %ld    line %ld    V:%0.2f mm    column %ld    %s    %ld %s    %ld %s    %s  %s   %2d:%02d:%02d",
                        cstatus.page, cstatus.pagecount, cstatus.line, cstatus.height / TWIPSPERMM , cstatus.column, (cstatus.mode ? "Insert" : "Overwrite"),
                        cstatus.wordcount, (cstatus.wordcount == 1 ? "Word" : "Words"), cstatus.charcount, (cstatus.charcount == 1 ? "Character" : "Characters"), (cstatus.showcontrol ? "Show" : "Hide"),
                        (cstatus.saving ? "- Saving...    " : ""), t2->tm_hour, t2->tm_min, t2->tm_sec) ;
            break ;

        default :
            status.sprintf("Page %ld of %ld    line %ld    V:%0.2f cm    column %ld    %s    %ld %s    %ld %s    %s  %s   %2d:%02d:%02d",
                        cstatus.page, cstatus.pagecount, cstatus.line, cstatus.height / TWIPSPERCM , cstatus.column, (cstatus.mode ? "Insert" : "Overwrite"),
                        cstatus.wordcount, (cstatus.wordcount == 1 ? "Word" : "Words"), cstatus.charcount, (cstatus.charcount == 1 ? "Character" : "Characters"), (cstatus.showcontrol ? "Show" : "Hide"),
                        (cstatus.saving ? "- Saving...    " : ""), t2->tm_hour, t2->tm_min, t2->tm_sec) ;
            break ;
    }
    
    mStatInfo->setText(status) ; // << status ;
    
    mStatStyle->setText(cstatus.style) ;
    mStatFont->setText(cstatus.font) ;

    if(cstatus.showcontrol)
    {
        mStatChange->setStyleSheet("QLabel { color : black;}") ;
    }
    else
    {
        mStatChange->setStyleSheet("QLabel { color : lightgray;}") ;
    }

    if(cstatus.bold)
    {
        mStatBold->setStyleSheet("QLabel { color : black;}") ;
    }
    else
    {
        mStatBold->setStyleSheet("QLabel { color : lightgray;}") ;
    }

    if(cstatus.italic)
    {
        mStatItalic->setStyleSheet("QLabel { color : black;}") ;
    }
    else
    {
        mStatItalic->setStyleSheet("QLabel { color : lightgray;}") ;
    }

    if(cstatus.underline)
    {
        mStatUnderline->setStyleSheet("QLabel { color : black;}") ;
    }
    else
    {
        mStatUnderline->setStyleSheet("QLabel { color : lightgray;}") ;
    }


    if(cstatus.just == JUST_LEFT)
    {
        mStatLeft->setStyleSheet("QLabel { color : black;}") ;
        mStatCenter->setStyleSheet("QLabel { color : lightgray;}") ;
        mStatRight->setStyleSheet("QLabel { color : lightgray;}") ;
        mStatJustify->setStyleSheet("QLabel { color : lightgray;}") ;
    }
    else if(cstatus.just == JUST_CENTER)
    {
        mStatLeft->setStyleSheet("QLabel { color : lightgray;}") ;
        mStatCenter->setStyleSheet("QLabel { color : black;}") ;
        mStatRight->setStyleSheet("QLabel { color : lightgray;}") ;
        mStatJustify->setStyleSheet("QLabel { color : lightgray;}") ;
    }
    else if(cstatus.just == JUST_RIGHT)
    {
        mStatLeft->setStyleSheet("QLabel { color : lightgray;}") ;
        mStatCenter->setStyleSheet("QLabel { color : lightgray;}") ;
        mStatRight->setStyleSheet("QLabel { color : black;}") ;
        mStatJustify->setStyleSheet("QLabel { color : lightgray;}") ;
    }
    else if(cstatus.just == JUST_JUST)
    {
        mStatLeft->setStyleSheet("QLabel { color : lightgray;}") ;
        mStatCenter->setStyleSheet("QLabel { color : lightgray;}") ;
        mStatRight->setStyleSheet("QLabel { color : lightgray;}") ;
        mStatJustify->setStyleSheet("QLabel { color : black;}") ;
    }

    if(cstatus.help == HELP_NONE && lasthelp != HELP_NONE)
    {
        mHelpCtrl->hide(); ;
        mHelpKCtrl->hide() ;
        mHelpPCtrl->hide(); ;
        mHelpQCtrl->hide() ;
        mHelpOCtrl->hide(); ;
        lasthelp = HELP_NONE ;
//        mEditorSizer->Layout() ;
    }
    else if(cstatus.help == HELP_MAIN && lasthelp != HELP_MAIN)
    {
        mHelpCtrl->show() ;
        mHelpKCtrl->hide() ;
        mHelpPCtrl->hide() ;
        mHelpQCtrl->hide() ;
        mHelpOCtrl->hide() ;
        lasthelp = HELP_MAIN ;
//        mEditorSizer->Layout() ;
    }

    if(cstatus.help == HELP_CTRLK && lasthelp != HELP_CTRLK)
    {
        counter = 0 ;
        lasthelp = HELP_CTRLK ;
    }
    else
    {
        counter++ ;
    }
    if(cstatus.help == HELP_CTRLK && counter == HELPDELAY)
    {
        mHelpCtrl->hide() ;
        mHelpKCtrl->show() ;
        mHelpPCtrl->hide() ;
        mHelpQCtrl->hide() ;
        mHelpOCtrl->hide() ;
        lasthelp = HELP_CTRLK ;
//        mEditorSizer->Layout() ;
    }

    if(cstatus.help == HELP_CTRLP && lasthelp != HELP_CTRLP)
    {
        counter = 0 ;
        lasthelp = HELP_CTRLP ;
    }
    else
    {
        counter++ ;
    }
    if(cstatus.help == HELP_CTRLP && counter == HELPDELAY)
    {
        mHelpCtrl->hide() ;
        mHelpKCtrl->hide() ;
        mHelpPCtrl->show() ;
        mHelpQCtrl->hide() ;
        mHelpOCtrl->hide() ;
        lasthelp = HELP_CTRLP ;
//        mEditorSizer->Layout() ;
    }

    if(cstatus.help == HELP_CTRLQ && lasthelp != HELP_CTRLQ)
    {
        counter = 0 ;
        lasthelp = HELP_CTRLQ ;
    }
    else
    {
        counter++ ;
    }
    if(cstatus.help == HELP_CTRLQ && counter == HELPDELAY)
    {
        mHelpCtrl->hide() ;
        mHelpKCtrl->hide() ;
        mHelpPCtrl->hide() ;
        mHelpQCtrl->show() ;
        mHelpOCtrl->hide() ;
        lasthelp = HELP_CTRLQ ;
//        mEditorSizer->Layout() ;
    }

    if(cstatus.help == HELP_CTRLO && lasthelp != HELP_CTRLO)
    {
        counter = 0 ;
        lasthelp = HELP_CTRLO ;
    }
    else
    {
        counter++ ;
    }
    if(cstatus.help == HELP_CTRLO && counter == HELPDELAY)
    {
        mHelpCtrl->hide() ;
        mHelpKCtrl->hide() ;
        mHelpPCtrl->hide() ;
        mHelpQCtrl->hide() ;
        mHelpOCtrl->show() ;
        lasthelp = HELP_CTRLO ;
//        mEditorSizer->Layout() ;
    }

//    mRuler->show() ;
//    mEditor->show() ;

//    counter++ ;
}



void cWordTsar::SetStatus(QString text, bool progress, int percent)
{
    static int oldpercent = 0 ;


    int top, left, right, bottom ;

    if(progress == true)
    {
        mBusy->show() ;
//        mBusy1->show() ;
//        mBusy2->show() ;
        mBusyIndicator->start() ;
        QApplication::processEvents() ;
/*
        if(oldpercent != percent)
        {
            QRect fr = mStatText->frameRect() ;

            QPixmap pixmap(fr.width(), fr.height()) ;
            QPainter paint(&pixmap) ;

            QColor background = QWidget::palette().color(QWidget::backgroundRole()) ;
            paint.setPen(background) ;
            paint.setBrush(background) ;
            paint.drawRect(fr) ;

            fr.setWidth(fr.width() * ((double)percent / 100.0)) ;

            QColor green(69, 139, 0) ;
            paint.setPen(green) ;
            paint.setBrush(green) ;
            paint.drawRect(fr) ;

            QColor black(0, 0, 0) ;
            paint.setPen(black) ;
            paint.setBrush(black) ;
            paint.drawText(5, fr.height() - 5, text) ;

            mStatText->setPixmap(pixmap) ;
            oldpercent = percent ;

        }
*/
    }
    else
    {
        mBusy->hide() ;
//        mBusy1->hide() ;
//        mBusy2->hide() ;
        mBusyIndicator->stop() ;

        mStatText->setText(text) ;
        QApplication::processEvents();
    }

/*
    static int oldpercent = 0 ;
    
    if(mStatusBottom != nullptr)
    {
        if(progress == true && oldpercent != percent)
        {
            QRect r ;
            mStatusBottom->GetFieldRect(0, r) ;
            
            wxClientDC dc(mStatusBottom) ;
            
            r.width = r.width * ((double)percent / 100.0) ;
        
            QColor green(69, 139, 0) ;
            dc.SetPen(green) ;
            dc.SetBrush(green) ;
            dc.DrawRectangle(r) ;
            dc.DrawText(text, 5, 2) ;
            
            oldpercent = percent ;
        }
        else
        {
            mStatusBottom->SetStatusText(text, 0) ; // << status ;
        }
    }
//    wxYield() ;
*/
}

/*
void cWordTsar::StatusOn(int index, QString text, wxClientDC &dc)
{
//    wxClientDC dc(mStatusTop);

//    dc.SetBrush(QColor(150, 150, 225)) ;
//#ifdef __WXOSX__
//    dc.SetTextForeground(*wxRED) ;
//    dc.SetPen(*wxRED_PEN);
//#endif // __WXOSX__
    QRect r;
//    if ( mStatusTop->GetFieldRect(index, r))
//    {
//        dc.DrawRectangle(r);
//    }

    mStatusTop->GetFieldRect(index, r) ;
    
    dc.SetTextForeground(*wxBLACK) ;
    dc.DrawText(text, r.x + 4, r.y + 2) ;
}


void cWordTsar::StatusOff(int index, QString text, wxClientDC &dc)
{
//    wxClientDC dc(mStatusTop);
//    dc.SetPen(*wxRED_PEN);
//    dc.SetBrush( mStatusTop->GetBackgroundColour() ) ;

    QRect r;
//    if ( mStatusTop->GetFieldRect(index, r) )
//    {
//        dc.DrawRectangle(r);
//    }

    mStatusTop->GetFieldRect(index, r) ;
    dc.SetTextForeground(QColor(160, 160, 160)) ;
    dc.DrawText(text, r.x + 4, r.y + 2) ;
}

*/

void cWordTsar::ReadConfig(void)
{
    CSimpleIniA ini ;

    QString dir = QDir::homePath() ;
    dir += "/.WordTsar" ;

    if(ini.LoadFile(dir.toUtf8().constData()) >= 0)
    {
        bool t =  ini.GetBoolValue("base", "showcontrols", true) ;
        if(t)
        {
            mEditor->SetShowControls(SHOW_ALL) ;
        }
        else
        {
            mEditor->SetShowControls(SHOW_DOT) ;
        }

        short red, green, blue ;
        red = ini.GetLongValue("base", "backgroundred", 245) ;
        green = ini.GetLongValue("base", "backgroundgreen", 245) ;
        blue = ini.GetLongValue("base", "backgroundblue", 245) ;
//        mEditor->SetBackgroundColour(QColor(red, green, blue)) ;
        mEditor->SetBGroundColour(QColor(red, green, blue)) ;

        red = ini.GetLongValue("base", "textred", 0) ;
        green = ini.GetLongValue("base", "textgreen", 0) ;
        blue = ini.GetLongValue("base", "textblue", 0) ;
        mEditor->SetTextColour(QColor(red, green, blue)) ;

        red = ini.GetLongValue("base", "highlightred", 0) ;
        green = ini.GetLongValue("base", "highlightgreen", 150) ;
        blue = ini.GetLongValue("base", "highlightblue", 200) ;
        mEditor->SetHighlightColour(QColor(red, green, blue, 127)) ;

        red = ini.GetLongValue("base", "dotred", 100) ;
        green = ini.GetLongValue("base", "dotgreen", 200) ;
        blue = ini.GetLongValue("base", "dotblue", 200) ;
        mEditor->SetDotColour(QColor(red, green, blue, 190)) ;

        red = ini.GetLongValue("base", "blockred", 50) ;
        green = ini.GetLongValue("base", "blockgreen", 100) ;
        blue = ini.GetLongValue("base", "blockblue", 200) ;
        mEditor->SetBlockColour(QColor(red, green, blue, 190)) ;

        red = ini.GetLongValue("base", "commentred", 255) ;
        green = ini.GetLongValue("base", "commentgreen", 178) ;
        blue = ini.GetLongValue("base", "commentblue", 102) ;
        mEditor->SetCommentColour(QColor(red, green, blue, 190)) ;

        red = ini.GetLongValue("base", "unknownred", 194) ;
        green = ini.GetLongValue("base", "unknowngreen", 70) ;
        blue = ini.GetLongValue("base", "unknownblue", 65) ;
        mEditor->SetUnknownColour(QColor(red, green, blue, 190)) ;

//        red = ini.GetLongValue("base", "specialred", ) ;
//        green = ini.GetLongValue("base", "specialgreen", ) ;
//        blue = ini.GetLongValue("base", "specialblue", ) ;
//        mEditor->SetSpecialColour(0, QColor(red, green, blue, 190)) ;

        mEditor->mShortName = ini.GetValue("base", "shortname", "none") ;
        mEditor->mLongName = ini.GetValue("base", "longname", "none") ;

        long width, height ;
        width = ini.GetLongValue("base", "windowwidth", 800) ;
        height = ini.GetLongValue("base", "windowheight", 600) ;

        resize(width, height) ;

//        mEditor->mAutoSaveTime = ini.GetLongValue("base", "autosave", 300) ;

//        mEditor->mWrapStyle = ini.GetLongValue("base", "wrapstyle", 1) ;

        mEditor->mHelpDisplay = static_cast<eHelpDisplay>(ini.GetLongValue("base", "showhelp", HELP_MAIN)) ;

//        mEditor->mCheckForUpdates = static_cast<char>(ini.GetLongValue("base", "checkupdate", 0)) ;

//        mEditor->mMetric = ini.GetBoolValue("base", "metric", true) ;
        mEditor->SetMeasurement(ini.GetValue("base", "measurement", "0cm")) ;
        
        mEditor->mDispRuler = ini.GetBoolValue("base", "dispruler", true) ;
        mEditor->mDispScrollBar = ini.GetBoolValue("base", "dispscroll", true) ; ;
        mEditor->mDispStatusBar = ini.GetBoolValue("base", "dispstatusbottom", true) ;
        mEditor->mDispStyleBar = ini.GetBoolValue("base", "dispstyle", true) ;
        mEditor->mAlwaysDot = ini.GetBoolValue("base", "dispdotalways", true) ;
        if(mEditor->GetShowControls() == SHOW_DOT && mEditor->mAlwaysDot == false)
        {
            mEditor->SetShowDot(false) ;
        }
        mEditor->mAlwaysFlag = ini.GetBoolValue("base", "dispflagalways", true) ;
        

    }
    else
    {
        mEditor->SetShowControls(SHOW_ALL) ;
//        mEditor->SetBackgroundColour(QColor(245, 245, 245)) ;
        mEditor->SetBGroundColour(QColor(245, 245, 245)) ;
        mEditor->SetTextColour(QColor(0, 0, 0)) ;
        mEditor->SetHighlightColour(QColor(0, 150, 200, 190)) ;
        mEditor->SetDotColour(QColor(100, 200, 200, 190)) ;
        mEditor->SetBlockColour(QColor(50, 100, 200, 190)) ;
        mEditor->SetCommentColour(QColor(255, 178, 102, 190)) ;
        mEditor->SetUnknownColour(QColor(194, 70, 65, 190)) ;
//        mEditor->SetSpecialColour(0, QColor()) ;
        
//        mEditor->mMetric = true ;
        mEditor->SetMeasurement("0cm") ;

        mEditor->mDispRuler = true ;
        mEditor->mDispScrollBar = true ;
        mEditor->mDispStatusBar = true ;
        mEditor->mDispStyleBar = true ;
        mEditor->mAlwaysDot = true ;
        mEditor->mAlwaysFlag = true ;
        
//        mEditor->mAutoSaveTime = 300 ;
//        mEditor->mWrapStyle = 1 ;
    }
/*
    if(mEditor->mDispScrollBar)
    {
        mEditor->ShowScrollbars(wxSHOW_SB_NEVER, wxSHOW_SB_ALWAYS) ;
    }
    else
    {
        mEditor->ShowScrollbars(wxSHOW_SB_NEVER, wxSHOW_SB_NEVER) ;
    }
    
    if(mEditor->mDispStyleBar)
    {
        mStatusTop->Show() ;
    }
    else
    {
        mStatusTop->Hide() ;
    }

    if(mEditor->mDispRuler)
    {
        mRuler->Show() ;
    }
    else
    {
        mRuler->Hide() ;
    }

    if(mEditor->mDispStatusBar)
    {
        mStatusBottom->Show() ;
    }
    else
    {
        mStatusBottom->Hide() ;
    }

    mEditorSizer->Layout() ;
*/
}


void cWordTsar::WriteConfig(void)
{
    CSimpleIniA ini ;

    QString dir = QDir::homePath() ;
    dir += "/.WordTsar" ;

//    if(ini.LoadFile(dir.char_str()) >= 0)
    ini.LoadFile(dir.toUtf8().constData()) ;

    {
        if(mEditor->GetShowControls() == SHOW_ALL)
        {
            ini.SetBoolValue("base", "showcontrols", true) ;
        }
        else
        {
            ini.SetBoolValue("base", "showcontrols", false) ;
        }
//        ini.SetLongValue("showhelp", mEditor->mHelpDisplay) ;

        short red, green, blue ;
        QColor color = mEditor->GetBGroundColour() ;
        red = ini.SetLongValue("base", "backgroundred", color.red()) ;
        green = ini.SetLongValue("base", "backgroundgreen", color.green()) ;
        blue = ini.SetLongValue("base", "backgroundblue", color.blue()) ;

        color = mEditor->GetTextColour() ;
        red = ini.SetLongValue("base", "textred", color.red()) ;
        green = ini.SetLongValue("base", "textgreen", color.green()) ;
        blue = ini.SetLongValue("base", "textblue", color.blue()) ;

        color = mEditor->GetHighlightColour() ;
        red = ini.SetLongValue("base", "highlightred", color.red()) ;
        green = ini.SetLongValue("base", "highlightgreen", color.green()) ;
        blue = ini.SetLongValue("base", "highlightblue", color.blue()) ;

        color = mEditor->GetDotColour() ;
        red = ini.SetLongValue("base", "dotred", color.red()) ;
        green = ini.SetLongValue("base", "dotgreen", color.green()) ;
        blue = ini.SetLongValue("base", "dotblue", color.blue()) ;

        color = mEditor->GetBlockColour() ;
        red = ini.SetLongValue("base", "blockred", color.red()) ;
        green = ini.SetLongValue("base", "blockgreen", color.green()) ;
        blue = ini.SetLongValue("base", "blockblue", color.blue()) ;

        color = mEditor->GetCommentColour() ;
        red = ini.SetLongValue("base", "commentred", color.red()) ;
        green = ini.SetLongValue("base", "commentgreen", color.green()) ;
        blue = ini.SetLongValue("base", "commentblue", color.blue()) ;

        color = mEditor->GetUnknownColour() ;
        red = ini.SetLongValue("base", "unknownred", color.red()) ;
        green = ini.SetLongValue("base", "unknowngreen", color.green()) ;
        blue = ini.SetLongValue("base", "unknownblue", color.blue()) ;

        ini.SetValue("base", "shortname", mEditor->mShortName.toUtf8().constData()) ;
        ini.SetValue("base", "longname", mEditor->mLongName.toUtf8().constData()) ;

        QSize wsize = size() ;
        red = ini.SetLongValue("base", "windowwidth", wsize.width()) ;
        blue = ini.SetLongValue("base", "windowheight", wsize.height()) ;

//        ini.SetLongValue("base", "autosave", mEditor->mAutoSaveTime) ;

//        ini.SetLongValue("base", "wrapstyle", mEditor->mWrapStyle) ;

        ini.SetLongValue("base", "showhelp", mEditor->mHelpDisplay) ;

//        ini.SetLongValue("base", "checkupdate", mEditor->mCheckForUpdates) ;

//        ini.SetBoolValue("base", "metric", mEditor->mMetric) ;
        ini.SetValue("base", "measurement", mEditor->GetMeasurement().toUtf8().constData()) ;

        ini.SetBoolValue("base", "dispruler", mEditor->mDispRuler) ;
        ini.SetBoolValue("base", "dispscroll", mEditor->mDispScrollBar) ;
        ini.SetBoolValue("base", "dispstatusbottom", mEditor->mDispStatusBar) ;
        ini.SetBoolValue("base", "dispstyle", mEditor->mDispStyleBar) ;
        ini.SetBoolValue("base", "dispdotalways", mEditor->mAlwaysDot) ;
        ini.SetBoolValue("base", "dispflagalways", mEditor->mAlwaysFlag) ;

        ini.SaveFile(dir.toUtf8().constData()) ;
    }
}


void cWordTsar::CreateMenus(void)
{
    menuBar = new QMenuBar() ;
//    int mh = menuBar->height() ;
    menuBar->setMaximumSize(16777215, 25) ;

    QMenu *filemenu = menuBar->addMenu("&File") ;
    QMenu *editmenu = menuBar->addMenu("&Edit") ;
    QMenu *viewmenu = menuBar->addMenu("&View") ;
    QMenu *insertmenu = menuBar->addMenu("&Insert") ;
    QMenu *stylemenu = menuBar->addMenu("&Style") ;
    QMenu *layoutmenu = menuBar->addMenu("&Layout") ;
    QMenu *utilitiesmenu = menuBar->addMenu("&Utilities") ;


    mOpenAction = new QAction("&Open", this) ;
    mSaveAction = new QAction("&Save", this) ;
    mSaveAsAction = new QAction("Save &As...", this) ;
    mSaveandCloseAction = new QAction("Save and &Close", this) ;
    mPrintAction = new QAction("Print...") ;
    mPrintPreviewAction = new QAction("Print Preview...") ;

    filemenu->addAction(mOpenAction) ;
    filemenu->addAction(mSaveAction) ;
    filemenu->addAction(mSaveAsAction) ;
    filemenu->addAction(mSaveandCloseAction) ;
    filemenu->addSeparator() ;
    filemenu->addAction(mPrintAction) ;
    filemenu->addAction(mPrintPreviewAction) ;

    // disable things not done yet
    mOpenAction->setDisabled(true) ;
    mSaveAction->setDisabled(true) ;
    mSaveAsAction->setDisabled(true) ;
    mSaveandCloseAction->setDisabled(true) ;
    mPrintAction->setDisabled(true) ;
    mPrintPreviewAction->setDisabled(true) ;

    menuBar->hide() ;
}
