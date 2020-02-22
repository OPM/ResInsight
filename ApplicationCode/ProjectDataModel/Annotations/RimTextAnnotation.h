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

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafPdmUiOrdering.h"

// Include to make Pdm work for cvf::Color
#include "cafPdmChildField.h"
#include "cafPdmFieldCvfColor.h"
#include "cafPdmFieldCvfVec3d.h"
#include "cafPdmProxyValueField.h"

#include "cvfObject.h"
#include "cvfVector3.h"

#include <memory>
#include <vector>

class QString;
class RicVec3dPickEventHandler;
class RimGridView;
class RimAnnotationTextAppearance;

//==================================================================================================
///
///
//==================================================================================================
class RimTextAnnotation : public caf::PdmObject
{
    friend class RimTextAnnotationInView;

    using Vec3d = cvf::Vec3d;

    CAF_PDM_HEADER_INIT;

public:
    RimTextAnnotation();
    ~RimTextAnnotation() override;

    Vec3d          anchorPoint() const;
    void           setAnchorPoint( const Vec3d& pointXyz );
    Vec3d          labelPoint() const;
    void           setLabelPoint( const Vec3d& pointXyz );
    void           setText( const QString& text );
    const QString& text() const;
    bool           isActive();
    bool           isVisible() const;
    void           enablePicking( bool enable );

    RimAnnotationTextAppearance* appearance() const;

protected:
    void                 defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                 fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    caf::PdmFieldHandle* userDescriptionField() override;
    caf::PdmFieldHandle* objectToggleField() override;
    void                 defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                QString                    uiConfigName,
                                                caf::PdmUiEditorAttribute* attribute ) override;

private:
    friend class RicTextAnnotation3dEditor;

    QString extractNameFromText() const;

    caf::PdmField<Vec3d>   m_anchorPointXyd;
    caf::PdmField<bool>    m_anchorPointPickEnabledButtonField;
    caf::PdmField<Vec3d>   m_labelPointXyd;
    caf::PdmField<bool>    m_labelPointPickEnabledButtonField;
    caf::PdmField<QString> m_text;
    caf::PdmField<bool>    m_isActive;

    caf::PdmChildField<RimAnnotationTextAppearance*> m_textAppearance;

    caf::PdmProxyValueField<QString> m_nameProxy;

    std::shared_ptr<RicVec3dPickEventHandler> m_anchorPointPickEventHandler;
    std::shared_ptr<RicVec3dPickEventHandler> m_labelPointPickEventHandler;
};
