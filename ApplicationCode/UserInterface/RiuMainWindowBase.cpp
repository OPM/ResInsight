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

#include "RiaApplication.h"
#include "RiaVersionInfo.h"

#include "RiuDockWidgetTools.h"
#include "RiuMdiSubWindow.h"

#include "RimViewWindow.h"
#include "RimProject.h"

#include "cafPdmObject.h"
#include "cafPdmUiTreeView.h"

#include <QDockWidget>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QSettings>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMainWindowBase::RiuMainWindowBase()
    : m_projectTreeView(nullptr)
    , m_allowActiveViewChangeFromSelection(true)
    , m_showFirstVisibleWindowMaximized(true)
{
    setDockNestingEnabled(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMdiWindowGeometry RiuMainWindowBase::windowGeometryForViewer(QWidget* viewer)
{
    RiuMdiSubWindow* mdiWindow = dynamic_cast<RiuMdiSubWindow*>(findMdiSubWindow(viewer));
    if (mdiWindow)
    {
        return mdiWindow->windowGeometry();
    }

    RimMdiWindowGeometry geo;
    return geo;
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
    QString regFolder = QString("%1_Qt%2/%3").arg(versionName).arg(QT_VERSION_STR).arg(mainWindowName());
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
void RiuMainWindowBase::enableShowFirstVisibleMdiWindowMaximized(bool enable)
{
    m_showFirstVisibleWindowMaximized = enable;
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMainWindowBase::addViewerToMdiArea(QMdiArea*     mdiArea,
                                           QWidget*      viewer,
                                           const QPoint& subWindowPos,
                                           const QSize&  subWindowSize)
{
    RiuMdiSubWindow* subWin = new RiuMdiSubWindow(nullptr, Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint | Qt::WindowMaximizeButtonHint);
    subWin->setAttribute(Qt::WA_DeleteOnClose); // Make sure the contained widget is destroyed when the MDI window is closed
    subWin->setWidget(viewer);

    bool initialStateTiled     = subWindowsAreTiled();
    bool initialStateMaximized = false;

    if (m_showFirstVisibleWindowMaximized && mdiArea->subWindowList().empty())
    {
        // Show first 3D view maximized
        initialStateMaximized = true;
    }

    if (mdiArea->currentSubWindow() && mdiArea->currentSubWindow()->isMaximized())
    {
        initialStateMaximized = true;
    }

    mdiArea->addSubWindow(subWin);

    if (subWindowPos.x() > -1)
    {
        subWin->move(subWindowPos);
    }
    subWin->resize(subWindowSize);

    if (initialStateMaximized)
    {
        subWin->showMaximized();
    }
    else
    {
        subWin->showNormal();
        if (initialStateTiled)
        {
            tileSubWindows();
        }
    }
}
