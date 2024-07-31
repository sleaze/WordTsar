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


//////////////////////////////////////////////////////////////////////////////
//
// The sole purpose of this class is to speed up compiles of layout.cpp
//
// compiling exprtk.h is a slow compile, and I want faster turn around while 
// working in there.
//////////////////////////////////////////////////////////////////////////////

#include "math.h"

#include "exprtk/exprtk.hpp"

exprtk::parser<double> gParser;                         ///< math expression parser

cMath::cMath()
{
}

cMath::~cMath()
{
}


double cMath::DoMath(std::string expression_str)
{
    double value = 0.0 ;
    // now we do any math in the expression
    typedef exprtk::expression<double>     expression_t;
//    typedef exprtk::parser<double>             parser_t;


    // Instantiate expression
    expression_t expression;

    if(gParser.compile(expression_str, expression))
    {
        value = expression.value();
    }
    else
    {
        value = 0.0 ;
    }

    return value ;
}
