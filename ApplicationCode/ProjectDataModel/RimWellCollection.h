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

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafAppEnum.h"
#include <QString>

#include "RimWell.h"

class RimReservoirView;

//==================================================================================================
///  
///  
//==================================================================================================
class RimWellCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:

    RimWellCollection();
    virtual ~RimWellCollection();

    void                                setReservoirView(RimReservoirView* ownerReservoirView);

    enum WellVisibilityType
    {
        PIPES_FORCE_ALL_OFF,
        PIPES_INDIVIDUALLY,
        PIPES_OPEN_IN_VISIBLE_CELLS,
        PIPES_FORCE_ALL_ON
    };
    typedef caf::AppEnum<RimWellCollection::WellVisibilityType> WellVisibilityEnum;

    enum WellCellsRangeFilterType
    {
        RANGE_ADD_ALL,
        RANGE_ADD_INDIVIDUAL,
        RANGE_ADD_NONE
    };
    typedef caf::AppEnum<RimWellCollection::WellCellsRangeFilterType> WellCellsRangeFilterEnum;

    enum WellFenceType
    {
        K_DIRECTION, 
        J_DIRECTION,
        I_DIRECTION
    };
    typedef caf::AppEnum<RimWellCollection::WellFenceType> WellFenceEnum;

    caf::PdmField<bool>                 showWellLabel;
    caf::PdmField<bool>                 active;

    caf::PdmField<WellCellsRangeFilterEnum>   wellCellsToRangeFilterMode;
    caf::PdmField<bool>                 showWellCellFences;
    caf::PdmField<WellFenceEnum>        wellCellFenceType;
    caf::PdmField<double>               wellCellTransparencyLevel;

    caf::PdmField<WellVisibilityEnum>   wellPipeVisibility;
    caf::PdmField<double>               pipeRadiusScaleFactor;
    caf::PdmField<int>                  pipeCrossSectionVertexCount;

    caf::PdmField<double>               wellHeadScaleFactor;
    caf::PdmField<bool>                 showWellHead;

    caf::PdmField<bool>                 isAutoDetectingBranches;

    caf::PdmPointersField<RimWell*>     wells;

    RimWell*                            findWell(QString name);
    bool                                hasVisibleWellCells();
    bool                                hasVisibleWellPipes();

    const std::vector<cvf::ubyte>&      isWellPipesVisible(size_t frameIndex);       
    void                                scheduleIsWellPipesVisibleRecalculation();

    virtual void                        fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual void                        defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering);
    virtual caf::PdmFieldHandle*        objectToggleField();
private:

    void                                calculateIsWellPipesVisible(size_t frameIndex);

    RimReservoirView*   m_reservoirView;
    std::vector< std::vector< cvf::ubyte > >             
                                        m_isWellPipesVisible;  
};
