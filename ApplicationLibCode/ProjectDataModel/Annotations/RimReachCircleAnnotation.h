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
#include "cafPdmObject.h"

#include "cvfObject.h"
#include "cvfVector3.h"

#include <vector>

class QString;
class RicVec3dPickEventHandler;
class RimGridView;
class RimReachCircleLineAppearance;

//==================================================================================================
///
///
//==================================================================================================
class RimReachCircleAnnotation : public caf::PdmObject
{
    friend class RimReachCircleAnnotationInView;

    using Vec3d = cvf::Vec3d;

    CAF_PDM_HEADER_INIT;

public:
    RimReachCircleAnnotation();
    ~RimReachCircleAnnotation() override {}

    bool isActive();
    bool isVisible();
    void enablePicking( bool enable );

    Vec3d                         centerPoint() const;
    double                        radius() const;
    QString                       name() const;
    RimReachCircleLineAppearance* appearance() const;

protected:
    void                 defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                 fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    caf::PdmFieldHandle* userDescriptionField() override;
    caf::PdmFieldHandle* objectToggleField() override;
    virtual void         defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                QString                    uiConfigName,
                                                caf::PdmUiEditorAttribute* attribute ) override;

private:
    caf::PdmField<bool> m_isActive;

    caf::PdmField<Vec3d>                              m_centerPointXyd;
    caf::PdmField<bool>                               m_centerPointPickEnabled;
    caf::PdmField<double>                             m_radius;
    caf::PdmField<QString>                            m_name;
    caf::PdmChildField<RimReachCircleLineAppearance*> m_appearance;

    std::shared_ptr<RicVec3dPickEventHandler> m_centerPointEventHandler;
};
