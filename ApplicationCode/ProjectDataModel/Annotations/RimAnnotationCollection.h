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

#include "RimAnnotationCollectionBase.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"

class QString;
class RimTextAnnotation;
class RimReachCircleAnnotation;
class RimUserDefinedPolylinesAnnotation;
class RimPolylinesFromFileAnnotation;
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

    void                            loadDataAndUpdate();

    void                            addAnnotation(RimReachCircleAnnotation* annotation);
    void                            addAnnotation(RimUserDefinedPolylinesAnnotation* annotation);
    void                            addAnnotation(RimPolylinesFromFileAnnotation* annotation);

    std::vector<RimReachCircleAnnotation*>       reachCircleAnnotations() const;
    std::vector<RimUserDefinedPolylinesAnnotation*> userDefinedPolylineAnnotations() const;
    std::vector<RimPolylinesFromFileAnnotation*> polylinesFromFileAnnotations() const;

    RimPolylinesFromFileAnnotation* importOrUpdatePolylinesFromFile(const QStringList& fileNames );

    size_t                          lineBasedAnnotationsCount() const;

private:
    void reloadPolylinesFromFile(const std::vector<RimPolylinesFromFileAnnotation *>& polyLinesObjsToReload);

    caf::PdmChildArrayField<RimReachCircleAnnotation*>          m_reachCircleAnnotations;
    caf::PdmChildArrayField<RimUserDefinedPolylinesAnnotation*> m_userDefinedPolylineAnnotations;
    caf::PdmChildArrayField<RimPolylinesFromFileAnnotation*>    m_polylineFromFileAnnotations;
};
