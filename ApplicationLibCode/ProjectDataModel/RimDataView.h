/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "cafPdmField.h"

#include "cafPdmObject.h"

#include "cvfBoundingBox.h"

class Rim3dOverlayInfoConfig;
class RimSurfaceInViewCollection;
class RimPolygonInViewCollection;
class RimWellPathInViewCollection;

class RimDataView : public Rim3dView
{
    CAF_PDM_HEADER_INIT;

public:
    RimDataView();
    ~RimDataView() override;

    RimSurfaceInViewCollection*  surfaceInViewCollection() const;
    RimPolygonInViewCollection*  polygonInViewCollection() const;
    RimWellPathInViewCollection* wellPathInViewCollection() const;

    RimCase*                      ownerCase() const override;
    RiaDefines::View3dContent     viewContent() const override;
    bool                          isGridVisualizationMode() const override;
    bool                          isUsingFormationNames() const override;
    std::vector<RimLegendConfig*> legendConfigs() const override;
    void                          scheduleGeometryRegen( RivCellSetEnum geometryType ) override;

    cvf::BoundingBox domainBoundingBox() override;
    void             updateGridBoxData() override;
    double           characteristicCellSize() const override;

    bool isWellPathVisibleInView( const RimWellPath* wellPath ) const override;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

    void   onCreateDisplayModel() override;
    void   onUpdateDisplayModelForCurrentTimeStep() override;
    void   onClampCurrentTimestep() override;
    size_t onTimeStepCountRequested() override;
    bool   isTimeStepDependentDataVisible() const override;
    void   defineAxisLabels( cvf::String* xLabel, cvf::String* yLabel, cvf::String* zLabel ) override;
    void   onCreatePartCollectionFromSelection( cvf::Collection<cvf::Part>* parts ) override;
    void   onUpdateStaticCellColors() override;
    void   onUpdateLegends() override;

    void onLoadDataAndUpdate() override;
    void selectOverlayInfoConfig() override;

    cvf::Transform* scaleTransform() override;

    QString createAutoName() const override;

    void setDefaultView() override;

    void updateViewTreeItems( RiaDefines::ItemIn3dView itemType ) override;

private:
    cvf::BoundingBox computeDomainBoundingBox() const;
    cvf::BoundingBox cachedDomainBoundingBox() const;
    void             invalidateDomainBoundingBox();

private:
    caf::PdmChildField<RimSurfaceInViewCollection*>  m_surfaceCollection;
    caf::PdmChildField<RimPolygonInViewCollection*>  m_polygonInViewCollection;
    caf::PdmChildField<RimWellPathInViewCollection*> m_wellPathInViewCollection;
    caf::PdmChildField<Rim3dOverlayInfoConfig*>      m_overlayInfoConfig;

    cvf::ref<cvf::ModelBasicList> m_surfaceVizModel;
    cvf::ref<cvf::ModelBasicList> m_polygonVizModel;
    cvf::ref<cvf::Transform>      m_scaleTransform;

    mutable cvf::BoundingBox m_domainBoundingBox;
    mutable bool             m_isDomainBoundingBoxCached;
};
