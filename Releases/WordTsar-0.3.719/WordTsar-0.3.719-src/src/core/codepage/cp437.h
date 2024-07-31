#ifndef CP_437_H
#define CP_437_H

#include <vector>
#include <../core/document/doctstructs.h>


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




class cCodePage437
{
public :
    cCodePage437(void) ;

    unsigned char toChar(unsigned long utf8char) ;
    unsigned long toUTF8(unsigned char) ;

private :
    // This structure converts between the extended characters of codepage 437
    // and unicode
    std::vector<sExtendedChars> mCodePage437 ;

};


#endif
