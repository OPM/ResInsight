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

#include "RiuMainWindowBase.h"
#include "QSettings"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuMainWindowBase::RiuMainWindowBase()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::loadWinGeoAndDockToolBarLayout()
{
    // Company and appname set through QCoreApplication
    QSettings settings;

    QVariant winGeo = settings.value(QString("%1/winGeometry").arg(mainWindowName()));
    QVariant layout = settings.value(QString("%1/dockAndToolBarLayout").arg(mainWindowName()));

    if (winGeo.isValid())
    {
        if (restoreGeometry(winGeo.toByteArray()))
        {
            if (layout.isValid())
            {
                restoreState(layout.toByteArray(), 0);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::saveWinGeoAndDockToolBarLayout()
{
    // Company and appname set through QCoreApplication
    QSettings settings;

    QByteArray winGeo = saveGeometry();
    settings.setValue(QString("%1/winGeometry").arg(mainWindowName()), winGeo);

    QByteArray layout = saveState(0);
    settings.setValue(QString("%1/dockAndToolBarLayout").arg(mainWindowName()), layout);

    settings.setValue(QString("%1/isMaximized").arg(mainWindowName()), isMaximized());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::showWindow()
{
    // Company and appname set through QCoreApplication
    QSettings settings;

    showNormal();

    QVariant isMax = settings.value(QString("%1/isMaximized").arg(mainWindowName()), false);
    if (isMax.toBool())
    {
        showMaximized();
    }
}
