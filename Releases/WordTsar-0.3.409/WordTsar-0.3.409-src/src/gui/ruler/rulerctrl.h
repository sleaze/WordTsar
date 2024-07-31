#ifndef CRULERCTRL_H
#define CRULERCTRL_H

//////////////////////////////////////////////////////////////////////////////
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

#include <vector>

#include <QWidget>

#include "src/core/include/config.h"



class cRulerCtrl: public QWidget
{
    Q_OBJECT

public:
    explicit cRulerCtrl(QWidget *parent = nullptr);

    void Init();

    void SetRuler(double ruler, eMeasurement measure) ;
    void SetParagraph(size_t offset) ;
    void SetPosition(size_t offset) ;
    void SetTabStops(std::vector<COORD_T> &tabs) ;
    void SetPageMargins(COORD_T left, COORD_T right) ;
    void SetRightMargin(COORD_T margin) ;

    
    void GetDrawAreainPixels(COORD_T &left, COORD_T &right) ;

signals :

public slots :

protected :
    void paintEvent(QPaintEvent *event) override ;

private :
    double mDistance ;                                  ///< page width in current units
    double mStart ;

    COORD_T mWidth ;
    COORD_T mHeight ;

    COORD_T mRightPMargin, mLeftPMargin ;               ///< right and left page margin on print, in twips
    COORD_T mParaIndent ;                               ///< paragraph indent (.pm)
    COORD_T mPosition ;                                 ///< carat position
    COORD_T mRightMargin ;                              ///< right margin (.rm)

    std::vector<COORD_T> mTabStops ;                         ///< tab stops. The first tab stop is also left marging (.tb and .lm)
    
    COORD_T mLeftPixels, mRightPixels ;                 ///< left and right drawing borders in pixels

    QWidget *mParent ;

    eMeasurement mMeasure ;
};


#endif // header guard
