#include "file.h"

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

/// @ingroup File
/// @{

cFile::cFile(cEditorCtrl *editor)
{
    mEditor = editor ;
    mDocument = &mEditor->mDocument ;
}


void cFile::UpdateProgress(int percent)
{
    if(mEditor != nullptr)
    {
        mEditor->FileIOProgress(percent) ;
    }
}

/// @}
