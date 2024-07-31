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

#include <QFile>
#include <QFileInfo>
#include <QProgressDialog>
#include <QApplication>

#include "textfile.h"
#include "src/core/document/document.h"

using namespace std ;


/// @ingroup Editor
/// @{

cTextFile::cTextFile(cEditorCtrl *editor)
    : cFile(editor)
{
//    mDocument = document ;
}

cTextFile::~cTextFile()
{
    //dtor
}

bool cTextFile::CheckType(QString filename)
{
    QString path, file, ext;
    QFileInfo info(filename) ;
    path = info.path() ;
    file = info.fileName() ;
    ext = info.suffix() ;

    return true ;           // try to load anything
}


bool cTextFile::LoadFile(QString filename)
{
    bool retval = false ;

    QFile infile(filename) ;

//    in.Open(filename, wxConvAuto(wxFONTENCODING_UTF8)) ;             // with wxConvAuto by default
    if(infile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&infile) ;
        in.setCodec("UTF-8") ;
        int lines = GetLineCount(in) ;        // yes, we read the file twice to do this
        int cline = 1 ;

        QProgressDialog progress("Loading File as UTF-8 plain text...", "Cancel", 0, lines, mEditor) ;
        progress.show() ;

        QString linetext ;
        while( !in.atEnd())
        {
            linetext = in.readLine();
            progress.setValue(cline++) ;

            mDocument->Insert(linetext.toUtf8().constData() ) ;

            mDocument->Insert(HARD_RETURN) ;

            QApplication::processEvents() ;

        }

        retval = true ;
    }
/*
    wxFFile in ;

    in.Open(filename, "r") ;
    if(in.IsOpened())
    {

        // @todo - read the whole file into a QString, to get UTF-8 conversion.  Not the best idea!
        QString temp ;
        bool done = in.ReadAll(&temp, wxConvUTF8) ;
        if(done == true)
        {
            long filesize = temp.Length() ;


            wxProgressDialog progress("Loading File as UTF-8 plain text...", "Loading", filesize, NULL, wxPD_APP_MODAL| wxPD_AUTO_HIDE | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME) ;
            wxYield() ;

            long modulo = filesize / 100 ;
            if(modulo == 0)
            {
                modulo = 1 ;
            }

            for(long loop = 0; loop < filesize; loop++)
            {
                if(loop % modulo == 0)
                {
                    QString temp ;
                    temp.Printf("%d %%", (long)((float)loop / (float)filesize * 100.0)) ;
                    progress.Update(loop, temp) ;
                    wxYield() ;
                }

                QString c = temp[loop] ;       // this needs to be QString rather an QChar because of compiler ambiguity
                if(c == '\n')           // convert line endings for internal use
                {
                    c = HARD_RETURN ;
                    mEditor->Insert(c) ;
                }

                if(c != HARD_RETURN)        // looks weird but needed
                {
                    mEditor->Insert(c) ;
                }
            }

            retval = true ;
        }
    }
*/
    return retval ;
}


bool cTextFile::SaveFile(QString filename, POSITION_T length)
{
    bool retval = true ;
    QFile outfile(filename) ;

    if(outfile.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        QTextStream out(&outfile) ;
        out.setCodec("UTF-8") ;

        for(POSITION_T loop = 0; loop < length; loop++)
        {
            string ch = mDocument->GetChar(loop) ;
            if(ch[0] == HARD_RETURN && ch.length() == 1)
            {
                // convert hard returns to CR/LF pairs
                ch[0] = 13 ;
                out << ch.c_str() ;
                ch = 10 ;
                out << ch.c_str() ;
            }
            else
            {
                // normal file write
                out << ch.c_str() ;
            }
        }
    }
    return retval ;
}


bool cTextFile::CanLoad(void)
{
    return true ;
}


bool cTextFile::CanSave(void)
{
    return true ;
}


std::string cTextFile::GetExtensions(void)
{
    return "All Files (*.*)" ;
}


int cTextFile::GetLineCount(QTextStream &in)
{
    int linecount = 0 ;
    QString line ;
    while( !in.atEnd())
    {
        line = in.readLine();
        linecount++;
    }

    in.seek(0) ;
    return linecount ;
}

/// @}
