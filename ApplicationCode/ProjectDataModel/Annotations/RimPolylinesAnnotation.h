/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     equinor ASA
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
#include "RimLineBasedAnnotation.h"

#include "cafPdmField.h"

// Include to make Pdm work for cvf::Color
#include "cvfBase.h"
#include "cvfObject.h"

class RigPolyLinesData;

//==================================================================================================
///
///
//==================================================================================================
class RimPolylinesAnnotation : public RimLineBasedAnnotation
{
    CAF_PDM_HEADER_INIT;

public:
    RimPolylinesAnnotation();
    ~RimPolylinesAnnotation();

    virtual cvf::ref<RigPolyLinesData> polyLinesData() = 0;
    virtual bool isEmpty() = 0;
};
