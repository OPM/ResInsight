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

    enum MeshModeType
    {
        FULL_MESH,
        FAULTS_MESH,
        NO_MESH
    };

    enum SurfaceModeType
    {
        SURFACE,
        FAULTS,
        NO_SURFACE
    };

    void                                                setGeoMechCase(RimGeoMechCase* gmCase);
    RimGeoMechCase*                                     geoMechCase(); 

    void                                                loadDataAndUpdate();
    virtual void                                        createDisplayModelAndRedraw();

    virtual void                                        setCurrentTimeStep(int frameIdx){}
    virtual void                                        updateCurrentTimeStepAndRedraw(){}
    virtual void                                        endAnimation() {}

    caf::PdmField<RimGeoMechResultSlot*>                cellResult;

    caf::PdmField< caf::AppEnum< MeshModeType > >       meshMode;
    caf::PdmField< caf::AppEnum< SurfaceModeType > >    surfaceMode;

private:
   
   virtual void                                         updateViewerWidgetWindowTitle();
   virtual void                                         resetLegendsInViewer();

   void                                                 updateLegends();
   caf::PdmPointer<RimGeoMechCase>                      m_geomechCase;
   cvf::ref<RivGeoMechPartMgr>                          m_geoMechVizModel;
};

#include "cvfArray.h"

class RivElmVisibilityCalculator
{
public:
    static void computeAllVisible(cvf::UByteArray* elmVisibilities, const RigFemPart* femPart );


};