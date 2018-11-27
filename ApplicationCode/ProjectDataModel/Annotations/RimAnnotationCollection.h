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
class RimReachCircleAnnotation;
class RimPolylinesAnnotation;
class RimPolylinesFromFileAnnotation;
class RimGridView;

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

    void                            loadDataAndUpdate();

    void                            addAnnotation(RimTextAnnotation* annotation);
    void                            addAnnotation(RimReachCircleAnnotation* annotation);
    void                            addAnnotation(RimPolylinesAnnotation* annotation);

    std::vector<RimTextAnnotation*>              textAnnotations() const;
    std::vector<RimReachCircleAnnotation*>       reachCircleAnnotations() const;
    std::vector<RimPolylinesAnnotation*>         polylineAnnotations() const;
    std::vector<RimPolylinesFromFileAnnotation*> polylinesFromFileAnnotations() const;

    void                            onAnnotationDeleted();

    RimPolylinesFromFileAnnotation* importOrUpdatePolylinesFromFile(const QStringList& fileNames );

    void                            scheduleRedrawOfRelevantViews();
    std::vector<RimGridView*>       gridViewsContainingAnnotations() const;

private:
    void reloadPolylinesFromFile(const std::vector<RimPolylinesFromFileAnnotation *>& polyLinesObjsToReload);

    caf::PdmChildArrayField<RimTextAnnotation*>              m_textAnnotations;
    caf::PdmChildArrayField<RimReachCircleAnnotation*>       m_reachCircleAnnotations;
    caf::PdmChildArrayField<RimPolylinesAnnotation*>         m_polylineAnnotations;
    caf::PdmChildArrayField<RimPolylinesFromFileAnnotation*> m_polylineFromFileAnnotations;
};
