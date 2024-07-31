#include "rtfstate.h"

cRTFState::cRTFState()
{
    Reset() ;
}



void cRTFState::CopyInto(cRTFState &newstate)
{
    newstate.mBold = mBold ;
    newstate.mItalics = mItalics ;
    newstate.mUnderline = mUnderline ;
    newstate.mStrikethrough = mStrikethrough ;
    newstate.mHidden = mHidden ;
    newstate.mDnup = mDnup ;
    newstate.mSubscript = mSubscript ;
    newstate.mSuperscript = mSuperscript ;
    newstate.mFontsize = mFontsize ;
    newstate.mFont = mFont ;
    newstate.mTextcolor = mTextcolor ;
    newstate.mBackgroundcolor = mBackgroundcolor ;
    newstate.mSmallCaps = mSmallCaps ;
    newstate.mAlign = mAlign ;

}



bool cRTFState::Compare(cRTFState &newstate)
{
    return    mBold == newstate.mBold
           && mItalics == newstate.mItalics
            && mUnderline == newstate.mUnderline
            && mStrikethrough == newstate.mStrikethrough
            && mHidden == newstate.mHidden
            && mDnup == newstate.mDnup
            && mSubscript == newstate.mSubscript
            && mSuperscript == newstate.mSuperscript
            && mFontsize == newstate.mFontsize
            && mFont == newstate.mFont
            && mTextcolor == newstate.mTextcolor
            && mBackgroundcolor == newstate.mBackgroundcolor
            && mSmallCaps == newstate.mSmallCaps ;
}




void cRTFState::Reset(void)
{
    // character format
    mBold = false ;
    mItalics = false ;
    mUnderline = false ;
    mStrikethrough = false ;
    mHidden = false ;
    mDnup = 0 ;
    mSubscript = false ;
    mSuperscript = false ;
    mSmallCaps = false ;

    // paragraph format
    mFontsize = 0 ;
    mFont = 0 ;
    mTextcolor = 0 ;
    mBackgroundcolor = 0 ;
    mLineSpace = 0.0 ;
}
