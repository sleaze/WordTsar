#ifndef WORDTSAR_H
#define WORDTSAR_H

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

#include <QMainWindow>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QLabel>
#include <QScrollArea>
#include <QProgressBar>
#include <QMovie>
#include <QMenuBar>
#include <QMenu>
#include <QDeadlineTimer>

//#include "src/gui/editor/editorctrl.h"
#include"src/gui/ruler/rulerctrl.h"

class cEditorCtrl ;

class cWordTsar : public QMainWindow
{
    Q_OBJECT

public:
    cWordTsar(int argc, char *argv[], QWidget *parent = nullptr);
    ~cWordTsar();

//    void SetFile(QString filename) ;
    void LoadFile(QString name) ;

    void UpdateStatus(cEditorCtrl *editor) ;
    void SetStatus(QString text, bool progress = false, int percent = 0) ;
    
    
    void ReadConfig(void) ;
    void WriteConfig(void) ;

    QScrollBar *mScrollbar ;

private :
    void CreateMenus(void) ;
//    void StatusOn(int index, QString text, wxClientDC &dc) ;
//    void StatusOff(int index, QString text, wxClientDC &dc) ;

protected :
    void closeEvent(QCloseEvent *event) override ;

public:
    QStatusBar *mStatusTop ;
    QStatusBar *mStatusBottom;
    cRulerCtrl *mRuler ;

private:
//    QWidget *mBaseWidget;
    QWidget *mBaseWidget ;
    QVBoxLayout *mBaseLayout;
    QVBoxLayout *mLayout;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;

    QLabel *mStatStyle ;
    QLabel *mStatFont ;
    QLabel *mStatBold ;
    QLabel *mStatItalic ;
    QLabel *mStatUnderline ;
    QLabel *mStatChange ;
    QLabel *mStatLeft ;
    QLabel *mStatCenter ;
    QLabel *mStatRight ;
    QLabel *mStatJustify ;
    QLabel *mStatInfo ;
    QLabel *mStatText ;

    QMovie *mBusyIndicator ;
    QLabel *mBusy ;
    QLabel *mBusy1 ;
    QLabel *mBusy2 ;

    QHBoxLayout *mEditorLayout ;
    QWidget *mBaseEditor ;

    cEditorCtrl *mHelpCtrl;
    cEditorCtrl *mHelpPCtrl;
    cEditorCtrl *mHelpKCtrl;
    cEditorCtrl *mHelpQCtrl;
    cEditorCtrl *mHelpOCtrl;
    cEditorCtrl *mEditor;


    QString mLoadFileName ;
    
    QAction *mOpenAction ;
    QAction *mSaveAction ;
    QAction *mSaveAsAction ;
    QAction *mSaveandCloseAction ;
    QAction *mPrintAction ;
    QAction *mPrintPreviewAction ;

    QDeadlineTimer mHelpTimer ;
};



#endif   // _WORDTSAR_H_
