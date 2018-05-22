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

#include "RiaVersionInfo.h"

#include "RiuDockWidgetTools.h"

#include "cafPdmObject.h"
#include "cafPdmUiTreeView.h"

#include <QSettings>
#include <QDockWidget>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuMainWindowBase::RiuMainWindowBase()
    : m_projectTreeView(nullptr)
    , m_allowActiveViewChangeFromSelection(true)
{
    setDockNestingEnabled(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::loadWinGeoAndDockToolBarLayout()
{
    // Company and appname set through QCoreApplication
    QSettings settings;

    QVariant winGeo = settings.value(QString("%1/winGeometry").arg(registryFolderName()));
    QVariant layout = settings.value(QString("%1/dockAndToolBarLayout").arg(registryFolderName()));

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
    settings.setValue(QString("%1/winGeometry").arg(registryFolderName()), winGeo);

    QByteArray layout = saveState(0);
    settings.setValue(QString("%1/dockAndToolBarLayout").arg(registryFolderName()), layout);

    settings.setValue(QString("%1/isMaximized").arg(registryFolderName()), isMaximized());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::showWindow()
{
    // Company and appname set through QCoreApplication
    QSettings settings;

    showNormal();

    QVariant isMax = settings.value(QString("%1/isMaximized").arg(registryFolderName()), false);
    if (isMax.toBool())
    {
        showMaximized();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RiuMainWindowBase::registryFolderName()
{
    QString versionName(STRPRODUCTVER);
    QString regFolder = QString("%1/%2").arg(versionName).arg(mainWindowName());
    return regFolder;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::selectAsCurrentItem(const caf::PdmObject* object, bool allowActiveViewChange)
{
    m_allowActiveViewChangeFromSelection = allowActiveViewChange;
    m_projectTreeView->selectAsCurrentItem(object);
    m_allowActiveViewChangeFromSelection = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::setExpanded(const caf::PdmUiItem* uiItem, bool expanded)
{
    m_projectTreeView->setExpanded(uiItem, expanded);
}

//-------------------------------------------------------------------------------------------------- 
///  
/// 
//-------------------------------------------------------------------------------------------------- 
void RiuMainWindowBase::slotDockWidgetToggleViewActionTriggered()
{
    if (!sender()) return;

    auto dockWidget = dynamic_cast<QDockWidget*>(sender()->parent());
    if (dockWidget)
    {
        if (dockWidget->isVisible())
        {
            // Raise the dock widget to make it visible if the widget is part of a tab widget 
            dockWidget->raise();
        }

        RiuDockWidgetTools::instance()->setDockWidgetVisibility(dockWidget->objectName(), dockWidget->isVisible());
    }
}
