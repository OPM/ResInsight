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

#include <QByteArray>
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
    static ads::CDockWidget* createDockWidget( QString title, QString dockName, QWidget* parent );

    static QString mainPlotWindowName();
    static QString main3DWindowName();

    static QString mainWindowPropertyEditorName();
    static QString mainWindowResultInfoName();
    static QString mainWindowProcessMonitorName();
    static QString mainWindowResultPlotName();
    static QString mainWindowDepthPlotName();
    static QString mainWindowRelPermPlotName();
    static QString mainWindowPvtPlotName();
    static QString mainWindowSeismicHistogramName();
    static QString mainWindowMessagesName();
    static QString mainWindowMohrsCirclePlotName();
    static QString mainWindowUndoStackName();

    static QString mainWindowProjectTreeName();
    static QString mainWindowDataSourceTreeName();
    static QString mainWindowScriptsTreeName();

    static QString plotMainWindowDataSourceTreeName();
    static QString plotMainWindowPlotsTreeName();
    static QString plotMainWindowTemplateTreeName();
    static QString plotMainWindowScriptsTreeName();
    static QString plotMainWindowCloudTreeName();

    static QString plotMainWindowPropertyEditorName();
    static QString plotMainWindowPropertyEditorRightName();
    static QString plotMainWindowMessagesName();
    static QString plotMainWindowUndoStackName();
    static QString plotMainWindowPlotManagerName();

    static QString dockState3DEclipseName();
    static QString dockState3DGeoMechName();
    static QString dockStatePlotWindowName();
    static QString dockStateHideAllPlotWindowName();
    static QString dockStateHideAll3DWindowName();

    static QAction* toggleActionForWidget( const ads::CDockManager* dockManager, const QString& dockWidgetName );

    static ads::CDockWidget* findDockWidget( const ads::CDockManager* dockManager, const QString& dockWidgetName );

    static void showDockWidget( const ads::CDockManager* dockManager, const QString& dockWidgetName );

    static QByteArray defaultDockState( const QString& layoutName );
    static QByteArray hideAllDocking3DState();
    static QByteArray hideAllDockingPlotState();

    static QIcon dockIcon( const QString dockWidgetName );

private:
    static QByteArray defaultEclipseDockState();
    static QByteArray defaultGeoMechDockState();
    static QByteArray defaultPlotDockState();
};
