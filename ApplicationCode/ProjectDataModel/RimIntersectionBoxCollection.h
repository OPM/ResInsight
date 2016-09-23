/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

class RimIntersectionBox;
class RimEclipseCellColors;
class RimEclipseWell;

namespace cvf {
    class ModelBasicList;
    class Transform;
}

//==================================================================================================
//
// 
//
//==================================================================================================
class RimIntersectionBoxCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimIntersectionBoxCollection();
    ~RimIntersectionBoxCollection();

    caf::PdmField<bool> isActive;

    void appendIntersectionBox(RimIntersectionBox* intersectionBox);

    bool hasActiveCrossSectionForSimulationWell(RimEclipseWell* eclipseWell) const;

    void initialize();

    // Visualization interface

    void applySingleColorEffect();
    void updateCellResultColor(size_t timeStepIndex);
    void appendPartsToModel(cvf::ModelBasicList* model, cvf::Transform* scaleTransform);

protected:
    virtual void                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual caf::PdmFieldHandle*    objectToggleField();

private:
    caf::PdmChildArrayField<RimIntersectionBox*> m_intersectionBoxes;
};
