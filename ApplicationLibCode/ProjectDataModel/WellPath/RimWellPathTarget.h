/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 -    Equinor
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
#include "cafPdmObject.h"

#include "RiaLineArcWellPathCalculator.h"

#include "cafAppEnum.h"
#include "cafPdmCoreVec3d.h"
#include "cafPdmField.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"
#include "cvfVector3.h"

class RimWellPathGeometryDef;

class RimWellPathTarget : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    caf::Signal<bool> moved;

public:
    RimWellPathTarget();
    ~RimWellPathTarget() override;

    void setEnabled( bool enable );
    bool isEnabled() const;

    void setPointXYZ( const cvf::Vec3d& point );
    void setAsPointTargetXYD( const cvf::Vec3d& point );
    void setAsPointTargetXYZ( const cvf::Vec3d& point );
    void setAsPointXYZAndTangentTarget( const cvf::Vec3d& point, const cvf::Vec3d& tangent );
    void setAsPointXYZAndTangentTarget( const cvf::Vec3d& point, double azimuth, double inclination );
    void setFixedAzimuth( double fixedAzimuth );
    void setFixedInclination( double fixedInclination );
    void setDerivedTangent( double azimuth, double inclination );
    void updateFrom3DManipulator( const cvf::Vec3d& pointXYD );

    RiaLineArcWellPathCalculator::WellTarget wellTargetData();

    enum class TargetTypeEnum
    {
        POINT_AND_TANGENT,
        POINT
    };

    cvf::Vec3d targetPointXYZ() const;
    double     azimuth() const;
    double     inclination() const;
    cvf::Vec3d tangent() const;
    double     radius1() const;
    double     radius2() const;
    void       setRadius1Data( bool isEditable, bool isIncorrect, double actualRadius );
    void       setRadius2Data( bool isEditable, bool isIncorrect, double actualRadius );

    std::vector<caf::PdmFieldHandle*> fieldsFor3dManipulator();

    void onMoved();

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;
    void initAfterRead() override;

    cvf::Vec3d targetPointForDisplayXYD() const;
    void       setTargetPointFromDisplayCoord( const cvf::Vec3d& coordInXYZ );
    double     measuredDepth() const;

    RimWellPathGeometryDef* geometryDefinition() const;
    void                    enableFullUpdate( bool enable );

private:
    caf::PdmField<bool>                 m_isEnabled;
    caf::PdmField<cvf::Vec3d>           m_targetPointXYD;
    caf::PdmProxyValueField<cvf::Vec3d> m_targetPointForDisplay;
    caf::PdmProxyValueField<double>     m_targetMeasuredDepth;

    caf::PdmField<double> m_azimuth;
    caf::PdmField<double> m_inclination;
    caf::PdmField<double> m_dogleg1;
    caf::PdmField<double> m_dogleg2;
    caf::PdmField<bool>   m_useFixedAzimuth;
    caf::PdmField<bool>   m_useFixedInclination;

    caf::PdmField<double> m_estimatedDogleg1;
    caf::PdmField<double> m_estimatedDogleg2;
    caf::PdmField<double> m_estimatedAzimuth;
    caf::PdmField<double> m_estimatedInclination;

    bool                                        m_isFullUpdateEnabled;
    caf::PdmField<bool>                         m_hasTangentConstraintUiField_OBSOLETE;
    caf::PdmField<caf::AppEnum<TargetTypeEnum>> m_targetType_OBSOLETE;
};
