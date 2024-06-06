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

class RimEclipseContourMapProjection;
class Rim3dOverlayInfoConfig;
class RimIntersectionCollection;
class RimIntersectionResultsDefinitionCollection;
class RimPropertyFilterCollection;
class RimGridCollection;
class RimCellFilterCollection;
class RimWellMeasurementInViewCollection;
class RimSurfaceInViewCollection;
class RimSeismicSectionCollection;
class RimPolygonInViewCollection;

class RimGridView : public Rim3dView
{
    CAF_PDM_HEADER_INIT;

public:
    RimGridView();

    caf::Signal<> cellVisibilityChanged;

    void showGridCells( bool enableGridCells );

    Rim3dOverlayInfoConfig* overlayInfoConfig() const;

    cvf::ref<cvf::UByteArray> currentTotalCellVisibility();

    RimIntersectionCollection*                  intersectionCollection() const;
    virtual RimSurfaceInViewCollection*         surfaceInViewCollection() const;
    RimIntersectionResultsDefinitionCollection* separateIntersectionResultsCollection() const;
    RimIntersectionResultsDefinitionCollection* separateSurfaceResultsCollection() const;
    RimWellMeasurementInViewCollection*         measurementCollection() const;
    RimSeismicSectionCollection*                seismicSectionCollection() const;
    RimPolygonInViewCollection*                 polygonInViewCollection() const;

    virtual const RimPropertyFilterCollection* propertyFilterCollection() const = 0;

    void                           cellFiltersUpdated();
    RimCellFilterCollection*       cellFilterCollection();
    const RimCellFilterCollection* cellFilterCollection() const;

    bool hasOverriddenCellFilterCollection();
    void setOverrideCellFilterCollection( RimCellFilterCollection* rfc );
    void replaceCellFilterCollectionWithOverride();

    bool isGridVisualizationMode() const override;

    void updateWellMeasurements();
    void updateViewTreeItems( RiaDefines::ItemIn3dView itemType ) override;

    RimGridCollection* gridCollection() const;

protected:
    virtual void updateViewFollowingCellFilterUpdates();
    void         onClearReservoirCellVisibilitiesIfNecessary() override;
    virtual void calculateCurrentTotalCellVisibility( cvf::UByteArray* totalVisibility, int timeStep ) = 0;
    void         selectOverlayInfoConfig() override;
    void         clearReservoirCellVisibilities();
    void         addRequiredUiTreeObjects( caf::PdmUiTreeOrdering& uiTreeOrdering );
    void         appendPolygonPartsToModel( caf::DisplayCoordTransform* scaleTransform, const cvf::BoundingBox& boundingBox );

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void initAfterRead() override;

    void appendIntersectionsForCurrentTimeStep();
    void appendIntersectionsToModel( bool cellFiltersActive, bool propertyFiltersActive );

    virtual void calculateCellVisibility( cvf::UByteArray* visibility, std::vector<RivCellSetEnum> geomTypes, int timeStep = 0 ) = 0;

protected:
    cvf::ref<cvf::ModelBasicList> m_surfaceVizModel;
    cvf::ref<cvf::ModelBasicList> m_intersectionVizModel;
    cvf::ref<cvf::ModelBasicList> m_polygonVizModel;

    // Fields
    caf::PdmChildField<RimIntersectionCollection*> m_intersectionCollection;

    caf::PdmChildField<RimIntersectionResultsDefinitionCollection*> m_intersectionResultDefCollection;
    caf::PdmChildField<RimIntersectionResultsDefinitionCollection*> m_surfaceResultDefCollection;

    caf::PdmChildField<Rim3dOverlayInfoConfig*>             m_overlayInfoConfig;
    caf::PdmChildField<RimGridCollection*>                  m_gridCollection;
    caf::PdmChildField<RimWellMeasurementInViewCollection*> m_wellMeasurementCollection;
    caf::PdmChildField<RimSurfaceInViewCollection*>         m_surfaceCollection;
    caf::PdmChildField<RimCellFilterCollection*>            m_cellFilterCollection;
    caf::PdmChildField<RimCellFilterCollection*>            m_overrideCellFilterCollection;
    caf::PdmChildField<RimSeismicSectionCollection*>        m_seismicSectionCollection;
    caf::PdmChildField<RimPolygonInViewCollection*>         m_polygonInViewCollection;

private:
    void onCreatePartCollectionFromSelection( cvf::Collection<cvf::Part>* parts ) override;

private:
    cvf::ref<cvf::UByteArray> m_currentReservoirCellVisibility;
    bool                      m_previousGridModeMeshLinesWasFaults;
};
