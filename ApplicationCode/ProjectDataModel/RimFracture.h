/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "cafPdmObject.h"

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfVector3.h"


class RimFractureDefinition;
class RigFracture;

//==================================================================================================
///  
///  
//==================================================================================================
class RimFracture : public caf::PdmObject
{
     CAF_PDM_HEADER_INIT;

public:
    RimFracture(void);
    virtual ~RimFracture(void);

    virtual cvf::Vec3d              centerPointForFracture() = 0;
    virtual RimFractureDefinition*  attachedFractureDefinition() = 0;
    
    bool                            hasValidGeometry() const;
    void                            computeGeometry();

    const std::vector<cvf::uint>&   polygonIndices() const;
    const std::vector<cvf::Vec3f>&  nodeCoords() const;

private:
    cvf::ref<RigFracture> m_rigFracture;
};
