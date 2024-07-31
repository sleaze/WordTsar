#ifndef CRTFCONTROLSYMBOL_H
#define CRTFCONTROLSYMBOL_H

#include "rtfelement.h"

class cRTFControlSymbol : public cRTFElement
{
public:
    cRTFControlSymbol();

    void dump(int level) ;

    char mSymbol ;
    int mParameter ;


};

#endif // CRTFCONTROLSYMBOL_H
