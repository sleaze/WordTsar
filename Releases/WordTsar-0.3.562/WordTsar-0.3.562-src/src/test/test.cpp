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


#include <stdio.h>
#include <iostream>
#include <chrono>

#include <stdarg.h>

//#include "utf8-cpp/utf8.h"
//#include "cpp-unicodelib-master/unicodelib.h"       // CPP-UNICODELIB does all the unicode stuff I need, but doesn't use iterators... it expects char32_t arrays
//#include "cpp-unicodelib-master/unicodelib_encodings.h"
#include "utf8proc-2.5.0/utf8proc.h"

#include "test.h"

#ifdef DO_TEST

//#include "../gui/layout/layout.h"
//#include "../gui/editor/editorctrl.h"


#define RST  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define FRED(x) KRED x RST
#define FGRN(x) KGRN x RST
#define FYEL(x) KYEL x RST
#define FBLU(x) KBLU x RST
#define FMAG(x) KMAG x RST
#define FCYN(x) KCYN x RST
#define FWHT(x) KWHT x RST

#ifndef _WIN32
#include <unistd.h>
#endif
#include <ios>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;


void mem_usage(void)
{
#ifndef _WIN32
   double vm_usage, resident_set ;
   vm_usage = 0.0;
   resident_set = 0.0;
   ifstream stat_stream("/proc/self/stat",ios_base::in); //get info from proc directory
   //create some variables to get info
   string pid, comm, state, ppid, pgrp, session, tty_nr;
   string tpgid, flags, minflt, cminflt, majflt, cmajflt;
   string utime, stime, cutime, cstime, priority, nice;
   string O, itrealvalue, starttime;
   unsigned long vsize;
   long rss;
   stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt >> utime >> stime >> cutime >> cstime >> priority >> nice >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest
   stat_stream.close();
   long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // for x86-64 is configured to use 2MB pages
   vm_usage = vsize / 1024.0;
   resident_set = rss * page_size_kb;

   printf("       virt memory %.2f   resident %.2f", vm_usage, resident_set) ;
#endif
    printf("\n");
}

void dump_buffer(cDocument &document)
{
    return ;

    POSITION_T len = document.GetTextSize() ;
    if(len > 32)
    {
        len = 32 ;
    }
    for(POSITION_T l = 0; l < len; l++)
    {
       string grapheme = document.GetChar(l) ;
       if(grapheme[0] == HARD_RETURN)
       {
           grapheme[0] = ' ' ;
       }
       printf("%s, ", grapheme.c_str()) ;
    }
    printf("\n\n") ;
}

const short WS_BUFFER_SIZE = 166 ;
const unsigned char WS_TEST_COLOR = 5 ;

CHAR_T gBuffer[WS_BUFFER_SIZE] =          // this is what the document buffer should look like after loading the WS test file
    {
             46,  46,  32, 100, 111, 116,  32,  99, 111, 109, 
            109,  97, 110, 100,  13, 114, 101, 103, 117, 108, 
             97, 114,  13,  13,   MARKER_CHAR,  98, 111, 108, 100,   MARKER_CHAR, 
             13,  13,   MARKER_CHAR, 117, 110, 100, 101, 114, 108, 105, 
            110, 101,   MARKER_CHAR,  13,   MARKER_CHAR,   MARKER_CHAR,  98, 111, 108, 100, 
             32, 117, 110, 100, 101, 114, 108, 105, 110, 101, 
              MARKER_CHAR,   MARKER_CHAR,  13,  13,   MARKER_CHAR, 105, 116,  97, 108, 105, 
             99,   MARKER_CHAR,  13,   MARKER_CHAR,   MARKER_CHAR,  98, 111, 108, 100,  32, 
            105, 116,  97, 108, 105,  99,   MARKER_CHAR,   MARKER_CHAR,  13,  13, 
             13,   MARKER_CHAR, 115, 117, 112, 101, 114,   MARKER_CHAR, 115,  99, 
            114, 105, 112, 116,  13,  13,   MARKER_CHAR, 115, 117,  98, 
              MARKER_CHAR, 115,  99, 114, 105, 112, 116,  13,  13,   MARKER_CHAR, 
            115, 116, 114, 105, 107, 101, 116, 104, 114, 111, 
            117, 103, 104,   MARKER_CHAR,  13,  13,   MARKER_CHAR, 105, 110, 100, 
            101, 120,   MARKER_CHAR,  13,  13,   MARKER_CHAR, 116,  97,  98,   MARKER_CHAR, 
             13,  13,   MARKER_CHAR, 102, 111, 110, 116,  32,  99, 104, 
             97, 110, 103, 101,  13,  13
    } ;

double now()
{
  using namespace std::chrono;
  auto now = high_resolution_clock::now();
  return duration_cast<duration<double>>(now.time_since_epoch()).count();
}




cTest::cTest()
{
    //ctor
}

cTest::~cTest()
{
    //dtor
}

#ifdef USE_CHAI
using namespace chaiscript ;
#endif // USE_CHAI

void cTest::StartTest(void)
{
    long total = 0 ;
    long pass = 0 ;
    long fail = 0 ;
    long skip = 0 ;


    printf("Maximum paragraph size is %ld (approx %ld words) with %ld paragraphs per document\n", MAX_BUFFER_SIZE, MAX_BUFFER_SIZE / 5, MAX_BUFFER_SIZE) ;
//    TestBuffer(total, pass, fail, skip) ;
//    printf("\n") ;

    TestDocument(total, pass, fail, skip) ;
    printf("\n") ;

//    TestWordStarFile(total, pass, fail, skip) ;
//    printf("\n") ;

//    TestLayout(total, pass, fail, skip) ;
//    printf("\n") ;


//    printf("Completed") ;



    printf("\n") ;
    printf("Passed tests   : %ld\n", pass) ;
    printf("Failed tests   : %ld\n", fail) ;
    printf("Skipped tests  : %ld\n", skip) ;
    printf("Completed tests: %ld\n", total) ;


//    Catch::Session().run() ;
}

/*
string cTest::GetBuffer(cBuffer &buffer)
{
    string str ;
    POSITION_T len = buffer.GetTextSize() ;
    for(POSITION_T loop = 0; loop < len; loop++)
    {
        str += buffer.GetChar(loop) ;
    }

    return str ;
}
*/

string cTest::GetDocBuffer(cDocument &document)
{
    string str ;
    size_t len = document.GetTextSize() ;
    for(POSITION_T loop = 0; loop < len; loop++)
    {
        str += document.GetChar(loop) ;
    }

    return str ;
}

/*
void cTest::TestBuffer(long &total, long &pass, long &fail, long &skip)
{
    printf("Testing cBuffer...\n") ;

    cBuffer buffer ;

    // make sure we start with a clear buffer
    ++total ;
    auto x = buffer.GetTextSize() ;
    if(x != 0)
    {
        ++fail ;
        printf("**FAIL: BUF-001 Freshly created buffer is not empty %ld", x) ;
    }
    else
    {
        ++pass ;
//        cout << FGRN("PASS") << ": BUF-001 Clear cleared the buffer" << x << "" ;
        printf("  PASS: BUF-001 Freshly created buffer is empty %ld", x) ;
    }
    mem_usage() ;

    /////////////////////////////////////////////////////////////////////////////////////

    // make sure the buffer is still clear
    ++total ;
    buffer.Clear() ;
    x = buffer.GetPosition() ;
    if(x != 0)
    {
        ++fail ;
        printf("**FAIL: BUF-002 Clear did not clear buffer %ld", x) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: BUF-002 Clear cleared the buffer %ld", x) ;
    }
    mem_usage() ;

    /////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    buffer.Insert('a') ;
    x = buffer.GetPosition() ;
    if(x != 1)
    {
        ++fail ;
        printf("**FAIL: BUF-003 Insert 1 character GetPosition %ld", x) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: BUF-003 Insert 1 character GetPosition %ld", x) ;
    }
    mem_usage() ;


    /////////////////////////////////////////////////////////////////////////////////////

    // make sure we have a text size of 1
    ++total ;
    x = buffer.GetTextSize() ;
    if(x != 1)
    {
        ++fail ;
        printf("**FAIL: BUF-004 Text buffer size sb 1  is %ld", x) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: BUF-004 Text buffer size sb 1 is %ld", x) ;
    }
    mem_usage() ;

    /////////////////////////////////////////////////////////////////////////////////////

    CHAR_T ch = 'b' ;
    for(auto loop = 0; loop < 10; ++loop)
    {
        buffer.Insert(ch) ;
        ch += 1 ;
    }

    ++total ;
    x = buffer.GetPosition() ;
    if(x != 11)
    {
        ++fail ;
        printf("**FAIL: BUF-005 Insert 10 character GetPosition sb 11 is %ld", x) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: BUF-005 Insert 1 character GetPosition sb 11 is %ld", x) ;
    }
    mem_usage() ;

    /////////////////////////////////////////////////////////////////////////////////////

    // make sure we have a text size of 11
    ++total ;
    x = buffer.GetTextSize() ;
    if(x != 11)
    {
        ++fail ;
        printf("**FAIL: BUF-006 Text buffer size sb 11 is %ld", x) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: BUF-006 Text buffer size sb 11 is %ld", x) ;
    }
    mem_usage() ;

    /////////////////////////////////////////////////////////////////////////////////////

    // check contents of buffer

    ++total ;
    ch = 'a' ;
    auto tfail = 0 ;
    for(POSITION_T loop = 0; loop < 11; ++loop)
    {
        CHAR_T ch1 = buffer.GetChar(loop) ;
        if(ch != ch1 )
        {
            ++ tfail ;
        }
        ++ ch ;
    }

    if(tfail != 0)
    {
        ++fail ;
        printf("**FAIL: BUF-007 Text buffer does not match abcdefghijk - ") ;

        for(POSITION_T loop = 0 ; loop < 12; loop++)
        {
            printf("%c", buffer.GetChar(loop)) ;
        }
//        printf("") ;
    }
    else
    {
        ++pass ;
        printf("  PASS: BUF-007 Text buffer matches abcdefghijk") ;
    }
    mem_usage() ;



    /////////////////////////////////////////////////////////////////////////////////////

    // make sure the buffer is still clear
    ++total ;
    buffer.Clear() ;
    x = buffer.GetTextSize() ;
    if(x != 0)
    {
        ++fail ;
        printf("**FAIL: BUF-008 Clear did not clear buffer %ld", x) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: BUF-008 Clear cleared the buffer %ld", x) ;
    }
    mem_usage() ;



    /////////////////////////////////////////////////////////////////////////////////////

    // make sure the position is 0
    ++total ;
    x = buffer.GetPosition() ;
    if(x != 0)
    {
        ++fail ;
        printf("**FAIL: BUF-009 Clear did not get position as 0 %ld", x) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: BUF-009 Clear got position of zero %ld", x) ;
    }
    mem_usage() ;

    string text = "The quick brown fox jumped over the lazy dog\n" ;
    auto num = 44000 ;  
//    num = 4000 ;
    size_t s = text.size() ;

    /////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    auto start = now() ;
    for(auto loop1 = 0; loop1 < num ; ++loop1)
    {
        for(size_t loop = 0; loop < s; ++loop)
        {
            buffer.Insert(text[loop]) ;
        }
    }
    auto end = now() ;

    x = buffer.GetTextSize() ;
    if( x != text.size() * num)
    {
        ++fail ;
        printf("**FAIL: BUF-010 inserted %ld characters (for novels, 396,000 words), but have %ld time %g ( %g cps)", text.size() * num, x, end - start, ((text.size() * num) / (end - start))) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: BUF-010 inserted %ld characters (four novels, 396,000 words) time %g ( %g cps)", text.size() * num, end - start, ((text.size() * num) / (end - start))) ;
    }
    mem_usage() ;


    /////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    POSITION_T y = x / 2 ;
    buffer.SetPosition(y) ;
    x = buffer.GetPosition() ;
    if(y != x )
    {
        ++fail ;
        printf("**FAIL: BUF-011 set position to %ld but have %ld", y, x) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: BUF-011 set position to %ld worked", y) ;
    }
    mem_usage() ;


    /////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    x = buffer.GetTextSize() / 2 - 10 ;
    auto z = buffer.GetTextSize() - x ;

    start = now() ;
    buffer.Delete(y, x) ;
    end = now() ;

    x = buffer.GetTextSize() ;
    if(x != z)
    {
        ++fail ;
        printf("**FAIL: BUF-012 Delete should leave %ld characters, but left %ld time %g", z, x, end - start) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: BUF-012 Delete %ld charcaters worked time %g (%g cps)", x, end - start, x / (end - start)) ;
    }
    mem_usage() ;


    ///////////////////////////////////////////////////////////////////////////////////////

    // clear the buffer and start checking integrity of info
    buffer.Clear() ;
    text = "1234567890ABCDEFGHIJ" ;
    for(POSITION_T loop = 0; loop < text.size(); ++loop)
    {
        buffer.Insert(text[loop]) ;
    }

    ++total ;
    POSITION_T tpos = 0 ;
    buffer.SetPosition(tpos) ;
    buffer.Insert('0') ;
    text = "0" + text ;

    auto str = GetBuffer(buffer) ;
    if(text != str)
    {
        ++fail ;
        printf("**FAIL: BUF-013 Insert into empty buffer failed sb %s is %s", text.c_str(), str.c_str()) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: BUF-013 Insert into empty buffer worked (%s).", str.c_str()) ;
    }
    mem_usage() ;

    ///////////////////////////////////////////////////////////////////////////////////////


    ++total ;
    tpos = 0 ;
    buffer.SetPosition(tpos) ;
    buffer.Insert('0') ;
    text = "0" + text ;

    str = GetBuffer(buffer) ;
    if(text != str)
    {
        ++fail ;
        printf("**FAIL: BUF-014 Insert beginning of buffer failed sb %s is %s", text.c_str(), str.c_str()) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: BUF-014 Insert beginning of buffer worked (%s).", str.c_str()) ;
    }
    mem_usage() ;

    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    POSITION_T px = buffer.GetTextSize() ;
    buffer.SetPosition(px) ;
    buffer.Insert('A') ;
    text = text + 'A' ;
    str = GetBuffer(buffer) ;
    if(text != str)
    {
        ++fail ;
        printf("**FAIL: BUF-015 Insert end of buffer failed sb HIJA is %s", str.c_str()) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: BUF-015 Insert end of buffer worked (%s).", str.c_str()) ;
    }
    mem_usage() ;



    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    px = 10 ;
    buffer.SetPosition(px) ;
    buffer.Insert('-') ;
    text.insert(10, "-") ;
    str = GetBuffer(buffer) ;
    if(text != str)
    {
        ++fail ;
        printf("**FAIL: BUF-016 Insert middle of buffer failed sb %s is %s", text.c_str(), str.c_str()) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: BUF-016 Insert middle of buffer worked (%s).", str.c_str()) ;
    }
    mem_usage() ;


    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    px = 0 ;
    buffer.SetPosition(px) ;
    buffer.Delete(px, 1) ;
    text.erase(px, 1) ;
    str = GetBuffer(buffer) ;
    if(text != str)
    {
        ++fail ;
        printf("**FAIL: BUF-017 delete front of buffer failed sb %s is %s", text.c_str(), str.c_str()) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: BUF-017 delete front of buffer worked. (%s).", str.c_str()) ;
    }
    mem_usage() ;


    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    px = buffer.GetTextSize() ;
    buffer.SetPosition(px) ;
    buffer.Delete(px - 1, 1) ;
    text.erase(px - 1, 1) ;
    str = GetBuffer(buffer) ;
    if(text != str)
    {
        ++fail ;
        printf("**FAIL: BUF-018 delete back of buffer failed sb %s is %s", text.c_str(), str.c_str()) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: BUF-018 delete back of buffer worked (%s).", str.c_str()) ;
    }
    mem_usage() ;


    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    px = 10 ;
    buffer.SetPosition(px) ;
    buffer.Delete(px, 1) ;
    text.erase(px, 1) ;
    str = GetBuffer(buffer) ;
    if(text != str)
    {
        ++fail ;
        printf("**FAIL: BUF-019 delete middle of buffer failed sb %s is %s", text.c_str(), str.c_str()) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: BUF-019 delete middle of buffer worked (%s).", str.c_str()) ;
    }
    mem_usage() ;


    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    px = buffer.GetTextSize() + 2 ;
    buffer.SetPosition(px) ;
    ch = buffer.GetChar(px) ;
    if(ch != 0)
    {
        ++fail ;
        printf("**FAIL: BUF-020 GetChar past end of buffer did not return 0") ;
    }
    else
    {
        ++pass ;
        printf("  PASS: BUF-020 GetChar past end of buffer worked.") ;
    }
    mem_usage() ;


    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    px = buffer.GetTextSize() ;
    buffer.SetPosition(0) ;
    for(size_t loop = 0; loop < px; loop++)
    {
        buffer.Delete(0, 1) ;
    }
    px = buffer.GetTextSize() ;
    str = GetBuffer(buffer) ;
    if(px != 0)
    {
        ++fail ;
        printf("**FAIL: BUF-021 Delete entire buffer failed %s", str.c_str()) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: BUF-021 Delete entire buffer buffer worked  %s", str.c_str()) ;
    }
    mem_usage() ;


    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    buffer.Clear() ;
    text = "1234567890ABCDEFGHIJ" ;
    for(POSITION_T loop = 0; loop < text.size(); ++loop)
    {
        buffer.Insert(text[loop]) ;
    }

    buffer.SaveUndo() ;         // save buffer with text in it
    buffer.Clear() ;            // clear the buffer
    if(buffer.GetTextSize() != 0)
    {
        printf("Can't test undo\n") ;
    }
    buffer.Undo() ;             // undo the clear

    str  = GetBuffer(buffer) ;
    if(text != str)
    {
        ++fail ;
        printf("**FAIL: BUF-022 Undo failed %s", str.c_str()) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: BUF-022 Undo worked  %s", str.c_str()) ;
    }
    mem_usage() ;


//    buffer.ShrinkDeques() ;     // for code coverage, I'm assuming the STL works

}
*/

void cTest::TestDocument(long &total, long &pass, long &fail, long &skip)
{
    printf("Testing cDocument...\n") ;

    cDocument document ;
    document.SetShowControl(SHOW_ALL) ;

    ++total ;
    POSITION_T len = document.GetTextSize() ;
    if(len != 1)            // length is one because of the ^Z
    {
        ++fail ;
        printf("**FAIL: DOC-001 Freshly created document should be empty, but isn't. It's  %ld", len) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-001 Freshly created document is empty") ;
    }
    mem_usage() ;
    dump_buffer(document) ;

    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    bool resp = document.Insert('B') ;
    if(resp == false)
    {
        ++fail ;
        printf("**FAIL: DOC-002 Inserting a charcater failed") ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-002 Inserting a character passed") ;
    }
    mem_usage() ;
    dump_buffer(document) ;

    /////////////////////////////////////////////////////////////////////////////////////

    // make sure the buffer is still clear
    ++total ;
    document.Clear() ;
    len = document.GetTextSize() ;
    if(len != 1)            // 1 because of the ^Z
    {
        ++fail ;
        printf("**FAIL: DOC-003 Cleared document buffer should be empty, but isn't %d") ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-003 Cleared document buffer is empty") ;
    }
    mem_usage() ;
    dump_buffer(document) ;

    /////////////////////////////////////////////////////////////////////////////////////



    ++total ;
    document.Insert('a') ;
    POSITION_T x = document.GetPosition() ;
    if(x != 1)
    {
        ++fail ;
        printf("**FAIL: DOC-004 Insert 1 character GetPosition %ld", x) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-004 Insert 1 character GetPosition %ld", x) ;
    }
    mem_usage() ;
    dump_buffer(document) ;

    /////////////////////////////////////////////////////////////////////////////////////

    // make sure we have a text size of 1
    ++total ;
    x = document.GetTextSize() ;
    if(x != 2)      // The ^Z
    {
        ++fail ;
        printf("**FAIL: DOC-005 Document buffer size sb 2  is %ld", x) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-005 Document buffer size sb 2 is %ld", x) ;
    }
    mem_usage() ;
    dump_buffer(document) ;

    /////////////////////////////////////////////////////////////////////////////////////

    string ch = "b" ;
    for(auto loop = 0; loop < 10; ++loop)
    {
        document.Insert(ch) ;
        ch[0] += 1 ;
    }

    ++total ;
    x = document.GetPosition() ;
    if(x != 11)
    {
        ++fail ;
        printf("**FAIL: DOC-006 Insert 10 character GetPosition sb 11 is %ld", x) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-006 Insert 10 character GetPosition sb 11 is %ld", x) ;
    }
    mem_usage() ;
    dump_buffer(document) ;

    /////////////////////////////////////////////////////////////////////////////////////

    // make sure we have a text size of 11
    ++total ;
    x = document.GetTextSize() ;
    if(x != 12)     // the ^Z
    {
        ++fail ;
        printf("**FAIL: DOC-007 Document buffer size sb 12 is %ld", x) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-007 Document buffer size sb 12 is %ld", x) ;
    }
    mem_usage() ;
    dump_buffer(document) ;

    /////////////////////////////////////////////////////////////////////////////////////

    // check contents of buffer

    ++total ;
    ch = "a" ;
    auto tfail = 0 ;
    for(POSITION_T loop = 0; loop < 11; ++loop)
    {
        string ch1 = document.GetChar(loop) ;
        if(ch[0] != ch1[0] )
        {
            ++ tfail ;
        }
        ++ ch[0] ;
    }

    if(tfail != 0)
    {
        ++fail ;
        printf("**FAIL: DOC-008 Document buffer does not match abcdefghijk - ") ;

        for(POSITION_T loop = 0 ; loop < 12; loop++)
        {
            string grapheme = document.GetChar(loop);
            printf("%c", grapheme[0]) ;
        }
        printf("") ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-008 Document buffer matches abcdefghijk ") ;
    }
    mem_usage() ;
    dump_buffer(document) ;



    /////////////////////////////////////////////////////////////////////////////////////

    // make sure the buffer is still clear
    ++total ;
    document.Clear() ;
    x = document.GetTextSize() ;
    if(x != 1)      // the ^Z
    {
        ++fail ;
        printf("**FAIL:DOC-009 Clear did not clear buffer %ld", x) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-009 Clear cleared the buffer %ld", x) ;
    }
    mem_usage() ;
    dump_buffer(document) ;



    /////////////////////////////////////////////////////////////////////////////////////

    // make sure the position is 0
    ++total ;
    x = document.GetPosition() ;
    if(x != 0)
    {
        ++fail ;
        printf("**FAIL: DOC-010 Clear did not get position as 0 %ld", x) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-010 Clear got position of zero %ld", x) ;
    }
    mem_usage() ;

//    string text = "The quick brown fox jumped over the lazy dog\r" ;
//    string text = "The brown\r" ;
//    string text = "The quick  brown fox jumped over the lazy dog" ;

    // these strings are the same, except one is escape sequences
//    string text = u8"काचं शक्नोम्यत्तुम् । नोपहिनस्ति माम् ॥\r" ;
    // windows won't display utf8 on it's console, but I use it here to use Visual Studio's profiler on the UTF code.
    string text = "\xe0\xa4\x95\xe0\xa4\xbe\xe0\xa4\x9a\xe0\xa4\x82\x20\xe0\xa4\xb6\xe0\xa4\x95\xe0\xa5\x8d\xe0\xa4\xa8\xe0\xa5\x8b\xe0\xa4\xae\xe0\xa5\x8d\xe0\xa4\xaf\xe0\xa4\xa4\xe0\xa5\x8d\xe0\xa4\xa4\xe0\xa5\x81\xe0\xa4\xae\xe0\xa5\x8d\x20\xe0\xa5\xa4\x20\xe0\xa4\xa8\xe0\xa5\x8b\xe0\xa4\xaa\xe0\xa4\xb9\xe0\xa4\xbf\xe0\xa4\xa8\xe0\xa4\xb8\xe0\xa5\x8d\xe0\xa4\xa4\xe0\xa4\xbf\x20\xe0\xa4\xae\xe0\xa4\xbe\xe0\xa4\xae\xe0\xa5\x8d\x20\xe0\xa5\xa5\r";

    auto num = 44000 ;
    num = 1000 ;
    size_t s = text.size() ;
//printf("unicode string len in codepoints %ld   byte string len %ld\n", unicode::utf8::codepoint_count(text), s) ; //    utf8::distance(text.begin(), text.end()), s) ;
    vector<POSITION_T> offsets ;
printf("utf8proc string len in graphemes  %ld  byte string len %ld\n", document.GraphemeCount(text, offsets), s) ;
printf("\n\n UTF8 %s \n\n", text.c_str()) ;

    /////////////////////////////////////////////////////////////////////////////////////

    size_t textsize = document.GraphemeCount(text, offsets);
    ++total ;
    auto start = now() ;

    for(auto loop1 = 0; loop1 < num ; ++loop1)
    {
//        for(size_t loop = 0; loop < s; ++loop)
//        {
//            document.Insert(text[loop]) ;
//        }
        document.Insert(text);
    }
    auto end = now() ;

    x = document.GetTextSize() ;
    if( x - 1 != textsize * num)  // - 1 for ^Z
    {
        ++fail ;
        printf("**FAIL: DOC-011 inserted %ld graphemes (four novels, %d words), but have %ld time %g  (%ld cps)", textsize * num, 9 * num, x - 1, end - start, (unsigned long)((text.size() * num) / (end - start))) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-011 inserted %ld graphemes (four novels, %d words) time %g  (%ld cps)", x, 9 * num, end - start, (unsigned long)((text.size() * num) / (end - start))) ;
    }
    mem_usage() ;
    dump_buffer(document) ;
/*
    // this is just a timing thing, not a test
    start = now() ;
    document.SetPosition(0) ;
    for(auto loop = 0 ; loop < 10000; loop++)
    {
        document.Insert('A') ;
    }
    end=now() ;
    printf("Insert 10000 at front time is %g  (%g cps)", end - start, 10000 / (end - start)) ;
    
    // this is just a timing thing, not a test
    start = now() ;
    document.SetPosition(0) ;
    for(auto loop = 0 ; loop < 100000; loop++)
    {
        document.Insert('A') ;
    }
    end=now() ;
    printf("Insert 100000 at front time is %g  (%g cps)", end - start, 100000 / (end - start)) ;
*/

    /////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    POSITION_T y = x / 2 ;
    document.SetPosition(y) ;
    x = document.GetPosition() ;
    if(y != x )
    {
        ++fail ;
        printf("**FAIL: DOC-012 set position to %ld but have %ld", y, x) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-012 set position to %ld worked", y) ;
    }
    mem_usage() ;
    dump_buffer(document) ;


    /////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    x = document.GetTextSize() / 2 - 10 ;
    auto z = document.GetTextSize() - x ;

    start = now() ;
    document.Delete(y, x) ;
    end = now() ;

    x = document.GetTextSize() ;
    if(x != z)
    {
        ++fail ;
        printf("**FAIL: DOC-013 Delete should leave %ld characters, but left %ld time %g", z, x, end - start) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-013 Delete %d charcaters worked time %g (%g cps)", x, end - start, x / (end - start)) ;
    }
    mem_usage() ;
    dump_buffer(document) ;


    ///////////////////////////////////////////////////////////////////////////////////////

    // clear the buffer and start checking integrity of info
    document.Clear() ;
//    text = "ணோ1234567890ABCDEFGHIJ";
    text = "\xe0\xae\xa3\xe0\xaf\x8b 1234567890ABCDEFGHIJ";
    document.Insert(text);

    ++total ;
    POSITION_T tpos = 0 ;
    document.SetPosition(tpos) ;
    document.Insert('0') ;
    text = "0" + text ;

    auto str = GetDocBuffer(document) ;
    // remove trailing ^Z
    str = str.substr(0, text.length()) ;
    if(text != str)
    {
        ++fail ;
        printf("**FAIL: DOC-014 Insert into empty document failed sb %s is %s", text.c_str(), str.c_str()) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-014 Insert into empty document worked (%s).", str.c_str()) ;
    }
    mem_usage() ;
    dump_buffer(document) ;

    ///////////////////////////////////////////////////////////////////////////////////////


    ++total ;
    tpos = 0 ;
    document.SetPosition(tpos) ;
    document.Insert('0') ;
    text = "0" + text ;

    str = GetDocBuffer(document) ;
    // remove trailing ^Z
    str = str.substr(0, text.length()) ;
    if(text != str)
    {
        ++fail ;
        printf("**FAIL: DOC-015 Insert beginning of document failed sb %s is %s", text.c_str(), str.c_str()) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-015 Insert beginning of document worked (%s).", str.c_str()) ;
    }
    mem_usage() ;
    dump_buffer(document) ;

    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    POSITION_T px = document.GetTextSize() ;
    document.SetPosition(px) ;
    document.Insert('A') ;
    text = text + 'A' ;
    str = GetDocBuffer(document) ;
    // remove trailing ^Z
    str = str.substr(0, text.length()) ;
    if(text != str)
    {
        ++fail ;
        printf("**FAIL: DOC-016 Insert end of document failed sb HIJA is %s", str.c_str()) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-016 Insert end of document worked (%s).", str.c_str()) ;
    }
    mem_usage() ;
    dump_buffer(document) ;



    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    px = 10 ;
    document.SetPosition(px) ;
    document.Insert('-') ;
    text.insert(15, "-") ;
    str = GetDocBuffer(document) ;
    // remove trailing ^Z
    str = str.substr(0, text.length()) ;
    if(text != str)
    {
        ++fail ;
        printf("**FAIL: DOC-017 Insert middle of document failed sb %s is %s", text.c_str(), str.c_str()) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-017 Insert middle of document worked (%s).", str.c_str()) ;
    }
    mem_usage() ;
    dump_buffer(document) ;



    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    px = 0 ;
    document.SetPosition(px) ;
    document.Delete(px, 1) ;
    text.erase(px, 1) ;
    str = GetDocBuffer(document) ;
    // remove trailing ^Z
    str = str.substr(0, text.length()) ;
    if(text != str)
    {
        ++fail ;
        printf("**FAIL: DOC-018 delete front of document failed sb %s is %s", text.c_str(), str.c_str()) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-018 delete front of document worked. (%s).", str.c_str()) ;
    }
    mem_usage() ;
    dump_buffer(document) ;



    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    px = document.GetTextSize() - 1 ;
    document.SetPosition(px) ;
    document.Delete(px, 1) ;
    text.erase(text.size() - 1, 1) ;    
    str = GetDocBuffer(document) ;
    // remove trailing ^Z
    str = str.substr(0, text.length()) ;
    if(text != str)
    {
        ++fail ;
        printf("**FAIL: DOC-019 delete back of document failed sb %s is %s", text.c_str(), str.c_str()) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-019 delete back of document worked (%s).", str.c_str()) ;
    }
    mem_usage() ;
    dump_buffer(document) ;


    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    px = 10 ;
    document.SetPosition(px) ;
    document.Delete(px, 1) ;
    text.erase(15, 1) ;            // hardcoded.. offset 15 in string should be position 10 in UTF8
    str = GetDocBuffer(document) ;
    // remove trailing ^Z
    str = str.substr(0, text.length()) ;
    if(text != str)
    {
        ++fail ;
        printf("**FAIL: DOC-020 delete middle of document failed sb %s is %s", text.c_str(), str.c_str()) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-020 delete middle of document worked (%s).", str.c_str()) ;
    }
    mem_usage() ;
    dump_buffer(document) ;


    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    px = document.GetTextSize() + 2 ;
    document.SetPosition(px) ;
    ch = document.GetChar(px) ;
    if(ch.length() != 0)
    {
        ++fail ;
        printf("**FAIL: DOC-021 GetChar past end of document did not return 0") ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-021 GetChar past end of document worked.") ;
    }
    mem_usage() ;
    dump_buffer(document) ;


    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    px = document.GetTextSize() ;
    document.SetPosition(0) ;
    for(size_t loop = 0; loop < px; loop++)
    {
        document.Delete(0, 1) ;
    }
    px = document.GetTextSize() ;
    str = GetDocBuffer(document) ;
    PARAGRAPH_T para = document.GetNumberofParagraphs() ;
    if((px != 1) || (para != 1))
    {
        ++fail ;
        printf("**FAIL: DOC-022 Delete entire document failed %s  chars %ld  para %ld", str.c_str(), px, para) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-022 Delete entire document document worked  %s", str.c_str()) ;
    }
    mem_usage() ;
    dump_buffer(document) ;

    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    document.Insert("\xe0\xae\xa3\xe0\xaf\x8b this\xe0\xae\xa3\xe0\xaf\x8b ") ;
    document.Insert(HARD_RETURN) ;
    document.Insert("is") ;
    document.Insert(HARD_RETURN) ;
    document.Insert("a") ;
    document.Insert(HARD_RETURN) ;
    document.Insert("test") ;

    para = document.GetNumberofParagraphs() ;
    if(para != 4)
    {
        ++fail ;
        printf("**FAIL: DOC-023 Number of paragraph is %ld (4)", para) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-023 Number of paragraph is %ld (4)", para) ;
    }
    mem_usage() ;
    dump_buffer(document) ;

    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    document.SetPosition(2) ;
    document.BeginBold() ;
    document.EndBold() ;

    POSITION_T pos = 2 ;
    ch = document.GetChar(pos) ;
    pos = 3 ;
    string ch1 = document.GetChar(pos) ;

    if((ch[0] != STYLE_BOLD) || (ch1[0] != STYLE_BOLD))
    {
        ++fail ;
        printf("**FAIL: DOC-024 Get Character got %d and %d (STYLE_BOLD = %d)", ch[0], ch1[0], STYLE_BOLD) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-024 Get Character got %d and %d (STYLE_BOLD = %d)", ch[0], ch1[0], STYLE_BOLD) ;
    }
    mem_usage() ;
    dump_buffer(document) ;


    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    document.SetPosition(2) ;
    document.BeginItalics() ;
    document.EndItalics() ;

    pos = 2 ;
    ch = document.GetChar(pos) ;
    pos = 3 ;
    ch1 = document.GetChar(pos) ;
    if((ch[0] != STYLE_ITALICS) || (ch1[0] != STYLE_ITALICS))
    {
        ++fail ;
        printf("**FAIL: DOC-025 Get Character got %d and %d (STYLE_ITALICS = %d)", ch[0], ch1[0], STYLE_ITALICS) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-025 Get Character got %d and %d (STYLE_ITALICS = %d)", ch[0], ch1[0], STYLE_ITALICS) ;
    }
    mem_usage() ;
    dump_buffer(document) ;

    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    document.SetPosition(2) ;
    document.BeginUnderline() ;
    document.EndUnderline() ;

    pos = 2 ;
    ch = document.GetChar(pos) ;
    pos = 3 ;
    ch1 = document.GetChar(pos) ;
    if((ch[0] != STYLE_UNDERLINE) || (ch1[0] != STYLE_UNDERLINE))
    {
        ++fail ;
        printf("**FAIL: DOC-026 Get Character got %d and %d (STYLE_UNDERLINE = %d)", ch[0], ch1[0], STYLE_UNDERLINE) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-026 Get Character got %d and %d (STYLE_UNDERLINE = %d)", ch[0], ch1[0], STYLE_UNDERLINE) ;
    }
    mem_usage() ;
    dump_buffer(document) ;

    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    document.SetPosition(2) ;
    document.BeginSubscript() ;
    document.EndSubscript() ;

    pos = 2 ;
    ch = document.GetChar(pos) ;
    pos = 3 ;
    ch1 = document.GetChar(pos) ;
    if((ch[0] != STYLE_SUBSCRIPT) || (ch1[0] != STYLE_SUBSCRIPT))
    {
        ++fail ;
        printf("**FAIL: DOC-027 Get Character got %d and %d (STYLE_SUBSCRIPT = %d)", ch[0], ch1[0], STYLE_SUBSCRIPT) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-027 Get Character got %d and %d (STYLE_SUBSCRIPT = %d)", ch[0], ch1[0], STYLE_SUBSCRIPT) ;
    }
    mem_usage() ;
    dump_buffer(document) ;

    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    document.SetPosition(2) ;
    document.BeginSuperscript() ;
    document.EndSuperscript() ;

    pos = 2 ;
    ch = document.GetChar(pos) ;
    pos = 3 ;
    ch1 = document.GetChar(pos) ;
    if((ch[0] != STYLE_SUPERSCRIPT) || (ch1[0] != STYLE_SUPERSCRIPT))
    {
        ++fail ;
        printf("**FAIL: DOC-028 Get Character got %d and %d (STYLE_SUPERSCRIPT = %d)", ch[0], ch1[0], STYLE_SUPERSCRIPT) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-028 Get Character got %d and %d (STYLE_SUPERSCRIPT = %d)", ch[0], ch1[0], STYLE_SUPERSCRIPT) ;
    }
    mem_usage() ;
    dump_buffer(document) ;

    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    document.SetPosition(2) ;
    document.BeginStrikeThrough() ;
    document.EndStrikeThrough() ;

    pos = 2 ;
    ch = document.GetChar(pos) ;
    pos = 3 ;
    ch1 = document.GetChar(pos) ;
    if((ch[0] != STYLE_STRIKETHROUGH) || (ch1[0] != STYLE_STRIKETHROUGH))
    {
        ++fail ;
        printf("**FAIL: DOC-029 Get Character got %d and %d (STYLE_STRIKETHROUGH = %d)", ch[0], ch1[0], STYLE_STRIKETHROUGH) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-029 Get Character got %d and %d (STYLE_STRIKETHROUGH = %d)", ch[0], ch1[0], STYLE_STRIKETHROUGH) ;
    }
    mem_usage() ;
    dump_buffer(document) ;

    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    document.SetPosition(2) ;
    document.BeginIndex() ;
    document.Insert("test") ;
    document.EndIndex() ;

    pos = 2 ;
    ch = document.GetChar(pos) ;
    pos = 7 ;
    ch1 = document.GetChar(pos) ;
    if((ch[0] != STYLE_INDEX) || (ch1[0] != STYLE_INDEX))
    {
        ++fail ;
        printf("**FAIL: DOC-030 Get Character got %d and %d (STYLE_INDEX = %d)", ch[0], ch1[0], STYLE_INDEX) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-030 Get Character got %d and %d (STYLE_INDEX = %d)", ch[0], ch1[0], STYLE_INDEX) ;
    }
    mem_usage() ;
    dump_buffer(document) ;

    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    document.SetPosition(2) ;
    sWSTab tab ;
    tab.type = TAB_TAB ;
    document.InsertTab(tab) ;

    pos = 2 ;
    ch = document.GetChar(pos) ;
    if(ch[0] != STYLE_TAB)
    {
        ++fail ;
        printf("**FAIL: DOC-031 Get Character got %d (STYLE_TAB = %d)", ch[0], STYLE_TAB) ;
    }
    else
    {
        sWSTab tab1 ;
        tab1 = document.GetTab(pos) ;

        if(tab1.type != TAB_TAB)
        {
            ++fail ;
            printf("**FAIL: DOC-031 Get tab type got %d (TAB_TAB = %d)", tab1.type, TAB_TAB) ;
        }
        else
        {
            ++pass ;
            printf("  PASS: DOC-031 Get Character got %d (STYLE_TAB = %d) Get tab type got %d (TAB_TAB = %d)", ch[0], STYLE_TAB, tab1.type, TAB_TAB) ;
        }
    }
    mem_usage() ;
    dump_buffer(document) ;

    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    document.SetPosition(2) ;
    tab.type = TAB_CENTER ;
    document.InsertTab(tab) ;

    pos = 2 ;
    ch = document.GetChar(pos) ;
    if(ch[0] != STYLE_TAB)
    {
        ++fail ;
        printf("**FAIL: DOC-032 Get Character got %d (STYLE_TAB = %d)", ch[0], STYLE_TAB) ;
    }
    else
    {
        sWSTab tab1 ;
        tab1 = document.GetTab(pos) ;

        if(tab1.type != TAB_CENTER)
        {
            ++fail ;
            printf("**FAIL: DOC-032 Get tab type got %d (TAB_CENTER = %d)", tab1.type, TAB_CENTER) ;
        }
        else
        {
            ++pass ;
            printf("  PASS: DOC-032 Get Character got %d (STYLE_TAB = %d) Get tab type got %d (TAB_CENTER = %d)", ch[0], STYLE_TAB, tab1.type, TAB_CENTER) ;
        }
    }
    mem_usage() ;
    dump_buffer(document) ;

    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    document.SetPosition(2) ;
    tab.type = TAB_DECIMAL ;
    document.InsertTab(tab) ;

    pos = 2 ;
    ch = document.GetChar(pos) ;
    if(ch[0] != STYLE_TAB)
    {
        ++fail ;
        printf("**FAIL: DOC-033 Get Character got %d (STYLE_TAB = %d)", ch[0], STYLE_TAB) ;
    }
    else
    {
        sWSTab tab1 ;
        tab1 = document.GetTab(pos) ;

        if(tab1.type != TAB_DECIMAL)
        {
            ++fail ;
            printf("**FAIL: DOC-033 Get tab type got %d (TAB_DECIMAL = %d)", tab1.type, TAB_DECIMAL) ;
        }
        else
        {
            ++pass ;
            printf("  PASS: DOC-033 Get Character got %d (STYLE_TAB = %d) Get tab type got %d (TAB_DECIMAL = %d)", ch[0], STYLE_TAB, tab1.type, TAB_DECIMAL) ;
        }
    }
    mem_usage() ;
    dump_buffer(document) ;

    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    document.SetPosition(2) ;
    tab.type = TAB_RIGHT ;
    document.InsertTab(tab) ;

    pos = 2 ;
    ch = document.GetChar(pos) ;
    if(ch[0] != STYLE_TAB)
    {
        ++fail ;
        printf("**FAIL: DOC-034 Get Character got %d (STYLE_TAB = %d)", ch[0], STYLE_TAB) ;
    }
    else
    {
        sWSTab tab1 ;
        tab1 = document.GetTab(pos) ;

        if(tab1.type != TAB_RIGHT)
        {
            ++fail ;
            printf("**FAIL: DOC-034 Get tab type got %d (TAB_RIGHT = %d)", tab1.type, TAB_RIGHT) ;
        }
        else
        {
            ++pass ;
            printf("  PASS: DOC-034 Get Character got %d (STYLE_TAB = %d) Get tab type got %d (TAB_RIGHT = %d)", ch[0], STYLE_TAB, tab1.type, TAB_RIGHT) ;
        }
    }
    mem_usage() ;
    dump_buffer(document) ;

    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    document.SetPosition(2) ;
    tab.type = TAB_RIGHT1 ;
    document.InsertTab(tab) ;

    pos = 2 ;
    ch = document.GetChar(pos) ;
    if(ch[0] != STYLE_TAB)
    {
        ++fail ;
        printf("**FAIL: DOC-035 Get Character got %d (STYLE_TAB = %d)", ch[0], STYLE_TAB) ;
    }
    else
    {
        sWSTab tab1 ;
        tab1 = document.GetTab(pos) ;

        if(tab1.type != TAB_RIGHT1)
        {
            ++fail ;
            printf("**FAIL: DOC-035 Get tab type got %d (TAB_RIGHT1 = %d)", tab1.type, TAB_RIGHT1) ;
        }
        else
        {
            ++pass ;
            printf("  PASS: DOC-035 Get Character got %d (STYLE_TAB = %d) Get tab type got %d (TAB_RIGHT1 = %d)", ch[0], STYLE_TAB, tab1.type, TAB_RIGHT1) ;
        }
    }
    mem_usage() ;
    dump_buffer(document) ;

    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    document.SetPosition(2) ;
    tab.type = TAB_SOFT ;
    document.InsertTab(tab) ;

    pos = 2 ;
    ch = document.GetChar(pos) ;
    if(ch[0] != STYLE_TAB)
    {
        ++fail ;
        printf("**FAIL: DOC-036 Get Character got %d (STYLE_TAB = %d)", ch[0], STYLE_TAB) ;
    }
    else
    {
        sWSTab tab1 ;
        tab1 = document.GetTab(pos) ;

        if(tab1.type != TAB_SOFT)
        {
            ++fail ;
            printf("**FAIL: DOC-036 Get tab type got %d (TAB_SOFT = %d)", tab1.type, TAB_SOFT) ;
        }
        else
        {
            ++pass ;
            printf("  PASS: DOC-036 Get Character got %d (STYLE_TAB = %d) Get tab type got %d (TAB_SOFT = %d)", ch[0], STYLE_TAB, tab1.type, TAB_SOFT) ;
        }
    }
    mem_usage() ;
    dump_buffer(document) ;

    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    document.SetPosition(2) ;
    sWSColor color ;
    color.colornumber = WS_TEST_COLOR ;
    color.prevcolornumber= 0 ;

    document.InsertColor(color) ;

    pos = 2 ;
    ch = document.GetChar(pos) ;
    if(ch[0] != STYLE_INTERNAL_COLOR)
    {
        ++fail ;
        printf("**FAIL: DOC-037 Get Character got %d (STYLE_INTERNAL_COLOR = %d)", ch[0], STYLE_INTERNAL_COLOR) ;
    }
    else
    {
        bool ok = document.GetColor(pos, color) ;

        if(color.colornumber != WS_TEST_COLOR)
        {
            ++fail ;
            printf("**FAIL: DOC-037 Get color got %d (%d)", color.colornumber, WS_TEST_COLOR) ;
        }
        else
        {
            ++pass ;
            printf("  PASS: DOC-037 Get color got %d (STYLE_INTERNAL_COLOR = %d)", color.colornumber, STYLE_INTERNAL_COLOR) ;
        }
    }
    mem_usage() ;
    dump_buffer(document) ;

    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    document.SetPosition(2) ;
    sInternalFonts font ;
    font.fontname = "Times New Roman" ;
    font.size = 12 ;

    document.InsertFont(font) ;

    pos = 2 ;
    ch = document.GetChar(pos) ;
    if(ch[0] != STYLE_FONT1)
    {
        ++fail ;
        printf("**FAIL: DOC-038 Get Character got %d (STYLE_FONT1 = %d)", ch[0], STYLE_FONT1) ;
    }
    else
    {
        sInternalFonts f ;
        bool ok = document.GetFont(pos, f) ;

        if((f.fontname != font.fontname) || (f.size != font.size))
        {
            ++fail ;
            printf("**FAIL: DOC-038 Get font got different font") ;
        }
        else
        {
            ++pass ;
            printf("  PASS: DOC-038 Get font got the right font") ;
        }
    }

    mem_usage() ;
    dump_buffer(document) ;

    ///////////////////////////////////////////////////////////////////////////////////////
    // sanity check, position 3 should be a color

    ++total ;
    pos = 3 ;

    ch = document.GetChar(pos) ;
    if(ch[0] != STYLE_INTERNAL_COLOR)
    {
        ++fail ;
        printf("**FAIL: DOC-039 Get Character got %d (STYLE_INTERNAL_COLOR = %d)", ch[0], STYLE_INTERNAL_COLOR) ;
    }
    else
    {
        sWSColor color1 ;
        bool ok = document.GetColor(pos, color1) ;

        if(color1.colornumber != WS_TEST_COLOR)
        {
            ++fail ;
            printf("**FAIL: DOC-039 Get color got %d (%d)", color1.colornumber, WS_TEST_COLOR) ;

        }
        else
        {
            ++pass ;
            printf("  PASS: DOC-039 Get Character got %d (STYLE_INTERNAL_COLOR = %d) Get tab type got %d (%d)", color1.colornumber, STYLE_INTERNAL_COLOR, ch1[0], WS_TEST_COLOR) ;
        }
    }
    mem_usage() ;
    dump_buffer(document) ;


    ///////////////////////////////////////////////////////////////////////////////////////
    // this test is here to exercise the cDocument.IncrementAttributes() code

    ++total ;
    document.SetPosition(1) ;
    resp = document.Insert("\xe0\xb2\xaf");
//    resp = document.Insert('A');
    if(resp == false)
    {
        ++fail ;
        printf("**FAIL: DOC-040 Inserting a character failed") ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-040 Inserting a character passed") ;
    }
    mem_usage() ;
    dump_buffer(document) ;

    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    px = 2 ;
    document.SetPosition(px) ;
    document.Delete(px, 2) ;

    px = 2 ;                        // position 2 should now be our color from previous insert
    ch = document.GetChar(px) ;

    if(ch[0] != STYLE_INTERNAL_COLOR)
    {
        ++fail ;
        printf("**FAIL: DOC-041 Deleted color got %d (STYLE_INTERNAL_COLOR = %d)", ch[0], STYLE_INTERNAL_COLOR) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-041 Deleted colort got %d (STYLE_INTERNAL_COLOR = %d) ", ch[0], STYLE_INTERNAL_COLOR) ;
    }
    mem_usage() ;
    dump_buffer(document) ;



    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    px = 2 ;
    document.SetPosition(px) ;
    document.Delete(px, 1) ;

    px = 2 ;                        // position 2 should be out tab from previous insert
    ch = document.GetChar(px) ;

    if(ch[0] != STYLE_TAB)
    {
        ++fail ;
        printf("**FAIL: DOC-042 Deleted tab got %d (STYLE_TAB = %d)", ch[0], STYLE_TAB) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-042 Deleted tab got %d (STYLE_TAB = %d) ", ch[0], STYLE_TAB) ;
    }
    mem_usage() ;
    dump_buffer(document) ;


    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    px = 2 ;
    document.SetPosition(px) ;
    document.Delete(px, 1) ;

    px = 2 ;                        // position 2 should be out tabfrom previous insert
    ch = document.GetChar(px) ;

    if(ch[0] != STYLE_TAB)
    {
        ++fail ;
        printf("**FAIL: DOC-043 Deleted tab got %d (STYLE_TAB = %d)", ch[0], STYLE_TAB) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-043 Deleted tab got %d (STYLE_TAB = %d) ", ch[0], STYLE_TAB) ;
    }
    mem_usage() ;
    dump_buffer(document) ;




    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    px = 24 ;
    ch = document.GetChar(px) ;
    if(ch[0] != STYLE_BOLD)
    {
        ++fail ;
        printf("**FAIL: DOC-044 Checking for bold got %d (STYLE_BOLD = %d)", ch[0], STYLE_BOLD) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-044 Checking for bold got %d (STYLE_BOLD = %d) ", ch[0], STYLE_BOLD) ;
    }
    mem_usage() ;
    dump_buffer(document) ;


    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    document.Clear() ;

    // add paragraphs
    document.Insert("this") ;
    document.Insert(HARD_RETURN) ;
    document.Insert("is") ;
    document.Insert(HARD_RETURN) ;
    document.Insert("a") ;
    document.Insert(HARD_RETURN) ;
    document.Insert("test") ;

    // delete a paragraph
    document.Delete(1, 6) ;

    para = document.GetNumberofParagraphs() ;

    if(para != 3)
    {
        ++fail ;
        printf("**FAIL: DOC-045 Deleting paragraph didn't reduce para count 3 (%ld)", para) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-045 Deleting paragraph worked count 3 (%ld)", para) ;
    }
    mem_usage() ;
    dump_buffer(document) ;



    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    document.Clear() ;

    // add paragraphs
    document.Insert("this is a test") ;
    document.Insert(HARD_RETURN) ;
    document.Insert("test") ;
dump_buffer(document) ;
    document.SetPosition(5) ;
    document.Insert(HARD_RETURN) ;
dump_buffer(document) ;
    para = document.GetNumberofParagraphs() ;

    if(para != 3)
    {
        ++fail ;
        printf("**FAIL: DOC-046 Inserting paragraph didn't increase para count 3 (%ld)", para) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-046 Inserting paragraph worked count 3 (%ld)", para) ;
    }
    mem_usage() ;
    dump_buffer(document) ;




    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    pos = 9 ;
    ch = document.GetChar(pos) ;
    if(ch[0] != STYLE_BOLD)
    {
        ++fail ;
        printf("**FAIL: DOC-047 Inserting paragraph didn't move format data STYLE_BOLD %d (%d)", STYLE_BOLD, ch[0]) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-047 Inserting paragraph moved format(etc) data STYLE_BOLD %d (%d)", STYLE_BOLD, ch[0]) ;
    }
    mem_usage() ;
    dump_buffer(document) ;

    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    document.Clear() ;
    document.BeginBold() ;
    pos = 0 ;
    ch = document.GetChar(pos) ;
    document.Insert("AThisAisAaAtest") ;
    document.SetPosition(1) ;
    document.Insert(HARD_RETURN) ;
    pos = 0 ;
    ch = document.GetChar(pos) ;
    if(ch[0] != STYLE_BOLD)
    {
        ++fail ;
        printf("**FAIL: DOC-048 Inserting paragraph after first bold removed STYLE_BOLD %d (%d)", STYLE_BOLD, ch[0]) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-048 Inserting paragraph after first bold kept STYLE_BOLD %d (%d)", STYLE_BOLD, ch[0]) ;
    }
    mem_usage() ;
    dump_buffer(document) ;

    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
//    document.SetPosition(0) ;
//    document.Insert(HARD_RETURN) ;
    document.Delete(1, 1) ;
    pos = 0 ;
    ch = document.GetChar(pos) ;
    if(ch[0] != STYLE_BOLD)
    {
        ++fail ;
        printf("**FAIL: DOC-049 Delete paragraph before first bold removed STYLE_BOLD %d (%d)", STYLE_BOLD, ch[0]) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-049 Delete paragraph before first bold kept STYLE_BOLD %d (%d)", STYLE_BOLD, ch[0]) ;
    }
    mem_usage() ;
    dump_buffer(document) ;

    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    document.Clear() ;
    document.Insert("This is a test") ;
    document.SaveUndo() ;
    document.Insert(" This is part two") ;
    document.SaveUndo() ;
    document.Insert(" This is part 3") ;

    string one = document.GetParagraphText(0) ;
    // remove trailing ^Z
    one = one.substr(0, one.length() - 1) ;

    document.Undo() ;
    string two = document.GetParagraphText(0) ;
    // remove trailing ^Z
    two = two.substr(0, two.length() - 1) ;
    document.Undo() ;
    string three = document.GetParagraphText(0) ;
    // remove trailing ^Z
    three = three.substr(0, three.length() - 1) ;

//printf("%s\n%s\n%s\n", one.c_str(), two.c_str(), three.c_str()) ;

    if(three != "This is a test" || two != "This is a test This is part two" || one != "This is a test This is part two This is part 3")
    {
        ++fail ;
        printf("**FAIL: DOC-049 Basic Undo failed") ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-049 Basic Undo worked") ;
    }
    mem_usage() ;
    dump_buffer(document) ;


    ///////////////////////////////////////////////////////////////////////////////////////
/*
    ++total ;
    document.Clear() ;
    document.Insert("This is a test\r") ;
    document.SaveUndo() ;
    document.Insert(" This is part two\r") ;
    document.SaveUndo() ;
    document.Insert(" This is part 3\r") ;

    string insert ;
    for(POSITION_T loop = 28; loop < 37; loop++)
    {
        insert += document.GetChar(loop) ;
    }

    document.SetPosition(10) ;
    document.Insert(insert) ;

    one = document.GetParagraphText(0) ;
    two = document.GetParagraphText(1) ;


    if(three != "This is a test" || two != "This is a test This is part two" || one != "This is a test This is part two This is part 3")
    {
        ++fail ;
        printf("\n**FAIL: DOC-048 Basic Undo failed \n1 - %s  \n2 - %s\n", one.c_str(), two.c_str()) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-048 Basic Undo worked") ;
    }
    mem_usage() ;
    dump_buffer(document) ;
*/

    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;

    document.Redo() ;
    two = document.GetParagraphText(0) ;
    document.Redo() ;
    three = document.GetParagraphText(0) ;

printf("%s\n%s\n", two.c_str(), three.c_str()) ;

    if(two != "This is a test" || three != "This is a test This is part two")
    {
        ++fail ;
        printf("**FAIL: DOC-048 Basic Redo failed") ;
    }
    else
    {
        ++pass ;
        printf("  PASS: DOC-048 Basic Redo worked") ;
    }
    mem_usage() ;
    dump_buffer(document) ;
}



/*

void cTest::TestWordStarFile(long &total, long &pass, long &fail)
{
    printf("Testing cWordstarFile...") ;

    cDocument document ;
    cWordstarFile file(&document) ;

    ++total ;
    bool resp = file.CanLoad() ;
    if(resp == false)
    {
        ++fail ;
        printf("**FAIL: WSFILE-001 Should be able to load file") ;
    }
    else
    {
        ++pass ;
        printf("  PASS: WSFILE-001 Says can load a wordstar file") ;
    }


    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    resp = file.CanSave() ;
    if(resp == false)
    {
        ++fail ;
        printf("**FAIL: WSFILE-002 Should be able to save file") ;
    }
    else
    {
        ++pass ;
        printf("  PASS: WSFILE-002 Says can save a wordstar file") ;
    }


    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    resp = file.CheckType("test.ws") ;
    if(resp == false)
    {
        ++fail ;
        printf("**FAIL: WSFILE-003 Can load .ws file") ;
    }
    else
    {
        ++pass ;
        printf("  PASS: WSFILE-003 Can load .ws file") ;
    }


    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    resp = file.CheckType("test.ws3") ;
    if(resp == false)
    {
        ++fail ;
        printf("**FAIL: WSFILE-004 Can load .ws3 file") ;
    }
    else
    {
        ++pass ;
        printf("  PASS: WSFILE-004 Can load .ws3 file") ;
    }


    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    resp = file.CheckType("test.ws4") ;
    if(resp == false)
    {
        ++fail ;
        printf("**FAIL: WSFILE-005 Can load .ws4 file") ;
    }
    else
    {
        ++pass ;
        printf("  PASS: WSFILE-005 Can load .ws4 file") ;
    }


    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    resp = file.CheckType("test.ws5") ;
    if(resp == false)
    {
        ++fail ;
        printf("**FAIL: WSFILE-006 Can load .ws5 file") ;
    }
    else
    {
        ++pass ;
        printf("  PASS: WSFILE-006 Can load .ws5 file") ;
    }


    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    resp = file.CheckType("test.ws6") ;
    if(resp == false)
    {
        ++fail ;
        printf("**FAIL: WSFILE-007 Can load .ws6 file") ;
    }
    else
    {
        ++pass ;
        printf("  PASS: WSFILE-007 Can load .ws6 file") ;
    }


    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    resp = file.CheckType("test.ws7") ;
    if(resp == false)
    {
        ++fail ;
        printf("**FAIL: WSFILE-008 Can load .ws7 file") ;
    }
    else
    {
        ++pass ;
        printf("  PASS: WSFILE-008 Can load .ws7 file") ;
    }


    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    resp = file.CheckType("test.ws8") ;
    if(resp == false)
    {
        ++fail ;
        printf("**FAIL: WSFILE-009 Can load .ws8 file") ;
    }
    else
    {
        ++pass ;
        printf("  PASS: WSFILE-009 Can load .ws8 file") ;
    }


    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    resp = file.CheckType("test.ws-bak") ;
    if(resp == false)
    {
        ++fail ;
        printf("**FAIL: WSFILE-010 Can load .ws-bak file") ;
    }
    else
    {
        ++pass ;
        printf("  PASS: WSFILE-010 Can load .ws-bak file") ;
    }


    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    resp = file.CheckType("test.ws-$$$") ;
    if(resp == false)
    {
        ++fail ;
        printf("**FAIL: WSFILE-011 Can load .ws-$$$ file") ;
    }
    else
    {
        ++pass ;
        printf("  PASS: WSFILE-011 Can load .ws-$$$ file") ;
    }


    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    resp = file.LoadFile("badfilename.ws") ;
    if(resp == true)
    {
        ++fail ;
        printf("**FAIL: WSFILE-012 Attempted to open non-existant file") ;
    }
    else
    {
        ++pass ;
        printf("  PASS: WSFILE-012 Correctly failed to open non existant file") ;
    }

    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    resp = file.LoadFile("testfiles/test.ws") ;
    if(resp == false)
    {
        ++fail ;
        printf("**FAIL: WSFILE-013 Failed to load test Wordstar file") ;
    }
    else
    {
        ++pass ;
        printf("  PASS: WSFILE-013 Loaded test Wordstar file") ;
    }

    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    POSITION_T len = document.GetTextSize() ;
    if(len != WS_BUFFER_SIZE)
    {
        ++fail ;
        printf("**FAIL: WSFILE-014 Buffer contains incorrrect number of glyphs (%ld sb %d)", len, WS_BUFFER_SIZE) ;
        printf("            ") ;
        for(POSITION_T loop = 0; loop < len; loop++)
        {
            CHAR_T ch = document.GetRawBufferChar(loop) ;
//            printf("%3d (%c), ", ch[0], (isprint(ch) ? ch : 32)) ;
            printf("%3d, ", ch) ;
            if((loop + 1) % 10 == 0)
            {
                printf("            ") ;
            }
        }
        printf("") ;
    }
    else
    {
        ++pass ;
        printf("  PASS: WSFILE-014 Buffer contains correct number of glyphs") ;
    }


    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    resp = true ;
    POSITION_T cloop ;
    for(cloop = 0; cloop < WS_BUFFER_SIZE; cloop++)
    {
        CHAR_T ch = document.GetRawBufferChar(cloop) ;
        if(ch != gBuffer[cloop])
        {
            resp = false ;
            break ;
        }
    }

    if(resp == false)
    {
        ++fail ;
        printf("**FAIL: WSFILE-015 Buffer is wrong starting at position %ld", cloop) ;
    }
    else
    {
        ++pass ;
        printf("  PASS: WSFILE-015 Buffer content is correct") ;
    }

    ///////////////////////////////////////////////////////////////////////////////////////

    ++total ;
    resp = file.SaveFile("testfiles/test1.ws", WS_BUFFER_SIZE) ;
    if(resp == false)
    {
        ++fail ;
        printf("**FAIL: WSFILE-016 Failed to save test Wordstar file") ;
    }
    else
    {
        ++pass ;
        printf("  PASS: WSFILE-016 Saved test Wordstar file") ;
    }

}




void cTest::TestLayout(long &total, long &pass, long &fail)
{
    printf("Testing cLayout...") ;

    cDocument document ;
    cEditorCtrl editor ;
    cWordstarFile file(&document) ;
    cLayout layout(&document, &editor) ;
    
// temp
    total = total ;
    pass = pass ;
    fail = fail ;
}

*/
#endif // DOTEST
