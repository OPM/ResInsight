/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include <QPointer>
#include <QString>

class RiuEclipseSelectionItem;
class RiuSelectionItem;
class Rim3dView;
class RimEclipseView;
class RigEclipseCaseData;
class RimEclipseResultDefinition;
class QWidget;

//==================================================================================================
//
//
//
//==================================================================================================
class RiuPlotUpdater
{
public:
    RiuPlotUpdater();

    void updateOnSelectionChanged( const RiuSelectionItem* selectionItem );
    void updateOnTimeStepChanged( Rim3dView* changedView );

    void doDelayedUpdate();

protected:
    virtual void     clearPlot() = 0;
    virtual QWidget* plotPanel() = 0;

    virtual bool queryDataAndUpdatePlot( const RimEclipseResultDefinition* eclipseResDef,
                                         size_t                            timeStepIndex,
                                         size_t                            gridIndex,
                                         size_t                            gridLocalCellIndex ) = 0;

    void storeDelayedInformation( const RiuEclipseSelectionItem* selItem );
    void clearDelayedInformation();

    static RiuEclipseSelectionItem* extractEclipseSelectionItem( const RiuSelectionItem* selectionItem, Rim3dView*& newFollowAnimView );

protected:
    const Rim3dView* m_viewToFollowAnimationFrom;

    // cached values for delayed plot updates
    const RimEclipseResultDefinition* m_eclipseResultDef;
    size_t                            m_timeStepIndex;
    size_t                            m_gridIndex;
    size_t                            m_gridLocalCellIndex;
};
