//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
//   This library is free software: you can redistribute it and/or modify 
//   it under the terms of the GNU General Public License as published by 
//   the Free Software Foundation, either version 3 of the License, or 
//   (at your option) any later version. 
//    
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY 
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or 
//   FITNESS FOR A PARTICULAR PURPOSE.   
//    
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>> 
//   for more details. 
//
//##################################################################################################

#pragma once

#include "cvfObject.h"
#include "cvfString.h"
#include "cvfVector2.h"


namespace cvf {

class Glyph;

//==================================================================================================
//
// Pure virtual font base class used to generate glyphs for a given character.
//
//==================================================================================================
class Font : public Object
{
public:
    Font();
    virtual ~Font();

    virtual const String&   name() const = 0;
    virtual ref<Glyph>      getGlyph(wchar_t character) = 0;
    virtual uint            advance(wchar_t character, wchar_t nextCharacter) = 0;
    virtual bool            isEmpty() = 0;

    Vec2ui                  textExtent(const String& text);        
};

} // namespace cvf
