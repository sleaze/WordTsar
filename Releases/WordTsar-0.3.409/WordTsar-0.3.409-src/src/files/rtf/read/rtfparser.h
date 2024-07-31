#ifndef CRTFPARSER_H
#define CRTFPARSER_H

#include <string>
#include <stack>
#include <vector>

#include "src/files/rtffile.h"

#include "rtftext.h"
#include "rtfcontrolsymbol.h"
#include "rtfcontrolword.h"
#include "rtfgroup.h"
#include "rtfstate.h"

#include "src/core/document/document.h"


//enum eAlign
//{
//    ALIGNCENTER,
//    ALIGNLEFT,
//    ALIGNRIGHT,
//    ALIGNJUSTIFY
//} ;


struct sRTFFontTable
{
    int number ;
    std::string name ;
    std::string altname ;
    std::string family ;
//    eControls family ;
    int charset ;               ///< @TODO not used yet
//    int pitch ;                 ///< not used yet
} ;

class cRTFFile ;

class cRTFParser
{
public:
    cRTFParser(FILE *fp, cDocument *doc, cRTFFile *rtffile);

private:
    void GetChar(void) ;
    void Parse(void) ;

    void ParseStartGroup(void) ;
    void ParseEndGroup(void) ;
    void ParseControl(void) ;
    void ParseControlWord(void) ;
    void ParseControlSymbol(void) ;
    void ParseText(void) ;

    void InsertRTF(void) ;
    void FormatGroup(cRTFGroup *group) ;
    void FormatControlWord(cRTFControlWord *word) ;
    void FormatControlSymbol(cRTFControlSymbol *symbol) ;
//    void FormatText(cRTFText *text) ;

    void PushState(void) ;
    void PopState(void) ;
    void ResetState(void) ;


    void GetFontTable(std::vector<cRTFElement *>element) ;

//    void ApplyStyle(std::string &text) ;

    void CheckFirstChar(void) ;

    void DoChanges(void) ;

    void EmitText(std::string &text) ;

    void EmitParagraph(void) ;
    void EmitPage(void) ;
    void EmitTab(void) ;

    void EmitParagraphSpace(void) ;
//            void EmitKeepLines(void) ;
//            void EmitKeepNext(void) ;
    void EmitIndent(void) ;
//            void EmitNumbering(void) ;
//            void EmitOutlineLevel(void) ;
//            void EmitBorder(void) ;
//            void EmitShading(void) ;
//            void EmitTabs(void) ;
    void EmitJustify(void) ;
    void EmitFont(void) ;
    void EmitAttributes(void) ;

/*
    void EmitBold(int param) ;
    void EmitItalic(int param) ;
    void EmitUnderline(int param) ;
    void EmitSubScript(int paramd) ;
    void EmitSuperScript(int param) ;
    void EmitStrikeThrough(int param) ;
    void EmitSmallCaps(int param) ;

    void EmitTextColor(int index) ;
    void EmitBackgroundColor(int index) ;
    void EmitPage(void) ;
    void EmitTab(void) ;
    void EmitCenter(eAlign param) ;
    void EmitLeft(eAlign param) ;
    void EmitRight(eAlign param) ;
    void EmitJustify(eAlign param) ;
    void EmitLineSpace(int ls) ;
    void EmitIndentFirst(int index) ;
    void EmitIndentParagraph(int index) ;
    void EmitIndentRight(int index) ;
    void EmitFont(int index, int size) ;
    void EmitParagraph(void) ;
    void EmitSpaceAfter(int param) ;
    void EmitSpaceBefore(int param) ;
*/
private:
    cDocument *mDocument ;
    cRTFFile *mRTFFile ;                    // for progress updates

    bool mRequireHardReturn ;       ///< true if we need a hard return, else false
    std::string mRTF ;                   // the RTF std::string
    char mChar ;                    // the current character from the RTF std::string
    size_t mRTFIndex ;              // index into the RTF std::string

//    int mFontIndex ;                // the font index when we see a 'f' command
//    int mFontSize ;                 // the font size when we see a 'fs' command
//    int mLastFontIndex ;                // the last emitted font index
//    int mLastFontSize ;                 // the last emitted font size

//    int mLastLineSpace ;            // the last line space emitted

    bool mFirstColumn ;             ///< true if in first column, else false

    int mPaperwidth ;
    int mPaperHeight ;
//    int mMarginLeft ;
//    int mMarginRight ;
//    int mMarginTop ;
//    int mMarginBottom ;

    cRTFGroup *mGroup ;          // current working group
    cRTFGroup *mRoot ;          // root of tree

    cRTFState mRTFState ;
    cRTFState mRTFPrevState ;
    std::stack<cRTFState> mState ;

    std::vector<sRTFFontTable> mFontTable ;

    long mIgnoreTable ;
    bool mHitRowCommand ;
    bool mInTable ;

};

#endif // CRTFPARSER_H
