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

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafAppEnum.h"

// Include to make Pdm work for cvf::Color
#include "cafPdmFieldCvfColor.h"   

#include "cvfObject.h"
#include "cvfVector3.h"

class RigSingleWellResultsData;
class RigWellResultFrame;
struct RigWellResultPoint;


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
    
    void                                setWellResults(RigSingleWellResultsData* wellResults, size_t resultWellIndex);
    RigSingleWellResultsData*           wellResults();
    const RigSingleWellResultsData*     wellResults() const;
    size_t                              resultWellIndex() const;

    bool                                isWellCellsVisible() const;
    bool                                isWellPipeVisible(size_t frameIndex) const;
    bool                                isWellSpheresVisible(size_t frameIndex) const;
    bool                                isUsingCellCenterForPipe() const;


    virtual caf::PdmFieldHandle*        userDescriptionField() override;
    virtual caf::PdmFieldHandle*        objectToggleField() override;

    void                                calculateWellPipeStaticCenterLine( std::vector< std::vector <cvf::Vec3d> >& pipeBranchesCLCoords,
                                                                           std::vector< std::vector <RigWellResultPoint> >& pipeBranchesCellIds);

    void                                calculateWellPipeDynamicCenterLine(size_t timeStepIdx, 
                                                                    std::vector< std::vector <cvf::Vec3d> >& pipeBranchesCLCoords,
                                                                    std::vector< std::vector <RigWellResultPoint> >& pipeBranchesCellIds);

    void                                wellHeadTopBottomPosition(size_t frameIndex, cvf::Vec3d* top,  cvf::Vec3d* bottom);
    double                              pipeRadius();
    caf::PdmField<bool>                 showWell;

    caf::PdmField<QString>              name;
    
    caf::PdmField<bool>                 showWellLabel;
    caf::PdmField<bool>                 showWellHead;
    caf::PdmField<bool>                 showWellPipe;
    caf::PdmField<bool>                 showWellSpheres;
    
    caf::PdmField<double>               wellHeadScaleFactor;
    caf::PdmField<double>               pipeScaleFactor;

    caf::PdmField<cvf::Color3f>         wellPipeColor;
    
    caf::PdmField<bool>                 showWellCells;
    caf::PdmField<bool>                 showWellCellFence;

protected:
    virtual void                        fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual void                        defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void                        defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "") override;

private:
    bool                                intersectsDynamicWellCellsFilteredCells(size_t frameIndex) const;
    bool                                intersectsStaticWellCellsFilteredCells() const;

    bool                                intersectsWellCellsFilteredCells(const RigWellResultFrame &wrsf, size_t frameIndex) const;

private:
    cvf::ref<RigSingleWellResultsData>  m_wellResults;
    size_t                              m_resultWellIndex;
};
