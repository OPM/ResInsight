/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "RiaSeismicDefines.h"

#include "Rim3dView.h"
#include "RimPolylinesDataInterface.h"

#include "cafPdmField.h"

#include "cafPdmObject.h"

class RimCase;
class RimSeismicDataInterface;
class RimSurfaceInViewCollection;
class RimSeismicSectionCollection;
class Rim3dOverlayInfoConfig;
class RivPolylinePartMgr;
class RigHistogramData;
class RimAnnotationInViewCollection;

class RimSeismicView : public Rim3dView, public RimPolylinesDataInterface
{
    CAF_PDM_HEADER_INIT;

public:
    RimSeismicView();
    ~RimSeismicView() override;

    void                     setSeismicData( RimSeismicDataInterface* data );
    RimSeismicDataInterface* seismicData() const;

    void addSlice( RiaDefines::SeismicSectionType sectionType );

    RimSurfaceInViewCollection*  surfaceInViewCollection() const;
    RimSeismicSectionCollection* seismicSectionCollection() const;

    RimCase*                      ownerCase() const override;
    RiaDefines::View3dContent     viewContent() const override;
    bool                          isGridVisualizationMode() const override;
    bool                          isUsingFormationNames() const override;
    std::vector<RimLegendConfig*> legendConfigs() const override;
    void                          scheduleGeometryRegen( RivCellSetEnum geometryType ) override;

    cvf::BoundingBox domainBoundingBox() override;
    void             updateGridBoxData() override;
    double           characteristicCellSize() const override;
    RigHistogramData histogramData();

    cvf::ref<RigPolyLinesData> polyLinesData() const override;

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

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
    caf::PdmChildField<RimSurfaceInViewCollection*>  m_surfaceCollection;
    caf::PdmChildField<RimSeismicSectionCollection*> m_seismicSectionCollection;

    caf::PdmChildField<Rim3dOverlayInfoConfig*> m_overlayInfoConfig;

    caf::PdmPtrField<RimSeismicDataInterface*> m_seismicData;

    cvf::ref<cvf::ModelBasicList> m_surfaceVizModel;
    cvf::ref<RivPolylinePartMgr>  m_polylinePartMgr;

    cvf::ref<cvf::Transform> m_scaleTransform;
};
