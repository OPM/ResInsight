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

#include "cafAppEnum.h"
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
class RimGeoMechPropertyFilterCollection;
class RimGeoMechResultDefinition;
class RimRegularLegendConfig;
class RimTensorResults;
class RiuViewer;
class RivGeoMechPartMgr;
class RivGeoMechVizLogic;
class RivTensorResultPartMgr;

namespace cvf {
    class CellRangeFilter;
    class Transform;
}

//==================================================================================================
///  
///  
//==================================================================================================
class RimGeoMechView : public RimGridView
{
    CAF_PDM_HEADER_INIT;

public:
    RimGeoMechView(void);
    virtual ~RimGeoMechView(void);

    void                                                setGeoMechCase(RimGeoMechCase* gmCase);
    RimGeoMechCase*                                     geoMechCase();
    virtual RimCase*                                    ownerCase() const override;

    caf::PdmChildField<RimGeoMechCellColors*>           cellResult;
    RimGeoMechResultDefinition*                         cellResultResultDefinition();

    virtual const RimPropertyFilterCollection*          propertyFilterCollection() const override;

    RimGeoMechPropertyFilterCollection*                 geoMechPropertyFilterCollection();
    const RimGeoMechPropertyFilterCollection*           geoMechPropertyFilterCollection() const;
    void                                                setOverridePropertyFilterCollection(RimGeoMechPropertyFilterCollection* pfc);

    bool                                                isTimeStepDependentDataVisible() const override ;

    virtual cvf::Transform*                             scaleTransform() override;
    virtual void                                        scheduleGeometryRegen(RivCellSetEnum geometryType) override;
    void                                                updateIconStateForFilterCollections();

    virtual void                                        axisLabels(cvf::String* xLabel, cvf::String* yLabel, cvf::String* zLabel) override;

    virtual bool                                        isUsingFormationNames() const override;

    virtual void                                        calculateCurrentTotalCellVisibility(cvf::UByteArray* totalVisibility, int timeStep) override;

    void                                                updateLegendTextAndRanges(RimRegularLegendConfig* legendConfig, int timeStepIndex);

    const cvf::ref<RivGeoMechVizLogic>                  vizLogic() const;
    const RimTensorResults*                             tensorResults() const;
    RimTensorResults*                                   tensorResults();

    std::vector<RimLegendConfig*>                       legendConfigs() const override;

    const RigFemPartCollection*                         femParts() const;
    RigFemPartCollection*                               femParts();

    void                                                convertCameraPositionFromOldProjectFiles();

protected:
    virtual void                                        defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "") override;
    virtual void                                        onLoadDataAndUpdate() override;

    virtual void                                        createPartCollectionFromSelection(cvf::Collection<cvf::Part>* parts) override;

private:
    virtual void                                        createDisplayModel() override;
    virtual void                                        updateScaleTransform() override;

    virtual void                                        clampCurrentTimestep() override;

    virtual void                                        updateCurrentTimeStep() override;
    virtual void                                        updateStaticCellColors() override;

    virtual void                                        resetLegendsInViewer() override;

    void                                                updateLegends() override;

    void                                                updateTensorLegendTextAndRanges(RimRegularLegendConfig* legendConfig, int timeStepIndex);

    virtual void                                        fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual void                                        initAfterRead() override;


    caf::PdmChildField<RimTensorResults*>                   m_tensorResults;
    caf::PdmChildField<RimGeoMechPropertyFilterCollection*> m_propertyFilterCollection;
    caf::PdmPointer<RimGeoMechPropertyFilterCollection>     m_overridePropertyFilterCollection;

    caf::PdmPointer<RimGeoMechCase>                     m_geomechCase;
    cvf::ref<RivGeoMechVizLogic>                        m_vizLogic;
    cvf::ref<cvf::Transform>                            m_scaleTransform;

    cvf::ref<RivTensorResultPartMgr>                    m_tensorPartMgr;
};

