///////////////////////////////////////////////////////////////////////////////
//
// WordTsar - Wordstar clone for modern systems http://wordtsar.ca
// Copyright (C) 2018 Gerald Brandt
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//////////////////////////////////////////////////////////////////////////////

#include <QPainter>
#include <QFont>
#include <QBrush>
#include <QRect>
#include <QPaintEvent>
#include <QPoint>
#include <QtGlobal>

#include "src/core/include/config.h"

#include "rulerctrl.h"

using namespace std ;


cRulerCtrl::cRulerCtrl(QWidget *parent) : QWidget (parent)
{
    setMinimumHeight(33);
    setMaximumHeight(33);

    Init() ;

    mParent = parent ;
}



/////////////////////////////////////////////////////////////////////////////
///
/// @return nothing
///
/// @brief
/// Initialize local data
///
/////////////////////////////////////////////////////////////////////////////
void cRulerCtrl::Init()
{

    mDistance = 6.5 + .5; // 165.1 ;   // 65.0 in inches
    mMeasure = MSR_CENTIMETERS; // MSR_INCHES; // MSR_MILLIMETERS ;
    mStart = 0 ; //25.4 ;
    mWidth = 100 ;
    mHeight = 33 ;

    mLeftPMargin = TWIPSPERINCH ;
    mRightPMargin = TWIPSPERINCH * 2 ;
    mParaIndent = TWIPSPERINCH ;

}


void cRulerCtrl::paintEvent(QPaintEvent *event)
{
    if(mParent == nullptr)
    {
        return ;
    }
    const QRectF &rect = event->rect() ;
    QPainter painter(this) ;

    painter.setRenderHint(QPainter::Antialiasing, false) ;

    QFont oldfont = painter.font() ;
    QFont font = oldfont ;
    font.setPointSize(7);
    painter.setFont(font);

    painter.eraseRect(rect) ;

    mWidth = rect.width() - 77 ;        // this should be a calculated value like in cEditor::Paint Editor where the vertical line is drawn
    mHeight = rect.height() - 10 ;

    painter.setPen(Qt::black);
    painter.setBrush(Qt::white);
    QRectF drect ;
    drect.setX(1.0) ;
    drect.setY(4.0) ;
    drect.setWidth(mWidth) ;
    drect.setHeight(mHeight) ;
    painter.drawRect(drect) ;

    QRectF trect = drect ;       // tabs marker background

    unsigned long tabs = mTabStops.size() ;
    double conv = 1.0 ;
    switch(mMeasure)
    {
        case MSR_TWIPS :
            break ;

        case MSR_POINTS :
            break ;

        case MSR_MILLIMETERS :
            conv = TWIPSPERMM ;
            break ;

        case MSR_CENTIMETERS :
            conv = TWIPSPERCM ;
            break ;

        case MSR_INCHES :
            conv = TWIPSPERINCH ;
            break ;
    }

    // loop through my total distance (6.5" = 65   165.1 mm = 1651)
    // vars are stored in twips
    double pixelspertwip = static_cast<double>(mWidth) / mDistance ;
    double tabstopixel = pixelspertwip / conv ;

    // Draw Margin Area in pixels
    COORD_T left = mLeftPMargin * tabstopixel ;
    COORD_T right = mRightPMargin * tabstopixel ;
    mLeftPixels = left ;
    mRightPixels = right ;

    painter.setBrush(Qt::darkGray);

    // shaded left margin
    drect.setX(1.0) ;
    drect.setY(4.0) ;
    drect.setWidth(left) ;
    drect.setHeight(mHeight) ;
    painter.drawRect(drect);

    trect.setLeft(drect.right()) ;

    // shaded right margin
    drect.setX(right) ;
    drect.setY(4.0) ;
    drect.setWidth(mWidth - right + 1.0) ;
    drect.setHeight(mHeight) ;
    painter.drawRect(drect);

    trect.setRight(drect.left()) ;
    trect.setTop(drect.bottom()) ;
    trect.setBottom(trect.top() + 9) ;

    // draw cursor position
    QPointF dpoint1, dpoint2 ;
    double drawx = mPosition * tabstopixel + left ;
    dpoint1.setX(drawx) ;
    dpoint1.setY(rect.y() - 29.0) ;
    dpoint2.setX(drawx) ;
    dpoint2.setY(rect.y() - 7.0) ;
    painter.drawLine(dpoint1, dpoint2) ;

    QBrush brush1(Qt::lightGray) ;
    painter.setBrush(brush1) ;

    // draw page margin (paragrapgh indent)
    drawx = mParaIndent * tabstopixel + left ;
    QPointF points[5] ;
    points[0].setX(drawx) ;
    points[0].setY(rect.height() - 22.0) ;
    points[1].setX(drawx - 6.0) ;
    points[1].setY(rect.height() - 25.0) ;
    points[2].setX(drawx - 6.0) ;
    points[2].setY(rect.height() - 29.0) ;
    points[3].setX(drawx + 6.0);
    points[3].setY( rect.height() - 29.0) ;
    points[4].setX(drawx + 6.0);
    points[4].setY(rect.height() - 25.0) ;
    painter.drawPolygon(points, 5, Qt::WindingFill);

    // draw right margin
    drawx = mRightMargin * tabstopixel + left ;
//    wxPoint points[5] ;
    points[0].setX(drawx) ;
    points[0].setY(rect.height() - 14) ;
    points[1].setX(drawx - 6) ;
    points[1].setY(rect.height() - 10) ;
    points[2].setX(drawx - 6) ;
    points[2].setY(rect.height() - 7) ;
    points[3].setX(drawx + 6);
    points[3].setY( rect.height() - 7) ;
    points[4].setX(drawx + 6);
    points[4].setY(rect.height() - 10) ;
    painter.drawPolygon(points, 5);

    // draw tabs
//    painter.setPen(Qt::black);
//    painter.setBrush(Qt::red);
    painter.drawRect(trect) ;

    for(unsigned long loop = 0; loop < tabs; loop++)
    {
        if(loop == 0)  // draw left margin triangle
        {
            drawx = mTabStops[loop] * tabstopixel + left ;
            QPointF points[5] ;
            points[0].setX(drawx) ;
            points[0].setY(rect.height() - 14) ;
            points[1].setX(drawx - 6) ;
            points[1].setY(rect.height() - 10) ;
            points[2].setX(drawx - 6) ;
            points[2].setY(rect.height() - 7) ;
            points[3].setX(drawx + 6);
            points[3].setY( rect.height() - 7) ;
            points[4].setX(drawx + 6);
            points[4].setY(rect.height() - 10) ;
            painter.drawPolygon(points, 5) ;
        }
        else        // draw tab 't'
        {
            // draw vertical
            double drawx = mTabStops[loop] * tabstopixel + left ;
            dpoint1.setX(drawx - 1) ;
            dpoint1.setY(rect.height() - 6) ;
            dpoint2.setX(drawx - 1) ;
            dpoint2.setY(rect.height()) ;
            painter.drawLine(dpoint1, dpoint2) ;

            dpoint1.setX(drawx) ;
            dpoint1.setY(rect.height() - 6) ;
            dpoint2.setX(drawx) ;
            dpoint2.setY(rect.height()) ;
            painter.drawLine(dpoint1, dpoint2) ;

            dpoint1.setX(drawx + 1) ;
            dpoint1.setY(rect.height() - 6) ;
            dpoint2.setX(drawx + 1) ;
            dpoint2.setY(rect.height()) ;
            painter.drawLine(dpoint1, dpoint2) ;

            // draw horizontal
            dpoint1.setX(drawx - 4) ;
            dpoint1.setY(rect.height() - 6) ;
            dpoint2.setX(drawx +4) ;
            dpoint2.setY(rect.height() -6) ;
            painter.drawLine(dpoint1, dpoint2) ;

            dpoint1.setX(drawx - 4) ;
            dpoint1.setY(rect.height() - 5) ;
            dpoint2.setX(drawx +4) ;
            dpoint2.setY(rect.height() -5) ;
            painter.drawLine(dpoint1, dpoint2) ;
        }
    }


    double startloop, loopadd ;         // starting point and skips for different measurements (inches, mm, cm)
    double longline = .5 ;                   // where to draw a long ruler mark instead of short
    long modulo ;                     // where to display numbers
    switch(mMeasure)
    {
        case MSR_MILLIMETERS :
            startloop = 2.5 ;
            loopadd = 2.5 ;
            modulo = 20 ;
            longline = 10.0 ;
            break ;

        case MSR_CENTIMETERS :
            startloop = .25 ;
            loopadd = .25 ;
            modulo = 1 ;
            longline = .5 ;
            break ;

        case MSR_INCHES :
        default :
            startloop = .125 ;
            loopadd = .125 ;
            modulo = 1 ;
            break ;
    }


    // draw lines and numbers
    for(double loop = startloop ; loop < mDistance; loop += loopadd)
    {
        double twips = loop * conv ;
        int drawy = static_cast<int>(twips * tabstopixel) ;

        bool drawnum = false ;

        // figure out if we need to draw a number or a line
        if(startloop > 1.0)
        {
            long mod = static_cast<long>(loop) ;
            if((mod % modulo) == 0)
            {
                drawnum = true ;
            }
        }
        else
        {
            if(qFuzzyCompare(loop - static_cast<long>(loop), 0))   // loop - static_cast<long>(loop) == 0)
            {
                drawnum = true ;
            }
        }
        if(drawnum == true)
        {
            QString number = QString(QString::number(loop));
            QFontMetricsF metrics(font) ;
            QSizeF nsize = metrics.size(Qt::TextSingleLine, number) ;
            double tx1 = drawy - (nsize.width() / 2) ;
            double ty1 = (mHeight - nsize.height()) + 8  ;
            painter.drawText(tx1, ty1, number) ;
        }
        else        // we'll draw the lines
        {
            double y1, y2 ;

            // figure out if we draw short or long lines
            if(longline < 1.0)
            {
                if(qFuzzyCompare(loop - static_cast<long>(loop), longline))    // loop - static_cast<long>(loop) == longline)
                {
                    y1 = 11 ;
                    y2 = 15 ;
                }
                else
                {
                    y1 = 12 ;
                    y2 = 14 ;
                }
            }
            else        // differnt test for long lines if longline is greater than one.
            {
                if(static_cast<long>(loop) % static_cast<long>(longline) == 0)
                {
                    y1 = 11 ;
                    y2 = 15 ;
                }
                else
                {
                    y1 = 12 ;
                    y2 = 14 ;
                }
            }
            double x = loop * pixelspertwip ;
            dpoint1.setX(x) ;
            dpoint1.setY(y1) ;
            dpoint2.setX(x) ;
            dpoint2.setY(y2) ;
            painter.drawLine(dpoint1, dpoint2);
//            gcdc.DrawLine(x, y1, x, y2) ;
        }
    }
}





/////////////////////////////////////////////////////////////////////////////
///
/// @param ruler - the total size of the ruler
///        measure - type type of ruler (inches, mm, cm, etc)
///
/// @return nothing
///
/// @brief
/// Set the size and type of ruler
///
/////////////////////////////////////////////////////////////////////////////
void cRulerCtrl::SetRuler(double ruler, eMeasurement measure)
{
    double distance ;

    distance = ruler ; // + .5 ; // * 10 ;

    if(distance != mDistance || mMeasure != measure)
    {
        mDistance = distance ;
//    mWidth = screen ;
        mMeasure = measure ;
//        update() ;
    }
}


/////////////////////////////////////////////////////////////////////////////
///
/// @param offset - the paragraph offset
///
/// @return nothing
///
/// @brief
/// not implemented
///
/// @TODO not implemented
/////////////////////////////////////////////////////////////////////////////
void cRulerCtrl::SetParagraph(size_t offset)
{
    if(mParaIndent != offset)
    {
        mParaIndent = offset ;

//        update() ;
    }
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param offset - the current cursor position
///
/// @return nothing
///
/// @brief
/// not implemented
///
/// @TODO not implemented
/////////////////////////////////////////////////////////////////////////////
void cRulerCtrl::SetPosition(size_t offset)
{
    if(mPosition != offset)
    {
        mPosition = offset ;

//        update() ;
    }
}

/////////////////////////////////////////////////////////////////////////////
///
/// @param tab - vector of tab stops
///
/// @return nothing
///
/// @brief
/// set the tab stop positions in the ruler.
///
/// tab stops are in twips
///
/////////////////////////////////////////////////////////////////////////////
void cRulerCtrl::SetTabStops(vector<COORD_T> &tabs)
{
    if(mTabStops != tabs)
    {
        mTabStops = tabs ;
        mTabStops.pop_back() ;           // get rid of right margin tab stop

//        update() ;

    }
}


/////////////////////////////////////////////////////////////////////////////
///
/// @param left - the left margin position
///        right - the right margin position
///
/// @return nothing
///
/// @brief
/// set the left and right page margins for the ruler
///
/// margins are in twips
///
/////////////////////////////////////////////////////////////////////////////
void cRulerCtrl::SetPageMargins(COORD_T left, COORD_T right)
{
//    unsigned long left1 = static_cast<unsigned long>(left * TWIPSPERMM) ;
//    unsigned long right1 = static_cast<unsigned long>(right * TWIPSPERMM) ;
//    if(left != mLeftPMargin || right != mRightPMargin)
    {
        mLeftPMargin = left ;
        mRightPMargin = right ;
//        update() ;
    }
}



/////////////////////////////////////////////////////////////////////////////
///
/// @param margin - the left margin position
///
/// @return nothing
///
/// @brief
/// set the right margin
///
/// margins are in twips
///
/////////////////////////////////////////////////////////////////////////////
void cRulerCtrl::SetRightMargin(COORD_T margin)
{
    if(mRightMargin != margin)
    {
        mRightMargin = margin ;
//        update() ;
    }
}


/////////////////////////////////////////////////////////////////////////////
///
/// @param  left    [OUT]  the left margin position in pixels
/// @param  right   [OUT]  the right margin position in pixels
///
/// @return nothing
///
/// @brief
/// Since the editor may or may not have a scroll bar (depending on OS),
/// we use thr ruler to tell us where the right and left margins should be.
/// This is in pixels, before any scaling
///
/////////////////////////////////////////////////////////////////////////////
void cRulerCtrl::GetDrawAreainPixels(COORD_T &left, COORD_T &right)
{
    left = mLeftPixels ;
    right = mRightPixels ;
}

