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

class QDockWidget;
class QObject;
class QAction;

class Rim3dView;

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

    static QString plotMainWindowProjectTreeName();
    static QString plotMainWindowPropertyEditorName();
    static QString plotMainWindowMessagesName();
    static QString plotMainWindowUndoStackName();

    static QAction* toggleActionForWidget( const QObject* parent, const QString& dockWidgetName );

    static QVariant dockWidgetsVisibility( const QObject* parent );
    static QVariant defaultDockWidgetVisibilities();

    static void workaroundForQwtDockWidgets();

    static void setVisibleDockingWindowsForEclipse();
    static void setVisibleDockingWindowsForGeoMech();

    static void setDockWidgetVisibility( const QObject* parent, const QString& dockWidgetName, bool isVisible );
    static void applyDockWidgetVisibilities( const QObject* parent, const QMap<QString, QVariant>& visibilityMap );

    static QDockWidget* findDockWidget( const QObject* parent, const QString& dockWidgetName );

private:
    static QMap<QString, QVariant> widgetVisibilitiesForEclipse();
    static QMap<QString, QVariant> widgetVisibilitiesForGeoMech();
};
