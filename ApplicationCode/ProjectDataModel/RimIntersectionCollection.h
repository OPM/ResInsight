/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"

class RimIntersection;
class RimIntersectionBox;
class RimEclipseCellColors;
class RimSimWellInView;

namespace cvf {
    class ModelBasicList;
    class Transform;
}

//==================================================================================================
//
// 
//
//==================================================================================================
class RimIntersectionCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimIntersectionCollection();
    ~RimIntersectionCollection();

    caf::PdmField<bool> isActive;

    void appendIntersectionAndUpdate(RimIntersection* intersection);
    
    void appendIntersectionBoxAndUpdate(RimIntersectionBox* intersectionBox);

    bool hasActiveIntersectionForSimulationWell(const RimSimWellInView* simWell) const;

    void updateIntersectionBoxGeometry();

    void syncronize2dIntersectionViews();

    // Visualization interface

    void applySingleColorEffect();
    void updateCellResultColor(size_t timeStepIndex);
    void appendPartsToModel(cvf::ModelBasicList* model, cvf::Transform* scaleTransform);

protected:
    virtual void                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual caf::PdmFieldHandle*    objectToggleField();

private:
    caf::PdmChildArrayField<RimIntersection*>    m_intersections;
    caf::PdmChildArrayField<RimIntersectionBox*> m_intersectionBoxes;
};
