#include <cstdio>

#include "rtfcontrolword.h"

cRTFControlWord::cRTFControlWord()
{
    mType = eRTFTypeControlWord ;
    mParameter = 0 ;
}


void cRTFControlWord::dump(int level)
{
    indent(level) ;
    printf("WORD %s %d\n", mWord.c_str(), mParameter) ;
    fflush(0) ;
}
