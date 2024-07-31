#ifndef CTEST_H
#define CTEST_H

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

#include "src/core/include/config.h"

#ifdef DO_TEST

#include <string>

#include "src/core/document/document.h"
//#include "../files/file.h"
//#include "../files/wordstarfile.h"

using namespace std ;

class cTest
{
    public:
        cTest();
        virtual ~cTest();

        void StartTest(void) ;

    protected:

    private:
//        string GetBuffer(cBuffer &buffer) ;
        string GetDocBuffer(cDocument &document) ;
//        void TestBuffer(long &total, long &pass, long &fail, long &skip) ;
        void TestDocument(long &total, long &pass, long &fail, long &skip) ;
//        void TestWordStarFile(long &total, long &pass, long &fail, long &skip) ;
//        void TestLayout(long &total, long &pass, long &fail, long &skip) ;
};

#endif // DO_TEST

#endif // CTEST_H
