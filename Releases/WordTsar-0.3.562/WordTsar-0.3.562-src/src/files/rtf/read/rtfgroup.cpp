
#include "rtfcontrolword.h"
#include "rtfcontrolsymbol.h"
#include "rtfgroup.h"

using namespace std ;

cRTFGroup::cRTFGroup()
{
    mParent = nullptr ;

    mType = eRTFTypeGroup ;
}


/////////////////////////////////////////////////////////////////////////////
///
/// @return  string - control word of first child or empty string
///
/// @brief
/// reads the rtf file and returns when end of file reached
///
/////////////////////////////////////////////////////////////////////////////
string cRTFGroup::GetType(void)
{
    string retval ;

    // if there's no children, then group type is null
    if(!mChildren.empty())
    {
        cRTFElement *child = mChildren[0] ;
        if(child->mType == eRTFTypeControlWord)
        {
            // if the first child is a control word then the group type is the control word
            retval = static_cast<cRTFControlWord *>(child)->mWord ;
        }
    }

    return retval ;
}


bool cRTFGroup::IsDestination(void)
{
    bool retval = false ;

    if(mChildren.size() != 0)
    {
        cRTFElement *child = mChildren[0] ;
        if(child->mType == eRTFTypeControlSymbol)
        {
            if(static_cast<cRTFControlSymbol *>(child)->mSymbol == '*')
            {
                retval = true ;
            }
        }
    }

    return retval ;
}

void cRTFGroup::dump(int level)
{
    indent(level) ;

    for(size_t loop = 0; loop < mChildren.size(); loop++)
    {
        cRTFElement *child = mChildren[loop] ;
        if(child->mType == eRTFTypeGroup)
        {
            cRTFGroup *group = static_cast<cRTFGroup *>(child) ;
/*
            if(group->GetType() == "fonttbl")
            {
                continue ;
            }
            if(group->GetType() == "colortbl")
            {
                continue ;
            }
            if(group->GetType() == "stylesheet")
            {
                continue ;
            }
            if(group->GetType() == "info")
            {
                continue ;
            }

            if(group->GetType().length() >= 4 && group->GetType().substr(0, 4) == "pict")
            {
                continue ;
            }
            if(group->IsDestination())
            {
                continue ;
            }
*/
        }

        child->dump(level + 1) ;
    }
}
