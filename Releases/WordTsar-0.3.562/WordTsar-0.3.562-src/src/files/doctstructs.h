#ifndef DOCTSTRUCTS_H_INCLUDED
#define DOCTSTRUCTS_H_INCLUDED

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

#include <stdint.h>

#include <QString>
#include <QFont>

#include "../include/config.h"

#pragma pack(push, 1)

const short WS_SEQ_HEADER = 2 ;                 ///< basically room for the size indicator
const short WS_SEQ_TRAILER = 3 ;                ///< all sequence end with 3 extra bytes that are included in the size




struct sSeqIntro
{
    char start ;
    uint16_t size ;
    char type ;
} ; // __attribute__((packed)) ;


struct sSeqClose
{
    uint16_t size ;
    char finish ;
} ; // __attribute__((packed)) ;


struct sWSHeader
{
    unsigned char version ;                     ///< version of file in hex 0x55 for 5.5 0x60 for 6.0, etc
    char driver[9] ;                            ///< nul terminated driver name for document
    char reserved[2] ;                          ///< reserved
    uint32_t styles ;                           ///< pointer to start of tyles (end of document)
    char reserved1[105] ;                       ///< reserved to buffer to 128 chars (include 0x1d start and end bytes)
} ; // __attribute__((packed)) ;

struct sWSColor
{
    unsigned char colornumber ;                 ///< 0x00 to 0x0F
    unsigned char prevcolornumber ;
};


struct sSeqNewColor                             // this is a non wordstar sequence that allows for full color specification
{
    unsigned char red ;
    unsigned char green ;
    unsigned char blue ;
    unsigned char alpha ;
} ;


struct sWSFont
{
    uint16_t width ;                            ///< in 1/1800 of an inch
    uint16_t height ;                           ///< in 1/1440 of an inch
    uint16_t style ;                            ///< type style
    uint16_t prevwidth ;                        ///< in 1/1800 of an inch
    uint16_t prevheight ;                       ///< in 1/1440 of an inch
    uint16_t prevstyle ;                        ///< type style
};


struct sWSFootnote
{
    uint16_t linecount ;                        ///< line count of footnote
    uint16_t offset ;                           ///< offset of fottnote number TAG (15 bit 15 == 1, other bits define
                                                ///<                                 the offset to an internal sequence with
                                                ///<                                 footnote TAG 0)
    unsigned char unused ;
// text area for footnote
};


struct sWSEndnote
{
    uint16_t linecount ;                        ///< linecount of endnote
    uint16_t offset ;                           ///< offset of endnote tage number
    char unused ;
} ;



struct sWSAnnotation
{
    uint16_t linenumber ;                       ///< line number of annotation
    uint16_t number ;                           ///< offset of TAG annotation
    char unused ;
} ;



struct sWSComment
{
    uint16_t linecount ;                        ///< line count of comment
    uint16_t unused ;
    char conversion ;                           ///< conversion flag
} ;



struct sWSTab
{
    int16_t tabsize ;                           ///< 1/1800 inch
    int16_t abstabsize ;                        ///< 1/1800 inch
    unsigned char type ;                        ///< tab type (tab, center just, right just, etc) (use char, not enum, to get size right)
    char size ;                                 ///< 1/10 inch
} ; // __attribute__((packed)) ;


struct sWSParagraphNumber
{
    char levelincrease ;                        ///< 0-same level  1-move forward inlevel (2 - 2.1)  >1-level moves forward from previous pararaph number
    char leveldecrease ;                        ///< 0-same level  1-move forward inlevel (2.1 - 2)  >1-level moves backward from previous pararaph number
    char currentlevel ;                         ///< level of current paragraph
    uint16_t levelnumber[8] ;                   ///< 1 - 8
    char paraformat[31] ;                       ///< string conatining paragrah format
} ;


struct sWSParagraphStyle
{
    uint16_t number ;                           ///< new paragraph style number
    uint16_t previous ;                         ///< previous paragraph style number
    uint16_t prevmodified ;                     ///< previous modified style number
    uint16_t prevprev ;                         ///< previous previous for reverting
} ;



struct sWSStyleLibrary
{
    uint16_t nextblock ;                        ///< next free 512 byte block (relative to start)
    unsigned char count ;                       ///< object count (currently 1)
} ;

#pragma pack(pop)

struct sInternalFonts
{
    QFont font ;
    sWSFont wsfont ;                            ///< if we read from a file, this is the original font structure
    QString name ;                             ///< wordstars name for this font
} ;

// for codepage conversion
struct sExtendedChars
{
    unsigned char wordstarchar ;
    unsigned long utf8char ;
} ;

#endif // DOCTSTRUCTS_H_INCLUDED
