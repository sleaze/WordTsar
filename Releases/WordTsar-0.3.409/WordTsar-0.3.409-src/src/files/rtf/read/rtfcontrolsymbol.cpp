
#include <cstdio>

#include "rtfcontrolsymbol.h"

cRTFControlSymbol::cRTFControlSymbol()
    : cRTFElement()
{
    mType = eRTFTypeControlSymbol ;
    mParameter = 0 ;
}


void cRTFControlSymbol::dump(int level)
{
    indent(level) ;
    printf("SYMBOL %c %d\n", mSymbol, mParameter) ;
    fflush(0) ;
}
