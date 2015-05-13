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

class RigFemPart;
namespace cvf {
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

    void                                                loadDataAndUpdate();

    virtual void                                        endAnimation() {}

    caf::PdmField<RimGeoMechResultSlot*>                cellResult;


    bool                                                isTimeStepDependentDataVisible();

private:
    virtual void                                        createDisplayModel();
    virtual void                                        updateDisplayModelVisibility();
    virtual void                                        updateScaleTransform();
    virtual cvf::Transform*                             scaleTransform();

    virtual void                                        clampCurrentTimestep();

    virtual void                                        updateCurrentTimeStep();
    virtual void                                        updateStaticCellColors();

    virtual void                                        updateViewerWidgetWindowTitle();
    virtual void                                        resetLegendsInViewer();

    void                                                updateLegends();

    caf::PdmPointer<RimGeoMechCase>                     m_geomechCase;
    cvf::ref<RivGeoMechPartMgr>                         m_geoMechFullModel;
    bool                                                m_isGeoMechFullGenerated;
    cvf::ref<cvf::Transform>                            m_scaleTransform;

};

#include "cvfArray.h"

class RivElmVisibilityCalculator
{
public:
    static void computeAllVisible(cvf::UByteArray* elmVisibilities, const RigFemPart* femPart );


};