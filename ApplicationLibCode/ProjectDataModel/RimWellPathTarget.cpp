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

#include "RimWellPathTarget.h"

#include "RiaOffshoreSphericalCoords.h"

#include "RigWellPath.h"

#include "RimModeledWellPath.h"
#include "RimWellPath.h"
#include "RimWellPathGeometryDef.h"

#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiLineEditor.h"

#include <cmath>

CAF_PDM_SOURCE_INIT( RimWellPathTarget, "WellPathTarget" );

namespace caf
{
template <>
void caf::AppEnum<RimWellPathTarget::TargetTypeEnum>::setUp()
{
    addItem( RimWellPathTarget::POINT_AND_TANGENT, "POINT_AND_TANGENT", "Point and Tangent" );
    addItem( RimWellPathTarget::POINT, "POINT", "Point" );
    setDefault( RimWellPathTarget::POINT_AND_TANGENT );
}
} // namespace caf
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathTarget::RimWellPathTarget()
    : moved( this )
    , m_targetType( POINT_AND_TANGENT )
    , m_targetPoint( cvf::Vec3d::ZERO )
    , m_azimuth( 0.0 )
    , m_inclination( 0.0 )
    , m_isFullUpdateEnabled( true )
{
    CAF_PDM_InitField( &m_isEnabled, "IsEnabled", true, "", "", "", "" );
    CAF_PDM_InitField( &m_isLocked, "IsLocked", false, "", "", "", "" );
    m_isLocked.uiCapability()->setUiHidden( true );
    // m_targetType.uiCapability()->setUiHidden(true);
    CAF_PDM_InitFieldNoDefault( &m_targetPoint, "TargetPoint", "Point", "", "", "" );
    CAF_PDM_InitField( &m_dogleg1, "Dogleg1", 3.0, "DL in", "", "[deg/30m]", "" );
    CAF_PDM_InitField( &m_dogleg2, "Dogleg2", 3.0, "DL out", "", "[deg/30m]", "" );
    CAF_PDM_InitFieldNoDefault( &m_targetType, "TargetType", "Type", "", "", "" );
    m_targetType.uiCapability()->setUiHidden( true );
    CAF_PDM_InitField( &m_hasTangentConstraintUiField, "HasTangentConstraint", false, "Dir", "", "", "" );
    m_hasTangentConstraintUiField.xmlCapability()->disableIO();
    CAF_PDM_InitField( &m_azimuth, "Azimuth", 0.0, "Azi(deg)", "", "", "" );
    CAF_PDM_InitField( &m_inclination, "Inclination", 0.0, "Inc(deg)", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_parentWellPath, "ParentWellPath", "Parent Well Path", "", "", "" );

    m_parentWellPath.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathTarget::~RimWellPathTarget()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathTarget::setEnabled( bool enable )
{
    m_isEnabled = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPathTarget::isEnabled() const
{
    return m_isEnabled;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathTarget::setAsPointTargetXYD( const cvf::Vec3d& point )
{
    m_targetType  = POINT;
    m_targetPoint = point;
    m_azimuth     = 0.0;
    m_inclination = 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathTarget::setAsPointTargetXYZ( const cvf::Vec3d& point )
{
    m_targetType  = POINT;
    m_targetPoint = cvf::Vec3d( point.x(), point.y(), -point.z() );
    m_azimuth     = 0.0;
    m_inclination = 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathTarget::setAsPointXYZAndTangentTarget( const cvf::Vec3d& point, const cvf::Vec3d& tangent )
{
    RiaOffshoreSphericalCoords sphericalCoords( tangent );
    setAsPointXYZAndTangentTarget( point, sphericalCoords.azi(), sphericalCoords.inc() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathTarget::setAsPointXYZAndTangentTarget( const cvf::Vec3d& point, double azimuth, double inclination )
{
    m_targetType  = POINT_AND_TANGENT;
    m_targetPoint = cvf::Vec3d( point.x(), point.y(), -point.z() );
    m_azimuth     = cvf::Math::toDegrees( azimuth );
    m_inclination = cvf::Math::toDegrees( inclination );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathTarget::setDerivedTangent( double azimuth, double inclination )
{
    if ( m_targetType == POINT )
    {
        m_azimuth     = cvf::Math::toDegrees( azimuth );
        m_inclination = cvf::Math::toDegrees( inclination );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaLineArcWellPathCalculator::WellTarget RimWellPathTarget::wellTargetData()
{
    RiaLineArcWellPathCalculator::WellTarget targetData;

    targetData.targetPointXYZ       = targetPointXYZ();
    targetData.isTangentConstrained = ( targetType() == POINT_AND_TANGENT );
    targetData.azimuth              = azimuth();
    targetData.inclination          = inclination();
    targetData.radius1              = radius1();
    targetData.radius2              = radius2();

    return targetData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathTarget::TargetTypeEnum RimWellPathTarget::targetType() const
{
    return m_targetType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimWellPathTarget::targetPointXYZ() const
{
    cvf::Vec3d xyzPoint( m_targetPoint() );
    xyzPoint.z() = -xyzPoint.z();
    return xyzPoint;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPathTarget::azimuth() const
{
    if ( m_targetType() == POINT_AND_TANGENT )
    {
        return cvf::Math::toRadians( m_azimuth );
    }
    else
    {
        return std::numeric_limits<double>::infinity();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPathTarget::inclination() const
{
    if ( m_targetType() == POINT_AND_TANGENT )
    {
        return cvf::Math::toRadians( m_inclination );
    }
    else
    {
        return std::numeric_limits<double>::infinity();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimWellPathTarget::tangent() const
{
    double aziRad = cvf::Math::toRadians( m_azimuth );
    double incRad = cvf::Math::toRadians( m_inclination );

    return RiaOffshoreSphericalCoords::unitVectorFromAziInc( aziRad, incRad );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPathTarget::radius1() const
{
    // Needs to be aware of unit to select correct DLS conversion
    // Degrees pr 100 ft
    // Degrees pr 10m

    // Degrees pr 30m
    if ( fabs( m_dogleg1 ) < 1e-6 ) return std::numeric_limits<double>::infinity();

    return 30.0 / cvf::Math::toRadians( m_dogleg1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPathTarget::radius2() const
{
    // Needs to be aware of unit to select correct DLS conversion
    // Degrees pr 100 ft
    // Degrees pr 10m

    // Degrees pr 30m

    if ( fabs( m_dogleg2 ) < 1e-6 ) return std::numeric_limits<double>::infinity();

    return 30.0 / cvf::Math::toRadians( m_dogleg2 );
}

double doglegFromRadius( double radius )
{
    return cvf::Math::toDegrees( 30.0 / radius );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathTarget::flagRadius1AsIncorrect( bool isEditable, bool isIncorrect, double actualRadius )
{
    if ( isIncorrect )
    {
        if ( actualRadius < radius1() )
        {
            m_dogleg1.uiCapability()->setUiContentTextColor( Qt::red );
            m_dogleg1.uiCapability()->setUiToolTip( "Actual Dogleg: " + QString::number( doglegFromRadius( actualRadius ) ) +
                                                    "\nThe dogleg constraint is not satisfied!" );
        }
        else
        {
            m_dogleg1.uiCapability()->setUiContentTextColor( Qt::darkGreen );
            m_dogleg1.uiCapability()->setUiToolTip( "Actual Dogleg: " +
                                                    QString::number( doglegFromRadius( actualRadius ) ) );
        }
    }
    else
    {
        m_dogleg1.uiCapability()->setUiContentTextColor( QColor() );
        m_dogleg1.uiCapability()->setUiToolTip( "" );
    }

    m_dogleg1.uiCapability()->setUiReadOnly( !isEditable );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathTarget::flagRadius2AsIncorrect( bool isEditable, bool isIncorrect, double actualRadius )
{
    if ( isIncorrect )
    {
        if ( actualRadius < radius2() )
        {
            m_dogleg2.uiCapability()->setUiContentTextColor( Qt::red );
            m_dogleg2.uiCapability()->setUiToolTip( "Actual Dogleg: " + QString::number( doglegFromRadius( actualRadius ) ) +
                                                    "\nThe dogleg constraint is not satisfied!" );
        }
        else
        {
            m_dogleg2.uiCapability()->setUiContentTextColor( Qt::darkGreen );
            m_dogleg2.uiCapability()->setUiToolTip( "Actual Dogleg: " +
                                                    QString::number( doglegFromRadius( actualRadius ) ) );
        }
    }
    else
    {
        m_dogleg2.uiCapability()->setUiContentTextColor( QColor() );
        m_dogleg2.uiCapability()->setUiToolTip( "" );
    }

    m_dogleg2.uiCapability()->setUiReadOnly( !isEditable );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathTarget::onMoved()
{
    moved.send( m_isFullUpdateEnabled );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathTarget::enableFullUpdate( bool enable )
{
    m_isFullUpdateEnabled = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathTarget::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                               QString                    uiConfigName,
                                               caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_targetPoint )
    {
        auto uiDisplayStringAttr = dynamic_cast<caf::PdmUiLineEditorAttributeUiDisplayString*>( attribute );

        if ( uiDisplayStringAttr )
        {
            uiDisplayStringAttr->m_displayString = QString::number( m_targetPoint()[0], 'f', 2 ) + " " +
                                                   QString::number( m_targetPoint()[1], 'f', 2 ) + " " +
                                                   QString::number( m_targetPoint()[2], 'f', 2 );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellPathTarget::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                        bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;
    if ( fieldNeedingOptions == &m_targetType )
    {
        options.push_back(
            caf::PdmOptionItemInfo( "o->",
                                    RimWellPathTarget::POINT_AND_TANGENT ) ); //, false,
                                                                              // QIcon(":/WellTargetPointTangent16x16.png")
                                                                              //));
        options.push_back( caf::PdmOptionItemInfo( "o", RimWellPathTarget::POINT ) ); //, false,
                                                                                      // QIcon(":/WellTargetPoint16x16.png")));
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathTarget::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                          const QVariant&            oldValue,
                                          const QVariant&            newValue )
{
    if ( changedField == &m_hasTangentConstraintUiField )
    {
        if ( m_hasTangentConstraintUiField )
            m_targetType = POINT_AND_TANGENT;
        else
            m_targetType = POINT;
    }

    moved.send( m_isFullUpdateEnabled );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathTarget::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    m_hasTangentConstraintUiField = ( m_targetType == POINT_AND_TANGENT );

    if ( m_isEnabled() && !m_isLocked() )
    {
        m_hasTangentConstraintUiField.uiCapability()->setUiReadOnly( false );
        m_targetType.uiCapability()->setUiReadOnly( false );
        m_targetPoint.uiCapability()->setUiReadOnly( false );

        if ( m_targetType == POINT )
        {
            m_azimuth.uiCapability()->setUiReadOnly( true );
            m_inclination.uiCapability()->setUiReadOnly( true );
        }
        else
        {
            m_azimuth.uiCapability()->setUiReadOnly( false );
            m_inclination.uiCapability()->setUiReadOnly( false );
        }
    }
    else
    {
        m_dogleg1.uiCapability()->setUiReadOnly( true );
        m_targetType.uiCapability()->setUiReadOnly( true );
        m_targetPoint.uiCapability()->setUiReadOnly( true );
        m_azimuth.uiCapability()->setUiReadOnly( true );
        m_inclination.uiCapability()->setUiReadOnly( true );
        m_dogleg2.uiCapability()->setUiReadOnly( true );
        m_hasTangentConstraintUiField.uiCapability()->setUiReadOnly( true );
    }

    if ( m_isLocked )
    {
        m_isEnabled.uiCapability()->setUiReadOnly( true );
    }
}
