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

#include "cvfBase.h"
#include "cvfArray.h"
#include "cvfCollection.h"

namespace cvf
{
    class ModelBasicList;
    class Transform;
}

class RimEclipseCellColors;
class RimCellEdgeColors;
class RimEclipseCase;
class RigEclipseCaseData;
class RimEclipseView;
class RivReservoirFaultsPartMgr;
class RivGridPartMgr;

//==================================================================================================
///
/// RivReservoirGeometry: Class to handle visualization structures that embodies a complete reservoir at a specific 
/// time step. 
///
//==================================================================================================

class RivReservoirPartMgr: public cvf::Object
{
public:
    void   clearAndSetReservoir(RimEclipseCase* eclipseCase, RimEclipseView* reservoirView);
    void   setTransform(cvf::Transform* scaleTransform);
    void   setCellVisibility(size_t gridIndex, cvf::UByteArray* cellVisibilities );
    
    void   forceWatertightGeometryOn();
    void   clearWatertightGeometryFlag();

    //size_t gridCount() { return m_allGrids.size(); }
    cvf::ref<cvf::UByteArray>  
           cellVisibility(size_t gridIdx);

    void   updateCellColor(cvf::Color4f color);
    void   updateCellResultColor(size_t timeStepIndex, RimEclipseCellColors* cellResultColors);
    void   updateCellEdgeResultColor(size_t timeStepIndex, RimEclipseCellColors* cellResultColors, 
                                     RimCellEdgeColors* cellEdgeResultColors);

    void   appendGridPartsToModel(cvf::ModelBasicList* model, const std::vector<size_t>& gridIdxes);
    void   appendGridPartsToModel(cvf::ModelBasicList* model);

    // Faults
    void   updateFaultColors(size_t timeStepIndex, RimEclipseCellColors* cellResultColors);
    void   updateFaultCellEdgeResultColor(size_t timeStepIndex, RimEclipseCellColors* cellResultColors,
        RimCellEdgeColors* cellEdgeResultColors);
    void   appendFaultPartsToModel(cvf::ModelBasicList* model);
    void   appendFaultLabelPartsToModel(cvf::ModelBasicList* model);
private:

    cvf::Collection<RivGridPartMgr>     m_allGrids; // Main grid and all LGR's 
    cvf::ref<RivReservoirFaultsPartMgr> m_faultsPartMgr;
};
