/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "cvfArray.h"

class RimAnnotationInViewCollection;
class RimEclipseContourMapProjection;
class Rim3dOverlayInfoConfig;
class RimIntersectionCollection;
class RimIntersectionResultsDefinitionCollection;
class RimPropertyFilterCollection;
class RimGridCollection;
class RimCellRangeFilterCollection;
class RimWellMeasurementInViewCollection;
class RimSurfaceInViewCollection;

class RimGridView : public Rim3dView
{
    CAF_PDM_HEADER_INIT;

public:
    RimGridView();
    ~RimGridView( void ) override;

    void showGridCells( bool enableGridCells );

    Rim3dOverlayInfoConfig* overlayInfoConfig() const;

    cvf::ref<cvf::UByteArray> currentTotalCellVisibility();

    RimIntersectionCollection*                  intersectionCollection() const;
    RimSurfaceInViewCollection*                 surfaceInViewCollection() const;
    RimIntersectionResultsDefinitionCollection* separateIntersectionResultsCollection() const;
    RimIntersectionResultsDefinitionCollection* separateSurfaceResultsCollection() const;
    RimAnnotationInViewCollection*              annotationCollection() const;
    RimWellMeasurementInViewCollection*         measurementCollection() const;

    virtual const RimPropertyFilterCollection* propertyFilterCollection() const = 0;
    void                                       rangeFiltersUpdated();
    RimCellRangeFilterCollection*              rangeFilterCollection();
    const RimCellRangeFilterCollection*        rangeFilterCollection() const;

    bool hasOverridenRangeFilterCollection();
    void setOverrideRangeFilterCollection( RimCellRangeFilterCollection* rfc );
    void replaceRangeFilterCollectionWithOverride();

    RimViewController* viewController() const override;
    RimViewLinker*     assosiatedViewLinker() const override;

    bool isGridVisualizationMode() const override;

    void updateWellMeasurements();
    void updateSurfacesInViewTreeItems();

protected:
    virtual void       updateViewFollowingRangeFilterUpdates();
    void               onClearReservoirCellVisibilitiesIfNeccessary() override;
    virtual void       calculateCurrentTotalCellVisibility( cvf::UByteArray* totalVisibility, int timeStep ) = 0;
    void               selectOverlayInfoConfig() override;
    RimGridCollection* gridCollection() const;
    void               clearReservoirCellVisibilities();

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void initAfterRead() override;

protected:
    cvf::ref<cvf::ModelBasicList> m_surfaceVizModel;

    // Fields
    caf::PdmChildField<RimIntersectionCollection*> m_intersectionCollection;

    caf::PdmChildField<RimIntersectionResultsDefinitionCollection*> m_intersectionResultDefCollection;
    caf::PdmChildField<RimIntersectionResultsDefinitionCollection*> m_surfaceResultDefCollection;

    caf::PdmChildField<Rim3dOverlayInfoConfig*>             m_overlayInfoConfig;
    caf::PdmChildField<RimCellRangeFilterCollection*>       m_rangeFilterCollection;
    caf::PdmChildField<RimCellRangeFilterCollection*>       m_overrideRangeFilterCollection;
    caf::PdmChildField<RimGridCollection*>                  m_gridCollection;
    caf::PdmChildField<RimAnnotationInViewCollection*>      m_annotationCollection;
    caf::PdmChildField<RimWellMeasurementInViewCollection*> m_wellMeasurementCollection;
    caf::PdmChildField<RimSurfaceInViewCollection*>         m_surfaceCollection;

private:
    void onCreatePartCollectionFromSelection( cvf::Collection<cvf::Part>* parts ) override;

    cvf::ref<cvf::UByteArray> m_currentReservoirCellVisibility;
    RimViewLinker*            viewLinkerIfMasterView() const;
    bool                      m_previousGridModeMeshLinesWasFaults;
};
