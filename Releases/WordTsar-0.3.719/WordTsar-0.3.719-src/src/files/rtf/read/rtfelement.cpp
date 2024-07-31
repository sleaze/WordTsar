#include <cstdio>

#include "rtfelement.h"

cRTFElement::cRTFElement()
{
}


void cRTFElement::indent(int level)
{
    for(int i = 0; i < level; i++)
    {
        printf("-") ;
    }
}
