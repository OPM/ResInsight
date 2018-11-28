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

#include "RiaEclipseUnitTools.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"

class QString;
class RimTextAnnotation;
class RimGridView;

//==================================================================================================
///  
///  
//==================================================================================================
class RimAnnotationCollectionBase : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimAnnotationCollectionBase();
    ~RimAnnotationCollectionBase() override;

    void                            addAnnotation(RimTextAnnotation* annotation);

    std::vector<RimTextAnnotation*> textAnnotations() const;

    void                            onAnnotationDeleted();

    void                            scheduleRedrawOfRelevantViews();
    std::vector<RimGridView*>       gridViewsContainingAnnotations() const;

private:
    caf::PdmChildArrayField<RimTextAnnotation*>    m_textAnnotations;
};
