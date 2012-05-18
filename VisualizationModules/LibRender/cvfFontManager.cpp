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

#include "cvfBase.h"
#include "cvfFontManager.h"


namespace cvf {

//==================================================================================================
///
/// \class cvf::FontManager
/// \ingroup Render
///
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
FontManager::FontManager()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
FontManager::~FontManager()
{
}


//--------------------------------------------------------------------------------------------------
/// Get number of loaded fonts
//--------------------------------------------------------------------------------------------------
uint FontManager::numFonts() const
{
    return 0;//m_fonts.size();
}


//--------------------------------------------------------------------------------------------------
/// Get already loaded font
//--------------------------------------------------------------------------------------------------
ref<Font> FontManager::getFontById(uint id)
{
    CVF_UNUSED(id);

    return NULL;
}


//--------------------------------------------------------------------------------------------------
/// Get embedded font. Load if needed
//--------------------------------------------------------------------------------------------------
ref<Font> FontManager::getEmbeddedFont(EmbeddedFont font)
{
    CVF_UNUSED(font);

    return NULL;
}


//--------------------------------------------------------------------------------------------------
/// Get font located on file. Load if needed.
//--------------------------------------------------------------------------------------------------
ref<Font> FontManager::getFontFromFile(const String& path)
{
    CVF_UNUSED(path);
    CVF_ASSERT(!path.isEmpty());

    // Not supported in this class
    return NULL;
}

}  // namespace cvf
