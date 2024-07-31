#ifndef CRTFSTATE_H
#define CRTFSTATE_H

#include <string>

class cRTFState
{
public:
    cRTFState() ;

    void CopyInto(cRTFState &newstate) ;
    bool Compare(cRTFState &newstate) ;
    void Reset(void) ;

    // character format state
    bool mBold ;
    bool mItalics ;
    bool mUnderline ;
    bool mStrikethrough ;
    bool mHidden ;
    // Attribute that specifies that the text should be beneath the baseline ("down", negative) or above the baseline ("up", positive) by N.
    // RTF "dnN" move down N half-points; does not imply font size reduction, thus font size is given separately --> value negative from param, fontsize unchanged.
    // RTF "upN" move up N half-points; does not imply font size reduction, thus font size is given separately --> value positive from param, fontsize unchanged.
    int mDnup ;
    bool mSubscript ;
    bool mSuperscript ;
    bool mSmallCaps ;

    // paragraph state
    int mFontsize ;
    int mFont ;
    int mTextcolor ;
    int mBackgroundcolor ;
    double mLineSpace ;
    std::string mAlign ;
    double mSpaceAfter ;         // spacing after this paragraph /sa
    double mSpaceBefore ;        // spacing before this paragraph /sb
    double mIndentFirst ;        // first line indent
    double mIndentPara ;         // paragraph indent
    double mIndentRight ;        // right indent
    bool mHyphennate ;           // hyphenation on or off

    int mMarginLeft ;
    int mMarginRight ;
    int mMarginTop ;
    int mMarginBottom ;

};

#endif // CRTFSTATE_H
