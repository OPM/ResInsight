/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 -     Statoil ASA
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
#include "cafPdmFieldHandle.h"
#include "cafPdmObject.h"
#include "cafPdmFieldCvfVec3d.h"

#include "cvfBase.h"
#include "cvfVector3.h"

#include <vector>


//==================================================================================================
///  
///  
//==================================================================================================
class RimFractureTemplate : public caf::PdmObject 
{
     CAF_PDM_HEADER_INIT;

public:
    RimFractureTemplate(void);
    virtual ~RimFractureTemplate(void);
    
    caf::PdmField<QString>   name;
    caf::PdmField<float>     azimuthAngle;
    caf::PdmField<float>     skinFactor;

    enum FracOrientationEnum
    {
        AZIMUTH,
        ALONG_WELL_PATH,
        TRANSVERSE_WELL_PATH
    };
    caf::PdmField< caf::AppEnum< FracOrientationEnum > > orientation;

    enum FracConductivityEnum
    {
        INFINITE_CONDUCTIVITY,
        FINITE_CONDUCTIVITY,
    };
    caf::PdmField< caf::AppEnum< FracConductivityEnum > >  fractureConductivity;
    
    virtual caf::PdmFieldHandle*    userDescriptionField() override;
    virtual void                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    
    virtual void                    fractureGeometry(std::vector<cvf::Vec3f>* nodeCoords, std::vector<cvf::uint>* triangleIndices) = 0;
    virtual std::vector<cvf::Vec3f> fracturePolygon() = 0;
protected:
    virtual void                    defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering);
};
