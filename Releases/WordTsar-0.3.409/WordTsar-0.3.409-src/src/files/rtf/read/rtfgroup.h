#ifndef CRTFGROUP_H
#define CRTFGROUP_H

#include <vector>
#include <string>

#include "rtfelement.h"


class cRTFGroup : public cRTFElement
{
public:
    cRTFGroup();

    std::string GetType(void) ;
    bool IsDestination(void) ;

    void dump(int level) ;

public :
    cRTFGroup *mParent ;
    std::vector<cRTFElement *> mChildren ;
};

#endif // CRTFGROUP_H
