#ifndef CTEXTFILE_H
#define CTEXTFILE_H

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

#include <QTextStream>

#include "src/core/document/document.h"
#include "file.h"

/// @ingroup Editor
/// @{

class cTextFile : public cFile
{
public:
    cTextFile(cEditorCtrl *editor = nullptr);
    ~cTextFile();

    bool CheckType(QString filename) ;

    bool LoadFile(QString filename) ;
    bool SaveFile(QString filename, POSITION_T size) ;

    bool CanLoad(void) ;
    bool CanSave(void) ;

    std::string GetExtensions(void) ;

protected:

private:
    int GetLineCount(QTextStream &file) ;
};


/// @}

#endif // CTEXTFILE_H
