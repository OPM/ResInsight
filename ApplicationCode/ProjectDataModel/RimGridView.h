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

#include "cvfBase.h"
#include "cvfArray.h"

class Rim2dGridProjection;
class Rim3dOverlayInfoConfig;
class RimIntersectionCollection;
class RimPropertyFilterCollection;
class RimGridCollection;
class RimCellRangeFilterCollection;

class RimGridView : public Rim3dView
{
    CAF_PDM_HEADER_INIT;
public:
    RimGridView();
    ~RimGridView(void) override;

    void                                              showGridCells(bool enableGridCells);
    
    Rim3dOverlayInfoConfig*                           overlayInfoConfig() const;
                                                      
    cvf::ref<cvf::UByteArray>                         currentTotalCellVisibility();

    RimIntersectionCollection*                        crossSectionCollection() const;
                                                      
    virtual const RimPropertyFilterCollection*        propertyFilterCollection() const = 0;

    void                                              rangeFiltersUpdated();
    RimCellRangeFilterCollection*                     rangeFilterCollection();
    const RimCellRangeFilterCollection*               rangeFilterCollection() const;
                                                      
    bool                                              hasOverridenRangeFilterCollection();
    void                                              setOverrideRangeFilterCollection(RimCellRangeFilterCollection* rfc);
    void                                              replaceRangeFilterCollectionWithOverride();
                                                      
    RimViewController*                                viewController() const override;
    RimViewLinker*                                    assosiatedViewLinker() const override;
                                                      

    bool                                              isGridVisualizationMode() const override;

protected:
    virtual void                              updateViewFollowingRangeFilterUpdates();
    void                                      initAfterRead() override;
    void                                      onTimeStepChanged() override;
    virtual void                                      calculateCurrentTotalCellVisibility(cvf::UByteArray* totalVisibility, int timeStep) = 0;
    void                                      selectOverlayInfoConfig() override;
                                                      
    void                                      fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

protected: // Fields
    caf::PdmChildField<RimIntersectionCollection*>    m_crossSectionCollection;
    caf::PdmChildField<Rim3dOverlayInfoConfig*>       m_overlayInfoConfig;
    caf::PdmChildField<RimCellRangeFilterCollection*> m_rangeFilterCollection;
    caf::PdmChildField<RimCellRangeFilterCollection*> m_overrideRangeFilterCollection;
    caf::PdmChildField<RimGridCollection*>            m_gridCollection;
protected:
    cvf::ref<cvf::UByteArray>                         m_currentReservoirCellVisibility;

private:
    RimViewLinker*                                    viewLinkerIfMasterView() const;
    bool                                              m_previousGridModeMeshLinesWasFaults;
};


