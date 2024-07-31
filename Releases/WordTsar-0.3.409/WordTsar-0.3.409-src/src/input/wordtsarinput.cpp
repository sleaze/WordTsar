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

#include <QFileDialog>
#include <QMessageBox>
#include <QCoreApplication>

#include "wordtsarinput.h"

/// @ingroup Keyboard
/// @{

cWordStarInput::cWordStarInput(cEditorCtrl *editor)
{    
    mEditor = editor ;
    
    mControlKMode = false ;
    mControlOMode = false ;
    mControlPMode = false ;
    mControlQMode = false ;
}
    

cWordStarInput::~cWordStarInput()
{
    
}


bool cWordStarInput::HandleKey(char ch, bool shift)
{
    bool handled = false ;
    
    if(ch == 27)                    // escape key
    {
        mControlKMode = false ;
        mControlOMode = false ;
        mControlPMode = false ;
        mControlQMode = false ;
        mEditor->mHelpDisplay = mOldHelpStatus ;
        handled = true ;
    }

    // handle special modes
    if(mControlKMode == true)
    {
        mEditor->mHelpDisplay = mOldHelpStatus ;
        OnControlKChar(ch) ;
        handled = true ;
    }
    else if(mControlQMode == true)
    {
        mEditor->mHelpDisplay = mOldHelpStatus ;
        OnControlQChar(ch) ;
        handled = true ;
    }
    else if(mControlPMode == true)
    {
        mEditor->mHelpDisplay = mOldHelpStatus ;
        OnControlPChar(ch) ;
        handled = true ;
    }
    else if(mControlOMode == true)
    {
        mEditor->mHelpDisplay = mOldHelpStatus ;
        OnControlOChar(ch) ;
        handled = true ;
    }

    if(handled == false)
    {
        switch(ch)
        {
            // deal with menus
            case CTRL_K :
                mControlKMode = true ;
                mOldHelpStatus = mEditor->mHelpDisplay ;
                mEditor->mHelpDisplay = HELP_CTRLK ;
                handled = true ;
                break ;
                
            case CTRL_Q :
                mControlQMode = true ;
                mOldHelpStatus = mEditor->mHelpDisplay ;
                mEditor->mHelpDisplay = HELP_CTRLQ ;
                handled = true ;
                break ;
                
            case CTRL_P :
                mControlPMode = true ;
                mOldHelpStatus = mEditor->mHelpDisplay ;
                mEditor->mHelpDisplay = HELP_CTRLP ;
                handled = true ;
                break ;
                
            case CTRL_O :
                mControlOMode = true ;
                mOldHelpStatus = mEditor->mHelpDisplay ;
                mEditor->mHelpDisplay = HELP_CTRLO ;
                handled = true ;
                break ;
                
            // deal with regular control keys
            case CTRL_J :
                if(mEditor->mHelpDisplay == HELP_MAIN )
                {
                    mEditor->mHelpDisplay = HELP_NONE ;
                    mOldHelpStatus = mEditor->mHelpDisplay ;
                }
                else
                {
                    mEditor->mHelpDisplay = HELP_MAIN ;
                    mOldHelpStatus = mEditor->mHelpDisplay ;
                }
                handled = true ;
                break ;
                
            case CTRL_E :
                mEditor->MoveUp() ;
                handled = true ;
                break ;
                
            case CTRL_X :
                mEditor->MoveDown() ;
                handled = true ;
                break ;
                
            case CTRL_S :
                mEditor->MoveLeft() ;
                handled = true ;
                break ;
                
            case CTRL_D :
                mEditor->MoveRight() ;
                handled = true ;
                break ;
                
            case CTRL_R :
                mEditor->PageUp() ;
                handled = true ;
                break ;
                
            case CTRL_C :
                mEditor->PageDown() ;
                handled = true ;
                break ;
                
            case CTRL_A :
                mEditor->WordLeft() ;
                handled = true ;
                break ;
                
            case CTRL_F :
                mEditor->WordRight() ;
                handled = true ;
                break ;
                
            case CTRL_W :
                mEditor->ScrollUp() ;
                handled = true ;
                break ;
                
            case CTRL_Z :
                mEditor->ScrollDown() ;
                handled = true ;
                break ;
                
            case CTRL_V :
                mEditor->mInsertMode = !mEditor->mInsertMode ;
                handled = true ;
                break ;
            
            case CTRL_G :
                mEditor->DeleteChar() ;
                handled = true ;
                break ;
                
            case CTRL_H :
                mEditor->MoveLeft() ;
                mEditor->DeleteChar() ;
                handled = true ;
                break ;
                
            case CTRL_T :
                mEditor->DeleteWordRight() ;
                handled = true ;
                break ;
                
            case CTRL_Y :
                mEditor->DeleteLine() ;
                handled = true ;
                break ;
                
            case CTRL_U :
                if(shift)
                {
                    mEditor->Redo() ;
                }
                else
                {
                    mEditor->Undo() ;
                }
                handled = true ;
                break ;
                
            case CTRL_N :
                mEditor->LineBreak() ;
                handled = true ;
                break ;
                
            case CTRL_L :
                mEditor->FindAgain() ;
                handled = true ;
                break ;
                
            case CTRL_M :
                mEditor->NotImplemented("^M") ;
                handled = true ;
                break ;
                
            case CTRL_I :
                sWSTab tab ;
                tab.type = TAB_TAB ;
                mEditor->mDocument.InsertTab(tab) ;
                break ;
        }
    }
    
    return handled ;
}



eHelpDisplay cWordStarInput::GetHelpStatus(void)
{
        return mEditor->mHelpDisplay ;
}



bool cWordStarInput::OnControlKChar(char ch)
{
    mControlKMode = false ;
    bool retval = false ;

    if(ch < ' ')
    {
        ch += 'A' ;
    }
    ch = tolower(ch) ;
    
    switch(ch)
    {
        case 'r' :          // insert/open a file
            {
                QString loadable ;
                bool first = true ;
                for(auto & mFileType : mEditor->mFileTypes)
                {
                    if(mFileType->CanLoad())
                    {
                        std::string ext = mFileType->GetExtensions() ;
                        if(!first)
                        {
                            ext = ";;" + ext ;
                        }
                        first = false ;
                        loadable.append(ext.c_str()) ;
                    }
                }
//                loadable.append(";;RTF Files (*.rtf *.RTF)") ;
//                loadable.append(";;DOCX Files (*.docx *.DOCX)") ;
//                loadable.append(";;All Files (*.*)") ;

                QString filename = QFileDialog::getOpenFileName(mEditor, "Insert a file...", QString(), loadable) ;
                if(!filename.isNull())
                {
                    POSITION_T position = mEditor->mDocument.GetPosition() ;
                    mEditor->setEnabled(false) ;

                    QString path, file ;
                    QFileInfo info(filename) ;
                    path = info.path() ;
                    file = info.fileName() ;

                    mEditor->mFileName = file ;
                    mEditor->mFileDir = path + '/' ;
                    mEditor->mFileSet = true ;
/*
                    int slash = mEditor->mFileDir.Find('/', true) ;
                    if(slash == wxNOT_FOUND)
                    {
                        slash = mEditor->mFileDir.Find('\\', true) ;
                    }
                    if(slash != wxNOT_FOUND)
                    {
                        mEditor->mFileDir = mEditor->mFileDir.Left(slash + 1) ;
                    }
*/
                    mEditor->LoadFile(mEditor->mFileDir + mEditor->mFileName) ;

                    mEditor->setEnabled(true) ;

                    mEditor->mDocument.SetPosition(position) ;

                }
                retval = true ;
            }
            break ;
            
        case 'd' :          // save file and clear buffer
            if(mEditor->mDocument.mChanged)
            {
                if(mEditor->mFileSet != false)      // if we have a directory, we have a valid file name
                {
                    QString filename = mEditor->mFileDir + "/" + mEditor->mFileName ;

                    bool ok = mEditor->SaveFile(filename) ;
                    if(ok == false)
                    {
                        QMessageBox msgBox(QMessageBox::Critical, "Error", "File Save failed", QMessageBox::Ok, mEditor) ;
                        msgBox.exec() ;
                    }
                    else
                    {
                        mEditor->mDocument.mChanged = false ;

//                        wxWindowDisabler disableAll;
//                        mEditor->InsertInfoString("       \n\n                      File Saved...\n\n\n") ;
                            retval = true ;
                    }
//                    break ;
                }
                else
                {
                    retval = OnControlKChar('T') ;
                }
            }
            
            if(retval == true)
            {
                mEditor->mDocument.Clear() ;
                mEditor->mFileDir = "./" ;
                mEditor->mFileName = "Unknown.ws" ;
                mEditor->mFileSet = false ;
            }
            break ;

        case 's' :          // save file
            if(mEditor->mDocument.mChanged == true)
            {
                if(mEditor->mFileSet != false)      // if we have a directory, we have a valid file name
                {
                    QString filename = mEditor->mFileDir + "/" + mEditor->mFileName ;

                    bool ok = mEditor->SaveFile(filename) ;
                    if(ok == false)
                    {
                        QMessageBox msgBox(QMessageBox::Critical, "Error", "File Save failed", QMessageBox::Ok, mEditor) ;
                        msgBox.exec() ;
                    }
                    else
                    {
                        mEditor->mDocument.mChanged = false ;

//                        wxWindowDisabler disableAll;
//                        mEditor->InsertInfoString("       \n\n                      File Saved...\n\n\n") ;

                        retval = true ;
                    }
                    break ;
                }
                else
                {
                    retval = OnControlKChar('T') ;
                }
            }
            break ;

        case 'x' :          // save and exit
            if(mEditor->mDocument.mChanged == true)
            {
                if(QMessageBox::question(mEditor, "Save and Exit", "Save File Before Exiting?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
                {
                    retval = OnControlKChar('S') ;
                    if(retval == true)
                    {
                        QCoreApplication::quit() ;
                    }
                }
                else
                {
                    QCoreApplication::quit() ;
                }
            }
            else
            {
                QCoreApplication::quit() ;
            }
            break ;

        case 'q' :          // abandoned
            if(mEditor->mDocument.mChanged == true)
            {
                if(QMessageBox::question(mEditor, "Quit", "Quit without Saving?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
                {
                    mEditor->mDocument.Clear() ;
                    mEditor->mFileDir = "./" ;
                    mEditor->mFileName = "Unknown.ws" ;
                    mEditor->mFileSet = false ;
                }
            }
            else
            {
                mEditor->mDocument.Clear() ;
                mEditor->mFileDir = "./" ;
                mEditor->mFileName = "Unknown.ws" ;
                mEditor->mFileSet = false ;
            }
            break ;

        case 't' :          // save as
            {
                QString loadable ;
                bool first = true ;
                for(auto & mFileType : mEditor->mFileTypes)
                {
                    if(mFileType->CanSave())
                    {
                        std::string ext = mFileType->GetExtensions() ;
                        if(!first)
                        {
                            ext = ";;" + ext ;
                        }
                        first = false ;
                        loadable.append(ext.c_str()) ;
                    }
                }
//                loadable.append("WordStar Files (WordStar Files (*.ws *.ws3 *.ws4 *.ws5 *.ws6 *.ws7 *.ws8 *.WS *.WS3 *.WS4 *.WS5 *.WS6 *.WS7 *.WS8)") ;
//                loadable.append(";;RTF Files (*.rtf *.RTF)") ;
//                loadable.append(";;DOCX Files (*.docx *.DOCX)") ;
//                loadable.append(";;All Files (*.*)") ;

                QString filename = QFileDialog::getSaveFileName(mEditor, "Save file...", QString(), loadable) ;
                if(!filename.isNull())
                {
                    QString path, file ;
                    QFileInfo info(filename) ;
                    mEditor->mFileDir = info.path() ;
                    mEditor->mFileName = info.fileName() ;
                    mEditor->mFileSet = true ;

        /*
                    int slash = mEditor->mFileDir.Find('/', true) ;
                    if(slash == wxNOT_FOUND)
                    {
                        slash = mEditor->mFileDir.Find('\\', true) ;
                    }
                    if(slash != wxNOT_FOUND)
                    {
                        mEditor->mFileDir = mEditor->mFileDir.Left(slash + 1) ;
                    }
        */
                    bool ok = mEditor->SaveFile(filename) ;
                    if(ok == false)
                    {
                        retval = false ;
                    }
                    else
                    {
                        QString st ;
                        st.sprintf("File %s saved", filename.toUtf8().constData()) ;
                        QMessageBox msgBox(QMessageBox::Information, "Save OK", st, QMessageBox::Ok, mEditor) ;
                        msgBox.exec() ;

                        std::string temp = mEditor->mFileName.toUtf8().constData() ;
//                        temp = temp + " - " + "WordTsar" ; // mEditor->mTitle ;
                        mEditor->SetTitle(temp) ;

                    }
                }
            }
            break ;

        case 'b' :          // begin block
            mEditor->SetBeginBlock() ;
            break ;
            
        case 'k' :          // end block
            mEditor->SetEndBlock() ;
            break ;
            
        case 'c' :          // copy block
            mEditor->CopyBlock() ;
            break ; 
            
        case 'v' :          // move a block
            mEditor->MoveBlock() ;
            break ;
            
        case 'y' :          // delete a block
            mEditor->DeleteBlock() ;
            break ;
            
        case '\"' :         // upper case block
            mEditor->UpperCaseBlock() ;
            break ;
        
        case '\'' :         // low case block
            mEditor->LowerCaseBlock() ;
            break ;
            
//        case '.' :
//            mEditor->SentenceBlock() ;
//            break ;

        case '<' :          // unset block
            mEditor->UnSetBlock() ;
            break ;

        case 'h' :          // toggle hide block
            mEditor->ToggleHideBlock() ;
            break ;
            
        case '[' :          // system clipboard paste
            mEditor->ClipboardPaste() ;
            break ;
            
        case ']' :          // system clipboard paste
            mEditor->ClipboardCopy() ;
            break ;
            
        case '0' :
        case '1' :
        case '2' :
        case '3' :
        case '4' :
        case '5' :
        case '6' :
        case '7' :
        case '8' :
        case '9' :
            {
                int offset = ch - '0' ;
                mEditor->SavePosition(offset) ;
            }
            break ;
            
        case '?' :
            mEditor->WordCountBlock() ;
            break ;
            
        case 'o' :
        case 'e' :
        case 'j' :
        case 'p' :
        case '\\' :
        case 'l' :
        case 'f' :
        case 'w' :
        case 'm' :
        case 'z' :
        case 'u' :
        case 'n' :
        case 'i' :
        case 'a' :
        case 'g' :
        case '.' :
            {
                QString t ;
                t.sprintf("^K-%c", ch) ;
                mEditor->NotImplemented(t) ;
            }
            break ;
    }
    
    return retval ;
}


void cWordStarInput::OnControlOChar(char ch)
{
    mControlOMode = false ;
    
    if(ch < ' ')
    {
        ch += 'A' ;
    }
    if(ch >= 'A' && ch <= 'Z')
    {
        ch += 31 ;          // lowercase
    }
    
    switch(ch)
    {
        case 'd' :
            mEditor->ToggleShowControl() ;
            mEditor->LayoutFullDocument() ;
            break ;

        case 'c' :
        {
            sWSTab tab ;
            tab.type = TAB_CENTER ;
            mEditor->mDocument.InsertTab(tab) ;
            break ;
        }
        
        case ']' :
        {
            sWSTab tab ;
            tab.type = TAB_RIGHT ;
            mEditor->mDocument.InsertTab(tab) ;
            break ;
        }
        
        case '?' :
            mEditor->About() ;
            break ;
            
        case 'p' :
            mEditor->PrintPreview() ;
            break ;
            
        case 'y' :
            mEditor->PageLayout() ;
            break ;
            
        case 'l' :
        case 'g' :
        case 'x' :
        case 'i' :
        case 'o' :
        case 'u' :
        case 'f' :
        case 's' :
        case 'v' :
        case 'e' :
        case 'h' :
        case 'j' :
        case 'a' :
        case 'w' :
        case ' ' :
        case 'b' :
        case 'k' :
        case 'm' :
        case 'z' :
        case '#' :
        case 'n' :
            {
                QString t ;
                t.sprintf("^O-%c", ch) ;
                mEditor->NotImplemented(t) ;
            }
            break ;
        
    }
}


void cWordStarInput::OnControlQChar(char ch)
{
    mControlQMode = false ;
    
    if(ch < ' ')
    {
        ch += 'A' ;
    }
    if(ch >= 'A' && ch <= 'Z')
    {
        ch += 31 ;          // lowercase
    }
    
    switch(ch)
    {
        case 'a' :
            mEditor->Replace() ;
            break ;
            
        case 'f' :
printf("Start Find\n") ;
            mEditor->Find() ;
            break ;
            
        case 'e' :
            mEditor->MoveCursorTopLeft() ;
            break ;
            
        case 'x' :
            mEditor->MoveCursorBottomRight() ;
            break ;
            
        case 'r' :
            mEditor->MoveCursorTopofFile() ;
            break ;
            
        case 'c' :
            mEditor->MoveCursorEndofFile() ;
            break ;
            
        case 'b' :
            mEditor->MoveCursorStartBlock() ;
            break ;
            
        case 'k' :
            mEditor->MoveCursorEndBlock() ;
            break ;
            
        case 's' :
            mEditor->MoveCursorStartLine() ;
            break ;
            
        case 'd' :
            mEditor->MoveCursorEndLine() ;
            break ;
            
        case '0' :
        case '1' :
        case '2' :
        case '3' :
        case '4' :
        case '5' :
        case '6' :
        case '7' :
        case '8' :
        case '9' :
            {
                int offset = ch - '0' ;
                mEditor->GotoSavePosition(offset) ;
            }
            break ;

        case 'u' :
            mEditor->LayoutFullDocument() ;
            break ;
            
        case 'l' :
            mEditor->SpellCheckDocument() ;
            break ;

        case 'n' :
//            mEditor->SpellCheckWord() ;
//            break ;
            
        case 'p' :
        case 'v' :
        case 'o' :
        case 'g' :
        case 'h' :
        case 'i' :
        case '=' :
        case '<' :
        case 'm' :
        case 'j' :
        case 'w' :
        case 'z' :
        case ' ' :   // del
        case 'y' :
        case 't' :
            {
                QString t ;
                t.sprintf("^Q-%c", ch) ;
                mEditor->NotImplemented(t) ;
            }
            break ;
        
    }
}



void cWordStarInput::OnControlPChar(char ch)
{
    mControlPMode = false ;
    
    if(ch < ' ')
    {
        ch += 'A' ;
    }
    if(ch >= 'A' && ch <= 'Z')
    {
        ch += 31 ;          // lowercase
    }
    
    switch(ch)
    {
        case 'b' :
            mEditor->mDocument.BeginBold() ;
            break ;
            
        case 's' :
            mEditor->mDocument.BeginUnderline() ;
            break ;
            
        case 'v' :
            mEditor->mDocument.BeginSubscript() ;
            break ;
        
        case 't' :
            mEditor->mDocument.BeginSuperscript() ;
            break ;
            
        case 'y' :
            mEditor->mDocument.BeginItalics() ;
            break ;
            
        case 'x' :
            mEditor->mDocument.BeginStrikeThrough() ;
            break ;
            
        case 'k' :
            mEditor->mDocument.BeginStrikeThrough() ;
            break ;
            
        case '=' :
        case '+' :
            mEditor->SelectFont() ;
            break ;
            
        case '-' :
            mEditor->SelectColor() ;
            break ;
            
        case 'd' :
        case 'n' :
        case 'a' :
        case 'h' :
        case ' ' :
        case 'f' :
        case 'g' :
        case '*' :
        case '&' :
        case 'o' :
        case 'c' :
        case 'i' :
        case '.' :
        case '0' :
        case 'q' :
        case 'w' :
        case 'e' :
        case 'r' :
        case '!' :
        case '?' :
            {
                QString t ;
                t.sprintf("^P-%c", ch) ;
                mEditor->NotImplemented(t) ;
            }
            break ;
            
    }
}


/// @}
