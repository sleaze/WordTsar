#ifndef CRTFELEMENT_H
#define CRTFELEMENT_H

enum eRTFElementType
{
    eRTFElementNone,
    eRTFTypeControlWord,
    eRTFTypeControlSymbol,
    eRTFTypeGroup,
    eRTFTypeText
};

class cRTFElement
{
public:
    cRTFElement();

    virtual void dump(int level) = 0 ;

    void indent(int level) ;

public:
    eRTFElementType mType ;
};

#endif // CRTFELEMENT_H
