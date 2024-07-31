#ifndef CEDITORFILE_H
#define CEDITORFILE_H

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


#include "src/gui/editor/editorctrl.h"
#include "src/core/document/document.h"


/// @ingroup File
/// @{

class cFile
{
public:
//    cFile(cDocument *document);
    cFile(cEditorCtrl *editor = nullptr) ;
    virtual ~cFile() = default;

    cDocument *mDocument;

    virtual bool CheckType(std::string filename) = 0 ;
    virtual bool LoadFile(std::string filename) = 0 ;
    virtual bool SaveFile(std::string filename, POSITION_T size) = 0 ;
    virtual bool CanLoad(void) = 0 ;
    virtual bool CanSave(void) = 0 ;
    virtual std::string GetExtensions(void) = 0 ;

//protected:
    void UpdateProgress(int percent) ;

    cEditorCtrl *mEditor ;

private:
};

/// @}

#endif // CEDITORFILE_H
