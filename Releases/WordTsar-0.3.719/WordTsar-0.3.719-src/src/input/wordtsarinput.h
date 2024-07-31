#ifndef WORDSTARINPUT_H
#define WORDSTARINPUT_H

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

#include "src/core/include/config.h"
#include "src/gui/editor/editorctrl.h"

class cEditorCtrl ;

/// @ingroup Keyboard
/// @{

enum eSpecialChars
{
    CTRL_A = 1,
    CTRL_B,
    CTRL_C,
    CTRL_D,
    CTRL_E,
    CTRL_F,
    CTRL_G,
    CTRL_H,
    CTRL_I,
    CTRL_J,
    CTRL_K,
    CTRL_L,
    CTRL_M,
    CTRL_N,
    CTRL_O,
    CTRL_P,
    CTRL_Q,
    CTRL_R,
    CTRL_S,
    CTRL_T,
    CTRL_U,
    CTRL_V,
    CTRL_W,
    CTRL_X,
    CTRL_Y,
    CTRL_Z,
    ESCAPE = 27,
} ;



class cWordStarInput
{
public:
    cWordStarInput(cEditorCtrl *editor) ;
    ~cWordStarInput() ;
    
    bool CheckControlMode(void) ;
    bool HandleKey(char ch, bool shift = false) ;
    eHelpDisplay GetHelpStatus(void) ;
    
//private:
    void OnControlJChar(char key) ;
    void OnControlOChar(char key) ;
    void OnControlPChar(char key) ;
    bool OnControlKChar(char key) ;
    void OnControlQChar(char key) ;

    
private:
    cEditorCtrl *mEditor ;
    
    bool mControlJMode ;
    bool mControlKMode ;                    ///> modes for key sequences
    bool mControlQMode ;
    bool mControlPMode ;
    bool mControlOMode ;
    
    eHelpDisplay mOldHelpStatus ;

} ;
    
    
/// @}

#endif  // WORDSTARINPUT_H
