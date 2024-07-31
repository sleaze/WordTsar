#ifndef CONFIG_H
#define CONFIG_H

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

#include <sys/types.h>


// yeah, my own assertion. I don't want to stop running, I just want to report
#ifndef NDEBUG
    #include <cassert>
    #include <iostream>
        #define MY_ASSERT(condition)   \
        {  \
            if(!(condition))  \
            {  \
                std::cerr << "ASSERTION! " << __FILE__ << "  " << __FUNCTION__  << "  line: " << __LINE__  << " COndition: " << #condition << std::endl ;  \
                abort() ; \
            } \
        }
#else
#define MY_ASSERT(condition) { }
#endif

#ifdef _WIN32
typedef long ssize_t;
#endif


#define LAYOUT_TIMER   1                ///< uncomment to time layout engine
#define DETAIL_LAYOUT_TIMER 1

//#define CACHE_DOT 1                     ///< uncomment to use the dot command cache (incomplete)

#define NOOPTIMIZEDISPLAY 1             ///< display optimization has some issues, uncomment to turn it on



// typedefs to help me stay consistant
typedef ssize_t POSITION_T ;                 // text buffer indexing
typedef ssize_t LINE_T ;
typedef ssize_t PARAGRAPH_T ;
typedef ssize_t PAGE_T ;

#define MAX_BUFFER_SIZE (LONG_MAX - 2)        ///< maximum size of above indexes (LONG_MAX)

// accessors for typedefs
#define CAST_POSITION_SIZE  static_cast<POSITION_T>

typedef double COORD_T ;


//typedef unsigned char CHAR_T ;
typedef char32_t CHAR_T ;                   // really bad way to store UTF8, which can be anywhere from 1 to 4 bytes in size 


/// @ todo make cBuffer and cGapBuffer UTF-8 aware (speed hit)


const double ONEINCH_INMMX10 = 254.0 ;
const double TWIPSPERINCH = 1440.0;
const double POINTSPERINCH = 1.0 / 72.0 ;
const double TWIPSPERMM = TWIPSPERINCH / 25.4 ;  /// @todo  NO NO NO 56.692913386 ;
const double TWIPSPERCM = TWIPSPERMM * 10;
const double POINTSTOTWIPS = TWIPSPERINCH / 72.0 ; ///< Font height in twips is 20 * pointsize = twips

#ifdef Q_OS_MACOS
const double FONTSCALE = (TWIPSPERINCH / 72.0)  ;    ///< fontscale based on 72 DPI
#else
const double FONTSCALE = (TWIPSPERINCH / 96.0)  ;    ///< fontscale based on 96 DPI
#endif

const CHAR_T HARD_RETURN = 13 ;
const CHAR_T SPACE = 32 ;
const CHAR_T MARKER_CHAR = 127 ;

const int MAX_HEADER_FOOTER = 5 ;           ///< the maximum number of lines in a header or footer

#define CHAR_RETURN  "↵"                    ///< the charcater to display for hard returns ↵ ¶'
#define CHAR_EOF     "‡"                    ///< the charcater to display for end of file

#define OVERWRITE_MODE  false
#define INSERT_MODE     true

#define MAXWSCOLORS 16                      ///< the number of colors the original Wordstar knows about.

#define MAX_UNDO_REDO 20480                 ///<maximum undo levels

#define UNUSED_ARGUMENT(x) (void)x          // also used for unused locals


///////////////////////////////////////////////////////////////////////
/// @enum eType
///
/// @note Used to match control chars with proper type
///
///////////////////////////////////////////////////////////////////////
enum eType
{
    TYPE_FORMAT,
    TYPE_TAB,
    TYPE_FONT,
    TYPE_COLOR,
    TYPE_INDEX,
    TYPE_FOOTNOTE,
    TYPE_ENDNOTE
};


///////////////////////////////////////////////////////////////////////
/// @enum eModifiers
///
/// @note Basic original wordstar styles (order is important)
///
///////////////////////////////////////////////////////////////////////
enum eModifiers
{
    STYLE_NOT_USED1,
    STYLE_FONT1,
    STYLE_BOLD,                         // done
    STYLE_NOT_USED2,                    // not used - pause printing for user input
    STYLE_NOT_USED3,                    // not used - double strike toggle
    STYLE_NOT_USED4,                    // not used - custom print control
    STYLE_PHANTOM_SPACE,                // not used
    STYLE_PHANTOM_BACKSPACE,            // not used
    STYLE_BACKSPACE,                    // not used
    STYLE_TAB,                          // done
    STYLE_LINEFEED,                     // done
    STYLE_INDEX,                        // PARTIAL
    STYLE_FORMFEED,                     // done - inserts .pa
    STYLE_RETURN,                       // done
    STYLE_NOBREAK_SPACE,                // done - inserted into document
    STYLE_CTRL_O,                       
    STYLE_INTERNAL_COLOR,               // reserved by wordstar, used by gerald for internal colors  STYLE_RESERVED,
    STYLE_NOT_USED7,                    // not used - custom print control
    STYLE_NOT_USED8,                    // not used - custom print control
    STYLE_UNDERLINE,                    // done
    STYLE_SUPERSCRIPT,                  // done
    STYLE_RESERVED1,
    STYLE_SUBSCRIPT,                    // done
    STYLE_NOT_USED9,                    // not used - custome print control
    STYLE_STRIKETHROUGH,                // done
    STYLE_ITALICS,                      // done
    STYLE_EOF,                          // done
    STYLE_EXTSTART,                     // done - next character is extended character if 2nd byte following is STYLE_EXTEND
    STYLE_EXTEND,                       // done - preceding character is extended character if 2nd byte preceding is STYLE_EXTSTART
    STYLE_SEQUENCE,                     // PARTIAL - read ok, not written
    STYLE_FOOTNOTE,                     // done (WS uses this for - soft-hypen for where wordwrap can occur)
    STYLE_ENDNOTE,                      // done (WS uses this for - soft-hypen for where wordwrap has occurred)
    STYLE_END_OF_STYLES
} ;



///////////////////////////////////////////////////////////////////////
/// @enum eTabTyoes
///
/// @note What type of tab are we
///
///////////////////////////////////////////////////////////////////////
enum eTabTypes
{
        TAB_TAB = ' ',
        TAB_DECIMAL = '#',
        TAB_CENTER = '!',
        TAB_RIGHT = '[',
        TAB_RIGHT1 = ']',                     // not in the docs, but a MicroPro document used it (WS7.0d print.tst)
        TAB_SOFT = 0xA0                      // soft space tab
};



///////////////////////////////////////////////////////////////////////
/// @enum eJustification
///
/// @note what justification are we
///
///////////////////////////////////////////////////////////////////////
enum eJustification
{
    JUST_LEFT,
    JUST_CENTER,
    JUST_RIGHT,
    JUST_JUST
};



///////////////////////////////////////////////////////////////////////
/// @enum eMeasurement
///
/// @note what measurement are we displaying
///
///////////////////////////////////////////////////////////////////////
enum eMeasurement
{
    MSR_TWIPS,
    MSR_POINTS,
    MSR_MILLIMETERS,
    MSR_CENTIMETERS,
    MSR_INCHES
};


///////////////////////////////////////////////////////////////////////
/// @enum eLayouts
///
/// @note what render layout engine are we using
///
///////////////////////////////////////////////////////////////////////
enum eLayouts
{
    LAYOUT_STANDARD,
    LAYOUT_PRINT,
};




///////////////////////////////////////////////////////////////////////
/// @enum eHelpDisplay
///
/// @note What are we displaying for help
///
///////////////////////////////////////////////////////////////////////
enum eHelpDisplay
{
    HELP_UNKNOWN,
    HELP_NONE,
    HELP_MAIN,
    HELP_CTRLJ,
    HELP_CTRLK,
    HELP_CTRLP,
    HELP_CTRLQ,
    HELP_CTRLO
} ;


///////////////////////////////////////////////////////////////////////
/// @enum eShowControl
///
/// @note Levels of show control
///
///////////////////////////////////////////////////////////////////////
enum eShowControl
{
    SHOW_ALL,
    SHOW_DOT,
    SHOW_NONE
};



///////////////////////////////////////////////////////////////////////
/// @enum eHeaderFooter
///
/// @note Header/footer types
///
///////////////////////////////////////////////////////////////////////
enum eHeaderFooter
{
    HEADER_BOTH,
    HEADER_EVEN,
    HEADER_ODD,
    FOOTER_BOTH,
    FOOTER_EVEN,
    FOOTER_ODD
} ;



enum eNoteSymbol
{
    NOTE_SYMBOLS,
    NOTE_UPPER,
    NOTE_LOWER,
    NOTE_NUMBER
};

#endif
