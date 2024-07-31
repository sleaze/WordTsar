#ifndef CEDITORBASE_H
#define CEDITORBASE_H

#include <string>

#include "src/core/include/config.h"
#include "src/core/document/document.h"

//using namespace std ;

class cEditorBase
{
public:
    cEditorBase();

public :
    std::string mFileDir ;                                   ///< the directory for the working file
    std::string mFileName ;                                  ///< the filename of the working file
    bool mFileSet ;                                     ///< true if a valid file name is set, else false

    cDocument mDocument ;                               ///< the document itslef (buffer, meta, etc)

protected:
    bool mFirstPaint;									///< flag to clear the DC on the first paint
    std::vector<COORD_T> mTabStops ;                         ///< tab positions in twips
    std::string mEditorName ;                                ///< the name of the editor

    LINE_T mFirstLine ;                                 ///< first line being displayed
    LINE_T mLastFirstLine ;                             ///< what was the first line the last time we went through
    LINE_T mDisplayLines ;                              ///< how many lines are in our window
    LINE_T mCurrentLine ;                               ///< the line number on our page
    PAGE_T mCurrentPage ;                               ///< the page we are on
    size_t mCurrentHeight ;                             ///< vertical position on page in twips
    size_t mTopMargin ;                                 ///< current page top margin

    POSITION_T mLastFindandReplace ;                    ///< the last find and replace position (or zero if no find done)

    PARAGRAPH_T mLastParagraph ;                        ///< to cleanup wxGTK carat drawing (temp)

    bool mDoDrawCaret ;
    bool mLayoutRest ;                                  ///< if true, layout rest of document
    bool mLayoutInt ;                                   ///< if true, layout was interrupted by user input
    bool mDrawFullDisplay ;                             ///< if true, redraw full display
    bool mHardReturn ;                                  ///< true if hard return done (display optimization)

    long mSaveLayoutPara ;                              ///< the paragraph we are on before interuppted
    long mWordCount ;                                   ///< word count of document

    bool mIsHelp ;                                      ///< true if editor is help display, else false
    bool mWordWrap ;                                    ///< true if word wrapping, else false
    eShowControl mShowControl ;                         ///< true if showing controls, else false
    bool mShowDot ;                                     ///< true if doing layout for printing, else false
    bool mInPrintPreview ;                              ///< true if in print preview (no editor displat updates), else false

    bool mFullScreen ;                                  ///< true if in full screen mode, else false

    std::string mCopyBuffer ;                                ///< the buffer used for copy/paste

    bool mDoSearch ;                                    ///< true doing search - false doing replace
    bool mSearchBackwards ;                             ///< are we backwards?
    bool mWholeWord ;                                   ///< find whole words only
    bool mWildCard ;                                    ///< do we have wildcards
    bool mCaseCmp ;                                     ///< ingnore case
    bool mWholeFile ;                                   ///< search the whole file
    std::string mSearchText ;                                ///< the text we are searching for

    bool mReplaceAsk ;                                  ///< ask on replace?
    POSITION_T mReplaceSize ;                           ///< size of text to cut
    std::string mReplaceText ;                               ///< text to replace with

    bool mHideCaret;                                    ///< whether to display caret or not
    bool mDrawnCaret;                                   ///< true if caret is displayed, else false

    bool mStatusBold ;                                  ///< set by layout engine for status control indicators
    bool mStatusItalic ;                                ///< set by layout engine for status control indicators
    bool mStatusUnderline ;                             ///< set by layout engine for status control indicators
    eJustification mStatusJust ;                        ///< set by layout engine for status control indicators

    bool mFirstIdle ;                                   ///< force a screen redraw on first idle event

    bool mIsReady ;                                     ///< is the window ready to draw on

};

#endif // CEDITORBASE_H
