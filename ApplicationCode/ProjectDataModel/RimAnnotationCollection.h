/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RiaEclipseUnitTools.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafAppEnum.h"

// Include to make Pdm work for cvf::Color
#include "cafPdmFieldCvfColor.h"    
#include "cafPdmChildField.h"

#include "cvfObject.h"

class QString;
class RimTextAnnotation;
class RimReachCircleAnnotation;
class RimPolylineAnnotation;


//==================================================================================================
///  
///  
//==================================================================================================
class RimAnnotationCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimAnnotationCollection();
    ~RimAnnotationCollection() override;

    void                            addAnnotation(RimTextAnnotation* annotation);
    void                            addAnnotation(RimReachCircleAnnotation* annotation);
    void                            addAnnotation(RimPolylineAnnotation* annotation);
    
    std::vector<RimTextAnnotation*>         textAnnotations() const;
    std::vector<RimReachCircleAnnotation*>  reachCircleAnnotations() const;
    std::vector<RimPolylineAnnotation*>     polylineAnnotations() const;

private:
    caf::PdmChildArrayField<RimTextAnnotation*>         m_textAnnotations;
    caf::PdmChildArrayField<RimReachCircleAnnotation*>  m_reachCircleAnnotations;
    caf::PdmChildArrayField<RimPolylineAnnotation*>     m_polylineAnnotations;
};
