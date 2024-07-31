
#include <QProgressDialog>

#include "printout.h"
#include "src/gui/editor/editorctrl.h"
#include "src/core/document/document.h"
#include "src/gui/layout/layout.h"

#include "src/core/include/config.h"




cPrintout::cPrintout(cEditorCtrl* frame, const QString &title )
        : QWidget(frame)
{ 
    UNUSED_ARGUMENT(title) ;
    mEditor = frame;
}


cPrintout::~cPrintout()
{
    if(mEditor->mPrintLayout != nullptr)
    {
        delete mEditor->mPrintLayout ;
        mEditor->mPrintLayout = nullptr ;
    }
}


void cPrintout::PrintPreview(void)
{
    QPrinter printer(QPrinter::HighResolution) ;
    printer.setPaperSize(QPrinter::Letter) ;
    printer.setFullPage(true) ;
    printer.setFromTo(1,mEditor->mPrintLayout->GetNumberofPages()) ;

    QPrintPreviewDialog preview(&printer, mEditor);
    connect(&preview, SIGNAL(paintRequested(QPrinter*)), this, SLOT(printDocument(QPrinter*)));
    preview.exec();

}






void cPrintout::printDocument(QPrinter *printer)
{
    int from = printer->fromPage() ;
    int to = printer->toPage() ;

    if(from == 0 && to == 0)            // whole document
    {
        to = mEditor->mPrintLayout->GetNumberofPages() ;
        from = 1 ;
        printer->setFromTo(from, to) ;
    }

    QProgressDialog progress(tr("Preparing Pages..."), tr("&Cancel"), from, to, mEditor);
    progress.setWindowModality(Qt::ApplicationModal);
    progress.setWindowTitle(tr("Print Preview"));
    progress.setMinimum(printer->fromPage() - 1);
    progress.setMaximum(printer->toPage());

    QPainter painter;
    painter.begin(printer);
    QSizeF psize = printer->pageSizeMM() ;
//    QRect pagerect = printer->pageRect() ;
//    QRect paperrect = printer->paperRect() ;
    int printerwidth = psize.width() * TWIPSPERMM ;
    int printerheight = psize.height() * TWIPSPERMM ;

    painter.setWindow(0, 0, printerwidth, printerheight);
//    painter.setViewport(0, 0, printerwidth, printerheight);

    int screenwidth = painter.device()->width() ;

    double scale = (double)screenwidth / (double)printerwidth ;
    mScale = scale ;

//    painter.scale(scale, scale) ;

    bool firstPage = true;

    for (int page = from; page <= to; ++page)
    {
        if (!firstPage)
        {
            printer->newPage();
        }

        QApplication::processEvents();
        if (progress.wasCanceled())
        {
            break;
        }

        printPage(page - 1, &painter, printer);
        progress.setValue(page);
        firstPage = false;
    }

    painter.end();
}



void cPrintout::printPage(int page, QPainter *paint, QPrinter *printer)
{
    page ++ ;

    // find first line of page
    LINE_T numlines = mEditor->mPrintLayout->GetNumberofLines() ;
    PARAGRAPH_T numparas = mEditor->mPrintLayout->GetNumberofParagraphs() ;

    if(page <= numlines)
    {
        PARAGRAPH_T para ;
        LINE_T currentline = 0 ;
        LINE_T paraline = 0 ;
        bool dobreak = false ;

        // go through all our paragraph until we find our page
        for(para = 0; para < numparas; para++)
        {
            for(paraline = 0; paraline < (ssize_t)mEditor->mPrintLayout->mParagraphLayout[para].lines.size(); paraline++)
            {
                currentline++ ;
                if(mEditor->mPrintLayout->mParagraphLayout[para].lines[paraline].pagenumber == page)
                {
                    dobreak = true ;
                    break ;
                }
            }

            if(dobreak == true)
            {
                break ;
            }
        }

        // now display until the next page
        dobreak = false ;
//        COORD_T drawy = 1440 ;
        sPageInfo *pageinfo = mEditor->mPrintLayout->GetPageInfo(page) ;
        COORD_T drawy = pageinfo->topmargin ;

        for(; para < numparas; para++)
        {
            // for each paragraph, go through the lines
            LINE_T numlines = mEditor->mPrintLayout->mParagraphLayout[para].lines.size() ;

            for(; paraline < numlines; paraline++)
            {
//                drawy = mEditor->mPrintLayout->mParagraphLayout[para].lines[paraline].y ;
                if(mEditor->mPrintLayout->mParagraphLayout[para].lines[paraline].pagenumber != page)
                {
                    dobreak = true ;
                    break ;
                }

                // don't print comments or commands
                if(mEditor->mPrintLayout->mParagraphLayout[para].isCommand || mEditor->mPrintLayout->mParagraphLayout[para].isComment)
                {
                    continue ;
                }

                // for each line, go through the segments
                COORD_T height = DrawSegments(mEditor->mPrintLayout->mParagraphLayout[para].lines[paraline], drawy, paint) ;

                drawy += (height * mEditor->mPrintLayout->mParagraphLayout[para].modifiers.linespace);
            }
            paraline = 0 ;

            if(dobreak)
            {
                break ;
            }
        }
    }


    // See if we have any headers that need printing
    COORD_T drawy = 720 ;
    bool printed = false ;
    if(page % 2)            // is this an odd page number?
    {
        printed = DrawHeadersFooters(mEditor->mPrintLayout->mHeadersOdd, drawy, page, paint, true) ;
    }
    else
    {
        printed = DrawHeadersFooters(mEditor->mPrintLayout->mHeadersEven, drawy, page, paint, true) ;
    }
    if(printed == false)    // odd/even page header & footers override both page ones
    {
        DrawHeadersFooters(mEditor->mPrintLayout->mHeaders, drawy, page, paint, true) ;
    }

    // See if we have any footers that need printing
    drawy = 15120 ;
    printed = false ;
    if(page % 2)            // is this an odd page number
    {
        printed = DrawHeadersFooters(mEditor->mPrintLayout->mFootersOdd, drawy, page, paint, false) ;
    }
    else
    {
        printed = DrawHeadersFooters(mEditor->mPrintLayout->mFootersEven, drawy, page, paint, false) ;
    }
    if(printed == false)    // odd/even page header & footers override both page ones
    {
        DrawHeadersFooters(mEditor->mPrintLayout->mFooters, drawy, page, paint, false) ;
    }
}


COORD_T cPrintout::DrawSegments(sLineLayout &line, COORD_T &drawy, QPainter *paint)
{

    size_t numsegments = line.segments.size() ;
    COORD_T x = line.x ;
//                COORD_T y = mEditor->mPrintLayout->mParagraphLayout[para].lines[paraline].y ;

    COORD_T height = 0 ;
    for(size_t segs = 0; segs < numsegments; segs++)
    {
        sSegmentLayout segment = line.segments[segs] ;
        QFont font1 = segment.font ;
        double fsize = font1.pointSizeF() ;
        font1.setPointSizeF(fsize / mScale);
        paint->setFont(font1) ;

        for(size_t loop1 = 0; loop1 < segment.glyph.size(); loop1++)
        {
            COORD_T drawx = x + (segment.position[loop1]);
            QString ch ;
            if(segment.glyph[loop1][0] == HARD_RETURN)
            {
                ch = " " ;
            }
            else
            {
                ch = QString::fromStdString(segment.glyph[loop1]) ;
            }

            paint->drawText(drawx, drawy, ch) ;
        }

        if(segment.segmentheight > height)
        {
            height = segment.segmentheight ;
        }

//        paint->setFont(wxNullFont);
    }

    return height ;

}

bool cPrintout::DrawHeadersFooters(vector<sHeaderFooter> &headerfooter, COORD_T &drawy, int &page, QPainter *paint, bool isheader)
{

    bool printed = false ;

    array<sLineLayout ,MAX_HEADER_FOOTER> headers ;
    for(int loop = 0; loop < headerfooter.size(); loop++)
    {
        if(headerfooter[loop].page <= page)
        {
            printed = true ;
            headers[headerfooter[loop].headernumber - 1] = headerfooter[loop].text ;
        }
        else
        {
            break ;
        }
    }

    // now print
    if(isheader == true)
    {
        for(int loop = 0; loop < MAX_HEADER_FOOTER; loop++)
        {
            COORD_T h = DrawSegments(headers[loop], drawy, paint) ;
            drawy += h ;
        }
    }
    else
    {
        for(int loop = MAX_HEADER_FOOTER - 1; loop >= 0; loop--)
        {
            COORD_T h = DrawSegments(headers[loop], drawy, paint) ;
            h *= mEditor->mFontScale ;
        }
    }

    return printed ;
}
