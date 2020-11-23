/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RimHistogramData.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"

#include "cvfAssert.h"
#include "cvfObject.h"

#include "cvfVector2.h"

#include <cmath>
#include <memory>

class RimGeoMechContourMapView;
class RimEclipseContourMapView;
class RimEclipseView;
class RimGeoMechView;
class RimGridView;
class RigStatisticsDataCache;
class RicGridStatisticsDialog;

//==================================================================================================
///
///
//==================================================================================================
class Rim3dOverlayInfoConfig : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    Rim3dOverlayInfoConfig();
    ~Rim3dOverlayInfoConfig() override;

    void update3DInfo();

    void setReservoirView( RimGridView* ownerView );

    void setPosition( cvf::Vec2ui position );

    RimHistogramData histogramData();
    QString          timeStepText();
    QString          caseInfoText();
    QString          resultInfoText( const RimHistogramData& histData );

    RicGridStatisticsDialog* getOrCreateGridStatisticsDialog();
    void                     showStatisticsInfoDialog( bool raise = true );
    QImage                   statisticsDialogScreenShotImage();

    bool showAnimProgress() const;
    bool showCaseInfo() const;
    bool showResultInfo() const;
    bool isActive() const;

    bool showVersionInfo() const;

    enum StatisticsTimeRangeType
    {
        ALL_TIMESTEPS,
        CURRENT_TIMESTEP
    };

    enum StatisticsCellRangeType
    {
        ALL_CELLS,
        VISIBLE_CELLS
    };

private:
    void                 fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    caf::PdmFieldHandle* objectToggleField() override;
    void                 defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                 updateEclipse3DInfo( RimEclipseView* reservoirView );
    void                 updateGeoMech3DInfo( RimGeoMechView* geoMechView );
    void                 update3DInfoIn2dViews() const;
    QString              timeStepText( RimEclipseView* eclipseView );
    QString              timeStepText( RimGeoMechView* geoMechView );
    RimHistogramData     histogramData( RimEclipseContourMapView* contourMap );
    RimHistogramData     histogramData( RimGeoMechContourMapView* contourMap );
    RimHistogramData     histogramData( RimEclipseView* eclipseView );
    RimHistogramData     histogramData( RimGeoMechView* geoMechView );
    QString              caseInfoText( RimEclipseView* eclipseView );
    QString              caseInfoText( RimGeoMechView* geoMechView );
    QString resultInfoText( const RimHistogramData& histData, RimEclipseView* eclipseView, bool showVolumeWeightedMean );
    QString resultInfoText( const RimHistogramData& histData, RimGeoMechView* geoMechView );

    void updateVisCellStatsIfNeeded();
    void displayPropertyFilteredStatisticsMessage( bool showSwitchToCurrentTimestep );
    bool hasInvalidStatisticsCombination();

private:
    caf::PdmField<bool> m_active;
    caf::PdmField<bool> m_showAnimProgress;
    caf::PdmField<bool> m_showCaseInfo;
    caf::PdmField<bool> m_showResultInfo;
    caf::PdmField<bool> m_showVolumeWeightedMean;
    caf::PdmField<bool> m_showHistogram;
    caf::PdmField<bool> m_showVersionInfo;

    caf::PdmField<caf::AppEnum<StatisticsTimeRangeType>> m_statisticsTimeRange;
    caf::PdmField<caf::AppEnum<StatisticsCellRangeType>> m_statisticsCellRange;

    caf::PdmPointer<RimGridView>     m_viewDef;
    cvf::Vec2ui                      m_position;
    bool                             m_isVisCellStatUpToDate;
    cvf::ref<RigStatisticsDataCache> m_visibleCellStatistics;

    std::unique_ptr<RicGridStatisticsDialog> m_gridStatisticsDialog;
};
