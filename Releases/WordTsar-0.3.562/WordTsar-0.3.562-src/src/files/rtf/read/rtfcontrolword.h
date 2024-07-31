#ifndef CRTFCONTROLWORD_H
#define CRTFCONTROLWORD_H

#include <string>

#include "rtfelement.h"


class cRTFControlWord : public cRTFElement
{
public:
    cRTFControlWord();

    void dump(int level) ;

public:
    std::string mWord ;
    int mParameter ;

};

#endif // CRTFCONTROLWORD_H
