#include "rtfparser.h"

using namespace std ;


cRTFParser::cRTFParser(FILE *fp, cDocument *doc, cRTFFile *rtffile)
{
    mDocument = doc ;
    mRTFFile = rtffile ;

    mGroup = nullptr ;
    mRoot = nullptr ;

    mFirstColumn = true ;
/*
    mFontIndex = -1 ;

    mLastFontIndex = -1 ;
    mLastFontSize = -1 ;

    mLastLineSpace = 0 ;
*/
    mRequireHardReturn = false ;
    mIgnoreTable = 0 ;

    mRTFState.Reset() ;

    mRTF.clear() ;
    if(mGroup != nullptr)
    {
        delete mGroup ;
    }
    mGroup = nullptr ;

    // move RTF file to a string
    while(true)
    {
        char ch = static_cast<char>(fgetc(fp)) ;
        if(ch == EOF)
        {
            break ;
        }
//        if(ch != 10 && ch != 13)
        {
            mRTF.push_back(ch) ;
        }
//        else
//        {
//            mRTF.push_back(' ') ;
//        }
    }

    Parse() ;

    if(mRoot != nullptr)
    {
        mRoot->dump(0) ;
        InsertRTF() ;
    }
}


void cRTFParser::GetChar(void)
{
    if(mRTFIndex < mRTF.length())
    {
        mChar = mRTF[mRTFIndex] ;
        mRTFIndex++ ;
    }
    else
    {
        mChar = 0 ;
    }
}


void cRTFParser::Parse(void)
{
    mRTFIndex = 0 ;
    mGroup = nullptr ;
    mRoot = nullptr ;

    long modulo = mRTF.length() / 100 ;
    if(modulo == 0)
    {
        modulo = 1 ;
    }

    while(mRTFIndex < mRTF.length())
    {
        int percent =  static_cast<int>(static_cast<double>(mRTFIndex) / static_cast<double>(mRTF.length()) * 100.0) ;
//        if(mRTFIndex % modulo == 0)
        {
//            string temp1 ;
            mRTFFile->UpdateProgress(percent) ;
//            oldpercent = percent ;
        }

        GetChar() ;
        if(mChar == 0)
        {
            break ;
        }

        // ignore carriage returns and linefeeds
        if(mChar == '\n' || mChar == '\r')
        {
            continue ;
        }

        switch(mChar)
        {
            case '{' :
                ParseStartGroup() ;
                break ;

            case '}' :
                ParseEndGroup() ;
                break ;

            case '\\' :
                ParseControl() ;
                break ;

            default :
                ParseText() ;
                break ;

        }
    }
}


void cRTFParser::ParseStartGroup(void)
{
    cRTFGroup *newgroup = new cRTFGroup ;
    if(mGroup != nullptr)
    {
        newgroup->mParent = mGroup ;
    }

    if(mRoot == nullptr)
    {
        mGroup = newgroup ;
        mRoot = newgroup ;
    }
    else
    {
        mGroup->mChildren.push_back(newgroup) ;
        mGroup = newgroup ;
    }
}


void cRTFParser::ParseEndGroup(void)
{
    mGroup = mGroup->mParent ;
}


void cRTFParser::ParseControl(void)
{
    GetChar() ;
    mRTFIndex-- ;

    if(isalpha(mChar))
    {
        ParseControlWord() ;
    }
    else
    {
        ParseControlSymbol() ;
    }
}



void cRTFParser::ParseControlWord(void)
{
    GetChar() ;

    string word ;

    while(isalpha(mChar))
    {
        word += mChar ;
        GetChar() ;
    }

    int parameter = -1 ;
    bool negative = false ;

    if(mChar == '-')
    {
        GetChar() ;
        negative = true ;
    }

    while(isdigit(mChar))
    {
        if(parameter == -1)
        {
            parameter = 0 ;
        }
        parameter = parameter * 10 + (mChar - '0') ;
        GetChar() ;
    }

    if(parameter == -1)
    {
        parameter = 1 ;
    }
    if(negative)
    {
        parameter = -parameter ;
    }


    // if this is a 'u', then parameter will be folowed by a character
    if(word == "u")
    {
        // ignore space delimiter
        if(mChar == ' ')
        {
            GetChar() ;
        }

        // if the replacement charcater is in hex \'hh then skip it
        if(mChar == '\\' && mRTF[mRTFIndex] == '\'')
        {
            mRTFIndex += 3 ;
        }

        // Convert to UTF unsigned decimal
        if(negative)
        {
            parameter += 65536 ;
        }
    }
    // if the current char is a space, then its a delimiter. It's consumed
    // if its not a space, then its part of the next item in the text, so put character back
    else
    {
        if(mChar != ' ')
        {
            mRTFIndex-- ;
        }
    }

    cRTFControlWord *RTFWord = new cRTFControlWord ;
    RTFWord->mWord = word ;
    RTFWord->mParameter = parameter ;
    mGroup->mChildren.push_back(RTFWord) ;
}



void cRTFParser::ParseControlSymbol(void)
{
    GetChar() ;

    char symbol = mChar ;

    // symbols normally have no parameter. But, if this is a ', then it is followed by a two digit hex code
    int parameter = 0 ;
    if(symbol == '\'')
    {
        GetChar() ;
        string hex ;
        hex += mChar ;

        GetChar() ;
        hex += mChar ;

        parameter =  static_cast<int>(strtol(hex.c_str(), nullptr, 16)) ; //  hexdec(hex) ;
    }

    cRTFControlSymbol *RTFSymbol = new cRTFControlSymbol ;
    RTFSymbol->mSymbol = symbol ;
    RTFSymbol->mParameter = parameter ;
    mGroup->mChildren.push_back(RTFSymbol) ;
}




/////////////////////////////////////////////////////////////////////////////
///
/// @return bool - true success else fail
///
/// @brief
/// parse plain text up to a back slash (\) or brace (unless escaped)
///
/////////////////////////////////////////////////////////////////////////////
void cRTFParser::ParseText(void)
{
    string text ;
    bool quit = false ;

    do
    {
        quit = false ;

//        // ignore carriage returns and linefeeds
//        if(mChar == '\n' || mChar == '\r')
//        {
//            GetChar() ;
//            continue ;
//        }

        // is this an escaped char?
        if(mChar == '\\')
        {
            GetChar() ;

            switch(mChar)
            {
                case '\\' :
                case '{' :
                case '}' :
                    break ;

                default :
                    // not an escape char, roll back
                    mRTFIndex -= 2 ;
                    quit = true ;
                    break ;

            }
        }
        else if(mChar == '{' || mChar == '}')
        {
            mRTFIndex-- ;
            quit = true ;
        }

        if(!quit)
        {
            if(mChar != 10 && mChar != 13)
            {
                text += mChar ;
            }
            GetChar() ;
        }
    } while(!quit && mRTFIndex < mRTF.length()) ;


    if(mGroup != nullptr)
    {
        cRTFText *rtftext = new cRTFText ;
        rtftext->mText = text ;

        mGroup->mChildren.push_back(rtftext) ;
    }
}



void cRTFParser::InsertRTF(void)
{
    ResetState() ;
    mIgnoreTable = 0 ;
    mInTable = false ;

    FormatGroup(mRoot) ;
}


void cRTFParser::FormatGroup(cRTFGroup *group)
{
    string type = group->GetType() ;

    PushState() ;

    if(type == "fonttbl")
    {
        GetFontTable(group->mChildren) ;
//        return ;
    }

    else if(type == "colortbl")
    {
//        GetColorTable(group->mChildren) ;
//        return ;
    }

    else if(type == "stylesheet")
    {
//        GetStyleSheet(group->mChildren) ;
//        return ;
    }

    else if(type == "info")
    {
//        GetInfo(group->mChildren) ;
//        return ;
    }

    else if(type == "pict")
    {
//        GetPicture(group->mChildren) ;
//        return ;
    }

    else if(type.substr(0, 6) == "footer")
    {
        // handle footers
    }

    else if(type.substr(0, 6) == "header")
    {
        // handle headers
    }

    else if(group->IsDestination())
    {
//        Handle Destination(group->mChildren) ;
//        return ;
    }

    else
    {
        // take care of all child nodes
        for(size_t loop = 0; loop < group->mChildren.size(); loop++)
        {
            switch(group->mChildren[loop]->mType)
            {
                case eRTFTypeGroup :
                    FormatGroup(static_cast<cRTFGroup *>(group->mChildren[loop])) ;
                    break ;

                case eRTFTypeControlWord :
                    FormatControlWord(static_cast<cRTFControlWord *>(group->mChildren[loop])) ;
                    break ;

                case eRTFTypeControlSymbol :
                    FormatControlSymbol(static_cast<cRTFControlSymbol *>(group->mChildren[loop])) ;
                    break ;

                case eRTFTypeText :
                    EmitText(static_cast<cRTFText *>(group->mChildren[loop])->mText) ;
//                    FormatText(static_cast<cRTFText *>(group->mChildren[loop])) ;
                    break ;

            }
        }
    }

    PopState() ;
}



// this is the big one. every RTF command comes through here
void cRTFParser::FormatControlWord(cRTFControlWord *word)
{
    string control = word->mWord ;
    int parameter = word->mParameter ;

    switch(control[0])
    {
        case 'a' :
            break ;

        case 'b' :
            if(control == "b")
            {
                mRTFState.mBold = parameter ;
//                EmitBold(parameter) ;
            }
            break ;

        case 'c' :
            break ;

        case 'd' :
            if(control == "deff")
            {
                mRTFState.mFont = parameter ;
                mRTFState.mFontsize = 24 ;
            }
            break ;

        case 'e' :
            break ;

        case 'f' :
            if(control == "fi")
            {
                mRTFState.mIndentFirst = parameter ;
//                EmitIndentFirst(parameter) ;
            }
            else if(control == "fs")
            {
                mRTFState.mFontsize = parameter ;
//                mFontSize = parameter ;
            }
            else if(control == "f")
            {
                mRTFState.mFont = parameter ;
//                mFontIndex = parameter ;
            }
            break ;

        case 'g' :
            break ;

        case 'h' :
            break ;

        case 'i' :
            if(control == "i")
            {
                mRTFState.mItalics = parameter ;
//                EmitItalic(parameter) ;
            }
            break ;

        case 'j' :
            break ;

        case 'k' :
            break ;

        case 'l' :
            if(control == "li")
            {
                mRTFState.mIndentPara = parameter ;
//                EmitIndentParagraph(parameter) ;
            }
            break ;

        case 'm' :
            if(control == "margl")
            {
                mRTFState.mMarginLeft = parameter ;
//                mMarginLeft = parameter ;
            }
            else if (control == "margr")
            {
                mRTFState.mMarginRight = parameter ;
//                mMarginRight = parameter ;
            }
            else if(control == "margt")
            {
                mRTFState.mMarginTop = parameter ;
//                mMarginTop = parameter ;
            }
            else if(control == "margb")
            {
                mRTFState.mMarginBottom = parameter ;
//                mMarginBottom = parameter ;
            }
            break ;

        case 'n' :
            if(control == "nosupersub")
            {
                mRTFState.mSubscript = false ;
                mRTFState.mSuperscript = false ;
//                EmitSubScript(0) ;
//                EmitSuperScript(0) ;
            }
            break ;

        case 'o' :
            break ;

        case 'p' :
            if(control == "plain" || control == "pard")
            {
                ResetState() ;
            }
            else if(control == "par")
            {
                // we only emit a hard return when new text is displayed
                // unless we have multiple \par in a row
                if(mRequireHardReturn == true)
                {
                    EmitParagraph() ;
                }
                mRequireHardReturn = true ;
//                EmitParagraph() ;
            }
            else if(control == "page")
            {
                EmitPage() ;
            }
            else if(control == "paperw")
            {
                mPaperwidth = parameter ;
            }
            else if(control == "paperh")
            {
                mPaperHeight = parameter ;
            }

            break ;

        case 'q' :
            if(control == "qc")
            {
                mRTFState.mAlign = control ;
//                EmitCenter(ALIGNCENTER) ;
            }
            else if(control == "qr")
            {
                mRTFState.mAlign = control ;
//                 EmitRight(ALIGNRIGHT) ;
            }
            else if(control == "ql")
            {
                mRTFState.mAlign = control ;
//                EmitLeft(ALIGNLEFT) ;
            }
            else if(control == "qj")
            {
                mRTFState.mAlign = control ;
//                EmitJustify(ALIGNJUSTIFY) ;
            }
            break ;

        case 'r' :
            if(control == "ri")
            {
                mRTFState.mIndentRight = parameter ;
//                EmitIndentRight(parameter) ;
            }
            else if(control == "row")
            {
                mHitRowCommand = true ;
                if(mIgnoreTable != 0)
                {
                    mIgnoreTable-- ;
                }
            }
            break ;

        case 's' :
            if(control == "strike")
            {
                mRTFState.mStrikethrough = parameter ;
//                EmitStrikeThrough(parameter) ;
            }
            else if(control == "sub")
            {
                mRTFState.mSubscript = parameter ;
//                EmitSubScript(parameter) ;
            }
            else if(control == "super")
            {
                mRTFState.mSuperscript = parameter ;
//                EmitSuperScript(parameter) ;
            }
            else if(control == "scaps")
            {
                mRTFState.mSmallCaps = parameter ;
//                EmitSmallCaps(parameter) ;
            }
            else if(control == "sl")
            {
                mRTFState.mLineSpace = parameter ;
//                EmitLineSpace(parameter) ;
            }
            else if(control == "sa")
            {
                mRTFState.mSpaceAfter = parameter ;
//                EmitSpaceAfter(parameter) ;
            }
            else if(control == "sb")
            {
                mRTFState.mSpaceBefore = parameter ;
//                EmitSpaceBefore(parameter) ;
            }
            break ;

        case 't' :
            if(control == "tab")
            {
                EmitTab() ;
            }
            else if(control == "trowd")
            {
                // only increment row count if we hit a /row command to end last row, otherwise this is illegal RTF
                if(mHitRowCommand)
                {
                    mIgnoreTable++ ;
                }
                mHitRowCommand = false ;
                if(mIgnoreTable == 1 && mInTable == false)
                {
                    mDocument->Insert("\n<<< TABLE >>>\n") ;
                }
                mInTable = true ;
            }
            break ;

        case 'u' :
            if(control == "u")
            {
                DoChanges() ;
                mDocument->Insert(parameter) ;
            }
            else if(control == "ul")
            {
                mRTFState.mUnderline = parameter ;
//                EmitUnderline(parameter) ;
            }
            else if(control == "ulnone")
            {
                mRTFState.mUnderline = false ;
//                EmitUnderline(0) ;
            }
            break ;

        case 'v' :
            break ;

        case 'w' :
            break ;

        case 'x' :
            break ;

        case 'y' :
            break ;

        case 'z' :
            break ;

    }
}



void cRTFParser::FormatControlSymbol(cRTFControlSymbol *symbol)
{
}


/*
void cRTFParser::FormatText(cRTFText *text)
{
    if(mFontSize != mRTFState.mFontsize || mFontIndex != mRTFState.mFont)
    {
        EmitFont(mFontIndex, mFontSize) ;
        mRTFState.mFont = mFontIndex ;
        mRTFState.mFontsize = mFontSize ;
    }
    ApplyStyle(text->mText) ;
}
*/

void cRTFParser::PushState(void)
{
    mState.push(mRTFState) ;
}



void cRTFParser::PopState(void)
{
    cRTFState state ;

    if(mState.empty() == false)
    {
        state = mState.top() ;
        mState.pop() ;
/*
        if(state.mBold != mRTFState.mBold)
        {
            EmitBold(state.mBold) ;
        }
        if(state.mItalics != mRTFState.mItalics)
        {
            EmitItalic(state.mItalics) ;
        }
        if(state.mUnderline != mRTFState.mUnderline)
        {
            EmitUnderline(state.mUnderline) ;
        }
        if(state.mHidden != mRTFState.mHidden)
        {
//            EmitHidden(state.mHidden) ;
        }
        if(state.mDnup != mRTFState.mDnup)
        {
//            EmitDnup(state.mDnup) ;
        }
        if(state.mSubscript != mRTFState.mSubscript)
        {
            EmitSubScript(state.mSubscript) ;
        }
        if(state.mSuperscript != mRTFState.mSuperscript)
        {
            EmitSuperScript(state.mSuperscript) ;
        }
        if(state.mSmallCaps != mRTFState.mSmallCaps)
        {
            EmitSmallCaps(state.mSmallCaps) ;
        }
*/

        // paragraph stuff

        mRTFState = state ;
    }
}


void cRTFParser::ResetState(void)
{
    cRTFState state ;

//    mRTFState.CopyInto(state) ;
    state = mRTFState ;

    state.Reset() ;
/*
    if(state.mBold != mRTFState.mBold)
    {
        EmitBold(state.mBold) ;
    }
    if(state.mItalics != mRTFState.mItalics)
    {
        EmitItalic(state.mItalics) ;
    }
    if(state.mUnderline != mRTFState.mUnderline)
    {
        EmitUnderline(state.mUnderline) ;
    }
    if(state.mHidden != mRTFState.mHidden)
    {
//            EmitHidden(state.mHidden) ;
    }
    if(state.mDnup != mRTFState.mDnup)
    {
//            EmitDnup(state.mDnup) ;
    }
    if(state.mSubscript != mRTFState.mSubscript)
    {
        EmitSubScript(state.mSubscript) ;
    }
    if(state.mSuperscript != mRTFState.mSuperscript)
    {
        EmitSuperScript(state.mSuperscript) ;
    }
    if(state.mSmallCaps != mRTFState.mSmallCaps)
    {
        EmitSmallCaps(state.mSmallCaps) ;
    }
*/
//    mRTFState.Reset() ;

}

void cRTFParser::GetFontTable(vector<cRTFElement *>element)
{
    sRTFFontTable fonttable ;

    // go through the entries in the font table
    // skip 0 since that is 'fonttbl'
    for(size_t loop = 1; loop < element.size(); loop++)
    {
        if(element[loop]->mType == eRTFTypeGroup)
        {
            cRTFGroup *group = static_cast<cRTFGroup *>(element[loop]) ;

            bool alt = false ;      // alternate font name

            // go through the font iself
            for(size_t attribloop = 0; attribloop < group->mChildren.size(); attribloop++)
            {
                cRTFElement *felement = group->mChildren[attribloop] ;
                if(felement->mType == eRTFTypeControlWord)
                {
                    cRTFControlWord *control = static_cast<cRTFControlWord *>(felement) ;

                    // check font family
                    if(control->mWord == "fnil"
                            || control->mWord == "froman"
                            || control->mWord == "fswiss"
                            || control->mWord == "fmodern"
                            || control->mWord == "fscript"
                            || control->mWord == "fdecor"
                            || control->mWord == "ftech"
                            || control->mWord == "fbidi"
                            )
                    {
                        fonttable.family = control->mWord ;
                    }
                    else if(control->mWord == "fcharset")
                    {
                        fonttable.charset = control->mParameter ;
                    }
                    else if(control->mWord == "falt")
                    {
                        alt = true ;
                    }
                    else if(control->mWord == "f")
                    {
                        fonttable.number = control->mParameter ;
                    }
                }
                else if(felement->mType == eRTFTypeText)                // font name
                {
                    cRTFText *name = static_cast<cRTFText *>(felement) ;

                    string fname = name->mText ;
                    size_t semi = fname.find_first_of(";") ;
                    if(semi != string::npos)
                    {
                        fname = fname.substr(0, semi) ;
                    }
                    if(alt == false)
                    {
                        fonttable.name = fname ;
                    }
                    else
                    {
                        fonttable.altname = fname ;
                    }
                }
            }
            mFontTable.push_back(fonttable) ;
        }
    }
}



/*
void cRTFParser::ApplyStyle(string &text)
{

//    mRTFState.CopyInto(mRTFPrevState) ;

    EmitText(text) ;
}
*/

/////////////////////////////////////////////////////////////////////////////
///
/// @return  nothing
///
/// @brief
/// If this is a dot command, make sure we are at the beginning of a line
///
/// This may introduce extra line feeds into the document, but we'll get
/// all our formatting
///
/////////////////////////////////////////////////////////////////////////////
void cRTFParser::CheckFirstChar(void)
{
    if(mFirstColumn == false)
    {
        EmitParagraph() ;
        mRequireHardReturn = false ;
    }
}



void cRTFParser::DoChanges(void)
{
    EmitParagraphSpace() ;
//            EmitKeepLines(node, run, newstyle) ;
//            EmitKeepNext(node, run, newstyle) ;
    EmitIndent() ;
//            EmitNumbering(node, run, newstyle) ;
//            EmitOutlineLevel(node, run, newstyle) ;
//            EmitBorder(node, run, newstyle) ;
//            EmitShading(node, run, newstyle) ;
//            EmitTabs(node, run, newstyle) ;
    EmitJustify() ;
    EmitFont() ;
    EmitAttributes() ;

    mRTFPrevState = mRTFState ;
//    mRTFState.CopyInto(mRTFPrevState) ;
}



void cRTFParser::EmitText(string &text)
{
    if(mIgnoreTable != 0)
    {
        return ;
    }
    else
    {
        mInTable = false ;
    }
    if(mRequireHardReturn)
    {
        EmitParagraph() ;
        mRequireHardReturn = false ;
    }
    DoChanges() ;
    mDocument->Insert(text) ;
    mFirstColumn = false ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return  nothing
///
/// @brief
/// do the work to get a new paragraph
///
/////////////////////////////////////////////////////////////////////////////
void cRTFParser::EmitParagraph(void)
{
    mDocument->Insert(HARD_RETURN) ;
    mFirstColumn = true ;
    mRequireHardReturn = false ;
}


void cRTFParser::EmitPage(void)
{
    CheckFirstChar() ;
    mDocument->Insert(".pa") ;
    mDocument->Insert(HARD_RETURN) ;
    mFirstColumn = true ;
}


void cRTFParser::EmitTab(void)
{
    sWSTab tab ;
    tab.abstabsize = 0 ;
    tab.size = 0 ;
    tab.tabsize = 0 ;
    tab.type = TAB_TAB ;

//    CheckParagraphFormatting() ;
    mDocument->InsertTab(tab) ;
//    mFirstColumn = false ;

    mFirstColumn = false ;
}



void cRTFParser::EmitParagraphSpace(void)
{
    if(mRTFState.mSpaceBefore != mRTFPrevState.mSpaceBefore)
    {
        CheckFirstChar() ;

        double in = static_cast<double>(mRTFState.mSpaceBefore) / TWIPSPERINCH ;
        char out[100] ;
        snprintf(out, 99, ".psb %.2fi%c", in, HARD_RETURN) ;
        mDocument->Insert(out) ;

        mFirstColumn = true ;
    }

    if(mRTFState.mSpaceAfter != mRTFPrevState.mSpaceAfter)
    {
        CheckFirstChar() ;

        double in = static_cast<double>(mRTFState.mSpaceAfter) / TWIPSPERINCH ;
        char out[100] ;
        snprintf(out, 99, ".psa %.2fi%c", in, HARD_RETURN) ;
        mDocument->Insert(out) ;

        mFirstColumn = true ;
    }

    if(mRTFState.mLineSpace != mRTFPrevState.mLineSpace)
    {
        CheckFirstChar() ;

        double nls = static_cast<double>(mRTFState.mLineSpace) / 240.0 ;
        if(nls < 1.0)
        {
            nls = 1.0 ;
        }

        char out[100] ;
        snprintf(out, 99, ".ls %.2fi%c", nls, HARD_RETURN) ;
        mDocument->Insert(out) ;

        mFirstColumn = true ;
    }
}



void cRTFParser::EmitIndent(void)
{
    if(mRTFState.mIndentFirst != mRTFPrevState.mIndentFirst)
    {
        double space = mRTFState.mIndentFirst / TWIPSPERINCH ;

        string out = string_sprintf(".pm %.2fi\n", space) ;
        mDocument->Insert(out) ;

        mFirstColumn = true ;
    }
}



void cRTFParser::EmitJustify(void)
{
    if(mRTFState.mAlign != mRTFPrevState.mAlign)
    {
        if(mRTFState.mAlign == "qc")
        {
            CheckFirstChar() ;
            mDocument->BeginCenter() ;
            mFirstColumn = false ;
        }
        else if(mRTFState.mAlign == "qr")
        {
            CheckFirstChar() ;
            mDocument->BeginRight() ;
            mFirstColumn = false ;
        }
        else if(mRTFState.mAlign == "ql")
        {
            CheckFirstChar() ;
            mDocument->BeginLeft() ;
            mFirstColumn = false ;
        }
        else if(mRTFState.mAlign == "qj")
        {
            CheckFirstChar() ;
            mDocument->BeginJustify() ;
            mFirstColumn = false ;
        }
    }
}



void cRTFParser::EmitFont(void)
{
    if(mRTFPrevState.mFontsize != mRTFState.mFontsize || mRTFPrevState.mFont != mRTFState.mFont)
    {
        size_t loop ;
        int index = mRTFState.mFont ;
        for(loop = 0 ; loop < mFontTable.size(); loop++)
        {
            if(mFontTable[loop].number == index)
            {
                index = loop ;
                break ;
            }
        }

        if(loop == mFontTable.size())
        {
            index = 0 ;
        }

        sInternalFonts internalfont ;

        internalfont.name = mFontTable[index].name ;
        internalfont.size = mRTFState.mFontsize / 2 ;
        internalfont.haveWSFont = false ;
        mDocument->InsertFont(internalfont) ;

        mFirstColumn = false ;
    }
}



void cRTFParser::EmitAttributes(void)
{
    if(mRTFState.mBold != mRTFPrevState.mBold)
    {
        mDocument->BeginBold() ;
    }
    if(mRTFState.mItalics != mRTFPrevState.mItalics)
    {
        mDocument->BeginItalics() ;
    }
    if(mRTFState.mUnderline != mRTFPrevState.mUnderline)
    {
        mDocument->BeginUnderline() ;
    }
    if(mRTFState.mStrikethrough != mRTFPrevState.mStrikethrough)
    {
        mDocument->BeginStrikeThrough() ;
    }
    if(mRTFState.mSuperscript != mRTFPrevState.mSuperscript)
    {
        mDocument->BeginSuperscript() ;
    }
    if(mRTFState.mSubscript != mRTFPrevState.mSubscript)
    {
        mDocument->BeginSubscript() ;
    }
    if(mRTFState.mSmallCaps != mRTFPrevState.mSmallCaps)
    {
//        mDocument->BeginSmallCaps() ;
    }
//    if(mRTFState.mShadow != mRTFPrevState.mShadow)
//    {
//        mDocument->BeginShadow() ;
//    }
}




/*
void cRTFParser::EmitBold(int param)
{
    if(mRTFState.mBold != param)
    {
        mDocument->BeginBold() ;
        mRTFState.mBold = param ;
        mFirstColumn = false ;
    }
}



void cRTFParser::EmitItalic(int param)
{
    if(mRTFState.mItalics != param)
    {
        mDocument->BeginItalics() ;
        mRTFState.mItalics = param ;
        mFirstColumn = false ;
    }
}



void cRTFParser::EmitUnderline(int param)
{
    if(mRTFState.mUnderline != param)
    {
        mDocument->BeginUnderline() ;
        mRTFState.mUnderline = param ;
        mFirstColumn = false ;
    }
}



void cRTFParser::EmitSubScript(int param)
{
    if(mRTFState.mSubscript != param)
    {
        mDocument->BeginSubscript() ;
        mRTFState.mSubscript = param ;
        mFirstColumn = false ;
    }
}



void cRTFParser::EmitSuperScript(int param)
{
    if(mRTFState.mSuperscript != param)
    {
        mDocument->BeginSuperscript() ;
        mRTFState.mSuperscript = param ;
        mFirstColumn = false ;
    }
}



void cRTFParser::EmitStrikeThrough(int param)
{
    if(mRTFState.mStrikethrough != param)
    {
        mDocument->BeginStrikeThrough() ;
        mRTFState.mStrikethrough = param ;
        mFirstColumn = false ;
    }
}


void cRTFParser::EmitSmallCaps(int param)
{
    if(mRTFState.mBold != param)
    {
        mRTFState.mItalics = param ;
//        mFirstColumn = false ;
    }
}


void cRTFParser::EmitPage(void)
{
    CheckFirstChar() ;
    mDocument->Insert(".pa") ;
    mDocument->Insert(HARD_RETURN) ;
    mFirstColumn = true ;
}


void cRTFParser::EmitTab(void)
{
    sWSTab tab ;
    tab.abstabsize = 0 ;
    tab.size = 0 ;
    tab.tabsize = 0 ;
    tab.type = TAB_TAB ;

//    CheckParagraphFormatting() ;
    mDocument->InsertTab(tab) ;
//    mFirstColumn = false ;

    mFirstColumn = false ;
}


void cRTFParser::EmitCenter(eAlign param)
{
    if(param != mRTFState.mAlign)
    {
        CheckFirstChar() ;
        mDocument->BeginCenter() ;
        mFirstColumn = false ;
        mRTFState.mAlign = param ;
    }
}


void cRTFParser::EmitLeft(eAlign param)
{
    if(param != mRTFState.mAlign)
    {
        CheckFirstChar() ;
        mDocument->BeginLeft() ;
        mFirstColumn = false ;
        mRTFState.mAlign = param ;
    }
}


void cRTFParser::EmitRight(eAlign param)
{
    if(param != mRTFState.mAlign)
    {
        CheckFirstChar() ;
        mDocument->BeginRight() ;
        mFirstColumn = false ;
        mRTFState.mAlign = param ;
    }
}


void cRTFParser::EmitJustify(eAlign param)
{
    if(param != mRTFState.mAlign)
    {
        CheckFirstChar() ;
        mDocument->BeginJustify() ;
        mFirstColumn = false ;
        mRTFState.mAlign = param ;
    }
}


void cRTFParser::EmitLineSpace(int ls)
{
    double nls = static_cast<double>(ls) / 240.0 ;
    if(nls < 1.0)
    {
        nls = 1.0 ;
    }

//    if(mRTFState.mLineSpace != nls)
    if(mRTFPrevState.mLineSpace != nls)
    {
        CheckFirstChar() ;

        char out[100] ;
        snprintf(out, 99, ".ls %.2f%c", nls, HARD_RETURN) ;
        mDocument->Insert(out) ;

        mFirstColumn = true ;
        mRTFState.mLineSpace = nls ;
        mRTFPrevState.mLineSpace = nls ;
    }
}


void cRTFParser::EmitIndentFirst(int indent)
{
    double in = static_cast<double>(indent) / TWIPSPERINCH ;

    if(in != mRTFState.mIndentFirst)
    {
        CheckFirstChar() ;

        char out[100] ;
        snprintf(out, 99, ".pm %.2fi%c", in, HARD_RETURN) ;
        mDocument->Insert(out) ;

        mFirstColumn = true ;
        mRTFState.mIndentFirst = in ;
    }
}


void cRTFParser::EmitIndentParagraph(int indent)
{
    double in = static_cast<double>(indent) / TWIPSPERINCH ;

    if(in != mRTFState.mIndentPara)
    {
        CheckFirstChar() ;

        char out[100] ;
        snprintf(out, 99, ".lm %.2fi%c", in, HARD_RETURN) ;
        mDocument->Insert(out) ;

        mFirstColumn = true ;
        mRTFState.mIndentPara = in ;
    }
}


void cRTFParser::EmitIndentRight(int indent)
{
//    indent = 9360 - indent ;      /// @todo do not hardcode to 8.5 x 11 page (9360 = 6.5" = 8.5" - 2"margins)
    indent = (mPaperwidth - mMarginLeft - mMarginRight) - indent ;
    double in = static_cast<double>(indent) / TWIPSPERINCH ;

    if(in != mRTFState.mIndentRight)
    {
        CheckFirstChar() ;

        char out[100] ;
        snprintf(out, 99, ".rm %.2fi%c", in, HARD_RETURN) ;
        mDocument->Insert(out) ;

        mFirstColumn = true ;
        mRTFState.mIndentRight = in ;
    }
}


void cRTFParser::EmitFont(int index, int size)
{
    size_t loop ;
    for(loop = 0 ; loop < mFontTable.size(); loop++)
    {
        if(mFontTable[loop].number == index)
        {
            index = loop ;
            break ;
        }
    }

    if(loop == mFontTable.size())
    {
        index = 0 ;
    }

//    if(index != mRTFState.mFont || size != mRTFState.mFontsize)
    if(index != mRTFPrevState.mFont || size != mRTFPrevState.mFontsize)
    {
        sInternalFonts internalfont ;

        internalfont.name = mFontTable[index].name ;
        internalfont.size = size / 2 ;
        mDocument->InsertFont(internalfont) ;

        mRTFState.mFont = index ;
        mRTFState.mFontsize = size ;

        mRTFPrevState.mFont = index ;
        mRTFPrevState.mFontsize = size ;

        mFirstColumn = false ;
    }
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return  nothing
///
/// @param param - space after a paragraph in TWIPS
/// @brief
/// emits the .psa (not standard Wordstar) command (space after a paragraph)
///
/////////////////////////////////////////////////////////////////////////////
void cRTFParser::EmitSpaceAfter(int param)
{
    double in = static_cast<double>(param) / TWIPSPERINCH ;

    if(in != mRTFPrevState.mSpaceAfter)
    {
        CheckFirstChar() ;

        char out[100] ;
        snprintf(out, 99, ".psa %.2fi%c", in, HARD_RETURN) ;
        mDocument->Insert(out) ;

        mFirstColumn = true ;
        mRTFState.mSpaceAfter = in ;
        mRTFPrevState.mSpaceAfter = in ;
    }
}



/////////////////////////////////////////////////////////////////////////////
///
/// @return  nothing
///
/// @param param - space after a paragraph in TWIPS
/// @brief
/// emits the .psb (not standard Wordstar) command (space before a paragraph)
///
/////////////////////////////////////////////////////////////////////////////
void cRTFParser::EmitSpaceBefore(int param)
{
    double in = static_cast<double>(param) / TWIPSPERINCH ;

    if(in != mRTFPrevState.mSpaceBefore)
    {
        CheckFirstChar() ;

        char out[100] ;
        snprintf(out, 99, ".psb %.2fi%c", in, HARD_RETURN) ;
        mDocument->Insert(out) ;

        mFirstColumn = true ;
        mRTFState.mSpaceBefore = in ;
        mRTFPrevState.mSpaceBefore = in ;
    }
}

*/
