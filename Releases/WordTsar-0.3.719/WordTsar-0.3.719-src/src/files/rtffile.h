#ifndef CRTFFILE_H
#define CRTFFILE_H

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

#include "src/core/include/config.h"
#include "src/core/document/document.h"

#include "file.h"


#include "rtf/read/rtfparser.h"

/// @ingroup Editor
/// @{



class cRTFFile : public cFile
{
public:
    cRTFFile(cEditorCtrl *editor);
    ~cRTFFile();

    bool CheckType(std::string filename) ;

    bool LoadFile(std::string filename) ;
    bool SaveFile(std::string filename, POSITION_T size) ;

    bool CanLoad(void) ;
    bool CanSave(void) ;

    std::string GetExtensions(void) ;

};


/// @}



#endif // CWORDSTARFILE_H
