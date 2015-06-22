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

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafAppEnum.h"
#include "cafPdmFieldCvfColor.h"   
#include "cafPdmFieldCvfMat4d.h"   

#include "cvfObject.h"
#include "RimView.h"

class RimGeoMechResultSlot;
class Rim3dOverlayInfoConfig;
class RiuViewer;
class RimGeoMechCase;
class RivGeoMechPartMgr;
class RimCellRangeFilterCollection;
class RivGeoMechVizLogic;

class RigFemPart;

namespace cvf {
    class Transform;
    class CellRangeFilter;
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

    virtual void                                        loadDataAndUpdate();

    caf::PdmField<RimGeoMechResultSlot*>                cellResult;

    bool                                                isTimeStepDependentDataVisible();

    virtual cvf::Transform*                             scaleTransform();
private:
    virtual void                                        scheduleGeometryRegen(RivCellSetEnum geometryType);
    virtual void                                        createDisplayModel();
    virtual void                                        updateDisplayModelVisibility();
    virtual void                                        updateScaleTransform();

    virtual void                                        clampCurrentTimestep();

    virtual void                                        updateCurrentTimeStep();
    virtual void                                        updateStaticCellColors();

    virtual void                                        updateViewerWidgetWindowTitle();
    virtual void                                        resetLegendsInViewer();

    void                                                updateLegends();

    virtual void                                        fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual void                                        initAfterRead();

    virtual RimCase*                                    ownerCase();

    caf::PdmPointer<RimGeoMechCase>                     m_geomechCase;
    cvf::ref<RivGeoMechVizLogic>                        m_vizLogic;
    cvf::ref<cvf::Transform>                            m_scaleTransform;

};

#include "cvfArray.h"

namespace cvf {
    class CellRangeFilter;
}

