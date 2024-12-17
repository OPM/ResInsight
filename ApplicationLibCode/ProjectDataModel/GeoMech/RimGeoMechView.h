/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RimGridView.h"

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfMat4d.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"

#include "cvfObject.h"

class RigFemPart;
class RigFemPartCollection;
class Rim3dOverlayInfoConfig;
class RimCellRangeFilterCollection;
class RimGeoMechCase;
class RimGeoMechCellColors;
class RimGeoMechFaultReactivationResult;
class RimGeoMechPartCollection;
class RimGeoMechPropertyFilterCollection;
class RimGeoMechResultDefinition;
class RimRegularLegendConfig;
class RimTensorResults;
class RiuViewer;
class RivGeoMechPartMgr;
class RivGeoMechVizLogic;
class RivTensorResultPartMgr;
class RimGeoMechPartCollection;

namespace cvf
{
class CellRangeFilter;
class Transform;
} // namespace cvf

//==================================================================================================
///
///
//==================================================================================================
class RimGeoMechView : public RimGridView
{
    CAF_PDM_HEADER_INIT;

public:
    RimGeoMechView();
    ~RimGeoMechView() override;

    RiaDefines::View3dContent viewContent() const override;

    void            setGeoMechCase( RimGeoMechCase* gmCase );
    RimGeoMechCase* geoMechCase() const;
    RimCase*        ownerCase() const override;

    caf::PdmChildField<RimGeoMechCellColors*> cellResult;
    RimGeoMechResultDefinition*               cellResultResultDefinition() const;

    const RimPropertyFilterCollection* propertyFilterCollection() const override;

    const RimGeoMechPartCollection* partsCollection() const;

    RimGeoMechPropertyFilterCollection*       geoMechPropertyFilterCollection();
    const RimGeoMechPropertyFilterCollection* geoMechPropertyFilterCollection() const;
    void                                      setOverridePropertyFilterCollection( RimGeoMechPropertyFilterCollection* pfc );

    bool isTimeStepDependentDataVisible() const override;

    cvf::Transform* scaleTransform() override;
    void            scheduleGeometryRegen( RivCellSetEnum geometryType ) override;
    void            updateIconStateForFilterCollections();

    void defineAxisLabels( cvf::String* xLabel, cvf::String* yLabel, cvf::String* zLabel ) override;

    bool isUsingFormationNames() const override;

    void calculateCurrentTotalCellVisibility( cvf::UByteArray* totalVisibility, int viewerTimeStep ) override;

    void updateLegendTextAndRanges( RimRegularLegendConfig* legendConfig, int viewerTimeStep );

    const cvf::ref<RivGeoMechVizLogic> vizLogic() const;
    const RimTensorResults*            tensorResults() const;
    RimTensorResults*                  tensorResults();

    std::vector<RimLegendConfig*> legendConfigs() const override;

    const RigFemPartCollection* femParts() const;
    RigFemPartCollection*       femParts();

    void convertCameraPositionFromOldProjectFiles();

    double displacementScaleFactor() const;
    bool   showDisplacements() const;
    void   setShowDisplacementsAndUpdate( bool show );

    std::pair<int, int> currentStepAndDataFrame() const;

    void resetVizLogic();

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void onLoadDataAndUpdate() override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void initAfterRead() override;

    void onCreateDisplayModel() override;

    RimPropertyFilterCollection* nativePropertyFilterCollection();

    void calculateCellVisibility( cvf::UByteArray* visibility, std::vector<RivCellSetEnum> geomTypes, int timeStep = 0 ) override;

private:
    QString createAutoName() const override;

    void   onClampCurrentTimestep() override;
    size_t onTimeStepCountRequested() override;

    void onUpdateDisplayModelForCurrentTimeStep() override;
    void onUpdateStaticCellColors() override;

    void childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField ) override;

    void onUpdateLegends() override;

    void updateTensorLegendTextAndRanges( RimRegularLegendConfig* legendConfig, int viewerTimeStep );

    void updateElementDisplacements();

    std::pair<int, int> viewerStepToTimeStepAndFrameIndex( int viewerTimeStep );

    caf::PdmChildField<RimTensorResults*>                   m_tensorResults;
    caf::PdmChildField<RimGeoMechPropertyFilterCollection*> m_propertyFilterCollection;
    caf::PdmChildField<RimGeoMechFaultReactivationResult*>  m_faultReactivationResult;
    caf::PdmPointer<RimGeoMechPropertyFilterCollection>     m_overridePropertyFilterCollection;
    caf::PdmChildField<RimGeoMechPartCollection*>           m_partsCollection;
    caf::PdmPointer<RimGeoMechCase>                         m_geomechCase;
    caf::PdmField<bool>                                     m_showDisplacement;
    caf::PdmField<double>                                   m_displacementScaling;

    cvf::ref<RivGeoMechVizLogic> m_vizLogic;
    cvf::ref<cvf::Transform>     m_scaleTransform;

    cvf::ref<RivTensorResultPartMgr> m_tensorPartMgr;

    int m_currentInternalTimeStep;
    int m_currentDataFrameIndex;
};
