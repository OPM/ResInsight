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

#include "RiaFontCache.h"
#include "RimAnnotationCollectionBase.h"

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafTristate.h"

class RimAnnotationCollection;
class RimAnnotationGroupCollection;
class RimTextAnnotation;
class RimTextAnnotationInView;
class RimReachCircleAnnotationInView;
class RimUserDefinedPolylinesAnnotationInView;
class RimPolylinesFromFileAnnotationInView;

//==================================================================================================
///
///
//==================================================================================================
class RimAnnotationInViewCollection : public RimAnnotationCollectionBase, public caf::FontHolderInterface
{
    CAF_PDM_HEADER_INIT;

public:
    RimAnnotationInViewCollection();
    ~RimAnnotationInViewCollection() override;

    double annotationPlaneZ() const;
    bool   snapAnnotations() const;

    std::vector<RimTextAnnotationInView*>                 globalTextAnnotations() const;
    std::vector<RimReachCircleAnnotationInView*>          globalReachCircleAnnotations() const;
    std::vector<RimUserDefinedPolylinesAnnotationInView*> globalUserDefinedPolylineAnnotations() const;
    std::vector<RimPolylinesFromFileAnnotationInView*>    globalPolylineFromFileAnnotations() const;

    void onGlobalCollectionChanged( const RimAnnotationCollection* globalCollection );

    int  fontSize() const override;
    void updateFonts() override;
    void onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                         std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

private:
    std::vector<caf::PdmObject*> allGlobalPdmAnnotations() const;
    void                         addGlobalAnnotation( caf::PdmObject* annotation );
    void                         deleteGlobalAnnotation( const caf::PdmObject* annotation );

private:
    caf::PdmField<double> m_annotationPlaneDepth;
    caf::PdmField<bool>   m_snapAnnotations;

    caf::PdmField<caf::FontTools::RelativeSizeEnum>   m_annotationFontSize;
    caf::PdmChildField<RimAnnotationGroupCollection*> m_globalTextAnnotations;
    caf::PdmChildField<RimAnnotationGroupCollection*> m_globalReachCircleAnnotations;
    caf::PdmChildField<RimAnnotationGroupCollection*> m_globalUserDefinedPolylineAnnotations;
    caf::PdmChildField<RimAnnotationGroupCollection*> m_globalPolylineFromFileAnnotations;
};
