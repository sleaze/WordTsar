/*
#ifndef CLINUXSPELLCHECK_H
#define CLINUXSPELLCHECK_H


#include "../../../third-party/hunspell/hunspell/hunspell.h"

#include "../editor/editorctrl.h"


enum eWordRet
{
    SPELL_CONTINUE,
    SPELL_QUIT,
    SPELL_REDO
};



class cHunspellSpellCheck
{
public:
    cHunspellSpellCheck(cEditorCtrl *editor);
    virtual ~cHunspellSpellCheck();

    void CheckDocument(void) ;
    eWordRet CheckWord(QString &word, POSITION_T &blockoffset, bool showgood = false) ;

protected:

private:
    cEditorCtrl *mEditor ;

    Hunhandle *mDictionary ;
};


#endif // CLINUXSPELLCHECK_H

*/
