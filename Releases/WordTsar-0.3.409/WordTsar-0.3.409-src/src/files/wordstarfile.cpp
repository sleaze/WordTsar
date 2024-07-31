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

#include <bitset>
#include <numeric>

#include "wordstarfile.h"
#include "src/core/codepage/cp437.h"

using namespace std ;


/// @ingroup File
/// @{

vector<string>gSysFontName ;  ///< closest matching system font name





/////////////////////////////////////////////////////////////////////////////
///
/// @param  editor   [IN] - the editor that controls us
///
/// @return nothing
///
/// @brief
/// constructor
///
/////////////////////////////////////////////////////////////////////////////
cWordstarFile::cWordstarFile(cEditorCtrl *editor)
    : cFile(editor)
{
    mEditor = editor ;

    mFontNames.push_back("LinePrinter") ;
    mFontNames.push_back("Pica") ;
    mFontNames.push_back("Elite") ;
    mFontNames.push_back("Courier New") ; //    mFontNames.push_back("Courier") ;
    mFontNames.push_back("Helvetica") ;
    mFontNames.push_back("Times New Roman") ; //    mFontNames.push_back("Times Roman") ;
    mFontNames.push_back("Gothic") ;
    mFontNames.push_back("Script") ;
    mFontNames.push_back("Prestige") ;
    mFontNames.push_back("Caslon") ;
    mFontNames.push_back("Orator") ;
    mFontNames.push_back("Presentations") ;
    mFontNames.push_back("Helvetica Cond") ;
    mFontNames.push_back("Serifa") ;
    mFontNames.push_back("Blippo") ;
    mFontNames.push_back("Windsor") ;
    mFontNames.push_back("Century") ;
    mFontNames.push_back("ZapfHumanist") ;
    mFontNames.push_back("Garamond") ;
    mFontNames.push_back("Cooper") ;
    mFontNames.push_back("Coronet") ;
    mFontNames.push_back("Broadway") ;
    mFontNames.push_back("Bondoni") ;
    mFontNames.push_back("Century Schoolbook") ;
    mFontNames.push_back("Times New Roman") ; //    mFontNames.push_back("Universe Roman") ;
    mFontNames.push_back("Helvetica Outline") ;
    mFontNames.push_back("Peignot") ;
    mFontNames.push_back("Clarendon") ;
    mFontNames.push_back("Stick") ;
    mFontNames.push_back("HP-GL Drafting") ;
    mFontNames.push_back("HP-GL Spline") ;
    mFontNames.push_back("Times New Roman") ; //    mFontNames.push_back("Times") ;
    mFontNames.push_back("HPLJ Soft Font") ;
    mFontNames.push_back("Borders") ;
    mFontNames.push_back("Uncle Sam Open") ;
    mFontNames.push_back("Raphael") ;
    mFontNames.push_back("Uncial") ;
    mFontNames.push_back("Manhattan") ;
    mFontNames.push_back("Dom Casual") ;
    mFontNames.push_back("Old English") ;
    mFontNames.push_back("Trium Condensed") ;
    mFontNames.push_back("Trium UltraComp") ;
    mFontNames.push_back("Trade ExtraCond") ;
    mFontNames.push_back("American Classic") ;
    mFontNames.push_back("Globe Gothic Outline") ;
    mFontNames.push_back("UniversCondensed") ;
    mFontNames.push_back("Univers") ;
    mFontNames.push_back("Times New Roman") ; //    mFontNames.push_back("TmsRmnCond") ;
    mFontNames.push_back("PrstElite") ;
    mFontNames.push_back("Optima") ;
    mFontNames.push_back("Aachen") ;
    mFontNames.push_back("Am Typewriter") ;
    mFontNames.push_back("Avant Garde") ;
    mFontNames.push_back("Beguiat") ;
    mFontNames.push_back("Brush Script") ;
    mFontNames.push_back("Carta") ;
    mFontNames.push_back("Centennial") ;
    mFontNames.push_back("Cheltenham") ;
    mFontNames.push_back("FranklinGothic") ;
    mFontNames.push_back("FrstyleScrpt") ;
    mFontNames.push_back("FrizQuadrata") ;
    mFontNames.push_back("Futura") ;
    mFontNames.push_back("Galliard") ;
    mFontNames.push_back("Glypha") ;
    mFontNames.push_back("Goudy") ;
    mFontNames.push_back("Hobo") ;
    mFontNames.push_back("LubalinGraph") ;
    mFontNames.push_back("Lucida") ;
    mFontNames.push_back("LucidaMath") ;
    mFontNames.push_back("Machine") ;
    mFontNames.push_back("Melior") ;
    mFontNames.push_back("NewBaskrvlle") ;
    mFontNames.push_back("NewCntSchlbk") ;
    mFontNames.push_back("News Gothic") ;
    mFontNames.push_back("Palantino") ;
    mFontNames.push_back("Park Avenue") ;
    mFontNames.push_back("Revue") ;
    mFontNames.push_back("Sonata") ;
    mFontNames.push_back("Stencil") ;
    mFontNames.push_back("Souvenir") ;
    mFontNames.push_back("TrmpMedievel") ;
    mFontNames.push_back("ZapfChancery") ;
    mFontNames.push_back("ZapfDingbats") ;
    mFontNames.push_back("Stone") ;
    mFontNames.push_back("CntryOldStyle") ;
    mFontNames.push_back("Corona") ;
    mFontNames.push_back("GoudyOldStyle") ;
    mFontNames.push_back("Excelsior") ;
    mFontNames.push_back("FuturaCondensed") ;
    mFontNames.push_back("HelvCompressed") ;
    mFontNames.push_back("HelvExtraCompressed") ;
    mFontNames.push_back("Helv Narrow") ;
    mFontNames.push_back("HelvUltaCompressed") ;
    mFontNames.push_back("KorinnaKursiv") ;
    mFontNames.push_back("Lucida Sans") ;
    mFontNames.push_back("Memphis") ;
    mFontNames.push_back("Stone Informal") ;
    mFontNames.push_back("Stone Sans") ;
    mFontNames.push_back("Stone Serif") ;
    mFontNames.push_back("Postscript") ;
    mFontNames.push_back("NPS Utility") ;
    mFontNames.push_back("NPS Draft") ;
    mFontNames.push_back("NPS Corr") ;
    mFontNames.push_back("NPS SanSer Qual") ;
    mFontNames.push_back("NPS Serif Qual") ;
    mFontNames.push_back("PS Utility") ;
    mFontNames.push_back("PS Draft") ;
    mFontNames.push_back("PS Corr") ;
    mFontNames.push_back("PS SanSer Qual") ;
    mFontNames.push_back("PS Serif Qual") ;
    mFontNames.push_back("Download") ;
    mFontNames.push_back("NPS ECS Qual") ;
    mFontNames.push_back("PS Plastic") ;
    mFontNames.push_back("PS Metal") ;
    mFontNames.push_back("CloisterBlack") ;
    mFontNames.push_back("Gill Sans") ;
    mFontNames.push_back("Rockwell") ;
    mFontNames.push_back("Tiffany") ;
    mFontNames.push_back("Clearface") ;
    mFontNames.push_back("Amelia") ;
    mFontNames.push_back("HandelGothic") ;
    mFontNames.push_back("OratorSC") ;
    mFontNames.push_back("Outline") ;
    mFontNames.push_back("Bookman Light") ;
    mFontNames.push_back("Humanist") ;
    mFontNames.push_back("Swiss Narrow") ;
    mFontNames.push_back("ZapfCalligraphic") ;
    mFontNames.push_back("Spreadsheet") ;
    mFontNames.push_back("Broughm") ;
    mFontNames.push_back("Anelia") ;
    mFontNames.push_back("LtrGothic") ;
    mFontNames.push_back("Boldface") ;
    mFontNames.push_back("High Density") ;
    mFontNames.push_back("High Speed") ;
    mFontNames.push_back("Super Focus") ;
    mFontNames.push_back("Swiss Outline") ;
    mFontNames.push_back("Swiss Display") ;
    mFontNames.push_back("Momento Outline") ;
    mFontNames.push_back("Courier Italic") ;
    mFontNames.push_back("Text Light") ;
    mFontNames.push_back("Momento Heavy") ;
    mFontNames.push_back("BarCode") ;
    mFontNames.push_back("EAN/UPC") ;
    mFontNames.push_back("Math-7") ;
    mFontNames.push_back("Math-8") ;
    mFontNames.push_back("Swiss") ;
    mFontNames.push_back("Dutch") ;
    mFontNames.push_back("Trend") ;
    mFontNames.push_back("Holsatia") ;
    mFontNames.push_back("Serif") ;
    mFontNames.push_back("Bandit") ;
    mFontNames.push_back("Bookman") ;
    mFontNames.push_back("Casual") ;
    mFontNames.push_back("Dot") ;
    mFontNames.push_back("EDP") ;
    mFontNames.push_back("ExtGraphics") ;
    mFontNames.push_back("Garland") ;
    mFontNames.push_back("PC Line") ;
    mFontNames.push_back("HP Line") ;
    mFontNames.push_back("Hamilton") ;
    mFontNames.push_back("Korinna") ;
    mFontNames.push_back("LineDrw") ;
    mFontNames.push_back("Modern") ;
    mFontNames.push_back("Momento") ;
    mFontNames.push_back("MX") ;
    mFontNames.push_back("PC") ;
    mFontNames.push_back("PI") ;
    mFontNames.push_back("Profile") ;
    mFontNames.push_back("Q-Fmt") ;
    mFontNames.push_back("Rule") ;
    mFontNames.push_back("SB") ;
    mFontNames.push_back("Taylor") ;
    mFontNames.push_back("Text") ;
    mFontNames.push_back("APL") ;
    mFontNames.push_back("Artisan") ;
    mFontNames.push_back("Triumvirate") ;
    mFontNames.push_back("Chart") ;
    mFontNames.push_back("Classic") ;
    mFontNames.push_back("Data") ;
    mFontNames.push_back("Document") ;
    mFontNames.push_back("Emperor") ;
    mFontNames.push_back("Essay") ;
    mFontNames.push_back("Forms") ;
    mFontNames.push_back("Facet") ;
    mFontNames.push_back("Micro") ;
    mFontNames.push_back("OCR-A") ;
    mFontNames.push_back("OCR-B") ;
    mFontNames.push_back("Apollo") ;
    mFontNames.push_back("Math") ;
    mFontNames.push_back("Scientific") ;
    mFontNames.push_back("Sonoran") ;
    mFontNames.push_back("Square 3") ;
    mFontNames.push_back("Symbol") ;
    mFontNames.push_back("Tempora") ;
    mFontNames.push_back("Title") ;
    mFontNames.push_back("Titan") ;
    mFontNames.push_back("Theme") ;
    mFontNames.push_back("TaxLineDraw") ;
    mFontNames.push_back("Vintage") ;
    mFontNames.push_back("XCP") ;
    mFontNames.push_back("Eletto") ;
    mFontNames.push_back("Est Elite") ;
    mFontNames.push_back("Idea") ;
    mFontNames.push_back("Italico") ;
    mFontNames.push_back("Kent") ;
    mFontNames.push_back("Mikron") ;
    mFontNames.push_back("Notizia") ;
    mFontNames.push_back("Roma") ;
    mFontNames.push_back("Presentor") ;
    mFontNames.push_back("Victoria") ;
    mFontNames.push_back("Draft Italic") ;
    mFontNames.push_back("PS Capita") ;
    mFontNames.push_back("Qual Italic") ;
    mFontNames.push_back("Antique Olive") ;
    mFontNames.push_back("Bauhaus") ;
    mFontNames.push_back("Era") ;
    mFontNames.push_back("Mincho") ;
    mFontNames.push_back("SerifGothic") ;
    mFontNames.push_back("Signet Roundland") ;
    mFontNames.push_back("Souvenir Gothic") ;
    mFontNames.push_back("Stymie") ;
    mFontNames.push_back("Bernhard Modern") ;
    mFontNames.push_back("Grand Ronde Script") ;
    mFontNames.push_back("Ondine") ;
    mFontNames.push_back("PT Barnum") ;
    mFontNames.push_back("Kaufmann") ;
    mFontNames.push_back("Bolt") ;
    mFontNames.push_back("AntOliveCompact") ;
    mFontNames.push_back("Garth Graphic") ;
    mFontNames.push_back("Ronda") ;
    mFontNames.push_back("EngSchreibschrift") ;
    mFontNames.push_back("Flash") ;
    mFontNames.push_back("Gothic Outline") ;
    mFontNames.push_back("Akzidenz-Grotesk") ;
    mFontNames.push_back("TD Logos") ;
    mFontNames.push_back("Shannon") ;
    mFontNames.push_back("Oberon") ;
    mFontNames.push_back("Callisto") ;
    mFontNames.push_back("Charter") ;
    mFontNames.push_back("Plantin") ;
    mFontNames.push_back("Helvetica Black") ;
    mFontNames.push_back("Helvetica Light") ;
    mFontNames.push_back("Arnold Bocklin") ;
    mFontNames.push_back("Fette Fraktur") ;
    mFontNames.push_back("Greek PS") ;

    if(gSysFontName.empty() == true)
    {
//        BuildFontList() ;
    }
    
    mInIndex = false ;
    mExtendedChar = false;
}

cWordstarFile::~cWordstarFile()
{
}


/////////////////////////////////////////////////////////////////////////////
///
/// @param  filename   [IN] - file name to check
///
/// @return bool - true we load this type of file, else false
///
/// @brief
/// Check to see if thos class can load the passed in file
///
/////////////////////////////////////////////////////////////////////////////
bool cWordstarFile::CheckType(string filename)
{
    string ext;

    size_t found = filename.find_last_of(".") ;
    ext = filename.substr(found + 1) ;

    for(size_t loop = 0; loop < ext.size(); loop++)
    {
        ext[loop] = tolower(ext[loop]) ;
    }

    return (ext == "ws" || ext == "ws3" || ext == "ws4" ||
            ext == "ws5" || ext == "ws6" || ext == "ws7" ||
            ext == "ws8" || ext == "ws-bak" || ext == "ws-$$$");
}


/////////////////////////////////////////////////////////////////////////////
///
/// @param  filename   [IN] - the file to load
///
/// @return bool - true on success, else false
///
/// @brief
/// load the file, with a progress dialog
///
/////////////////////////////////////////////////////////////////////////////
bool cWordstarFile::LoadFile(string filename)
{
    bool retval = false ;
    mInIndex = false ;
    
    mFile = fopen(filename.c_str(), "rb") ;
    if(mFile)
    {
        // wordstar is an ascii based protocol, do 1 byte at a time
        fseek(mFile, 0, SEEK_END) ;
        size_t filesize = ftell(mFile) ;
        fseek(mFile, 0, SEEK_SET) ;

//        mDocument->SetLoading(true) ;

        string label ;

        // load a wordstar file from version 4.0 and under
        char inchar ;
        fread(&inchar, 1, 1, mFile) ;
        if(inchar != 0x1D)
        {
            string_sprintf("Loading File as WordStar 4.0 or less...") ;
            fseek(mFile, 0, SEEK_SET) ;
        }
        else        // if the file starts with a 0x1D, then it's a file from version 5.0 or greater
        {
            fread(&inchar, 1, 1, mFile) ;
            fread(&inchar, 1, 1, mFile) ;
            fread(&inchar, 1, 1, mFile) ;
            fread(&inchar, 1, 1, mFile) ;
            unsigned char ver = static_cast<unsigned char>(inchar) ;
            char low = ver & 0x0F ;
            char high = (ver & 0xF0) >> 4 ;

            string_sprintf("Loading File as Wordstar %d.%d...", high, low) ;

            fseek(mFile, 0, SEEK_SET) ;
        }

//        QProgressDialog progress(label, "Cancel", 0, 100, mEditor) ;
//        progress.show() ;

        long modulo = filesize / 100 ;
        if(modulo == 0)
        {
            modulo = 1 ;
        }

        // WordStar is a 7 bit format, so that's what we read in here.
        int oldpercent = 90 ;
        for(qint64 readloop = 0; readloop < filesize; readloop++)
        {
            int percent =  static_cast<int>(static_cast<double>(readloop) / static_cast<double>(filesize) * 100.0) ;
            if(readloop % modulo == 0)
            {
                string temp1 ;
                UpdateProgress(percent) ;
                oldpercent = percent ;
            }

            fread(&inchar, 1, 1, mFile) ;

            if((inchar == static_cast<unsigned char>(STYLE_EOF)) && (mExtendedChar == false))
            {
                string temp1 ;
                UpdateProgress(100) ;
                break ;
            }

            unsigned char c = static_cast<unsigned char>(inchar) ;
            HandleChar(c, static_cast<size_t>(readloop)) ;

//            QApplication::processEvents() ;
        }

        fclose(mFile) ;

//        mDocument->SetLoading(false) ;

        retval = true ;
    }

    return retval ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @param  filename   [IN] - The file to save to
/// @param  length     [IN] - the length of the file to save (data in buffer)
///
/// @return bool - true on sucess, else false
///
/// @brief
/// save a wordstar formated file in wordstar 7 format
///
/// @todo Highly incomplete
/////////////////////////////////////////////////////////////////////////////
bool cWordstarFile::SaveFile(string filename, POSITION_T length)
{
    bool retval = false ;

    /// @todo we ned to check to see if it can be saved as Wordstar fromat (i.e. Unicode chars)

    mFile = fopen(filename.c_str(), "wb") ;
    if(mFile)
    {
        uint32_t stylecounter = 0 ;

        // write the WordStar 7.0d header
        sSeqIntro intro ;
        sSeqClose close ;

        sWSHeader header ;
        memset(&header, 0, sizeof(header)) ;

        intro.start = 0x1d ;
        intro.size = 125 ;
        intro.type = 0 ;

        close.size = 125 ;
        close.finish = 0x1d ;

        header.version = 0x70 ;
        sprintf(header.driver, "HP4  ") ;
        header.styles = static_cast<unsigned int>(length) ;

        fwrite(&intro, sizeof(sSeqIntro), 1, mFile) ;
        fwrite(&header, sizeof(sWSHeader), 1, mFile) ;
        fwrite(&close, sizeof(sSeqClose), 1, mFile) ;

        stylecounter += sizeof(sWSHeader) + sizeof(sSeqIntro) + sizeof(sSeqClose) ;

        // go through the buffer
        for(POSITION_T loop = 0; loop < length; loop++)
        {
            // write out the standard character
            string str = mDocument->GetChar(loop) ;
            char ch1 = str[0] ;
            if(str.length() == 1)
            {
                if(ch1 == MARKER_CHAR)
                {
                    ch1 = mDocument->GetControlChar(loop) ;
                }

                switch(ch1)
                {
                    case 13 : // HARD_RETURN :
                        // convert hard returns to CR/LF pairs
                        ch1 = 13 ;
                        fputc(ch1, mFile) ;
                        ch1 = 10 ;
                        fputc(ch1, mFile) ;
                        stylecounter += 2 ;
                        break ;

                    case STYLE_TAB :
                        {
                            sWSTab tab ;

                            tab = mDocument->GetTab(loop) ;

                            intro.size = 10 ;
                            intro.type = eSequence::SEQ_TAB ;

                            close.size = 10 ;

                            fwrite(&intro, sizeof(sSeqIntro), 1, mFile) ;
                            fwrite(&tab, sizeof(sWSTab), 1, mFile) ;
                            fwrite(&close, sizeof(sSeqClose), 1, mFile) ;

                            stylecounter += sizeof(sWSTab) + sizeof(sSeqIntro) + sizeof(sSeqClose) ;
                        }
                        break ;

                    case STYLE_INTERNAL_COLOR :
                        {
                            sWSColor color ;

                            if(mDocument->GetColor(loop, color) == true)
                            {
                                intro.size = sizeof(sWSColor) + sizeof(sSeqClose) + 1 ;
                                intro.type = SEQ_COLOR ;

                                close.size = intro.size ;

                                fwrite(&intro, sizeof(sSeqIntro), 1, mFile) ;
                                fwrite(&color, sizeof(sWSColor), 1, mFile) ;
                                fwrite(&close, sizeof(sSeqClose), 1, mFile) ;
                            }
                            stylecounter += sizeof(sWSColor) + sizeof(sSeqIntro) + sizeof(sSeqClose) ;
                        }
                        break ;

                    case STYLE_FONT1 :
                        {
                            sInternalFonts font ;
    //                        sInternalFonts prevfont ;

                            sWSFont wsfont ;

                            if(mDocument->GetFont(loop, font) == true)
                            {
                                intro.size = sizeof(sWSFont) + sizeof(sSeqClose) + 1 ;
                                intro.type = eSequence::SEQ_FONT ;

                                close.size = intro.size ;

                                // if we have a valid wsfont structure, use it...
                                if(font.haveWSFont)
                                {
                                    wsfont = font.wsfont ;
                                }
                                else
                                {
                                    std::bitset<16> style ;

                                    QFont nfont(QString::fromStdString(font.name), static_cast<int>(font.size)) ;

                                    QString f = nfont.defaultFamily() ;
                                    QFontDatabase db ;
                                    if(db.isFixedPitch(f, "Normal"))
                                    {
                                        style.set(11, 0) ;
                                        style.set(10, 0) ;
                                        style.set(15, 0) ;
                                    }
                                    else
                                    {

                                        // set the font family
                                        switch(nfont.styleHint())
                                        {
                                            case QFont::SansSerif :
                                                style.set(11, 0) ;
                                                style.set(10, 0) ;
                                                style.set(15, 1) ;                  // proportional or not
                                                break ;

                                            case QFont::Serif :
                                                style.set(11, 0) ;
                                                style.set(10, 1) ;
                                                style.set(15, 1) ;                  // proportional or not
                                                break ;

                                            case QFont::Cursive :
                                                style.set(11, 1) ;
                                                style.set(10, 0) ;
                                                style.set(15, 1) ;                  // proportional or not
                                                break ;

                                            case QFont::TypeWriter :
                                                style.set(11, 0) ;
                                                style.set(10, 0) ;
                                                style.set(15, 0) ;                  // proportional or not
                                                break ;

                                            default :
                                                style.set(11, 1) ;
                                                style.set(10, 1) ;
                                                style.set(15, 1) ;                  // proportional or not
                                                break ;
                                        }
                                    }
                                    // set the font encode
    //                                switch(encoding)
    //                                {
    //                                    case QFontENCODING_CP437 :
                                            style.set(13, 0) ;
                                            style.set(12, 0) ;
    //                                        break ;
    //
    //                                    case QFontENCODING_CP850 :
    //                                        style.set(13, 0) ;
    //                                        style.set(12, 1) ;
    //                                        break ;
    //
    //                                    default :
    //                                        style.set(13, 0) ;
    //                                        style.set(12, 0) ;
    //                                        break ;
    //                                }

                                    // set the font typestyle.set
                                    // @todo FONTS
                                    style.set(8, 1) ;
                                    style.set(7, 1) ;
                                    style.set(6, 1) ;
                                    style.set(5, 1) ;
                                    style.set(4, 1) ;
                                    style.set(3, 1) ;
                                    style.set(2, 1) ;
                                    style.set(1, 1) ;
                                    style.set(0, 1) ;

                                    wsfont.style = style.to_ulong() ;
                                    wsfont.height = nfont.pointSize() * 20 ;
                                }
                                fwrite(&intro, sizeof(sSeqIntro), 1, mFile) ;
                                fwrite(&wsfont, sizeof(sWSFont), 1, mFile) ;
                                fwrite(&close, sizeof(sSeqClose), 1, mFile) ;

                                stylecounter += sizeof(sWSColor) + sizeof(sSeqIntro) + sizeof(sSeqClose) ;
                            }
                        }
                        break ;

                    case STYLE_EOF :
                        loop = length ;  // get out of for loop
                        break ;

                    default :
                    {
/*
                        bool ext = false ;

                        // check if this is a extended char.
                        if(static_cast<unsigned long>(ch1) < 32 || static_cast<unsigned long>(ch1) > 126)
                        {
                            cCodePage437 cp ;
                            unsigned char chws = cp.toChar(ch1) ;
                            if(chws != 0)
                            {
                                ch1 = STYLE_EXTSTART ;
                                mFile.write((char *)&ch1, 1) ;
                                mFile.write((char *)&chws, 1) ;
                                ch1 = STYLE_EXTEND ;
                                mFile.write((char *)&ch1, 1) ;
                                ext = true ;
                            }
                        }

                        if(ext == false)
*/
                        {
                            fputc(ch1, mFile) ;
                        }
                        stylecounter++ ;
                        break ;
                    }
                }    // switch
            }  // if
            else            // this is a unicode (UTF-8 char)
            {
                // check if this is a extended char.
                if(static_cast<unsigned long>(ch1) < 32 || static_cast<unsigned long>(ch1) > 126)
                {
                    vector<utf8proc_int32_t> codepoints ;
                    string outstr ;

                    size_t len = mDocument->GetCodePoints(str, codepoints) ;

                    if(len > 0)
                    {
                        cCodePage437 cp ;
                        unsigned char chws = cp.toChar(codepoints[0]) ;
                        if(chws != 0)
                        {
                            ch1 = STYLE_EXTSTART ;
                            fputc(ch1, mFile) ;
                            fputc(chws, mFile) ;
                            ch1 = STYLE_EXTEND ;
                            fputc(ch1, mFile) ;
    //                        ext = true ;
                        }
                    }
                }
            }
        }

        long eofsize = 128 - (stylecounter % 128) ;         // buffer end of file to 128 byte marker
        if(eofsize == 0)
        {
            eofsize = 128 ;
        }
        char ch = STYLE_EOF ;
        for(long eofloop = 0; eofloop < eofsize; eofloop++)
        {
            fputc(ch, mFile) ;
            stylecounter++ ;
        }

        // rewrite header now that we know style table offset
        fseek(mFile, 0, SEEK_SET) ;

        intro.start = 0x1d ;
        intro.size = 125 ;
        intro.type = 0 ;

        close.size = 125 ;
        close.finish = 0x1d ;

        header.version = 0x70 ;
        sprintf(header.driver, "HP4  ") ;
        header.styles = stylecounter ;

        fwrite(&intro, sizeof(sSeqIntro), 1, mFile) ;
        fwrite(&header, sizeof(sWSHeader), 1, mFile) ;
        fwrite(&close, sizeof(sSeqClose), 1, mFile) ;

        retval = true ;
        fclose(mFile) ;
    }
    return retval ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return bool - true if this class can load its file types, else false
///
/// @brief
/// can this class load files of the type it handles
///
/////////////////////////////////////////////////////////////////////////////
bool cWordstarFile::CanLoad(void)
{
    return true ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return bool - true if this class can save its file types, else false
///
/// @brief
/// can this class save files of the type it handles
///
/////////////////////////////////////////////////////////////////////////////
bool cWordstarFile::CanSave(void)
{
    return true ;
}



string cWordstarFile::GetExtensions(void)
{
    return "WordStar Files (WordStar Files (*.ws *.ws3 *.ws4 *.ws5 *.ws6 *.ws7 *.ws8 *.WS *.WS3 *.WS4 *.WS5 *.WS6 *.WS7 *.WS8)" ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param  c   [IN] - the character we need to deal with
/// @param loop [IN/OUT] - our position in the file.
///
/// @return nothing
///
/// @brief
///
///
/////////////////////////////////////////////////////////////////////////////
void cWordstarFile::HandleChar(unsigned char c, size_t loop)
{
    bool insert = true ;
    static unsigned char last = 0 ;
    static size_t position = 0 ;

    if(mExtendedChar == true)            // we've seen a STYLE_EXTSTART
    {
        if((loop - position >= 2) && (c != STYLE_EXTEND))       // if we should have seen a STYLE_EXTEND by now
        {
            mExtendedChar = false ;
            size_t npos = position + 1 ;
            HandleChar(last, npos) ;                    // treat the char after the STYLE_EXTSTART as a normal one
        }
    }

    // we'll only do this if we are not in an extended char or if we are and are about to exit an extended char
    if((mExtendedChar == false) || ((mExtendedChar == true) && (c == STYLE_EXTEND)))
    {
        switch(c)
        {
            case 0x8A :                 // phantom soft linefeed
            case STYLE_LINEFEED :
                if(last == 0x8D)        // if the last was a phantom CR, then we need to ignore this LF
                {
                    insert = false ;
                }
                c = 13 ; // HARD_RETURN ;
                break ;

            case 0x8D :             // phantom soft CR
            case 13 :               // CR
                insert = false ;
                break ;

            case STYLE_BOLD :       // ^B - boldface toggle
                mDocument->BeginBold() ;
                insert = false ;
                break ;

            case STYLE_ITALICS :    // ^Y - italics toggle
                mDocument->BeginItalics() ;
                insert = false ;
                break ;

            case STYLE_UNDERLINE :  // ^S - underline toggle
                mDocument->BeginUnderline() ;
                insert = false ;
                break ;

            case STYLE_SUPERSCRIPT :
                mDocument->BeginSuperscript() ;
                insert = false ;
                break ;

            case STYLE_SUBSCRIPT :
                mDocument->BeginSubscript() ;
                insert = false ;
                break ;

            case STYLE_STRIKETHROUGH :
                mDocument->BeginStrikeThrough() ;
                insert = false ;
                break ;

            case STYLE_TAB :
                sWSTab tab ;
                tab.abstabsize = 0 ;
                tab.size = 0 ;
                tab.tabsize = 0 ;
                tab.type = TAB_TAB ;

                mDocument->InsertTab(tab) ;
                insert = false ;
                break ;

            case STYLE_NOBREAK_SPACE :
                insert = true ;
                break ;

            case STYLE_SEQUENCE :
                HandleSequence(loop) ;
                insert = false  ;
                break ;

            case STYLE_FONT1 :
            case STYLE_BACKSPACE :
            case 0xA0 :                         // wordstar makes extra spaces for alignment with this
            case STYLE_PHANTOM_SPACE :          // daisy wheel stuff
            case STYLE_PHANTOM_BACKSPACE :      // daisy wheel stuff
            case STYLE_EOF :                    // eof filler
                insert = false ;
                break ;

            case STYLE_INDEX :
                mDocument->BeginIndex() ;
                insert = false ;
                break ;

            case STYLE_FORMFEED :           // not quite right.  Wordstar ejects the page and doesn't print footers
                mDocument->Insert(".pa") ;
                insert = false ;
                break ;

            case STYLE_EXTSTART :
                mExtendedChar = true ;
                position = loop ;
                insert = false ;
                break ;
                
            case STYLE_EXTEND :
                InsertExtendedChar(last) ;
                mExtendedChar = false ;
                insert = false ;
                break ;

            // the following are not implemented or not used
            case STYLE_NOT_USED1 :
            case STYLE_NOT_USED2 :
            case STYLE_NOT_USED3 :
            case STYLE_NOT_USED4 :
            case STYLE_CTRL_O :
            case STYLE_NOT_USED7 :
            case STYLE_NOT_USED8 :
            case STYLE_NOT_USED9 :

    //        case STYLE_RESERVED :         replaced by internal STYLE_INT_COLOR
            case STYLE_RESERVED1 :
//            case STYLE_RESERVED5 :        replaced by internal STYLE_FOOTNOTE
//            case STYLE_RESERVED6 :        replaced by internal STYLE_ENDNOTE
                insert = false ;
                break ;
        }

        char print ;
        if(c > 128)                 // strip the high bit if it is set, on an ascii charcater only
        {
            print = c - 128 ;
        }
        else
        {
            print = c ;
        }


        if(insert)
        {
            mDocument->Insert(print) ;

            if(mInIndex == true)
            {
                mIndexWord += print ;
            }
        }
    }
    
    last = c ;
}



/*bool cWordstarFile::GetFontMatchList(long count, string &wsfont, string &sysfont)
{
    bool retval = false ;

    if(static_cast<size_t>(count) < mFontNames.size())
    {
        wsfont = mFontNames[count] ;
        sysfont = gSysFontName[count] ;

        retval = true ;
    }

    return retval ;
}*/


/////////////////////////////////////////////////////////////////////////////
///
/// @param  event   [IN] - the paint event
///
/// @return nothing
///
/// @brief
///
///
/////////////////////////////////////////////////////////////////////////////
void cWordstarFile::HandleSequence(size_t &loop)
{
    unsigned char in, type ;
    long size, temp ;

    // the size is the sequence size subtract the sequence closer
    in = fgetc(mFile) ;
    size = in ;

    in = fgetc(mFile) ;
    temp = in ;
    size += temp >> 8 ;

    type = fgetc(mFile) ;
    switch(type)
    {
        case eSequence::SEQ_HEADER :            // header
            // we skip the header (size - 1 == last byte of sequence)
            for(long skip = 0; skip < size - 4; skip++)
            {
                in = fgetc(mFile) ;
            }
            loop += size - 1 ;
            break ;

        case eSequence::SEQ_COLOR :            // colour sequence
            {
                sWSColor color ;
                fread(&color, sizeof(sWSColor), 1, mFile) ;

                mDocument->InsertColor(color) ;
            }
            break ;

        case eSequence::SEQ_FONT :            // font
            {
                sWSFont font ;
                fread(&font, sizeof(sWSFont), 1, mFile) ;

                std::bitset<16> style = font.style ;
                unsigned char fontindex = font.style & 0x00FF ;         // the index into the font table

                bool proportional = style.test(15) ;
//                bool letterquality = style.test(14) ;

//                char symbolmap = (style.test(13) << 1) + style.test(12) ;
                char genericstyle = static_cast<char>((style.test(11) << 1) + style.test(10)) ;

//                bool symdiff = style.test(9) ;

//                short typestyle = (style.test(8) << 8) + (style.test(7) << 7) + (style.test(6) << 6) + (style.test(5) << 5) +
//                                    (style.test(4) << 4) + (style.test(3) << 3) + (style.test(2) << 2) + (style.test(1) << 1) + style.test(0) ;

                // build new font
                string fontname2 ;

                if(fontindex < mFontNames.size())
                {
                        fontname2 = mFontNames[fontindex] ;
                }
                else
                {
                    if(proportional == true)
                    {
                        fontname2 = "Times New Roman" ; // GetSystemFontname("Times New Roman", false, QFontENCODING_DEFAULT) ;
                    }
                    else
                    {
                        fontname2 = "Courier New" ; // GetSystemFontname("Courier New", true, QFontENCODING_DEFAULT) ;
                    }
                }
/*
                QString qfontname = QString::fromStdString(fontname2) ;
                QFont newfont(qfontname, font.height / 20) ;
                switch(genericstyle)
                {
                case 0 :
                    newfont.setStyleHint(QFont::SansSerif) ;
                    break ;

                case 1 :
                    newfont.setStyleHint(QFont::Serif) ;
                    break ;

                case 2 :
                    newfont.setStyleHint(QFont::Cursive) ;
                    break ;

                case 3 :
                    newfont.setStyleHint(QFont::SansSerif) ;
                    break ;

                }

                if(proportional == false)
                {
                    newfont.setStyleHint(QFont::Monospace) ;
                }
*/
//                QFontInfo finfo(newfont) ;
//printf("%s - %s\n", newfont.family().toLocal8Bit().constData(), finfo.family().toLocal8Bit().constData()) ;

                sInternalFonts sif ;
//                sif.font = newfont ;
                sif.name = fontname2 ;
                sif.haveWSFont = true ;
                sif.wsfont = font ;
                sif.size = font.height / 20 ; //  newfont.pointSize() ;
                mDocument->InsertFont(sif) ;
            }
            break ;



        /// @todo handle different tab sizes
        case eSequence::SEQ_TAB :             // tabs
            {
                sWSTab tab ;
//                int b = sizeof(sWSTab) ;
                fread(&tab, sizeof(sWSTab), 1, mFile) ;

                mDocument->InsertTab(tab) ;
            }
            break ;

        case eSequence::SEQ_FOOTNOTE :
        case eSequence::SEQ_ENDNOTE :
            {
                char inchar ;
                string str ;

                for(long loop = 0; loop < size - 4; loop++)
                {
                    inchar = fgetc(mFile) ;
                    str.push_back(inchar) ;
                }

                ParseNote(str, type) ;
            }
            break ;

        case eSequence::SEQ_ANNOTATION :
        case eSequence::SEQ_COMMENT :
            {
                char inchar ;
                string str ;

                for(long loop = 0; loop < size - 4; loop++)
                {
                    inchar = fgetc(mFile) ;
                    str.push_back(inchar) ;
                }

                ParseComment(str) ;
            }
            break ;

        case eSequence::SEQ_ENDOFPAGE :
        case eSequence::SEQ_PAGEOFFSET :
        case eSequence::SEQ_PARAGRAPHNUMBER :
        case eSequence::SEQ_INDEXENTRY :
        case eSequence::SEQ_PRINTERCONTROL :
        case eSequence::SEQ_GRAPHICS :
        case eSequence::SEQ_PARAGRAPHSTYLE :
        case eSequence::SEQ_ALTFONT :
        default :
            // we skip what we don't know (size - 1 == last byte of sequence)
            for(long skip = 0; skip < size - 4; skip++)
            {
                in = fgetc(mFile) ;
            }
            loop += size - 1 ;
            break ;
    }

    // skip trailing 3 bytes
    in = fgetc(mFile) ;
    in = fgetc(mFile) ;
    in = fgetc(mFile) ;
}


void cWordstarFile::InsertExtendedChar(unsigned char c)
{
    bool found = false ;

    cCodePage437 cp ;
    unsigned long chutf8 = cp.toUTF8(c) ;
    if(chutf8 != 0)
    {
        mDocument->Insert(chutf8) ;
        found = true ;
    }
/*
    for(int loop = 0; loop < gExtendedSize; loop++)
    {
        if(c == gCodePage437[loop].wordstarchar)
        {
            mDocument->Insert(gCodePage437[loop].utf8char.toLatin1()) ;
            found = true ;
            break ;
        }
    }
*/
    if(found == false)
    {
        mDocument->Insert(c) ;
    }
}



void cWordstarFile::ParseNote(string sequence, char type)
{
    // main sequence information
    union udata
    {
        char byte[4] ;
        sWSBasenote note ;
    };
    udata data ;

    // sub sequence information
    union sdata
    {
        char byte[5] ;
        sWSFootnote note ;
    };
    sdata subdata ;

    // prefill incase we don't have a subsequence
    subdata.note.linecount = 0 ;
    subdata.note.number = 0 ;
    subdata.note.conversion = 0 ;
    subdata.note.format = 3 ;


    size_t loop ;
    for(loop = 0; loop < 4; loop++)
    {
        data.byte[loop] = sequence[loop] ;
    }

    // see if we have an embedded footnote
    if(data.note.tag == 1)
    {
        for(loop = 9; loop < 13; loop++)
        {
            subdata.byte[loop - 9] = sequence[loop] ;
        }

        // get to end of sub sequence
        while(sequence[loop] != STYLE_SEQUENCE)
        {
            loop++ ;
        }
    }

    loop++ ;  // get past the STYLE_SEQUENCE if in sub sequence or unused char if not


    string text ;
    for(; loop < sequence.size(); loop++)
    {
        text.push_back(sequence[loop]) ;
    }

    ///< @todo use subdata.note.conversion if set to change note type and insert properly

    sNote note ;
    note.symbol = static_cast<eNoteSymbol>(subdata.note.format) ;
    note.text = text ;
    if(type == eSequence::SEQ_FOOTNOTE)
    {
        mDocument->InsertFootnote(note) ;
    }
    else if(type == eSequence::SEQ_ENDNOTE)
    {
        InsertEndnote() ;
    }
}


void cWordstarFile::InsertFootnote(void)
{
printf("insert footnote\n") ;
fflush(0) ;
}


void cWordstarFile::InsertEndnote(void)
{
printf("insert endnote\n") ;
fflush(0) ;
}


void cWordstarFile::ParseComment(string sequence)
{
printf("hello comment") ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @param  event   [IN] - the paint event
///
/// @return nothing
///
/// @brief
///
///
/////////////////////////////////////////////////////////////////////////////void cWordstarFile::HandleChar(QChar c, size_t &loop)
/*void cWordstarFile::WriteSpecialFile(wxFFile &out, char var)
{
    mFile.write(&var, 1) ;
}*/

/*
class MyFontEnumerator : public QFontEnumerator
{
public:
    bool GotAny() const
        { return !m_facenames.IsEmpty(); }

    const wxArrayString& GetFacenames() const
        { return m_facenames; }

protected:
    virtual bool OnFacename(const string& facename)
    {
        m_facenames.Add(facename);
        return true;
    }

    private:
        wxArrayString m_facenames;
}; //  fontEnumerator;





string cWordstarFile::GetSystemFontname(string name, bool fixedwidth, QFontEncoding encoding)
{
    UNUSED_ARGUMENT(encoding) ;

    string fontname ;
    MyFontEnumerator fontEnumerator;

    fontEnumerator.EnumerateFacenames(QFontENCODING_SYSTEM, fixedwidth);

    if ( fontEnumerator.GotAny() )
    {
        int nFacenames = fontEnumerator.GetFacenames().GetCount();
        string *facenames = new string[nFacenames];
        for (long n = 0; n < nFacenames; n++ )
        {
            facenames[n] = fontEnumerator.GetFacenames().Item(n);
        }

        size_t index = 100 ;          // the match must be fairly accurate

        // get the best (lowest) match
        for(long n = 0; n < nFacenames; n++)
        {
            size_t value = uiLevenshteinDistance(name.ToStdString(), facenames[n].ToStdString()) ;
            if(value < index)
            {
                fontname = facenames[n] ;
                index = value ;
            }
        }

        delete [] facenames ;
    }

    return fontname ;
}



// builds a font compparator list for outside this class (doesn't take into account proportional or not)
void cWordstarFile::BuildFontList(void)
{
    MyFontEnumerator fontEnumerator;

    fontEnumerator.EnumerateFacenames(QFontENCODING_SYSTEM, false);

    if ( fontEnumerator.GotAny() )
    {
        int nFacenames = fontEnumerator.GetFacenames().GetCount();
        string *facenames = new string[nFacenames];
        for (long n = 0; n < nFacenames; n++ )
        {
            facenames[n] = fontEnumerator.GetFacenames().Item(n);
        }

        string fontname ;
        for(size_t loop = 0; loop < mFontNames.size(); loop++)
        {
            size_t index =100 ; // = 5 ;          // the match must be fairly accurate

            // get the best (lowest) match
            for(long n = 0; n < nFacenames; n++)
            {
                size_t value = uiLevenshteinDistance(mFontNames[loop], (std::string)facenames[n].mb_str()) ;
                if(value < index)
                {
                    fontname = facenames[n] ;
                    index = value ;
                }
            }

            gSysFontName.push_back(fontname) ;
        }

        delete [] facenames ;

    }
}



size_t cWordstarFile::uiLevenshteinDistance(const std::string &s1, const std::string &s2)
{
    // before we try to compute difference, lets see if we get a word match
    if(s2.find(s1) != string::npos)
    {
        return 0 ;
    }
    if(s1.find(s2) != string::npos)
    {
        return 0 ;
    }
    
    
    int s1len = s1.size();
    int s2len = s2.size();

    auto column_start = (decltype(s1len))1;

    auto column = new decltype(s1len)[s1len + 1];
    std::iota(column + column_start, column + s1len + 1, column_start);

    for (auto x = column_start; x <= s2len; x++) 
    {
        column[0] = x;
        auto last_diagonal = x - column_start;
        for (auto y = column_start; y <= s1len; y++) 
        {
            auto old_diagonal = column[y];
            auto possibilities = 
            {
                column[y] + 1,
                column[y - 1] + 1,
                last_diagonal + (s1[y - 1] == s2[x - 1]? 0 : 1)
            };
            column[y] = std::min(possibilities);
            last_diagonal = old_diagonal;
        }
    }
    auto result = column[s1len];
    delete[] column;
    return result;
}
*/

/*
size_t cWordstarFile::uiLevenshteinDistance(const std::string &s1, const std::string &s2)
{
  const size_t m(s1.size());
  const size_t n(s2.size());

  if( m==0 ) return n;
  if( n==0 ) return m;

  size_t *costs = new size_t[n + 1];

  for( size_t k=0; k<=n; k++ ) costs[k] = k;

  size_t i = 0;
  for ( std::string::const_iterator it1 = s1.begin(); it1 != s1.end(); ++it1, ++i )
  {
    costs[0] = i+1;
    size_t corner = i;

    size_t j = 0;
    for ( std::string::const_iterator it2 = s2.begin(); it2 != s2.end(); ++it2, ++j )
    {
      size_t upper = costs[j+1];
      if( *it1 == *it2 )
      {
		  costs[j+1] = corner;
	  }
      else
	  {
		size_t t(upper<corner?upper:corner);
        costs[j+1] = (costs[j]<t?costs[j]:t)+1;
	  }

      corner = upper;
    }
  }

  size_t result = costs[n];
  delete [] costs;

  return result;
}
*/



/// @}


