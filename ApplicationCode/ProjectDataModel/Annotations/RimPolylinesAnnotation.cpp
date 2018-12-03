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

#include "RimPolylinesAnnotation.h"

#include "RimAnnotationInViewCollection.h"
#include "RimGridView.h"
#include "RimProject.h"
#include "RimTools.h"
#include "QFile"
#include "RimAnnotationCollection.h"
#include "QFileInfo"

CAF_PDM_ABSTRACT_SOURCE_INIT(RimPolylinesAnnotation, "RimPolylinesAnnotation");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolylinesAnnotation::RimPolylinesAnnotation()
{
    CAF_PDM_InitObject("PolylineAnnotation", ":/WellCollection.png", "", "");

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolylinesAnnotation::~RimPolylinesAnnotation()
{

}

