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
        FORCE_ALL_OFF,
        ALL_ON,
        RANGE_INTERSECTING,
        FORCE_ALL_ON
    };
    typedef caf::AppEnum<RimWellCollection::WellVisibilityType> WellVisibilityEnum;

    enum WellFenceType
    {
        K_DIRECTION, 
        J_DIRECTION,
        I_DIRECTION
    };
    typedef caf::AppEnum<RimWellCollection::WellFenceType> WellFenceEnum;

    caf::PdmField<bool>                 showWellLabel;

    caf::PdmField<WellVisibilityEnum>   wellCellVisibility;
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

    virtual void                        fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue );

private:
    RimReservoirView*   m_reservoirView;
};
