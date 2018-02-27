/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include "Riu3DMainWindowTools.h"
#include "RiuMainWindow.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QWidget* Riu3DMainWindowTools::mainWindowWidget()
{
    return RiuMainWindow::instance();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Riu3DMainWindowTools::setActiveViewer(QWidget* subWindow)
{
    RiuMainWindow::instance()->setActiveViewer(subWindow);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Riu3DMainWindowTools::setExpanded(const caf::PdmUiItem* uiItem, bool expanded /*= true*/)
{
    RiuMainWindow::instance()->setExpanded(uiItem, expanded);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Riu3DMainWindowTools::selectAsCurrentItem(const caf::PdmObject* object, bool allowActiveViewChange /*= true*/)
{
    RiuMainWindow::instance()->selectAsCurrentItem(object, allowActiveViewChange);
}



