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
#include "cafPdmFieldCvfColor.h"

#include "RigSingleWellResultsData.h"

class RimReservoirView;
//==================================================================================================
///  
///  
//==================================================================================================
class RimWell : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:

    RimWell();
    virtual ~RimWell();
    
    void                                setReservoirView(RimReservoirView* ownerReservoirView);
    
    void                                setWellResults(RigSingleWellResultsData* wellResults) { m_wellResults = wellResults;}
    RigSingleWellResultsData*           wellResults() { return m_wellResults.p(); }
    
    bool                                isWellVisible(size_t frameIndex);

    virtual caf::PdmFieldHandle*        userDescriptionField();
    virtual caf::PdmFieldHandle*        objectToggleField();

    virtual void                        fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual void                        defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering);

    caf::PdmField<QString>              name;
    caf::PdmField<bool>                 showWellLabel;
    
    caf::PdmField<bool>                 showWellCells;
    caf::PdmField<bool>                 showWellCellFence;
    
    caf::PdmField<bool>                 showWellPipes;
    caf::PdmField<cvf::Color3f>         wellPipeColor;
    caf::PdmField<double>               pipeRadiusScaleFactor;

private:
    cvf::ref<RigSingleWellResultsData>  m_wellResults;

    RimReservoirView*                   m_reservoirView;
};
