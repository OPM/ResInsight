/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"
#include "RimView.h"
#include "cvfVector3.h"

class RimFractureDefinition;
class RimWellPath;

//==================================================================================================
///  
///  
//==================================================================================================
class RimFracture : public caf::PdmObject
{
     CAF_PDM_HEADER_INIT;

public:
    enum FractureWellEnum
    {
        FRACTURE_WELL_PATH,
        FRACTURE_SIMULATION_WELL
    };

public:
    RimFracture(void);
    virtual ~RimFracture(void);

    caf::PdmField<QString>                              name;
    caf::PdmPtrField<RimFractureDefinition* >           fractureDefinition;
    caf::PdmField< caf::AppEnum< FractureWellEnum > >   welltype;

    caf::PdmPtrField<RimWellPath*>  wellpath;
    caf::PdmField<float>            measuredDepth;
    caf::PdmField<cvf::Vec3d>       positionAtWellpath;

    caf::PdmField<int>              i;
    caf::PdmField<int>              j;
    caf::PdmField<int>              k;



    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly) override;


protected:
    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering);



};
