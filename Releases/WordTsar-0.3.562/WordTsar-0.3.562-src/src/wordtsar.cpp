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

#include "src/core/include/utils.h"

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

    char str[2048] ;
    
    switch(mEditor->mMeasure)
    {
        case MSR_INCHES :
            snprintf(str, 2000, "Page %ld of %ld    line %ld    V:%0.2f in    column %ld    %s    %ld %s    %ld %s    %s  %s   %2d:%02d:%02d",
                        cstatus.page, cstatus.pagecount, cstatus.line, cstatus.height / TWIPSPERINCH , cstatus.column, (cstatus.mode ? "Insert" : "Overwrite"),
                        cstatus.wordcount, (cstatus.wordcount == 1 ? "Word" : "Words"), cstatus.charcount, (cstatus.charcount == 1 ? "Character" : "Characters"), (cstatus.showcontrol ? "Show" : "Hide"),
                        (cstatus.saving ? "- Saving...    " : ""), t2->tm_hour, t2->tm_min, t2->tm_sec) ;
            status = str ;
            break ;
        
        case MSR_MILLIMETERS :
            snprintf(str, 2000, "Page %ld of %ld    line %ld    V:%0.2f mm    column %ld    %s    %ld %s    %ld %s    %s  %s   %2d:%02d:%02d",
                        cstatus.page, cstatus.pagecount, cstatus.line, cstatus.height / TWIPSPERMM , cstatus.column, (cstatus.mode ? "Insert" : "Overwrite"),
                        cstatus.wordcount, (cstatus.wordcount == 1 ? "Word" : "Words"), cstatus.charcount, (cstatus.charcount == 1 ? "Character" : "Characters"), (cstatus.showcontrol ? "Show" : "Hide"),
                        (cstatus.saving ? "- Saving...    " : ""), t2->tm_hour, t2->tm_min, t2->tm_sec) ;
            status = str ;
            break ;

        default :
            snprintf(str, 2000, "Page %ld of %ld    line %ld    V:%0.2f cm    column %ld    %s    %ld %s    %ld %s    %s  %s   %2d:%02d:%02d",
                        cstatus.page, cstatus.pagecount, cstatus.line, cstatus.height / TWIPSPERCM , cstatus.column, (cstatus.mode ? "Insert" : "Overwrite"),
                        cstatus.wordcount, (cstatus.wordcount == 1 ? "Word" : "Words"), cstatus.charcount, (cstatus.charcount == 1 ? "Character" : "Characters"), (cstatus.showcontrol ? "Show" : "Hide"),
                        (cstatus.saving ? "- Saving...    " : ""), t2->tm_hour, t2->tm_min, t2->tm_sec) ;
            status = str ;
            break ;
    }
    
    mStatInfo->setText(status) ; // << status ;
    
    mStatStyle->setText(cstatus.style) ;
    mStatFont->setText(cstatus.font) ;

    if(cstatus.showcontrol)
    {
        mStatChange->setStyleSheet("QLabel { font-weight : bold; font-style : normal;}") ;
    }
    else
    {
        mStatChange->setStyleSheet("QLabel { font-weight : normal; font-style : italic;}") ;
    }

    if(cstatus.bold)
    {
        mStatBold->setStyleSheet("QLabel { font-weight : bold; font-style : normal;}") ;
    }
    else
    {
        mStatBold->setStyleSheet("QLabel { font-weight : normal; font-style : italic;}") ;
    }

    if(cstatus.italic)
    {
        mStatItalic->setStyleSheet("QLabel { font-weight : bold; font-style : normal;}") ;
    }
    else
    {
        mStatItalic->setStyleSheet("QLabel { font-weight : normal; font-style : italic;}") ;
    }

    if(cstatus.underline)
    {
        mStatUnderline->setStyleSheet("QLabel { font-weight : bold; font-style : normal;}") ;
    }
    else
    {
        mStatUnderline->setStyleSheet("QLabel { font-weight : normal; font-style : italic;}") ;
    }


    if(cstatus.just == JUST_LEFT)
    {
        mStatLeft->setStyleSheet("QLabel { font-weight : bold; font-style : normal;}") ;
        mStatCenter->setStyleSheet("QLabel { font-weight : normal; font-style : italic;}") ;
        mStatRight->setStyleSheet("QLabel { font-weight : normal; font-style : italic;}") ;
        mStatJustify->setStyleSheet("QLabel { font-weight : normal; font-style : italic;}") ;
    }
    else if(cstatus.just == JUST_CENTER)
    {
        mStatLeft->setStyleSheet("QLabel { font-weight : normal; font-style : italic;}") ;
        mStatCenter->setStyleSheet("QLabel { font-weight : bold; font-style : normal;}") ;
        mStatRight->setStyleSheet("QLabel { font-weight : normal; font-style : italic;}") ;
        mStatJustify->setStyleSheet("QLabel { font-weight : normal; font-style : italic;}") ;
    }
    else if(cstatus.just == JUST_RIGHT)
    {
        mStatLeft->setStyleSheet("QLabel { font-weight : normal; font-style : italic;}") ;
        mStatCenter->setStyleSheet("QLabel { font-weight : normal; font-style : italic;}") ;
        mStatRight->setStyleSheet("QLabel { font-weight : bold; font-style : normal;}") ;
        mStatJustify->setStyleSheet("QLabel { font-weight : normal; font-style : italic;}") ;
    }
    else if(cstatus.just == JUST_JUST)
    {
        mStatLeft->setStyleSheet("QLabel { font-weight : normal; font-style : italic;}") ;
        mStatCenter->setStyleSheet("QLabel { font-weight : normal; font-style : italic;}") ;
        mStatRight->setStyleSheet("QLabel { font-weight : normal; font-style : italic;}") ;
        mStatJustify->setStyleSheet("QLabel { font-weight : bold; font-style : normal;}") ;
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
        ini.SetLongValue("base", "backgroundred", color.red()) ;
        ini.SetLongValue("base", "backgroundgreen", color.green()) ;
        ini.SetLongValue("base", "backgroundblue", color.blue()) ;

        color = mEditor->GetTextColour() ;
        ini.SetLongValue("base", "textred", color.red()) ;
        ini.SetLongValue("base", "textgreen", color.green()) ;
        ini.SetLongValue("base", "textblue", color.blue()) ;

        color = mEditor->GetHighlightColour() ;
        ini.SetLongValue("base", "highlightred", color.red()) ;
        ini.SetLongValue("base", "highlightgreen", color.green()) ;
        ini.SetLongValue("base", "highlightblue", color.blue()) ;

        color = mEditor->GetDotColour() ;
        ini.SetLongValue("base", "dotred", color.red()) ;
        ini.SetLongValue("base", "dotgreen", color.green()) ;
        ini.SetLongValue("base", "dotblue", color.blue()) ;

        color = mEditor->GetBlockColour() ;
        ini.SetLongValue("base", "blockred", color.red()) ;
        ini.SetLongValue("base", "blockgreen", color.green()) ;
        ini.SetLongValue("base", "blockblue", color.blue()) ;

        color = mEditor->GetCommentColour() ;
        ini.SetLongValue("base", "commentred", color.red()) ;
        ini.SetLongValue("base", "commentgreen", color.green()) ;
        ini.SetLongValue("base", "commentblue", color.blue()) ;

        color = mEditor->GetUnknownColour() ;
        ini.SetLongValue("base", "unknownred", color.red()) ;
        ini.SetLongValue("base", "unknowngreen", color.green()) ;
        ini.SetLongValue("base", "unknownblue", color.blue()) ;

        ini.SetValue("base", "shortname", mEditor->mShortName.toUtf8().constData()) ;
        ini.SetValue("base", "longname", mEditor->mLongName.toUtf8().constData()) ;

        QSize wsize = size() ;
        ini.SetLongValue("base", "windowwidth", wsize.width()) ;
        ini.SetLongValue("base", "windowheight", wsize.height()) ;

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

    // removed (commented) all SetSHortcuts, as Qt steals the keypresses, and command entry no longer works from the keyboard.

    // file menu
    QAction *OpenAction = new QAction("&Open/Read\t^KR", this) ;
    QAction *SaveAction = new QAction("&Save\t^KS", this) ;
    QAction *SaveAsAction = new QAction("Save &As...\t^KT", this) ;
    QAction *SaveandCloseAction = new QAction("Save and &Close\t^KD", this) ;
    QAction *PrintAction = new QAction("Print...\t^KP") ;
    QAction *PrintPreviewAction = new QAction("Print Preview...\t^OP") ;
    QAction *ExitAction = new QAction("E&xit WordTsar\t^KX") ;

    filemenu->addAction(OpenAction) ;
    filemenu->addAction(SaveAction) ;
    filemenu->addAction(SaveAsAction) ;
    filemenu->addAction(SaveandCloseAction) ;
    filemenu->addSeparator() ;
    filemenu->addAction(PrintAction) ;
    filemenu->addAction(PrintPreviewAction) ;
    filemenu->addSeparator() ;
    filemenu->addAction(ExitAction) ;

    // disable things not done yet
/*
    mOpenAction->setDisabled(true) ;
    mSaveAction->setDisabled(true) ;
    mSaveAsAction->setDisabled(true) ;
    mSaveandCloseAction->setDisabled(true) ;
    mPrintAction->setDisabled(true) ;
    mPrintPreviewAction->setDisabled(true) ;
*/
    // connect things up
    connect(OpenAction, &QAction::triggered, this, &cWordTsar::Open) ;
    connect(SaveAction, &QAction::triggered, this, &cWordTsar::Save) ;
    connect(SaveAsAction, &QAction::triggered, this, &cWordTsar::SaveAs) ;
    connect(SaveandCloseAction, &QAction::triggered, this, &cWordTsar::SaveandClose) ;
    connect(PrintAction, &QAction::triggered, this, &cWordTsar::PrintPreview) ;
    connect(PrintPreviewAction, &QAction::triggered, this, &cWordTsar::PrintPreview) ;
    connect(ExitAction, &QAction::triggered, this, &cWordTsar::ExitWordTsar) ;

    //edit menu
    QAction *UndoAction = new QAction("&Undo\t^U", this) ;
UndoAction->setEnabled(false) ;
    QAction *MarkBlockStartAction = new QAction("Mark Block &Beginning\t^KB", this) ;
    QAction *MarkBlockEndAction = new QAction("Mark Block &End\t^KK", this) ;

    QAction *MoveBlock = new QAction("&Block\t^KV", this) ;
//    QAction *MoveBlockWindow = new QAction("&Block from Other Window", this) ;

    QAction *CopyBlockAction = new QAction("&Block\t^KC", this) ;
//    QAction *CopyBlockWindowAction = new QAction("Mark Block &Previous", this) ;
    QAction *CopyfromClipboardAction = new QAction("&From Clipboard\t^K[", this) ;
    QAction *CopytoClipboardAction = new QAction("&To Clipboard\t^K]", this) ;
//    QAction *CopytoOtherFileAction = new QAction("Mark Block &Previous", this) ;

    QAction *DeleteBlockAction = new QAction("&Block\t^KY", this) ;
    QAction *DeleteWordAction = new QAction("&Word\t^T", this) ;
    QAction *DeleteLineAction = new QAction("&Line\t^Y", this) ;
    QAction *DeleteLineLeftAction = new QAction("L&ine Left of Cursor\t^QDel", this) ;
    QAction *DeleteLineRightAction = new QAction("Line &Right of Cursor\t^QY", this) ;
    QAction *DeleteToCharAction = new QAction("&To Character...\t^QT", this) ;
DeleteToCharAction->setEnabled(false) ;

    QAction *MarkPreviousBlockAction = new QAction("Mark &Previous Block\t^KU", this) ;
MarkPreviousBlockAction->setEnabled(false) ;

    QAction *FindAction = new QAction("&Find\t^QF", this) ;
    QAction *FindandReplaceAction = new QAction("Find and &Replace...\t^QA", this) ;
    QAction *FindNextAction = new QAction("Find Ne&xt\t^L", this) ;
    QAction *GotoCharAction = new QAction("&Go to Charcater...\t^QG", this) ;
GotoCharAction->setEnabled(false) ;
    QAction *GotoPageAction = new QAction("Goto &Page...\t^QI", this) ;
GotoPageAction->setEnabled(false) ;
    QAction *GotoMarker1Action = new QAction("&1\t^Q1", this) ;
    QAction *GotoMarker2Action = new QAction("&2\t^Q2", this) ;
    QAction *GotoMarker3Action = new QAction("&3\t^Q3", this) ;
    QAction *GotoMarker4Action = new QAction("&4\t^Q4", this) ;
    QAction *GotoMarker5Action = new QAction("&5\t^Q5", this) ;
    QAction *GotoMarker6Action = new QAction("&6\t^Q6", this) ;
    QAction *GotoMarker7Action = new QAction("&7\t^Q7", this) ;
    QAction *GotoMarker8Action = new QAction("&8\t^Q8", this) ;
    QAction *GotoMarker9Action = new QAction("&9\t^Q9", this) ;
    QAction *GotoMarker0Action = new QAction("&0\t^Q0", this) ;

    QAction *GotoFontTagAction = new QAction("&Font Tag\t^Q=", this) ;
GotoFontTagAction->setEnabled(false) ;
    QAction *GotoStyleTagAction = new QAction("S&tyle Tag\t^Q<", this) ;
GotoStyleTagAction->setEnabled(false) ;
    QAction *GotoNoteAction = new QAction("Note...\t^ONG", this) ;
GotoNoteAction->setEnabled(false) ;
    QAction *GotoPreviousPositionAction = new QAction("&Previous Position\t^QP", this) ;
GotoPreviousPositionAction->setEnabled(false) ;
    QAction *GotoLastFindReplaceAction = new QAction("&Last Find/Replace\t^QV", this) ;
GotoLastFindReplaceAction->setEnabled(false) ;
    QAction *GotoBeginningofBlockAction = new QAction("Beginning of Block\t^QB", this) ;
    QAction *GotoEndofBlockAction = new QAction("&End of Block\t^QK", this) ;
    QAction *GotoDocumentStartAction = new QAction("&Document Beginning\t^QR", this) ;
    QAction *GotoDocumentEndAction = new QAction("D&ocument End\t^QC", this) ;
    QAction *GotoScrollUpAction = new QAction("Scroll Continuously &Up\t^QW", this) ;
    QAction *GotoScrollDownAction = new QAction("&Scroll COntinuously Down\t^QZ", this) ;

    QAction *SetMarker1Action = new QAction("&1\t^K1", this) ;
    QAction *SetMarker2Action = new QAction("&2\t^K2", this) ;
    QAction *SetMarker3Action = new QAction("&3\t^K3", this) ;
    QAction *SetMarker4Action = new QAction("&4\t^K4", this) ;
    QAction *SetMarker5Action = new QAction("&5\t^K5", this) ;
    QAction *SetMarker6Action = new QAction("&6\t^K6", this) ;
    QAction *SetMarker7Action = new QAction("&7\t^K7", this) ;
    QAction *SetMarker8Action = new QAction("&8\t^K8", this) ;
    QAction *SetMarker9Action = new QAction("&9\t^K9", this) ;
    QAction *SetMarker0Action = new QAction("&0\t^K0", this) ;

    QAction *EditNoteAction = new QAction("Edit &Note\t^OND", this) ;
EditNoteAction->setEnabled(false) ;

    QAction *NoteStartNumberAction = new QAction("&Starting Number for Note...", this) ;
NoteStartNumberAction->setEnabled(false) ;
    QAction *NoteConvertAction = new QAction("&Convert Note...\t^ONV", this) ;
NoteConvertAction->setEnabled(false) ;
    QAction *NoteCovertPrintAction = new QAction("Convert at &Print...\t.cv", this) ;
NoteCovertPrintAction->setEnabled(false) ;
    QAction *NotEndnoteLocationAction = new QAction("&Endnote Location\t.pe", this) ;
NotEndnoteLocationAction->setEnabled(false) ;

    QAction *ColumnBlockModeAction = new QAction("&Column Block Mode\t^KN", this) ;
ColumnBlockModeAction->setEnabled(false) ;
    QAction *ColumnReplaceModeAction = new QAction("Column &Replace Mode\t^KI", this) ;
ColumnReplaceModeAction->setEnabled(false) ;
    QAction *AutoAlineAction = new QAction("&Auto Align\t^OA", this) ;
AutoAlineAction->setEnabled(false) ;
    QAction *CloseDialogAction = new QAction("↵ Closes Dialog\t^O↵", this) ;
CloseDialogAction->setEnabled(false) ;

    editmenu->addAction(UndoAction) ;
    editmenu->addSeparator() ;
    editmenu->addAction(MarkBlockStartAction) ;
    editmenu->addAction(MarkBlockEndAction) ;

    QMenu *MoveMenu = editmenu->addMenu("Mo&ve") ;
    MoveMenu->addAction(MoveBlock) ;

    QMenu *CopyMenu = editmenu->addMenu("&Copy") ;
    CopyMenu->addAction(CopyBlockAction) ;
    CopyMenu->addAction(CopyfromClipboardAction) ;
    CopyMenu->addAction(CopytoClipboardAction) ;

    QMenu *DeleteMenu = editmenu->addMenu("&Delete") ;
    DeleteMenu->addAction(DeleteBlockAction) ;
    DeleteMenu->addAction(DeleteWordAction) ;
    DeleteMenu->addAction(DeleteLineAction) ;
    DeleteMenu->addAction(DeleteLineLeftAction) ;
    DeleteMenu->addAction(DeleteLineRightAction) ;
    DeleteMenu->addAction(DeleteToCharAction) ;

    editmenu->addAction(MarkPreviousBlockAction) ;
    editmenu->addSeparator() ;
    editmenu->addAction(FindAction) ;
    editmenu->addAction(FindandReplaceAction) ;
    editmenu->addAction(FindNextAction) ;
    editmenu->addAction(GotoCharAction) ;
    editmenu->addAction(GotoPageAction) ;

    QMenu *GotoMarkerMenu = editmenu->addMenu("Go to &Marker") ;
    GotoMarkerMenu->addAction(GotoMarker1Action) ;
    GotoMarkerMenu->addAction(GotoMarker2Action) ;
    GotoMarkerMenu->addAction(GotoMarker3Action) ;
    GotoMarkerMenu->addAction(GotoMarker4Action) ;
    GotoMarkerMenu->addAction(GotoMarker5Action) ;
    GotoMarkerMenu->addAction(GotoMarker6Action) ;
    GotoMarkerMenu->addAction(GotoMarker7Action) ;
    GotoMarkerMenu->addAction(GotoMarker8Action) ;
    GotoMarkerMenu->addAction(GotoMarker9Action) ;
    GotoMarkerMenu->addAction(GotoMarker0Action) ;

    QMenu *GotoOtherMenu = editmenu->addMenu("Go to &Other") ;
    GotoOtherMenu->addAction(GotoFontTagAction) ;
    GotoOtherMenu->addAction(GotoStyleTagAction) ;
    GotoOtherMenu->addAction(GotoNoteAction) ;
    GotoOtherMenu->addAction(GotoPreviousPositionAction) ;
    GotoOtherMenu->addAction(GotoLastFindReplaceAction) ;
    GotoOtherMenu->addAction(GotoBeginningofBlockAction) ;
    GotoOtherMenu->addAction(GotoEndofBlockAction) ;
    GotoOtherMenu->addAction(GotoDocumentStartAction) ;
    GotoOtherMenu->addAction(GotoDocumentEndAction) ;
    GotoOtherMenu->addAction(GotoScrollUpAction) ;
    GotoOtherMenu->addAction(GotoScrollDownAction) ;

    QMenu *SetMarkerMenu = editmenu->addMenu("&Set Marker") ;
    SetMarkerMenu->addAction(SetMarker1Action) ;
    SetMarkerMenu->addAction(SetMarker2Action) ;
    SetMarkerMenu->addAction(SetMarker3Action) ;
    SetMarkerMenu->addAction(SetMarker4Action) ;
    SetMarkerMenu->addAction(SetMarker5Action) ;
    SetMarkerMenu->addAction(SetMarker6Action) ;
    SetMarkerMenu->addAction(SetMarker7Action) ;
    SetMarkerMenu->addAction(SetMarker8Action) ;
    SetMarkerMenu->addAction(SetMarker9Action) ;
    SetMarkerMenu->addAction(SetMarker0Action) ;

    editmenu->addAction(EditNoteAction) ;

    QMenu *NoteOptionsMenu = editmenu->addMenu("No&te Options") ;
    NoteOptionsMenu->addAction(NoteStartNumberAction) ;
    NoteOptionsMenu->addAction(NoteConvertAction) ;
    NoteOptionsMenu->addAction(NoteCovertPrintAction) ;
    NoteOptionsMenu->addAction(NotEndnoteLocationAction) ;

    QMenu *EditSettingsMenu = editmenu->addMenu("Ed&iting Settings") ;
    EditSettingsMenu->addAction(ColumnBlockModeAction) ;
    EditSettingsMenu->addAction(ColumnReplaceModeAction) ;
    EditSettingsMenu->addAction(AutoAlineAction) ;
    EditSettingsMenu->addAction(CloseDialogAction) ;

    // connect things up
    connect(UndoAction, &QAction::triggered, this, &cWordTsar::Undo) ;
    connect(MarkBlockStartAction, &QAction::triggered, this, &cWordTsar::MarkBlockStart) ;
    connect(MarkBlockEndAction, &QAction::triggered, this, &cWordTsar::MarkBlockEnd) ;
    connect(MoveBlock, &QAction::triggered, this, &cWordTsar::MoveBlock) ;
    connect(CopyBlockAction, &QAction::triggered, this, &cWordTsar::CopyBlock) ;
    connect(CopyfromClipboardAction, &QAction::triggered, this, &cWordTsar::CopyFromClipboard) ;
    connect(CopytoClipboardAction, &QAction::triggered, this, &cWordTsar::CopyToClipboard) ;
    connect(DeleteBlockAction, &QAction::triggered, this, &cWordTsar::DeleteBlock) ;
    connect(DeleteWordAction, &QAction::triggered, this, &cWordTsar::DeleteWord) ;
    connect(DeleteLineAction, &QAction::triggered, this, &cWordTsar::DeleteLine) ;
    connect(DeleteLineLeftAction, &QAction::triggered, this, &cWordTsar::DeleteLineLeft) ;
    connect(DeleteLineRightAction, &QAction::triggered, this, &cWordTsar::DeleteLineRight) ;
    connect(DeleteToCharAction, &QAction::triggered, this, &cWordTsar::DeleteToChar) ;
    connect(MarkPreviousBlockAction, &QAction::triggered, this, &cWordTsar::MarkPrevBlock) ;
    connect(FindAction, &QAction::triggered, this, &cWordTsar::Find) ;
    connect(FindandReplaceAction, &QAction::triggered, this, &cWordTsar::FindandReplace) ;
    connect(FindNextAction, &QAction::triggered, this, &cWordTsar::FindNext) ;
    connect(GotoCharAction, &QAction::triggered, this, &cWordTsar::GotoChar) ;
    connect(GotoPageAction, &QAction::triggered, this, &cWordTsar::GotoPage) ;
    connect(GotoMarker1Action, &QAction::triggered, this, &cWordTsar::Goto1) ;
    connect(GotoMarker2Action, &QAction::triggered, this, &cWordTsar::Goto2) ;
    connect(GotoMarker3Action, &QAction::triggered, this, &cWordTsar::Goto3) ;
    connect(GotoMarker4Action, &QAction::triggered, this, &cWordTsar::Goto4) ;
    connect(GotoMarker5Action, &QAction::triggered, this, &cWordTsar::Goto5) ;
    connect(GotoMarker6Action, &QAction::triggered, this, &cWordTsar::Goto6) ;
    connect(GotoMarker7Action, &QAction::triggered, this, &cWordTsar::Goto7) ;
    connect(GotoMarker8Action, &QAction::triggered, this, &cWordTsar::Goto8) ;
    connect(GotoMarker9Action, &QAction::triggered, this, &cWordTsar::Goto9) ;
    connect(GotoMarker0Action, &QAction::triggered, this, &cWordTsar::Goto0) ;
    connect(GotoFontTagAction, &QAction::triggered, this, &cWordTsar::GotoFont) ;
    connect(GotoStyleTagAction, &QAction::triggered, this, &cWordTsar::GotoStyle) ;
    connect(GotoNoteAction, &QAction::triggered, this, &cWordTsar::GotoNote) ;
    connect(GotoPreviousPositionAction, &QAction::triggered, this, &cWordTsar::GotoPrevPos) ;
    connect(GotoLastFindReplaceAction, &QAction::triggered, this, &cWordTsar::GotoLastFindandReplace) ;
    connect(GotoBeginningofBlockAction, &QAction::triggered, this, &cWordTsar::GotoStartBlock) ;
    connect(GotoEndofBlockAction, &QAction::triggered, this, &cWordTsar::GotoEndBlock) ;
    connect(GotoDocumentStartAction, &QAction::triggered, this, &cWordTsar::GotoDocumentStart) ;
    connect(GotoDocumentEndAction, &QAction::triggered, this, &cWordTsar::GotoDocumentEnd) ;
    connect(GotoScrollUpAction, &QAction::triggered, this, &cWordTsar::GotoScrollUp) ;
    connect(GotoScrollDownAction, &QAction::triggered, this, &cWordTsar::GotoScrollDown) ;
    connect(SetMarker1Action, &QAction::triggered, this, &cWordTsar::Set1) ;
    connect(SetMarker2Action, &QAction::triggered, this, &cWordTsar::Set2) ;
    connect(SetMarker3Action, &QAction::triggered, this, &cWordTsar::Set3) ;
    connect(SetMarker4Action, &QAction::triggered, this, &cWordTsar::Set4) ;
    connect(SetMarker5Action, &QAction::triggered, this, &cWordTsar::Set5) ;
    connect(SetMarker6Action, &QAction::triggered, this, &cWordTsar::Set6) ;
    connect(SetMarker7Action, &QAction::triggered, this, &cWordTsar::Set7) ;
    connect(SetMarker8Action, &QAction::triggered, this, &cWordTsar::Set8) ;
    connect(SetMarker9Action, &QAction::triggered, this, &cWordTsar::Set9) ;
    connect(SetMarker0Action, &QAction::triggered, this, &cWordTsar::Set0) ;
    connect(EditNoteAction, &QAction::triggered, this, &cWordTsar::EditNote) ;
    connect(NoteStartNumberAction, &QAction::triggered, this, &cWordTsar::NoteStartNumber) ;
    connect(NoteConvertAction, &QAction::triggered, this, &cWordTsar::NoteCOnvert) ;
    connect(NoteCovertPrintAction, &QAction::triggered, this, &cWordTsar::NoteConcertForPrint) ;
    connect(NotEndnoteLocationAction, &QAction::triggered, this, &cWordTsar::NoteEndNoteLocation) ;
    connect(ColumnBlockModeAction, &QAction::triggered, this, &cWordTsar::ColumnBlockMode) ;
    connect(ColumnReplaceModeAction, &QAction::triggered, this, &cWordTsar::ColumnReplaceMode) ;
    connect(AutoAlineAction, &QAction::triggered, this, &cWordTsar::AutoAlign) ;
    connect(CloseDialogAction, &QAction::triggered, this, &cWordTsar::CloseDialog) ;

    // View menu
    QAction *PreviewAction = new QAction("&Print Preview\t^OP", this) ;
    QAction *CommandTagsAction = new QAction("&Command Tags\t^OD", this) ;
    QAction *BlockHighlightingAction = new QAction("&Block Highlighting\t^KH", this) ;
    QAction *ScreenSettingsAction = new QAction("&Screen Settings...\t^OB", this) ;

    viewmenu->addAction(PreviewAction) ;
    viewmenu->addSeparator() ;
    viewmenu->addAction(CommandTagsAction) ;
    viewmenu->addAction(BlockHighlightingAction) ;
    viewmenu->addSeparator() ;
    viewmenu->addAction(ScreenSettingsAction) ;

    connect(PreviewAction, &QAction::triggered, this, &cWordTsar::PrintPreview) ;
    connect(CommandTagsAction, &QAction::triggered, this, &cWordTsar::CommandTags) ;
    connect(BlockHighlightingAction, &QAction::triggered, this, &cWordTsar::BlockHighlight) ;
    connect(ScreenSettingsAction, &QAction::triggered, this, &cWordTsar::ScreenSettings) ;

    // Insert menu
    QAction *PageBreakAction = new QAction("&Page Break\t.pa", this) ;
    QAction *ColumnBreakAction = new QAction("&Column Break\t^.cb", this) ;
ColumnBreakAction->setEnabled(false) ;
    QAction *DateAction = new QAction("&Today's Date Value\t^J@", this) ;
DateAction->setEnabled(false) ;

    QAction *TimeAction = new QAction("Current &Time\t^J!", this) ;
TimeAction->setEnabled(false) ;
    QAction *ResultAction = new QAction("Last &Math Result\t^J=", this) ;
ResultAction->setEnabled(false) ;
    QAction *ExpressionAction = new QAction("Last Math &Expression\t^J#", this) ;
ExpressionAction->setEnabled(false) ;
    QAction *DollarAction = new QAction("&Last Math as Dollar\t^J$", this) ;
DollarAction->setEnabled(false) ;
    QAction *FilenameAction = new QAction("Current &Filename\t^J*", this) ;
FilenameAction->setEnabled(false) ;
    QAction *DriveAction = new QAction("Current &Drive\t^J:", this) ;
DriveAction->setEnabled(false) ;
    QAction *DirectoryAction = new QAction("Current D&irectory\t^J.", this) ;
DirectoryAction->setEnabled(false) ;
    QAction *PathAction = new QAction("Current P&ath\t^J\\", this) ;
PathAction->setEnabled(false) ;

    QAction *VarDateAction = new QAction("&Date\t&@&", this) ;
VarDateAction->setEnabled(false) ;
    QAction *VarTimeAction = new QAction("&Time\t&!&", this) ;
VarTimeAction->setEnabled(false) ;
    QAction *VarPageAction = new QAction("&Page\t&#&", this) ;
VarPageAction->setEnabled(false) ;
    QAction *VarLineAction = new QAction("&Line\t&_&", this) ;
VarLineAction->setEnabled(false) ;
    QAction *VarFileAction = new QAction("&Filename\t&*&", this) ;
VarFileAction->setEnabled(false) ;
    QAction *VarDriveAction = new QAction("Dri&ve\t&:&", this) ;
VarDriveAction->setEnabled(false) ;
    QAction *VarDirAction = new QAction("D&irectory\t&.&", this) ;
VarDirAction->setEnabled(false) ;
    QAction *VarPathAction = new QAction("P&ath\t&\\&", this) ;
VarPathAction->setEnabled(false) ;

    QAction *ExtendedCharAction = new QAction("&Extended Character...\t^PO", this) ;
ExtendedCharAction->setEnabled(false) ;

    QAction *FileAction = new QAction("&File...\t^KR", this) ;
    QAction *FileAtPrintAction = new QAction("Fi&le at Print Time...\t.fi", this) ;
FileAtPrintAction->setEnabled(false) ;
    QAction *GraphicAction = new QAction("&Graphic...\t^P*", this) ;
GraphicAction->setEnabled(false) ;

    QAction *NoteCommentAction = new QAction("&Comment...\t^ONC", this) ;
NoteCommentAction->setEnabled(false) ;
    QAction *NoteFootnoteAction = new QAction("&Footnote...\t^ONF", this) ;
NoteFootnoteAction->setEnabled(false) ;
    QAction *NoteEndnoteAction = new QAction("&Endnote...\t^ONE", this) ;
NoteEndnoteAction->setEnabled(false) ;
    QAction *NoteAnnotationAction = new QAction("&Annotation\t^ONA", this) ;
NoteAnnotationAction->setEnabled(false) ;

    QAction *TOCEntryAction = new QAction("&TOC Entry...\t.tc", this) ;
TOCEntryAction->setEnabled(false) ;
    QAction *IndexEntryAction = new QAction("&Index Entry...\t^ONI", this) ;
IndexEntryAction->setEnabled(false) ;
    QAction *MarkTextforIndexAction = new QAction("&Mark Text for Index\t^PK", this) ;
MarkTextforIndexAction->setEnabled(false) ;
    QAction *DotLeaderAction = new QAction("&Dot Leader to Tab\t^P.", this) ;
DotLeaderAction->setEnabled(false) ;

    QAction *ParOutlineNumberAction = new QAction("Par. &Outline Number...\t^OZ", this) ;
ParOutlineNumberAction->setEnabled(false) ;

//    QAction *ChangePrinterCodesAction = new QAction("C&hange Printer Codes...", this) ;


    insertmenu->addAction(PageBreakAction) ;
    insertmenu->addAction(ColumnBreakAction) ;
    insertmenu->addSeparator() ;
    insertmenu->addAction(DateAction) ;

    QMenu *OtherValueMenu = insertmenu->addMenu("Other V&alue") ;
    OtherValueMenu->addAction(TimeAction) ;
    OtherValueMenu->addAction(ResultAction) ;
    OtherValueMenu->addAction(ExpressionAction) ;
    OtherValueMenu->addAction(DollarAction) ;
    OtherValueMenu->addAction(FilenameAction) ;
    OtherValueMenu->addAction(DriveAction) ;
    OtherValueMenu->addAction(DirectoryAction) ;
    OtherValueMenu->addAction(PathAction) ;

    QMenu *VariableMenu = insertmenu->addMenu("&Variable") ;
    VariableMenu->addAction(VarDateAction) ;
    VariableMenu->addAction(VarTimeAction) ;
    VariableMenu->addAction(VarPageAction) ;
    VariableMenu->addAction(VarLineAction) ;
    VariableMenu->addAction(VarFileAction) ;
    VariableMenu->addAction(VarDriveAction) ;
    VariableMenu->addAction(VarDirAction) ;
    VariableMenu->addAction(VarPathAction) ;

    insertmenu->addAction(ExtendedCharAction) ;
    insertmenu->addSeparator() ;
    insertmenu->addAction(FileAction) ;
    insertmenu->addAction(FileAtPrintAction) ;
    insertmenu->addAction(GraphicAction) ;

    QMenu *NoteMenu = insertmenu->addMenu("&Note") ;
    NoteMenu->addAction(NoteCommentAction) ;
    NoteMenu->addAction(NoteFootnoteAction) ;
    NoteMenu->addAction(NoteEndnoteAction) ;
    NoteMenu->addAction(NoteAnnotationAction) ;

    insertmenu->addSeparator() ;

    QMenu *IndexMenu = insertmenu->addMenu("&Index/TOC Entry") ;
    IndexMenu->addAction(TOCEntryAction) ;
    IndexMenu->addAction(IndexEntryAction) ;
    IndexMenu->addAction(MarkTextforIndexAction) ;
    IndexMenu->addAction(DotLeaderAction) ;

    insertmenu->addAction(ParOutlineNumberAction) ;

    connect(PageBreakAction, &QAction::triggered, this, &cWordTsar::PageBreak) ;
    connect(ColumnBreakAction, &QAction::triggered, this, &cWordTsar::ColumnBreak) ;
    connect(DateAction, &QAction::triggered, this, &cWordTsar::InsertDate) ;
    connect(TimeAction, &QAction::triggered, this, &cWordTsar::InsertTime) ;
    connect(ResultAction, &QAction::triggered, this, &cWordTsar::MathResult) ;
    connect(ExpressionAction, &QAction::triggered, this, &cWordTsar::MathExpression) ;
    connect(DollarAction, &QAction::triggered, this, &cWordTsar::MathDollar) ;
    connect(FilenameAction, &QAction::triggered, this, &cWordTsar::Filename) ;
    connect(DriveAction, &QAction::triggered, this, &cWordTsar::Drive) ;
    connect(DirectoryAction, &QAction::triggered, this, &cWordTsar::Directory) ;
    connect(PathAction, &QAction::triggered, this, &cWordTsar::Path) ;
    connect(VarDateAction, &QAction::triggered, this, &cWordTsar::VarDate) ;
    connect(VarTimeAction, &QAction::triggered, this, &cWordTsar::VarTime) ;
    connect(VarPageAction, &QAction::triggered, this, &cWordTsar::VarPage) ;
    connect(VarLineAction, &QAction::triggered, this, &cWordTsar::VarLine) ;
    connect(VarFileAction, &QAction::triggered, this, &cWordTsar::VarFilename) ;
    connect(VarDriveAction, &QAction::triggered, this, &cWordTsar::VarDrive) ;
    connect(VarDirAction, &QAction::triggered, this, &cWordTsar::VarDirectory) ;
    connect(VarPathAction, &QAction::triggered, this, &cWordTsar::VarPath) ;
    connect(ExtendedCharAction, &QAction::triggered, this, &cWordTsar::ExtendedChar) ;
    connect(FileAction, &QAction::triggered, this, &cWordTsar::Open) ;
    connect(FileAtPrintAction, &QAction::triggered, this, &cWordTsar::FileAtPrint) ;
    connect(GraphicAction, &QAction::triggered, this, &cWordTsar::Graphic) ;
    connect(NoteCommentAction, &QAction::triggered, this, &cWordTsar::NoteComment) ;
    connect(NoteFootnoteAction, &QAction::triggered, this, &cWordTsar::NoteFootnote) ;
    connect(NoteEndnoteAction, &QAction::triggered, this, &cWordTsar::NoteEndnote) ;
    connect(NoteAnnotationAction, &QAction::triggered, this, &cWordTsar::NoteAnnotation) ;
    connect(TOCEntryAction, &QAction::triggered, this, &cWordTsar::TOCEntry) ;
    connect(IndexEntryAction, &QAction::triggered, this, &cWordTsar::IndexEntry) ;
    connect(MarkTextforIndexAction, &QAction::triggered, this, &cWordTsar::MarkIndex) ;
    connect(DotLeaderAction, &QAction::triggered, this, &cWordTsar::DotLeader) ;
    connect(ParOutlineNumberAction, &QAction::triggered, this, &cWordTsar::ParOutlineNumber) ;

    // Style menu
    QAction *BoldAction = new QAction("&Bold\t^PB", this) ;
    QAction *ItalicAction = new QAction("&Italic\t^PY", this) ;
    QAction *UnderlineAction = new QAction("&Underline\t^PS", this) ;
    QAction *FontAction = new QAction("&Font\t^P=", this) ;

    QAction *StrikeoutAction = new QAction("&Strikeout\t^PX", this) ;
    QAction *SubscriptAction = new QAction("Su&bscript\t^PV", this) ;
    QAction *SuperscriptAction = new QAction("Su&perscript\t^PT", this) ;
    QAction *DoubleStrikeAction = new QAction("&Doublestrike\t^PD", this) ;
    QAction *ColorAction = new QAction("&Color...\t^P-", this) ;

    QAction *SelectParStyleAction = new QAction("&Select Paragraph Style...\t^OFS", this) ;
SelectParStyleAction->setEnabled(false) ;
    QAction *ReturntoPrevStyleAction = new QAction("&Return to Previous Style\t^OFP", this) ;
ReturntoPrevStyleAction->setEnabled(false) ;
    QAction *DefineParStyleAction = new QAction("&Defne Paragraph Styule\t^OFD", this) ;
DefineParStyleAction->setEnabled(false) ;

    QAction *CopyStyletoLibraryAction = new QAction("&Copy Style to Library\t^OFO", this) ;
CopyStyletoLibraryAction->setEnabled(false) ;
    QAction *DelLibraryStyleAction = new QAction("&Delete Library Style\t^OFY", this) ;
DelLibraryStyleAction->setEnabled(false) ;
    QAction *RenameLibraryStyleAction = new QAction("&Rename Library Style\t^OFR", this) ;
RenameLibraryStyleAction->setEnabled(false) ;
    QAction *RenameDocStyleAction = new QAction("R&ename Document Style\t^OFE", this) ;
RenameDocStyleAction->setEnabled(false) ;

    QAction *UppercaseAction = new QAction("&Uppercase\t^K\"", this) ;
    QAction *LowercaseAction = new QAction("&Lowercase\t^K\'", this) ;
    QAction *SentenceCaseAction = new QAction("&Sentence Case\t^K.", this) ;
SentenceCaseAction->setEnabled(false) ;

    QAction *SettingsAction = new QAction("S&ettings", this) ;
SettingsAction->setEnabled(false) ;

    stylemenu->addAction(BoldAction) ;
    stylemenu->addAction(ItalicAction) ;
    stylemenu->addAction(UnderlineAction) ;
    stylemenu->addAction(FontAction) ;

    QMenu *OtherMenu = stylemenu->addMenu("&Other") ;
    OtherMenu->addAction(StrikeoutAction) ;
    OtherMenu->addAction(SubscriptAction) ;
    OtherMenu->addAction(SuperscriptAction) ;
    OtherMenu->addAction(DoubleStrikeAction) ;
    OtherMenu->addAction(ColorAction) ;

    stylemenu->addAction(SelectParStyleAction) ;
    stylemenu->addAction(ReturntoPrevStyleAction) ;
    stylemenu->addAction(DefineParStyleAction) ;

    QMenu *ManageMenu = stylemenu->addMenu("&Manage Paragraph Styles") ;
    ManageMenu->addAction(CopyStyletoLibraryAction) ;
    ManageMenu->addAction(DelLibraryStyleAction) ;
    ManageMenu->addAction(RenameLibraryStyleAction) ;
    ManageMenu->addAction(RenameDocStyleAction) ;

    QMenu *ConvertMenu = stylemenu->addMenu("&Convert Case") ;
    ConvertMenu->addAction(UppercaseAction) ;
    ConvertMenu->addAction(LowercaseAction) ;
    ConvertMenu->addAction(SentenceCaseAction) ;

    stylemenu->addAction(SettingsAction) ;

    connect(BoldAction, &QAction::triggered, this, &cWordTsar::Bold) ;
    connect(ItalicAction, &QAction::triggered, this, &cWordTsar::Italics) ;
    connect(UnderlineAction, &QAction::triggered, this, &cWordTsar::Underline) ;
    connect(FontAction, &QAction::triggered, this, &cWordTsar::font) ;
    connect(StrikeoutAction, &QAction::triggered, this, &cWordTsar::Strikeout) ;
    connect(SubscriptAction, &QAction::triggered, this, &cWordTsar::Subscript) ;
    connect(SuperscriptAction, &QAction::triggered, this, &cWordTsar::Superscript) ;
    connect(DoubleStrikeAction, &QAction::triggered, this, &cWordTsar::DoubleStrike) ;
    connect(ColorAction, &QAction::triggered, this, &cWordTsar::Color) ;
    connect(SelectParStyleAction, &QAction::triggered, this, &cWordTsar::SelectParStyle) ;
    connect(ReturntoPrevStyleAction, &QAction::triggered, this, &cWordTsar::ReturntoPrevStyle) ;
    connect(DefineParStyleAction, &QAction::triggered, this, &cWordTsar::DefineParStyle) ;
    connect(CopyStyletoLibraryAction, &QAction::triggered, this, &cWordTsar::CopyStyletoLibrary) ;
    connect(DelLibraryStyleAction, &QAction::triggered, this, &cWordTsar::DeleteLibraryStyle) ;
    connect(RenameLibraryStyleAction, &QAction::triggered, this, &cWordTsar::RenameLibraryStyle) ;
    connect(RenameDocStyleAction, &QAction::triggered, this, &cWordTsar::RenameDocStyle) ;
    connect(UppercaseAction, &QAction::triggered, this, &cWordTsar::Uppercase) ;
    connect(LowercaseAction, &QAction::triggered, this, &cWordTsar::Lowercase) ;
    connect(SentenceCaseAction, &QAction::triggered, this, &cWordTsar::Sentencecase) ;
    connect(SettingsAction, &QAction::triggered, this, &cWordTsar::Settings) ;

    // Layout Menu
    QAction *CenterLineAction = new QAction("&Center Line\t^OC", this) ;
    QAction *RightAlignAction = new QAction("R&ight Align Line\t^OJ", this) ;
RightAlignAction->setEnabled(false) ;
    QAction *RulerLineAction = new QAction("&Ruler Line...\t^OL", this) ;
RulerLineAction->setEnabled(false) ;
    QAction *ColumnsAction = new QAction("C&olumns...\t^OU", this) ;
ColumnsAction->setEnabled(false) ;
    QAction *PageAction = new QAction("&Page...\t^OY", this) ;
PageAction->setEnabled(false) ;
    QAction *HeaderAction = new QAction("&Header...\t.he", this) ;
    QAction *FooterAction = new QAction("&Footer...\t.fo", this) ;
    QAction *PageNumberingAction = new QAction("Page &Numbering...\t^O#", this) ;
PageNumberingAction->setEnabled(false) ;
    QAction *LineNumberingAction = new QAction("Line Numbering...\t.l#", this) ;
LineNumberingAction->setEnabled(false) ;
    QAction *AlignmentSpacingAction = new QAction("&Alignment and Spacing\t^OS", this) ;
AlignmentSpacingAction->setEnabled(false) ;
    QAction *OverPrintCharAction = new QAction("Overprint &Character\t^PH", this) ;
OverPrintCharAction->setEnabled(false) ;
    QAction *OverprintLineAction = new QAction("Overprint &Line\t^P↵", this) ;
OverprintLineAction->setEnabled(false) ;
    QAction *OptionalHyphenAction = new QAction("Option &Hyphen\t^OE", this) ;
OptionalHyphenAction->setEnabled(false) ;
    QAction *VerticalCenterAction = new QAction("&Vertically Center Text on Page\t^OV", this) ;
VerticalCenterAction->setEnabled(false) ;
    QAction *KeepWordsTogetherAction = new QAction("&Keep Word Together on Line\t^PO", this) ;
KeepWordsTogetherAction->setEnabled(false) ;
    QAction *KeepLinesTogetherPageAction = new QAction("Keep Lines Together on &Page...\t.cp", this) ;
KeepLinesTogetherPageAction->setEnabled(false) ;
    QAction *KeepLinesTogetherColumnAction = new QAction("Keep Lines Together on C&olumn\t.cc", this) ;
KeepLinesTogetherColumnAction->setEnabled(false) ;

    layoutmenu->addAction(CenterLineAction) ;
    layoutmenu->addAction(RightAlignAction) ;
    layoutmenu->addSeparator() ;
    layoutmenu->addAction(RulerLineAction) ;
    layoutmenu->addAction(ColumnsAction) ;
    layoutmenu->addAction(PageAction) ;

    QMenu *hfMenu = layoutmenu->addMenu("&Headers/Footers") ;
    hfMenu->addAction(HeaderAction) ;
    hfMenu->addAction(FooterAction) ;

    layoutmenu->addAction(PageNumberingAction) ;
    layoutmenu->addAction(LineNumberingAction) ;
    layoutmenu->addAction(AlignmentSpacingAction) ;

    QMenu *SpecialMenu = layoutmenu->addMenu("Special &Effects") ;
    SpecialMenu->addAction(OverPrintCharAction) ;
    SpecialMenu->addAction(OverprintLineAction) ;
    SpecialMenu->addAction(OptionalHyphenAction) ;
    SpecialMenu->addAction(VerticalCenterAction) ;
    SpecialMenu->addAction(KeepWordsTogetherAction) ;
    SpecialMenu->addAction(KeepLinesTogetherPageAction) ;
    SpecialMenu->addAction(KeepLinesTogetherColumnAction) ;

    connect(CenterLineAction, &QAction::triggered, this, &cWordTsar::CenterLine) ;
    connect(RightAlignAction, &QAction::triggered, this, &cWordTsar::RightAlign) ;
    connect(RulerLineAction, &QAction::triggered, this, &cWordTsar::RulerLine) ;
    connect(ColumnsAction, &QAction::triggered, this, &cWordTsar::Columns) ;
    connect(PageAction, &QAction::triggered, this, &cWordTsar::Page) ;
    connect(HeaderAction, &QAction::triggered, this, &cWordTsar::Header) ;
    connect(FooterAction, &QAction::triggered, this, &cWordTsar::Footer) ;
    connect(PageNumberingAction, &QAction::triggered, this, &cWordTsar::PageNumbering) ;
    connect(LineNumberingAction, &QAction::triggered, this, &cWordTsar::LineNumbering) ;
    connect(AlignmentSpacingAction, &QAction::triggered, this, &cWordTsar::Alignment) ;
    connect(OverPrintCharAction, &QAction::triggered, this, &cWordTsar::OverprintChar) ;
    connect(OverprintLineAction, &QAction::triggered, this, &cWordTsar::OverprintLine) ;
    connect(OptionalHyphenAction, &QAction::triggered, this, &cWordTsar::OptionalHyphen) ;
    connect(VerticalCenterAction, &QAction::triggered, this, &cWordTsar::VerticalCenter) ;
    connect(KeepWordsTogetherAction, &QAction::triggered, this, &cWordTsar::KeepWordsTogether) ;
    connect(KeepLinesTogetherPageAction, &QAction::triggered, this, &cWordTsar::KeepLinesTogetherPage) ;
    connect(KeepLinesTogetherColumnAction, &QAction::triggered, this, &cWordTsar::KeepLinesTogetherColumn) ;


    // Utilities Menu
    QAction *SpellCheckGlobalAction = new QAction("&Spell Check Global\t^QR^QL", this) ;
SpellCheckGlobalAction->setEnabled(false) ;
    QAction *SpellCheckRestAction = new QAction("&Rest of Document\t^QL", this) ;
SpellCheckRestAction->setEnabled(false) ;
    QAction *SpellCheckWordAction = new QAction("&Word\t^QN", this) ;
SpellCheckWordAction->setEnabled(false) ;
    QAction *SpellCheckTypeAction = new QAction("&Type Word...\t^QO", this) ;
SpellCheckTypeAction->setEnabled(false) ;
    QAction *SpellCheckNotesAction = new QAction("Rest of &Notes\t^ONL", this) ;
SpellCheckNotesAction->setEnabled(false) ;
    QAction *ThesaurusAction = new QAction("&Thesaurus\t^QJ", this) ;
ThesaurusAction->setEnabled(false) ;
    QAction *LanguageChangeAction = new QAction("Language Change...\t.la", this) ;
LanguageChangeAction->setEnabled(false) ;
    QAction *InsetAction = new QAction("&Inset\t^P&&", this) ;
InsetAction->setEnabled(false) ;
    QAction *CalculatorAction = new QAction("&Calculator\t^QM", this) ;
CalculatorAction->setEnabled(false) ;
    QAction *BlockMathAction = new QAction("&Block Math\t^KM", this) ;
BlockMathAction->setEnabled(false) ;
    QAction *SortBlockAscAction = new QAction("&Ascending\t^KZA", this) ;
SortBlockAscAction->setEnabled(false) ;
    QAction *SortBlockDesAction = new QAction("&Descending\t^KZD", this) ;
SortBlockDesAction->setEnabled(false) ;
    QAction *WordCountAction = new QAction("&Word Count\t^K?", this) ;
    QAction *PlayMacroAction = new QAction("&Play...\t^JP", this) ;
PlayMacroAction->setEnabled(false) ;
    QAction *RecordMacroAction = new QAction("&Record...\t^JR", this) ;
RecordMacroAction->setEnabled(false) ;
    QAction *EditMacroAction = new QAction("&Edit/Create...\t^JD", this) ;
EditMacroAction->setEnabled(false) ;
    QAction *SingleStepAction = new QAction("&Single Step...\t^JS", this) ;
SingleStepAction->setEnabled(false) ;
    QAction *CopyMacroAction = new QAction("&Copy...\t^JO", this) ;
CopyMacroAction->setEnabled(false) ;
    QAction *DeleteMacroAction = new QAction("&Delete...\t^JY", this) ;
DeleteMacroAction->setEnabled(false) ;
    QAction *RenameMacroAction = new QAction("Re&name...\t^JE", this) ;
RenameMacroAction->setEnabled(false) ;
    QAction *DataFileAction = new QAction("&Data File...\t.df", this) ;
DataFileAction->setEnabled(false) ;
    QAction *NameVarsAction = new QAction("&Name Variables...\t.rv", this) ;
NameVarsAction->setEnabled(false) ;
    QAction *SetVarAction = new QAction("&Set Variable...\t.sv", this) ;
SetVarAction->setEnabled(false) ;
    QAction *SetVarMathAction = new QAction("Set &Variable to Math Result...\t.ma", this) ;
SetVarMathAction->setEnabled(false) ;
    QAction *AskVarAction = new QAction("&Ask for Variable...\t.av", this) ;
AskVarAction->setEnabled(false) ;
    QAction *IfAction = new QAction("&If...\t.if", this) ;
IfAction->setEnabled(false) ;
    QAction *ElseAction = new QAction("E&lse\t.el", this) ;
ElseAction->setEnabled(false) ;
    QAction *EndIfAction = new QAction("&End If\t.ei", this) ;
EndIfAction->setEnabled(false) ;
    QAction *TopAction = new QAction("Go to &Top of Document\t.go t", this) ;
TopAction->setEnabled(false) ;
    QAction *BottomAction = new QAction("Go to &Bottom of Document\t.go b", this) ;
BottomAction->setEnabled(false) ;
    QAction *ClearAction = new QAction("&Clear Screen While Printing...\t.cs", this) ;
ClearAction->setEnabled(false) ;
    QAction *DisplayAction = new QAction("Display &Message...\t.dm", this) ;
DisplayAction->setEnabled(false) ;
    QAction *PrintNTimesAction = new QAction("P&rint File n Times...\t.rp", this) ;
PrintNTimesAction->setEnabled(false) ;
    QAction *ReformatRestAction = new QAction("&Rest of Document\t^QU", this) ;
    QAction *ReformatParaAction = new QAction("&Paragraph\t^B", this) ;
ReformatParaAction->setEnabled(false) ;
    QAction *ReformatNotesAction = new QAction("Resst of &Notes\t^ONU", this) ;
ReformatNotesAction->setEnabled(false) ;
    QAction *RepeatKeyAction = new QAction("R&epeat Next Keystroke\t^QQ", this) ;
RepeatKeyAction->setEnabled(false) ;

    utilitiesmenu->addAction(SpellCheckGlobalAction) ;

    QMenu *SpellMenu = utilitiesmenu->addMenu("Spell Check &Other") ;
    SpellMenu->addAction(SpellCheckRestAction) ;
    SpellMenu->addAction(SpellCheckWordAction) ;
    SpellMenu->addAction(SpellCheckTypeAction) ;
    SpellMenu->addAction(SpellCheckNotesAction) ;

    utilitiesmenu->addAction(ThesaurusAction) ;
    utilitiesmenu->addAction(LanguageChangeAction) ;
    utilitiesmenu->addSeparator() ;

    utilitiesmenu->addAction(InsetAction) ;
    utilitiesmenu->addAction(CalculatorAction) ;
    utilitiesmenu->addAction(BlockMathAction) ;

    QMenu *SortMenu = utilitiesmenu->addMenu("Sort Bloc&k") ;
    SortMenu->addAction(SortBlockAscAction) ;
    SortMenu->addAction(SortBlockDesAction) ;

    utilitiesmenu->addAction(WordCountAction) ;
    utilitiesmenu->addSeparator() ;

    QMenu *MacrosMenu = utilitiesmenu->addMenu("&Macros") ;
    MacrosMenu->addAction(PlayMacroAction) ;
    MacrosMenu->addAction(RecordMacroAction) ;
    MacrosMenu->addAction(EditMacroAction) ;
    MacrosMenu->addAction(SingleStepAction) ;
    MacrosMenu->addAction(CopyMacroAction) ;
    MacrosMenu->addAction(DeleteMacroAction) ;
    MacrosMenu->addAction(RenameMacroAction) ;

    QMenu *MergeMenu = utilitiesmenu->addMenu("Merge &Print Commands") ;
    MergeMenu->addAction(DataFileAction) ;
    MergeMenu->addAction(NameVarsAction) ;
    MergeMenu->addAction(SetVarAction) ;
    MergeMenu->addAction(SetVarMathAction) ;
    MergeMenu->addAction(AskVarAction) ;
    MergeMenu->addSeparator() ;
    MergeMenu->addAction(IfAction) ;
    MergeMenu->addAction(ElseAction) ;
    MergeMenu->addAction(EndIfAction) ;
    MergeMenu->addAction(TopAction) ;
    MergeMenu->addAction(BottomAction) ;
    MergeMenu->addSeparator() ;
    MergeMenu->addAction(ClearAction) ;
    MergeMenu->addAction(DisplayAction) ;
    MergeMenu->addAction(PrintNTimesAction) ;

    QMenu *ReformatMenu = utilitiesmenu->addMenu("&Reformat") ;
    ReformatMenu->addAction(ReformatRestAction) ;
    ReformatMenu->addAction(ReformatParaAction) ;
    ReformatMenu->addAction(ReformatNotesAction) ;

    utilitiesmenu->addAction(RepeatKeyAction) ;

    connect(SpellCheckGlobalAction, &QAction::triggered, this, &cWordTsar::SpellCheckGlobal) ;
    connect(SpellCheckRestAction, &QAction::triggered, this, &cWordTsar::SpellCheckRest) ;
    connect(SpellCheckWordAction, &QAction::triggered, this, &cWordTsar::SpellCheckWord) ;
    connect(SpellCheckTypeAction, &QAction::triggered, this, &cWordTsar::SpellCheckType) ;
    connect(SpellCheckNotesAction, &QAction::triggered, this, &cWordTsar::SpellCheckNotes) ;
    connect(ThesaurusAction, &QAction::triggered, this, &cWordTsar::Thesaurus) ;
    connect(LanguageChangeAction, &QAction::triggered, this, &cWordTsar::LanguageChange) ;
    connect(InsetAction, &QAction::triggered, this, &cWordTsar::Inset) ;
    connect(CalculatorAction, &QAction::triggered, this, &cWordTsar::Calculator) ;
    connect(BlockMathAction, &QAction::triggered, this, &cWordTsar::BlockMath) ;
    connect(SortBlockAscAction, &QAction::triggered, this, &cWordTsar::SortBlockAsc) ;
    connect(SortBlockDesAction, &QAction::triggered, this, &cWordTsar::SortBlockDes) ;
    connect(WordCountAction, &QAction::triggered, this, &cWordTsar::WordCount) ;
    connect(PlayMacroAction, &QAction::triggered, this, &cWordTsar::PlayMacro) ;
    connect(RecordMacroAction, &QAction::triggered, this, &cWordTsar::RecordMacro) ;
    connect(EditMacroAction, &QAction::triggered, this, &cWordTsar::EditMacro) ;
    connect(SingleStepAction, &QAction::triggered, this, &cWordTsar::SingleStep) ;
    connect(CopyMacroAction, &QAction::triggered, this, &cWordTsar::CopyMacro) ;
    connect(DeleteMacroAction, &QAction::triggered, this, &cWordTsar::DeleteMacro) ;
    connect(RenameMacroAction, &QAction::triggered, this, &cWordTsar::RenameMacro) ;
    connect(DataFileAction, &QAction::triggered, this, &cWordTsar::DataFile) ;
    connect(NameVarsAction, &QAction::triggered, this, &cWordTsar::NameVars) ;
    connect(SetVarAction, &QAction::triggered, this, &cWordTsar::SetVar) ;
    connect(SetVarMathAction, &QAction::triggered, this, &cWordTsar::SetVarMath) ;
    connect(AskVarAction, &QAction::triggered, this, &cWordTsar::AskVar) ;
    connect(IfAction, &QAction::triggered, this, &cWordTsar::If) ;
    connect(ElseAction, &QAction::triggered, this, &cWordTsar::Else) ;
    connect(EndIfAction, &QAction::triggered, this, &cWordTsar::EndIf) ;
    connect(TopAction, &QAction::triggered, this, &cWordTsar::Top) ;
    connect(BottomAction, &QAction::triggered, this, &cWordTsar::Bottom) ;
    connect(ClearAction, &QAction::triggered, this, &cWordTsar::Clear) ;
    connect(DisplayAction, &QAction::triggered, this, &cWordTsar::Display) ;
    connect(PrintNTimesAction, &QAction::triggered, this, &cWordTsar::PrintNTimes) ;
    connect(ReformatRestAction, &QAction::triggered, this, &cWordTsar::ReformatRest) ;
    connect(ReformatParaAction, &QAction::triggered, this, &cWordTsar::ReformatPara) ;
    connect(ReformatNotesAction, &QAction::triggered, this, &cWordTsar::ReformatNotes) ;
    connect(RepeatKeyAction, &QAction::triggered, this, &cWordTsar::RepeatKey) ;


//    menuBar->hide() ;
}

// we handle menus by generating keys as if from the user
void cWordTsar::Open(void)
{
    mEditor->mInput->HandleKey(CTRL_K) ;
    mEditor->mInput->HandleKey('r') ;
}



void cWordTsar::Save(void)
{
    mEditor->mInput->HandleKey(CTRL_K) ;
    mEditor->mInput->HandleKey('s') ;
}



void cWordTsar::SaveAs(void)
{
    mEditor->mInput->HandleKey(CTRL_K) ;
    mEditor->mInput->HandleKey('t') ;
}



void cWordTsar::SaveandClose(void)
{
    mEditor->mInput->HandleKey(CTRL_K) ;
    mEditor->mInput->HandleKey('d') ;
}



void cWordTsar::Print(void)
{
    Save() ;
    mEditor->mInput->HandleKey(CTRL_O) ;
    mEditor->mInput->HandleKey('p') ;
}



void cWordTsar::PrintPreview(void)
{
    Save() ;
    mEditor->mInput->HandleKey(CTRL_O) ;
    mEditor->mInput->HandleKey('p') ;
}


void cWordTsar::ExitWordTsar(void)
{
    mEditor->mInput->HandleKey(CTRL_K) ;
    mEditor->mInput->HandleKey('x') ;
}


void cWordTsar::Undo(void)
{
    mEditor->mInput->HandleKey(CTRL_U) ;
}


void cWordTsar::MarkBlockStart(void)
{
    mEditor->mInput->HandleKey(CTRL_K) ;
    mEditor->mInput->HandleKey('b') ;
}


void cWordTsar::MarkBlockEnd(void)
{
    mEditor->mInput->HandleKey(CTRL_K) ;
    mEditor->mInput->HandleKey('k') ;
}


void cWordTsar::MoveBlock(void)
{
    mEditor->mInput->HandleKey(CTRL_K) ;
    mEditor->mInput->HandleKey('v') ;
}


void cWordTsar::CopyBlock(void)
{
    mEditor->mInput->HandleKey(CTRL_K) ;
    mEditor->mInput->HandleKey('c') ;
}


void cWordTsar::CopyFromClipboard(void)
{
    mEditor->mInput->HandleKey(CTRL_K) ;
    mEditor->mInput->HandleKey('[') ;
}


void cWordTsar::CopyToClipboard(void)
{
    mEditor->mInput->HandleKey(CTRL_K) ;
    mEditor->mInput->HandleKey(']') ;
}


void cWordTsar::DeleteBlock(void)
{
    mEditor->mInput->HandleKey(CTRL_K) ;
    mEditor->mInput->HandleKey('y') ;
}


void cWordTsar::DeleteWord(void)
{
    mEditor->mInput->HandleKey(CTRL_T) ;
}


void cWordTsar::DeleteLine(void)
{
    mEditor->mInput->HandleKey(CTRL_Y) ;
}


void cWordTsar::DeleteLineLeft(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey(' ') ;               // @todo send delete key
}


void cWordTsar::DeleteLineRight(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('Y') ;
}


void cWordTsar::DeleteToChar(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('t') ;
}


void cWordTsar::MarkPrevBlock(void)
{
    mEditor->mInput->HandleKey(CTRL_K) ;
    mEditor->mInput->HandleKey('u') ;
}


void cWordTsar::Find(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('f') ;
}


void cWordTsar::FindandReplace(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('a') ;
}


void cWordTsar::FindNext(void)
{
    mEditor->mInput->HandleKey(CTRL_L) ;
}


void cWordTsar::GotoChar(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('G') ;
}


void cWordTsar::GotoPage(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('I') ;
}


void cWordTsar::Goto1(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('1') ;
}


void cWordTsar::Goto2(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('2') ;
}


void cWordTsar::Goto3(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('3') ;
}


void cWordTsar::Goto4(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('4') ;
}


void cWordTsar::Goto5(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('5') ;
}


void cWordTsar::Goto6(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('6') ;
}


void cWordTsar::Goto7(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('7') ;
}


void cWordTsar::Goto8(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('8') ;
}


void cWordTsar::Goto9(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('9') ;
}


void cWordTsar::Goto0(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('0') ;
}


void cWordTsar::GotoFont(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('=') ;
}


void cWordTsar::GotoStyle(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('<') ;
}


void cWordTsar::GotoNote(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('N') ;
    mEditor->mInput->HandleKey('G') ;
}


void cWordTsar::GotoPrevPos(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('P') ;
}


void cWordTsar::GotoLastFindandReplace(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('V') ;
}


void cWordTsar::GotoStartBlock(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('B') ;
}


void cWordTsar::GotoEndBlock(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('k') ;
}


void cWordTsar::GotoDocumentStart(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('r') ;
}


void cWordTsar::GotoDocumentEnd(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('c') ;
}


void cWordTsar::GotoScrollUp(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('w') ;
}


void cWordTsar::GotoScrollDown(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('z') ;
}


void cWordTsar::Set1(void)
{
    mEditor->mInput->HandleKey(CTRL_K) ;
    mEditor->mInput->HandleKey('1') ;
}


void cWordTsar::Set2(void)
{
    mEditor->mInput->HandleKey(CTRL_K) ;
    mEditor->mInput->HandleKey('2') ;
}


void cWordTsar::Set3(void)
{
    mEditor->mInput->HandleKey(CTRL_K) ;
    mEditor->mInput->HandleKey('3') ;
}


void cWordTsar::Set4(void)
{
    mEditor->mInput->HandleKey(CTRL_K) ;
    mEditor->mInput->HandleKey('4') ;
}



void cWordTsar::Set5(void)
{
    mEditor->mInput->HandleKey(CTRL_K) ;
    mEditor->mInput->HandleKey('5') ;
}


void cWordTsar::Set6(void)
{
    mEditor->mInput->HandleKey(CTRL_K) ;
    mEditor->mInput->HandleKey('6') ;
}


void cWordTsar::Set7(void)
{
    mEditor->mInput->HandleKey(CTRL_K) ;
    mEditor->mInput->HandleKey('7') ;
}


void cWordTsar::Set8(void)
{
    mEditor->mInput->HandleKey(CTRL_K) ;
    mEditor->mInput->HandleKey('8') ;
}


void cWordTsar::Set9(void)
{
    mEditor->mInput->HandleKey(CTRL_K) ;
    mEditor->mInput->HandleKey('9') ;
}



void cWordTsar::Set0(void)
{
    mEditor->mInput->HandleKey(CTRL_K) ;
    mEditor->mInput->HandleKey('0') ;
}


void cWordTsar::EditNote(void)
{
    mEditor->mInput->HandleKey(CTRL_O) ;
    mEditor->mInput->HandleKey('n') ;
    mEditor->mInput->HandleKey('d') ;
}



void cWordTsar::NoteStartNumber(void)
{
}


void cWordTsar::NoteCOnvert(void)
{
    mEditor->mInput->HandleKey(CTRL_O) ;
    mEditor->mInput->HandleKey('n') ;
    mEditor->mInput->HandleKey('v') ;
}


void cWordTsar::NoteConcertForPrint(void)
{
}


void cWordTsar::NoteEndNoteLocation(void)
{
    mEditor->MoveCursorStartLine() ;

    mEditor->mDocument.MaybeInsertHardReturn() ;
    mEditor->mDocument.Insert(".pe\n") ;
}


void cWordTsar::ColumnBlockMode(void)
{
    mEditor->mInput->HandleKey(CTRL_K) ;
    mEditor->mInput->HandleKey('n') ;
}


void cWordTsar::ColumnReplaceMode(void)
{
    mEditor->mInput->HandleKey(CTRL_K) ;
    mEditor->mInput->HandleKey('i') ;
}


void cWordTsar::AutoAlign(void)
{
    mEditor->mInput->HandleKey(CTRL_O) ;
    mEditor->mInput->HandleKey('a') ;
}


void cWordTsar::CloseDialog(void)
{
    mEditor->mInput->HandleKey(CTRL_O) ;
    mEditor->mInput->HandleKey(HARD_RETURN) ;
}


void cWordTsar::CommandTags(void)
{
    mEditor->mInput->HandleKey(CTRL_O) ;
    mEditor->mInput->HandleKey('D') ;
}


void cWordTsar::BlockHighlight(void)
{
    mEditor->mInput->HandleKey(CTRL_K) ;
    mEditor->mInput->HandleKey('H') ;
}


void cWordTsar::ScreenSettings(void)
{
    mEditor->Preferences(PT_SCREEN) ;
}


// InsertMenu
void cWordTsar::PageBreak(void)
{
    mEditor->MoveCursorStartLine() ;

    mEditor->mDocument.MaybeInsertHardReturn() ;
    mEditor->mDocument.Insert(".pa\n") ;
};


void cWordTsar::ColumnBreak(void)
{
    mEditor->MoveCursorStartLine() ;

    mEditor->mDocument.MaybeInsertHardReturn() ;
    mEditor->mDocument.Insert(".cb\n") ;
}


void cWordTsar::InsertDate(void)
{
    mEditor->mInput->HandleKey(CTRL_J) ;
    mEditor->mInput->HandleKey('@') ;
}


void cWordTsar::InsertTime(void)
{
    mEditor->mInput->HandleKey(CTRL_J) ;
    mEditor->mInput->HandleKey('!') ;
}


void cWordTsar::MathResult(void)
{
    mEditor->mInput->HandleKey(CTRL_J) ;
    mEditor->mInput->HandleKey('=') ;
}


void cWordTsar::MathExpression(void)
{
    mEditor->mInput->HandleKey(CTRL_J) ;
    mEditor->mInput->HandleKey('#') ;
}


void cWordTsar::MathDollar(void)
{
    mEditor->mInput->HandleKey(CTRL_J) ;
    mEditor->mInput->HandleKey('$') ;
}


void cWordTsar::Filename(void)
{
    mEditor->mInput->HandleKey(CTRL_J) ;
    mEditor->mInput->HandleKey('*') ;
}


void cWordTsar::Drive(void)
{
    mEditor->mInput->HandleKey(CTRL_J) ;
    mEditor->mInput->HandleKey(':') ;
}


void cWordTsar::Directory(void)
{
    mEditor->mInput->HandleKey(CTRL_J) ;
    mEditor->mInput->HandleKey('.') ;
}


void cWordTsar::Path(void)
{
    mEditor->mInput->HandleKey(CTRL_J) ;
    mEditor->mInput->HandleKey('\\') ;
}


void cWordTsar::VarDate(void)
{
    mEditor->mDocument.Insert("&@&") ;
}


void cWordTsar::VarTime(void)
{
    mEditor->mDocument.Insert("&!&") ;
}


void cWordTsar::VarPage(void)
{
    mEditor->mDocument.Insert("&#&") ;
}


void cWordTsar::VarLine(void)
{
    mEditor->mDocument.Insert("&_&") ;
}


void cWordTsar::VarFilename(void)
{
    mEditor->mDocument.Insert("&*&") ;
}


void cWordTsar::VarDrive(void)
{
    mEditor->mDocument.Insert("&:&") ;
}


void cWordTsar::VarDirectory(void)
{
    mEditor->mDocument.Insert("&.&") ;
}


void cWordTsar::VarPath(void)
{
    mEditor->mDocument.Insert("&\\&") ;
}


void cWordTsar::ExtendedChar(void)
{
    mEditor->mInput->HandleKey(CTRL_P) ;
    mEditor->mInput->HandleKey('0') ;
}


void cWordTsar::FileAtPrint(void)
{
    // @TODO - get file via dialog
    mEditor->MoveCursorStartLine() ;

    mEditor->mDocument.MaybeInsertHardReturn() ;
    mEditor->mDocument.Insert(".fi\n") ;
}


void cWordTsar::Graphic(void)
{
    // @TODO - get file via dialog
    mEditor->mInput->HandleKey(CTRL_P) ;
    mEditor->mInput->HandleKey('*') ;
}


void cWordTsar::NoteComment(void)
{
    mEditor->mInput->HandleKey(CTRL_O) ;
    mEditor->mInput->HandleKey('n') ;
    mEditor->mInput->HandleKey('c') ;
}


void cWordTsar::NoteFootnote(void)
{
    mEditor->mInput->HandleKey(CTRL_O) ;
    mEditor->mInput->HandleKey('n') ;
    mEditor->mInput->HandleKey('f') ;
}


void cWordTsar::NoteEndnote(void)
{
    mEditor->mInput->HandleKey(CTRL_O) ;
    mEditor->mInput->HandleKey('n') ;
    mEditor->mInput->HandleKey('e') ;
}


void cWordTsar::NoteAnnotation(void)
{
    mEditor->mInput->HandleKey(CTRL_O) ;
    mEditor->mInput->HandleKey('n') ;
    mEditor->mInput->HandleKey('a') ;
}


void cWordTsar::TOCEntry(void)
{
    // @TODO - get text via dialog
    mEditor->MoveCursorStartLine() ;

    mEditor->mDocument.MaybeInsertHardReturn() ;
    mEditor->mDocument.Insert(".tc\n") ;
}


void cWordTsar::IndexEntry(void)
{
    mEditor->mInput->HandleKey(CTRL_O) ;
    mEditor->mInput->HandleKey('n') ;
    mEditor->mInput->HandleKey('i') ;
}


void cWordTsar::MarkIndex(void)
{
    mEditor->mInput->HandleKey(CTRL_P) ;
    mEditor->mInput->HandleKey('k') ;
}


void cWordTsar::DotLeader(void)
{
    mEditor->mInput->HandleKey(CTRL_P) ;
    mEditor->mInput->HandleKey('.') ;
}


void cWordTsar::ParOutlineNumber(void)
{
    mEditor->mInput->HandleKey(CTRL_O) ;
    mEditor->mInput->HandleKey('z') ;
}


void cWordTsar::Bold(void)
{
    mEditor->mInput->HandleKey(CTRL_P) ;
    mEditor->mInput->HandleKey('b') ;
}


void cWordTsar::Italics(void)
{
    mEditor->mInput->HandleKey(CTRL_P) ;
    mEditor->mInput->HandleKey('y') ;
}


void cWordTsar::Underline(void)
{
    mEditor->mInput->HandleKey(CTRL_P) ;
    mEditor->mInput->HandleKey('s') ;
}


void cWordTsar::font(void)
{
    mEditor->mInput->HandleKey(CTRL_P) ;
    mEditor->mInput->HandleKey('=') ;
}


void cWordTsar::Strikeout(void)
{
    mEditor->mInput->HandleKey(CTRL_P) ;
    mEditor->mInput->HandleKey('x') ;
}


void cWordTsar::Subscript(void)
{
    mEditor->mInput->HandleKey(CTRL_P) ;
    mEditor->mInput->HandleKey('v') ;
}


void cWordTsar::Superscript(void)
{
    mEditor->mInput->HandleKey(CTRL_P) ;
    mEditor->mInput->HandleKey('t') ;
}


void cWordTsar::DoubleStrike(void)
{
    mEditor->mInput->HandleKey(CTRL_P) ;
    mEditor->mInput->HandleKey('d') ;
}


void cWordTsar::Color(void)
{
    mEditor->mInput->HandleKey(CTRL_P) ;
    mEditor->mInput->HandleKey('-') ;
}


void cWordTsar::SelectParStyle(void)
{
    mEditor->mInput->HandleKey(CTRL_O) ;
    mEditor->mInput->HandleKey('f') ;
    mEditor->mInput->HandleKey('s') ;
}


void cWordTsar::ReturntoPrevStyle(void)
{
    mEditor->mInput->HandleKey(CTRL_O) ;
    mEditor->mInput->HandleKey('f') ;
    mEditor->mInput->HandleKey('p') ;
}


void cWordTsar::DefineParStyle(void)
{
    mEditor->mInput->HandleKey(CTRL_O) ;
    mEditor->mInput->HandleKey('f') ;
    mEditor->mInput->HandleKey('d') ;
}


void cWordTsar::CopyStyletoLibrary(void)
{
    mEditor->mInput->HandleKey(CTRL_O) ;
    mEditor->mInput->HandleKey('f') ;
    mEditor->mInput->HandleKey('o') ;
}


void cWordTsar::DeleteLibraryStyle(void)
{
    mEditor->mInput->HandleKey(CTRL_O) ;
    mEditor->mInput->HandleKey('f') ;
    mEditor->mInput->HandleKey('y') ;
}


void cWordTsar::RenameLibraryStyle(void)
{
    mEditor->mInput->HandleKey(CTRL_O) ;
    mEditor->mInput->HandleKey('f') ;
    mEditor->mInput->HandleKey('r') ;
}


void cWordTsar::RenameDocStyle(void)
{
    mEditor->mInput->HandleKey(CTRL_O) ;
    mEditor->mInput->HandleKey('f') ;
    mEditor->mInput->HandleKey('e') ;
}


void cWordTsar::Uppercase(void)
{
    mEditor->mInput->HandleKey(CTRL_K) ;
    mEditor->mInput->HandleKey('\"') ;
}


void cWordTsar::Lowercase(void)
{
    mEditor->mInput->HandleKey(CTRL_K) ;
    mEditor->mInput->HandleKey('\'') ;
}


void cWordTsar::Sentencecase(void)
{
    mEditor->mInput->HandleKey(CTRL_K) ;
    mEditor->mInput->HandleKey('.') ;
}


void cWordTsar::Settings(void)
{
    // @TODO implement
}


void cWordTsar::CenterLine(void)
{
    mEditor->mInput->HandleKey(CTRL_O) ;
    mEditor->mInput->HandleKey('c') ;
}


void cWordTsar::RightAlign(void)
{
    mEditor->mInput->HandleKey(CTRL_O) ;
    mEditor->mInput->HandleKey(']') ;
}


void cWordTsar::RulerLine(void)
{
    mEditor->mInput->HandleKey(CTRL_O) ;
    mEditor->mInput->HandleKey('l') ;
}


void cWordTsar::Columns(void)
{
    mEditor->mInput->HandleKey(CTRL_O) ;
    mEditor->mInput->HandleKey('U') ;
}


void cWordTsar::Page(void)
{
    mEditor->mInput->HandleKey(CTRL_O) ;
    mEditor->mInput->HandleKey('Y') ;
}


void cWordTsar::Header(void)
{
    mEditor->MoveCursorStartLine() ;

    mEditor->mDocument.MaybeInsertHardReturn() ;
    mEditor->mDocument.Insert(".he\n") ;
}


void cWordTsar::Footer(void)
{
    mEditor->MoveCursorStartLine() ;

    mEditor->mDocument.MaybeInsertHardReturn() ;
    mEditor->mDocument.Insert(".fo\n") ;
}


void cWordTsar::PageNumbering(void)
{
    mEditor->mInput->HandleKey(CTRL_O) ;
    mEditor->mInput->HandleKey('#') ;
}


void cWordTsar::LineNumbering(void)
{
    mEditor->MoveCursorStartLine() ;

    mEditor->mDocument.MaybeInsertHardReturn() ;
    mEditor->mDocument.Insert(".l#\n") ;
}


void cWordTsar::Alignment(void)
{
    mEditor->mInput->HandleKey(CTRL_O) ;
    mEditor->mInput->HandleKey('s') ;
}


void cWordTsar::OverprintChar(void)
{
    mEditor->mInput->HandleKey(CTRL_P) ;
    mEditor->mInput->HandleKey('H') ;
}


void cWordTsar::OverprintLine(void)
{
    mEditor->mInput->HandleKey(CTRL_P) ;
    mEditor->mInput->HandleKey(HARD_RETURN) ;
}


void cWordTsar::OptionalHyphen(void)
{
    mEditor->mInput->HandleKey(CTRL_O) ;
    mEditor->mInput->HandleKey('e') ;
}


void cWordTsar::VerticalCenter(void)
{
    mEditor->mInput->HandleKey(CTRL_O) ;
    mEditor->mInput->HandleKey('v') ;
}


void cWordTsar::KeepWordsTogether(void)
{
    mEditor->mInput->HandleKey(CTRL_P) ;
    mEditor->mInput->HandleKey('O') ;
}


void cWordTsar::KeepLinesTogetherPage(void)
{
    mEditor->MoveCursorStartLine() ;

    mEditor->mDocument.MaybeInsertHardReturn() ;
    mEditor->mDocument.Insert(".cp\n") ;
}


void cWordTsar::KeepLinesTogetherColumn(void)
{
    mEditor->MoveCursorStartLine() ;

    mEditor->mDocument.MaybeInsertHardReturn() ;
    mEditor->mDocument.Insert(".cc\n") ;
}


void cWordTsar::SpellCheckGlobal(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('r') ;
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('l') ;
}


void cWordTsar::SpellCheckRest(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('l') ;
}


void cWordTsar::SpellCheckWord(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('N') ;
}


void cWordTsar::SpellCheckType(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('o') ;
}


void cWordTsar::SpellCheckNotes(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('O') ;
    mEditor->mInput->HandleKey('L') ;
}


void cWordTsar::Thesaurus(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('j') ;
}


void cWordTsar::LanguageChange(void)
{
    mEditor->MoveCursorStartLine() ;

    mEditor->mDocument.MaybeInsertHardReturn() ;
    mEditor->mDocument.Insert(".la\n") ;
}


void cWordTsar::Inset(void)
{
    mEditor->mInput->HandleKey(CTRL_P) ;
    mEditor->mInput->HandleKey('&') ;
}


void cWordTsar::Calculator(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('M') ;
}


void cWordTsar::BlockMath(void)
{
    mEditor->mInput->HandleKey(CTRL_K) ;
    mEditor->mInput->HandleKey('m') ;
}


void cWordTsar::SortBlockAsc(void)
{
    mEditor->mInput->HandleKey(CTRL_K) ;
    mEditor->mInput->HandleKey('z') ;
    mEditor->mInput->HandleKey('a') ;
}


void cWordTsar::SortBlockDes(void)
{
    mEditor->mInput->HandleKey(CTRL_K) ;
    mEditor->mInput->HandleKey('z') ;
    mEditor->mInput->HandleKey('d') ;
}


void cWordTsar::WordCount(void)
{
    mEditor->mInput->HandleKey(CTRL_K) ;
    mEditor->mInput->HandleKey('?') ;
}


void cWordTsar::PlayMacro(void)
{
    mEditor->mInput->HandleKey(CTRL_J) ;
    mEditor->mInput->HandleKey('p') ;
}


void cWordTsar::RecordMacro(void)
{
    mEditor->mInput->HandleKey(CTRL_J) ;
    mEditor->mInput->HandleKey('r') ;
}


void cWordTsar::EditMacro(void)
{
    mEditor->mInput->HandleKey(CTRL_J) ;
    mEditor->mInput->HandleKey('D') ;
}


void cWordTsar::SingleStep(void)
{
    mEditor->mInput->HandleKey(CTRL_J) ;
    mEditor->mInput->HandleKey('s') ;
}


void cWordTsar::CopyMacro(void)
{
    mEditor->mInput->HandleKey(CTRL_J) ;
    mEditor->mInput->HandleKey('O') ;
}


void cWordTsar::DeleteMacro(void)
{
    mEditor->mInput->HandleKey(CTRL_J) ;
    mEditor->mInput->HandleKey('y') ;
}


void cWordTsar::RenameMacro(void)
{
    mEditor->mInput->HandleKey(CTRL_J) ;
    mEditor->mInput->HandleKey('e') ;
}


void cWordTsar::DataFile(void)
{
    mEditor->MoveCursorStartLine() ;

    mEditor->mDocument.MaybeInsertHardReturn() ;
    mEditor->mDocument.Insert(".df\n") ;
}


void cWordTsar::NameVars(void)
{
    mEditor->MoveCursorStartLine() ;

    mEditor->mDocument.MaybeInsertHardReturn() ;
    mEditor->mDocument.Insert(".rv\n") ;
}


void cWordTsar::SetVar(void)
{
    mEditor->MoveCursorStartLine() ;

    mEditor->mDocument.MaybeInsertHardReturn() ;
    mEditor->mDocument.Insert(".sv\n") ;
}


void cWordTsar::SetVarMath(void)
{
    mEditor->MoveCursorStartLine() ;

    mEditor->mDocument.MaybeInsertHardReturn() ;
    mEditor->mDocument.Insert(".ma\n") ;
}


void cWordTsar::AskVar(void)
{
    mEditor->MoveCursorStartLine() ;

    mEditor->mDocument.MaybeInsertHardReturn() ;
    mEditor->mDocument.Insert(".av\n") ;
}


void cWordTsar::If(void)
{
    mEditor->MoveCursorStartLine() ;

    mEditor->mDocument.MaybeInsertHardReturn() ;
    mEditor->mDocument.Insert(".if\n") ;
}


void cWordTsar::Else(void)
{
    mEditor->MoveCursorStartLine() ;

    mEditor->mDocument.MaybeInsertHardReturn() ;
    mEditor->mDocument.Insert(".el\n") ;
}


void cWordTsar::EndIf(void)
{
    mEditor->MoveCursorStartLine() ;

    mEditor->mDocument.MaybeInsertHardReturn() ;
    mEditor->mDocument.Insert(".ei\n") ;
}


void cWordTsar::Top(void)
{
    mEditor->MoveCursorStartLine() ;

    mEditor->mDocument.MaybeInsertHardReturn() ;
    mEditor->mDocument.Insert(".go t\n") ;
}


void cWordTsar::Bottom(void)
{
    mEditor->MoveCursorStartLine() ;

    mEditor->mDocument.MaybeInsertHardReturn() ;
    mEditor->mDocument.Insert(".go b\n") ;
}


void cWordTsar::Clear(void)
{
    mEditor->MoveCursorStartLine() ;

    mEditor->mDocument.MaybeInsertHardReturn() ;
    mEditor->mDocument.Insert(".cs\n") ;
}


void cWordTsar::Display(void)
{
    mEditor->MoveCursorStartLine() ;

    mEditor->mDocument.MaybeInsertHardReturn() ;
    mEditor->mDocument.Insert(".dm\n") ;
}


void cWordTsar::PrintNTimes(void)
{
    mEditor->MoveCursorStartLine() ;

    mEditor->mDocument.MaybeInsertHardReturn() ;
    mEditor->mDocument.Insert(".rp\n") ;
}


void cWordTsar::ReformatRest(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('u') ;
}


void cWordTsar::ReformatPara(void)
{
    mEditor->mInput->HandleKey(CTRL_B) ;
}


void cWordTsar::ReformatNotes(void)
{
    mEditor->mInput->HandleKey(CTRL_O) ;
    mEditor->mInput->HandleKey('n') ;
    mEditor->mInput->HandleKey('u') ;
}


void cWordTsar::RepeatKey(void)
{
    mEditor->mInput->HandleKey(CTRL_Q) ;
    mEditor->mInput->HandleKey('q') ;
}
