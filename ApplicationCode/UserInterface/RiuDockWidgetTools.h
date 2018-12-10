/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Statoil ASA
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
    static RiuDockWidgetTools* instance();

    QString projectTreeName() const;
    QString propertyEditorName() const;
    QString resultInfoName() const;
    QString processMonitorName() const;
    QString resultPlotName() const;
    QString relPermPlotName() const;
    QString pvtPlotName() const;
    QString messagesName() const;
    QString mohrsCirclePlotName() const;

    QAction* toggleActionForWidget(const QObject* parent, const QString& dockWidgetName);
    void     setDockWidgetVisibility(const QString& dockWidgetName, bool isVisible);
    void     changeDockWidgetVisibilityBasedOnView(Rim3dView* view);
    void     saveDockWidgetsState();

private:
    enum UserDefinedVisibility
    {
        USER_DEFINED_ON,
        USER_DEFINED_OFF,
        USER_DEFINED_UNKNOWN
    };

    RiuDockWidgetTools();
    void                  setVisibleDockingWindowsForEclipse();
    void                  setVisibleDockingWindowsForGeoMech();
    void                  loadDockWidgetsState();
    UserDefinedVisibility visibilityForWidget(const QString& dockWidgetName);
    static QDockWidget*   findDockWidget(const QObject* parent, const QString& dockWidgetName);
    void                  trySetDockWidgetVisibility(const QObject* parent, const QString& dockWidgetName, bool isVisible);

private:
    QMap<QString, QVariant> m_userDefinedDockWidgetVisibility;
};
