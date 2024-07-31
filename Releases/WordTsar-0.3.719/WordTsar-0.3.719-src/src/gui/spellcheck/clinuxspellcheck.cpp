/////////////////////////////////////////////////////////////////////////////
///
///  clinuxspellcheck.cpp
///
///  Project  :  Wordstar
///
///  Copyright (c) 2008, 2009, 2010  Majentis Technologies
///
///  @author  Gerald Brandt
///
///  @note   Spell Checker for Linux
///
///  $Id$
///
/////////////////////////////////////////////////////////////////////////////

/*

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/textfile.h>

#include "clinuxspellcheck.h"
#include "cspellcheck.h"
#include "centerword.h"


// the following define makes a copy of the text buffer for spell check, and it works.
// if this is commented, the original buffer is checked, and it doesn't clear punctuation
// or other non alpha charcaters
#define SPELL_COPY 1

cHunspellSpellCheck::cHunspellSpellCheck(cEditorCtrl *editor)
{
    mEditor = editor ;

//    mDictionary = Hunspell_create("/usr/share/hunspell/en_US.aff", "/usr/share/hunspell/en_US.dic");
    mDictionary = Hunspell_create("/usr/share/hunspell/en_US.aff", "/usr/share/hunspell/en_US.dic");
//    mCustomDictionary = Hunspell_create("~/.Wordtsar_Custom/custom.aff", "~/.Wordtsar_Custom/custom.dic") ;

    QString home = wxGetHomeDir() ;
    QString filename = home + "/.Wordtsar_custom_dictionary" ;
    wxTextFile file(filename) ;
    if(file.Exists() == false)
    {
        file.Create() ;
    }

    file.Open() ;
    if(file.IsOpened())
    {
        QString str ;
        for ( str = file.GetFirstLine(); !file.Eof(); str = file.GetNextLine() )
        {
            Hunspell_add(mDictionary, static_cast<const char *>(str.char_str())) ;
        }

        file.Close() ;
    }
}

cHunspellSpellCheck::~cHunspellSpellCheck()
{
    Hunspell_destroy(mDictionary) ;
}



void cHunspellSpellCheck::CheckDocument(void)
{
    bool quit = false ;
    QString replace = wxT("\\.()[]{},;:?+-*=/&~|\"<>!\r\n\t#$%^&") ;           // do not break apostrophes (closer to MS Word wordcount)
    replace += static_cast<QChar>(HARD_RETURN) ;
    
    QString newchar = " " ;

    // we set the show control chars so we get correct offsets into buffer
//    bool showcontrols = mEditor->GetShowControls() ; 
    mEditor->SetShowControls(SHOW_ALL) ;

    // save block start and stop
    size_t bstart = mEditor->mStartBlock ;
    size_t bend = mEditor->mEndBlock ;
    bool bset = mEditor->mBlockSet ;

    // build a string of each paragraph
    PARAGRAPH_T paras = mEditor->mDocument.GetNumberofParagraphs() ;
    PARAGRAPH_T currentparagraph = mEditor->mDocument.GetParagraphFromPosition(mEditor->mDocument.GetPosition()) ;
    for(PARAGRAPH_T loop = currentparagraph; loop < paras; loop++)
    {
        if(quit == true)
        {
            break ;
        }
        
        QString text1 = mEditor->mDocument.GetParagraphText(loop) ;

        // replace from replace list above
        // we do this here to allow leading .'s on comments line to go through
        QString lead = text1[0] ;
        bool replacedot = false ;
        if(lead[0] == '.')
        {
            replacedot = true ;
        }

        for(size_t loop = 0; loop < replace.length(); loop++)
        {
            // we want to keep leading dots.
            QString oldchar = replace[loop] ;
            text1.Replace(oldchar, newchar, true) ;
        }

        if(replacedot == true)
        {
            text1[0] = lead[0] ;        // replace first char
        }

        // and now a loop for control charcaters and contractions
        for(size_t loop = 0; loop < text1.length(); loop++)
        {
            if(text1[loop] <= STYLE_END_OF_STYLES)
            {
                text1[loop] = ' ' ;
            }

            // Hunspell doesn't like apostropies around words, but want's them in contractions, so run a special check
            if(text1[loop] == '\'')
            {
                if(loop != 0)
                {
                    // if the apostrophe is not surround by text (non white space), replace it
                    if(text1[loop - 1] == ' ' || text1[loop + 1] == ' ')
                    {
                        text1[loop] = ' ' ;
                    }
                }
            }
        }
        
        // do the spell check
        // brute force
        bool space = false ;
        QChar currentchar ;
        POSITION_T wordstart = 0 ;
        POSITION_T wordend = 0 ;
        QString word ;

//        ssize_t oldline = -1 ;
//        ssize_t newline ;
//        bool firstline = true ;
        POSITION_T parastart, paraend ;
        mEditor->mDocument.GetParagraphStartandEnd(loop, parastart, paraend ) ;

        for ( auto i = text1.begin(); i != text1.end(); ++i )
        {
            currentchar = *i;
            if(currentchar == ' ' && space == false)        // got space after a non-space
            {
                // if we are on a space and have a word, spell check it.
                if(word.size() != 0)
                {
                    if(text1[0] != '.')          // don't spell check dot commands
                    {
                        mEditor->mDocument.SetPosition(wordstart + parastart - 1) ;
                        mEditor->mStartBlock = wordstart + parastart ;
                        mEditor->mEndBlock = wordend + parastart ;
                        mEditor->mBlockSet = true ;
                        mEditor->MoveRight() ;                      // scrolls the word into view

                        eWordRet cont = CheckWord(word, parastart) ;
                        if(cont == SPELL_QUIT)
                        {
                            quit = true ;
                            break ;
                        }
                    }
                }

                space = true ;
                wordstart = i - text1.begin() + 1 ;
                word.clear() ;
            }
            else if(currentchar == ' ')
            {
                space = true ;
                wordstart = i - text1.begin() + 1 ;
                word.clear() ;
            }
            else if(currentchar != ' ')
            {
                space = false ;
                word += currentchar ;
                wordend = i - text1.begin()  + 1 ;
            }
        }
    }

    mEditor->mStartBlock = bstart  ;
    mEditor->mEndBlock = bend ;
    mEditor->mBlockSet = bset ;
}



eWordRet cHunspellSpellCheck::CheckWord(QString &word, POSITION_T &blockoffset, bool showgood)
{
    eWordRet retval = SPELL_CONTINUE ;
    long morestart = 0 ;

    int bad = Hunspell_spell(mDictionary, static_cast<const char *>(word.char_str())) ;
    if((bad == 0) || (showgood == true))
    {
        mEditor->Refresh() ;
        wxYield() ;

moreloop:
        cSpellCheck spellcheck(mEditor) ;
//        mEditor->SetCaretPosition(wordstart + blockoffset) ;
//        mEditor->mStartBlock = wordstart + blockoffset ;
//        mEditor->mEndBlock = wordend + blockoffset ;
//        mEditor->mBlockSet = true ;

        char **suggestions ;
        int list_size = Hunspell_suggest(mDictionary, &suggestions, static_cast<char *>(word.char_str()));

        if(list_size != 0)
        {
            long tmp = spellcheck.mSpellList->InsertItem(0, "1. ");
            spellcheck.mSpellList->SetItem(tmp, 1, suggestions[morestart]) ;
        }
        if(morestart + 1 < list_size)
        {
            long tmp = spellcheck.mSpellList->InsertItem(1, "2. ");
            spellcheck.mSpellList->SetItem(tmp, 1, suggestions[morestart + 1]) ;
        }
        if(morestart + 2 < list_size)
        {
            long tmp = spellcheck.mSpellList->InsertItem(2, "3. ");
            spellcheck.mSpellList->SetItem(tmp, 1, suggestions[morestart + 2]) ;
        }
        if(morestart + 3 < list_size)
        {
            long tmp = spellcheck.mSpellList->InsertItem(3, "4. ");
            spellcheck.mSpellList->SetItem(tmp, 1, suggestions[morestart + 3]) ;
        }
        if(morestart + 4 < list_size)
        {
            long tmp = spellcheck.mSpellList->InsertItem(4, "5. ");
            spellcheck.mSpellList->SetItem(tmp, 1, suggestions[morestart + 4]) ;
        }


        spellcheck.mSpellList->SetColumnWidth(0, wxLIST_AUTOSIZE) ;
        spellcheck.mSpellList->SetColumnWidth(1, wxLIST_AUTOSIZE) ;

        spellcheck.mBadWord->SetDefaultStyle(wxTextAttr(*wxRED)) ;
        spellcheck.mBadWord->SetValue(word) ;
        int ret = spellcheck.ShowModal() ;
        if(ret == wxID_IGNORE)
        {
            retval = SPELL_CONTINUE ;
        }
        else if(ret == wxID_CANCEL)
        {
            retval = SPELL_QUIT ;
        }
        else if(ret == ID_SPELL_MORE)
        {
            morestart += 5 ;
            if((morestart > 15) || (morestart > list_size))
            {
                morestart = 0 ;
            }
            goto moreloop ;
        }
        else if(ret == ID_SPELL_ENTER)
        {
            cEnterWord enter(mEditor) ;
            enter.mWord->SetFocus() ;
            int ret = enter.ShowModal() ;

            if(ret == wxID_OK)
            {
                // block offset is old word length - new word length
                QString newword = enter.mWord->GetValue() ;
                blockoffset += newword.length() - (mEditor->mEndBlock - mEditor->mStartBlock) ;

                mEditor->DeleteBlock() ;
                mEditor->mDocument.SetPosition(mEditor->mStartBlock) ;
                mEditor->InsertWordStarString(newword) ;
                retval = SPELL_CONTINUE ;
            }
        }
        else if(ret == ID_SPELL_ADD)
        {
//                            morestart = 0 ;
            QString str ;
            str.Printf("Add \"%s\" to dictionery as correct spelling?", word) ;
            int answer = wxMessageBox(str, "Add to dictionary", wxYES_NO | wxCENTER, mEditor) ;
            if(answer == wxYES)
            {
                Hunspell_add(mDictionary, static_cast<const char *>(word.char_str())) ;

                QString home = wxGetHomeDir() ;
                QString filename = home + "/.Wordtsar_custom_dictionary" ;
                wxTextFile file(filename) ;
                file.Open() ;
                if(file.IsOpened())
                {
                    file.AddLine(word) ;
                    file.Write() ;
                    file.Close() ;
                }
                retval = SPELL_CONTINUE ;
            }
            else
            {
                goto moreloop ;
            }
        }
        else if(
                   (ret == ID_SPELL_SELECT1)
                || (ret == ID_SPELL_SELECT2)
                || (ret == ID_SPELL_SELECT3)
                || (ret == ID_SPELL_SELECT4)
                || (ret == ID_SPELL_SELECT5)
                )
        {
//                            morestart = 0 ;
            int index = ret - ID_SPELL_SELECT1 + morestart ;
            if(index < list_size)
            {
                // block offset is old word length - new word length
                blockoffset += strlen(suggestions[index]) - (mEditor->mEndBlock - mEditor->mStartBlock) ;

                mEditor->DeleteBlock() ;
                mEditor->mDocument.SetPosition(mEditor->mStartBlock) ;
                mEditor->InsertWordStarString(suggestions[index]) ;
                retval = SPELL_CONTINUE ;
            }
            else
            {
                goto moreloop ;
            }
        }
    }

    return retval ;
}
*/
