#ifndef printout_h
#define printout_h

#include <QtPrintSupport/qtprintsupportglobal.h>
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrintPreviewDialog>
#include <QtWidgets>

#include "../editor/editorctrl.h"

using namespace std ;


class cPrintout : public QWidget
{
    Q_OBJECT

public:
    cPrintout(cEditorCtrl* frame, const QString &title = "Print Preview") ;
    virtual ~cPrintout() ;

//    virtual bool OnPrintPage(int page) ;
//    virtual bool HasPage(int page) ;
//    virtual bool OnBeginDocument(int startPage, int endPage) ;
//    virtual void GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo) ;
    
    void DrawPage(QPainter &paintc, int page);
    void PrintPreview(void) ;
    
public slots :
    void printDocument(QPrinter *printer);
    void printPage(int index, QPainter *painter);

private :
    COORD_T DrawSegments(sLineLayout &line, COORD_T &drawy, QPainter *printer) ;
    bool DrawHeadersFooters(vector<sHeaderFooter> &headerfooter, COORD_T &drawy, int &page, QPainter *printer, bool header = true) ;
    
private:
    cEditorCtrl *mEditor ;

    double mScale ;
};

#endif // printout_h
