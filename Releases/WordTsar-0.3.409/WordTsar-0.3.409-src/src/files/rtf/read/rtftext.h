#ifndef CRTFTEXT_H
#define CRTFTEXT_H

#include <string>

#include "rtfelement.h"


class cRTFText : public cRTFElement
{
public:
    cRTFText();

    void dump(int level) ;

public:
    std::string mText ;
};

#endif // CRTFTEXT_H
