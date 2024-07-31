#ifndef CWORDSTARFILE_H
#define CWORDSTARFILE_H

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
#include <QTextStream>

#include <vector>
#include <string>

#include "../core/document/doctstructs.h"
//#include "doctstructs.h"
#include "file.h"

#include "../core/document/document.h"
#include "../gui/editor/editorctrl.h"

/// @ingroup File
/// @{

                                // uncommented means not done
enum eSequence
{
    SEQ_HEADER,                 // done
    SEQ_COLOR,                  // done
    SEQ_FONT,                   // done
    SEQ_FOOTNOTE,               // PARTIAL
    SEQ_ENDNOTE,
    SEQ_ANNOTATION,
    SEQ_COMMENT,
    SEQ_RESERVED1,
    SEQ_RESERVED2,
    SEQ_TAB,                    // PARTIAL (needs testing)
    SEQ_RESERVED3,
    SEQ_ENDOFPAGE,
    SEQ_PAGEOFFSET,
    SEQ_PARAGRAPHNUMBER,
    SEQ_INDEXENTRY,
    SEQ_PRINTERCONTROL,
    SEQ_GRAPHICS,
    SEQ_PARAGRAPHSTYLE,
    SEQ_RESERVED4,
    SEQ_RESERVED5,
    SEQ_RESERVED6,
    SEQ_ALTFONT,
    SEQ_TRUNCATE,
    SEQ_JAPANESE,

    SEQ_NEWCOLOR                        // this is NOT a standard wordstar sequence
} ;


class cWordstarFile : public cFile
{
public:
    cWordstarFile(cEditorCtrl *editor = nullptr);
    ~cWordstarFile();

    bool CheckType(std::string filename) ;

    bool LoadFile(std::string filename) ;
    bool SaveFile(std::string filename, POSITION_T size) ;

    bool CanLoad(void) ;
    bool CanSave(void) ;

    std::string GetExtensions(void) ;

    void HandleChar(unsigned char c, size_t loop) ;

    bool GetFontMatchList(long count, std::string &wsfont, std::string &sysfont) ;

private :
    void HandleSequence(size_t &loop) ;
    
    void InsertExtendedChar(unsigned char c) ;
    
    void WriteSpecialFile(QTextStream &out, char var) ;

//    Qstd::string GetSystemFontname(Qstd::string name, bool fixedwidth, QFontEncoding encoding) ;
//    void BuildFontList(void) ;

    size_t uiLevenshteinDistance(const std::string &s1, const std::string &s2) ;

    void ParseNote(std::string sequence, char type) ;
    void InsertFootnote(void) ;
    void InsertEndnote(void) ;
    void ParseComment(std::string sequence) ;

protected:

private:
    FILE *mFile ;
    bool mInIndex ;                     ///< true if we are in an index word, else false.
    std::string mIndexWord ;               ///< the word we need to index

    std::vector<std::string>mFontNames ;        ///< Wordstar font names
//    std::vector<Qstd::string>mSysFontName ;  ///< closest matching system font name

    bool mExtendedChar ;                ///< true if we've seen a STYLE_EXTSTART recently

};


/// @}

#endif // CWORDSTARFILE_H
