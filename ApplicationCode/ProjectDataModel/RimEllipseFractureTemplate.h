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
#include "cafPdmFieldHandle.h"
#include "cafPdmObject.h"


//==================================================================================================
///  
///  
//==================================================================================================
class RimEllipseFractureTemplate : public caf::PdmObject //Template i stedet for definition
{
     CAF_PDM_HEADER_INIT;

public:
    RimEllipseFractureTemplate(void);
    virtual ~RimEllipseFractureTemplate(void);
    
    caf::PdmField<QString>   name;

    caf::PdmField<float>     halfLength;
    caf::PdmField<float>     height;

    caf::PdmField<float>     width;
    caf::PdmField<float>     skinFactor;
    caf::PdmField<float>     permeability;

    enum FracOrientationEnum
    {
        AZIMUTH,
        ALONG_WELL_PATH,
        TRANSVERSE_WELL_PATH
    };
    caf::PdmField< caf::AppEnum< FracOrientationEnum > > orientation;
    
    virtual caf::PdmFieldHandle*    userDescriptionField() override;
    virtual void                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    
    double                          effectiveKh();
    
    


    //TODO: Method to get 2D geometry. Returning indexed triangles
    //Code from RimFracture::ComputeGeometry

    //TODO: Method to return a polygon from indexed triangles. 
    //clipper execute method, union for general case. 
    //FOr now: void RigEllipsisTesselator::computeCirclePoints(size_t numSlices) og gi ut rekke med punkter... 
    //Return punkter std::vector<cvf::Vec3d> 



protected:
    virtual void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering);
};
