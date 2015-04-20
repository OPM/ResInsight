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

class RimResultSlot;
class Rim3dOverlayInfoConfig;

//==================================================================================================
///  
///  
//==================================================================================================
class RimGeoMechView : public caf::PdmObject
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

    caf::PdmField<RimResultSlot*>                       cellResult;
    caf::PdmField<Rim3dOverlayInfoConfig*>              overlayInfoConfig;

     // Fields:                                        
    caf::PdmField<QString>                              name;
    caf::PdmField<double>                               scaleZ;

    caf::PdmField< caf::AppEnum< MeshModeType > >       meshMode;
    caf::PdmField< caf::AppEnum< SurfaceModeType > >    surfaceMode;

    caf::PdmField< cvf::Color3f >                       backgroundColor;
protected:
    virtual caf::PdmFieldHandle* userDescriptionField();

private:
};
