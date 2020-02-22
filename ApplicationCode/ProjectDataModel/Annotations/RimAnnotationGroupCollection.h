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
class RimAnnotationGroupCollection : public caf::PdmObject
{
    friend class RimAnnotationCollection;
    friend class RimAnnotationInViewCollection;

    CAF_PDM_HEADER_INIT;

public:
    const static QString TEXT_ANNOTATION_UI_NAME;
    const static QString REACH_CIRCLE_ANNOTATION_UI_NAME;
    const static QString USED_DEFINED_POLYLINE_ANNOTATION_UI_NAME;
    const static QString POLYLINE_FROM_FILE_ANNOTATION_UI_NAME;

public:
    RimAnnotationGroupCollection();
    ~RimAnnotationGroupCollection() override;

    bool isActive() const;
    bool isVisible() const;

    void                         addAnnotation( caf::PdmObject* annotation );
    std::vector<caf::PdmObject*> annotations() const;

protected:
    void                 fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    caf::PdmFieldHandle* objectToggleField() override;

protected:
    caf::PdmField<bool>                      m_isActive;
    caf::PdmChildArrayField<caf::PdmObject*> m_annotations;
};
