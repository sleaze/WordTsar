#include "rtftext.h"

cRTFText::cRTFText()
{
    mType = eRTFTypeText ;
}

void cRTFText::dump(int level)
{
    indent(level) ;
    printf("TEXT %s\n", mText.c_str()) ;
    fflush(0) ;
}
