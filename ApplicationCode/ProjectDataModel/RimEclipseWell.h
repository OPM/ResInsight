/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RigSingleWellResultsData.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafAppEnum.h"

#include "cvfObject.h"

// Include to make Pdm work for cvf::Color
#include "cafPdmFieldCvfColor.h"    

class RimEclipseView;

//==================================================================================================
///  
///  
//==================================================================================================
class RimEclipseWell : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:

    RimEclipseWell();
    virtual ~RimEclipseWell();
    
    void                                setReservoirView(RimEclipseView* ownerReservoirView);

    void                                setWellResults(RigSingleWellResultsData* wellResults, size_t resultWellIndex);
    RigSingleWellResultsData*           wellResults() { return m_wellResults.p(); }
    size_t                              resultWellIndex() { return m_resultWellIndex; }

    bool                                isWellPipeVisible(size_t frameIndex);

    bool                                calculateWellPipeVisibility(size_t frameIndex);

    virtual caf::PdmFieldHandle*        userDescriptionField();
    virtual caf::PdmFieldHandle*        objectToggleField();

    virtual void                        fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual void                        defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering);

    caf::PdmField<bool>                 showWell;

    caf::PdmField<QString>              name;
    caf::PdmField<bool>                 showWellLabel;
    
    caf::PdmField<bool>                 showWellCells;
    caf::PdmField<bool>                 showWellCellFence;
    
    caf::PdmField<bool>                 showWellPipes;
    caf::PdmField<cvf::Color3f>         wellPipeColor;
    caf::PdmField<double>               pipeRadiusScaleFactor;

private:
    cvf::ref<RigSingleWellResultsData>  m_wellResults;
    size_t                              m_resultWellIndex;

    RimEclipseView*                   m_reservoirView;
};
