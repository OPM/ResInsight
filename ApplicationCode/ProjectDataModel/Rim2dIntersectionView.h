/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include "Rim3dView.h"
#include "cafPdmPtrField.h"

class RimIntersection;
class RivIntersectionPartMgr;
class RimLegendConfig;
class RimTernaryLegendConfig;

namespace cvf
{
    class ModelBasicList;
}

//==================================================================================================
///  
///  
//==================================================================================================
class Rim2dIntersectionView : public Rim3dView
{
    CAF_PDM_HEADER_INIT;
public:
    Rim2dIntersectionView(void);
    virtual ~Rim2dIntersectionView(void);

    void                       setVisible(bool isVisible);
    void                       setIntersection(RimIntersection* intersection);
    RimIntersection*           intersection();

    virtual bool               isUsingFormationNames() const override;
    virtual void               scheduleGeometryRegen(RivCellSetEnum geometryType) override;
    virtual RimCase*           ownerCase() const override;
    virtual void               selectOverlayInfoConfig() override {}

    virtual RimViewLinker*     assosiatedViewLinker() const override { return nullptr; }
    virtual RimViewController* viewController() const override       { return nullptr; }

    virtual bool               isTimeStepDependentDataVisible() const override;

    void                       updateLegends();

protected:

    virtual void               axisLabels(cvf::String* xLabel, cvf::String* yLabel, cvf::String* zLabel) override;
    virtual void               createDisplayModel() override;
    virtual void               createPartCollectionFromSelection(cvf::Collection<cvf::Part>* parts) override;
    virtual void               clampCurrentTimestep() override;
    virtual void               updateCurrentTimeStep() override;
    virtual void               onTimeStepChanged() override;
    virtual void               updateStaticCellColors() override;
    virtual void               updateScaleTransform() override;
    virtual cvf::Transform*    scaleTransform() override;
    virtual void               resetLegendsInViewer() override;
    virtual void               onLoadDataAndUpdate() override;
    virtual bool               isWindowVisible() override;

    virtual void               fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual void               defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;

    bool                       hasResults();
    int                        timeStepCount();


    caf::PdmChildField<RimLegendConfig*>        m_legendConfig;
    caf::PdmChildField<RimTernaryLegendConfig*> m_ternaryLegendConfig;

    caf::PdmPtrField<RimIntersection*> m_intersection;

    cvf::ref<RivIntersectionPartMgr>   m_flatIntersectionPartMgr;
    cvf::ref<cvf::ModelBasicList>      m_intersectionVizModel;
    cvf::ref<cvf::Transform>           m_scaleTransform;


};
