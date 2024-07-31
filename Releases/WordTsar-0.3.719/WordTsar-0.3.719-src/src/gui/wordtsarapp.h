#ifndef _WORDTSARAPP_H_
#define _WORDTSARAPP_H_

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

#include <QMainWindow>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QLabel>
#include <QScrollBar>

#include "src/gui/editor/editorctrl.h"
#include "src/gui/ruler/rulerctrl.h"


class cWordTsarApp: public QMainWindow
{    
    Q_OBJECT

public:
    /// Constructor
    cWordTsarApp(QWidget *parent = NULL);
    ~cWordTsarApp() ;

    QString mInitialFileName ;
};

#endif
