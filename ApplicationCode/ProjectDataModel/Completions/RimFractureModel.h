/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-2018 Statoil ASA
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

#include "Rim3dPropertiesInterface.h"
#include "RimCheckableNamedObject.h"
#include "RimFractureTemplate.h"
#include "RimWellPathComponentInterface.h"

#include "cvfMatrix4.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include "cafPdmChildField.h"
#include "cafPdmFieldCvfVec3d.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

class RimEclipseCase;
class RimEllipseFractureTemplate;
class RivWellFracturePartMgr;
class RimFractureTemplate;
class RigFracturedEclipseCellExportData;
class RigMainGrid;

//==================================================================================================
///
///
//==================================================================================================
class RimFractureModel : public RimCheckableNamedObject, public Rim3dPropertiesInterface, public RimWellPathComponentInterface
{
    CAF_PDM_HEADER_INIT;

public:
    enum ThicknessType
    {
        TRUE_VERTICAL_THICKNESS,
        TRUE_STRATIGRAPHIC_THICKNESS,
    };

    RimFractureModel( void );
    ~RimFractureModel( void ) override;

    cvf::Vec3d anchorPosition() const;

    cvf::Mat4d transformMatrix() const;

    void       fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    cvf::Vec3d fracturePosition() const;

    // RimWellPathCompletionsInterface overrides.
    RiaDefines::WellPathComponentType componentType() const override;
    QString                           componentLabel() const override;
    QString                           componentTypeLabel() const override;
    cvf::Color3f                      defaultComponentColor() const override;
    double                            startMD() const override;
    double                            endMD() const override;
    bool                              isEnabled() const override;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

private:
    cvf::BoundingBox boundingBoxInDomainCoords() const override;
    void             updatePositionFromMeasuredDepth();
    void             updateThicknessDirection();
    cvf::Vec3d       calculateTSTDirection() const;

protected:
    caf::PdmField<double>                                                 m_MD;
    caf::PdmField<caf::AppEnum<RimFractureTemplate::FracOrientationEnum>> m_orientationType;
    caf::PdmField<caf::AppEnum<ThicknessType>>                            m_thicknessType;
    caf::PdmField<double>                                                 m_azimuth;
    caf::PdmField<double>                                                 m_dip;
    caf::PdmField<double>                                                 m_tilt;
    caf::PdmField<cvf::Vec3d>                                             m_anchorPosition;
    caf::PdmField<cvf::Vec3d>                                             m_thicknessDirection;
};
