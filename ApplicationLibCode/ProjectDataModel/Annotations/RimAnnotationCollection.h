/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RimAnnotationCollectionBase.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"

class QString;
class RimAnnotationGroupCollection;
class RimTextAnnotation;
class RimReachCircleAnnotation;
class RimGridView;

//==================================================================================================
///
///
//==================================================================================================
class RimAnnotationCollection : public RimAnnotationCollectionBase
{
    CAF_PDM_HEADER_INIT;

public:
    RimAnnotationCollection();
    ~RimAnnotationCollection() override;

    void addAnnotation( RimReachCircleAnnotation* annotation );

    std::vector<RimReachCircleAnnotation*> reachCircleAnnotations() const;

    size_t lineBasedAnnotationsCount() const;

    void updateViewAnnotationCollections() override;
    void onAnnotationDeleted() override;

    // Used by sync code
    std::vector<caf::PdmObject*> allPdmAnnotations() const;

    void onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

protected:
    void initAfterRead() override;

private:
    caf::PdmChildField<RimAnnotationGroupCollection*> m_reachCircleAnnotations;

    caf::PdmChildField<RimAnnotationGroupCollection*> m_userDefinedPolylineAnnotations_OBSOLETE;
    caf::PdmChildField<RimAnnotationGroupCollection*> m_polylineFromFileAnnotations_OBSOLETE;
};
