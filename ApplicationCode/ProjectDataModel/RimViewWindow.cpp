/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "RimViewWindow.h"


CAF_PDM_XML_ABSTRACT_SOURCE_INIT(RimViewWindow, "ViewWindow"); // Do not use. Abstract class 

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewWindow::RimViewWindow(void)
{
    CAF_PDM_InitFieldNoDefault(&m_windowGeometry, "WindowGeometry", "", "", "", "");
    m_windowGeometry.uiCapability()->setUiHidden(true);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewWindow::~RimViewWindow(void)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimViewWindow::setMdiWindowGeometry(const RimMdiWindowGeometry& windowGeometry)
{
    std::vector<int> geom;
    geom.clear();
    if (windowGeometry.isValid())
    {
        geom.push_back(windowGeometry.x);
        geom.push_back(windowGeometry.y);
        geom.push_back(windowGeometry.width);
        geom.push_back(windowGeometry.height);
        geom.push_back(windowGeometry.isMaximized);
    }
    m_windowGeometry.setValue(geom);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimMdiWindowGeometry RimViewWindow::mdiWindowGeometry()
{

    RimMdiWindowGeometry wg;
    if (m_windowGeometry.value().size() == 5)
    {
        wg.x = m_windowGeometry.value()[0];
        wg.y = m_windowGeometry.value()[1];
        wg.width = m_windowGeometry.value()[2];
        wg.height = m_windowGeometry.value()[3];
        wg.isMaximized = m_windowGeometry.value()[4];
    }

    return wg;
}

