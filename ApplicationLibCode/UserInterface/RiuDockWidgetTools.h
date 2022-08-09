/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#pragma once

#include <QMap>
#include <QString>
#include <QVariant>

class QObject;
class QAction;

class Rim3dView;

namespace ads
{
class CDockWidget;
class CDockManager;
}; // namespace ads

//==================================================================================================
//
//
//
//==================================================================================================
class RiuDockWidgetTools
{
public:
    static QString projectTreeName();
    static QString propertyEditorName();
    static QString resultInfoName();
    static QString processMonitorName();
    static QString resultPlotName();
    static QString relPermPlotName();
    static QString pvtPlotName();
    static QString messagesName();
    static QString mohrsCirclePlotName();
    static QString undoStackName();
    static QString summaryPlotManagerName();

    static QString mainWindowProjectTreeName();
    static QString mainWindowDataSourceTreeName();
    static QString mainWindowScriptsTreeName();

    static QString plotMainWindowDataSourceTreeName();
    static QString plotMainWindowPlotsTreeName();
    static QString plotMainWindowTemplateTreeName();
    static QString plotMainWindowScriptsTreeName();

    static QString plotMainWindowPropertyEditorName();
    static QString plotMainWindowMessagesName();
    static QString plotMainWindowUndoStackName();

    static QAction* toggleActionForWidget( const ads::CDockManager* dockManager, const QString& dockWidgetName );

    static ads::CDockWidget* findDockWidget( const ads::CDockManager* dockManager, const QString& dockWidgetName );

    static void showDockWidget( const ads::CDockManager* dockManager, const QString& dockWidgetName );
};
