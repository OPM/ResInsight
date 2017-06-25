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

#include "RimView.h"

#include "cafAppEnum.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmFieldCvfColor.h"   
#include "cafPdmFieldCvfMat4d.h"   
#include "cafPdmObject.h"
#include "cafPdmPointer.h"

#include "cvfObject.h"

class RigFemPart;
class Rim3dOverlayInfoConfig;
class RimCellRangeFilterCollection;
class RimGeoMechCase;
class RimGeoMechCellColors;
class RimGeoMechResultDefinition;
class RimGeoMechPropertyFilterCollection;
class RiuViewer;
class RivGeoMechPartMgr;
class RivGeoMechVizLogic;

namespace cvf {
    class CellRangeFilter;
    class Transform;
}

//==================================================================================================
///  
///  
//==================================================================================================
class RimGeoMechView : public RimView
{
    CAF_PDM_HEADER_INIT;

public:
    RimGeoMechView(void);
    virtual ~RimGeoMechView(void);

    void                                                setGeoMechCase(RimGeoMechCase* gmCase);
    RimGeoMechCase*                                     geoMechCase();
    virtual RimCase*                                    ownerCase() const override;

    virtual void                                        loadDataAndUpdate();

    caf::PdmChildField<RimGeoMechCellColors*>           cellResult;
    RimGeoMechResultDefinition*                         cellResultResultDefinition();

    virtual const RimPropertyFilterCollection*          propertyFilterCollection() const;

    RimGeoMechPropertyFilterCollection*                 geoMechPropertyFilterCollection();
    const RimGeoMechPropertyFilterCollection*           geoMechPropertyFilterCollection() const;
    void                                                setOverridePropertyFilterCollection(RimGeoMechPropertyFilterCollection* pfc);

    bool                                                isTimeStepDependentDataVisible();

    virtual cvf::Transform*                             scaleTransform();
    virtual void                                        scheduleGeometryRegen(RivCellSetEnum geometryType);
    void                                                updateIconStateForFilterCollections();

    virtual void                                        axisLabels(cvf::String* xLabel, cvf::String* yLabel, cvf::String* zLabel);

    virtual bool                                        isUsingFormationNames() const override;

protected:
    virtual void                                        defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "");
    
    virtual void                                        createPartCollectionFromSelection(cvf::Collection<cvf::Part>* parts);

private:
    virtual void                                        createDisplayModel();
    virtual void                                        updateDisplayModelVisibility();
    virtual void                                        updateScaleTransform();

    virtual void                                        clampCurrentTimestep();

    virtual void                                        updateCurrentTimeStep();
    virtual void                                        updateStaticCellColors();

    virtual void                                        resetLegendsInViewer();

    void                                                updateLegends();

    virtual void                                        fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual void                                        initAfterRead();

    virtual void calculateCurrentTotalCellVisibility(cvf::UByteArray* totalVisibility);


    caf::PdmChildField<RimGeoMechPropertyFilterCollection*> m_propertyFilterCollection;
    caf::PdmPointer<RimGeoMechPropertyFilterCollection>     m_overridePropertyFilterCollection;

    caf::PdmPointer<RimGeoMechCase>                     m_geomechCase;
    cvf::ref<RivGeoMechVizLogic>                        m_vizLogic;
    cvf::ref<cvf::Transform>                            m_scaleTransform;

};

