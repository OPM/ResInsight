#include "RicfSetMainWindowSize.h"
/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
// 
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RicfSetMainWindowSize.h"

#include "RiuMainWindow.h"

CAF_PDM_SOURCE_INIT(RicfSetMainWindowSize, "setMainWindowSize");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicfSetMainWindowSize::RicfSetMainWindowSize()
{
    RICF_InitField(&m_height, "height",  -1, "Height",  "", "", "");
    RICF_InitField(&m_width,  "width",   -1, "Width",  "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfSetMainWindowSize::execute()
{
    RiuMainWindow::instance()->resize(m_width, m_height);
}
