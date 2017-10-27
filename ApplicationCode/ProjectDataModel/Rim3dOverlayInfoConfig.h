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

#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafPdmField.h"
#include "cafAppEnum.h"

#include "cvfAssert.h"
#include "cvfObject.h"

#include "cvfVector2.h"

class RimEclipseView;
class RimGeoMechView;
class RimView;
class RigStatisticsDataCache;

//==================================================================================================
///  
///  
//==================================================================================================
class Rim3dOverlayInfoConfig : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    Rim3dOverlayInfoConfig();
    virtual ~Rim3dOverlayInfoConfig();

    void update3DInfo();

    void setReservoirView(RimView* ownerView);

    void                                        setPosition(cvf::Vec2ui position);

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

protected:
    virtual void                                fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual caf::PdmFieldHandle*                objectToggleField();

    virtual void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

private:
    void updateEclipse3DInfo(RimEclipseView * reservoirView);
    void updateGeoMech3DInfo(RimGeoMechView * geoMechView);

    caf::PdmField<bool>                         active;
    caf::PdmField<bool>                         showAnimProgress;
    caf::PdmField<bool>                         showCaseInfo;
    caf::PdmField<bool>                         showResultInfo;
    caf::PdmField<bool>                         showHistogram;

    caf::PdmField<caf::AppEnum<StatisticsTimeRangeType> > m_statisticsTimeRange;
    caf::PdmField<caf::AppEnum<StatisticsCellRangeType> > m_statisticsCellRange;

    caf::PdmPointer<RimView>                    m_viewDef;

    cvf::Vec2ui                                 m_position;
    
    void updateVisCellStatsIfNeeded();
    void displayPropertyFilteredStatisticsMessage(bool showSwitchToCurrentTimestep);
    bool hasInvalidStatisticsCombination();
    bool                                        m_isVisCellStatUpToDate;
    cvf::ref<RigStatisticsDataCache>            m_visibleCellStatistics;
};
