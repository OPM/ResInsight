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

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapabilityCvfVec3d.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiLineEditor.h"

#include <cmath>

CAF_PDM_SOURCE_INIT( RimWellPathTarget, "WellPathTarget" );

namespace caf
{
template <>
void caf::AppEnum<RimWellPathTarget::TargetTypeEnum>::setUp()
{
    addItem( RimWellPathTarget::TargetTypeEnum::POINT_AND_TANGENT, "POINT_AND_TANGENT", "Point and Tangent" );
    addItem( RimWellPathTarget::TargetTypeEnum::POINT, "POINT", "Point" );
    setDefault( RimWellPathTarget::TargetTypeEnum::POINT );
}
} // namespace caf
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathTarget::RimWellPathTarget()
    : moved( this )
    , m_targetType_OBSOLETE( TargetTypeEnum::POINT )
    , m_targetPointXYD( cvf::Vec3d::ZERO )
    , m_azimuth( 0.0 )
    , m_inclination( 0.0 )
    , m_isFullUpdateEnabled( true )
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "Well Target",
                                                    "",
                                                    "",
                                                    "",
                                                    "WellPathTarget",
                                                    "Class containing the Well Target definition" );

    CAF_PDM_InitField( &m_isEnabled, "IsEnabled", true, "" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_targetPointXYD, "TargetPoint", "Relative Coord" );
    CAF_PDM_InitFieldNoDefault( &m_targetPointForDisplay, "TargetPointForDisplay", "UTM Coord" );
    m_targetPointForDisplay.registerGetMethod( this, &RimWellPathTarget::targetPointForDisplayXYD );
    m_targetPointForDisplay.registerSetMethod( this, &RimWellPathTarget::setTargetPointFromDisplayCoord );

    CAF_PDM_InitScriptableFieldNoDefault( &m_targetMeasuredDepth, "TargetMeasuredDepth", "MD" );
    m_targetMeasuredDepth.registerGetMethod( this, &RimWellPathTarget::measuredDepth );

    CAF_PDM_InitScriptableField( &m_dogleg1, "Dogleg1", 3.0, "DL in", "", "[deg/30m]", "" );
    CAF_PDM_InitScriptableField( &m_dogleg2, "Dogleg2", 3.0, "DL out", "", "[deg/30m]", "" );

    CAF_PDM_InitScriptableField( &m_estimatedDogleg1, "EstimatedDogleg1", 0.0, "Est DL in", "", "[deg/30m]", "" );
    m_estimatedDogleg1.uiCapability()->setUiReadOnly( true );
    CAF_PDM_InitScriptableField( &m_estimatedDogleg2, "EstimatedDogleg2", 0.0, "Est DL out", "", "[deg/30m]", "" );
    m_estimatedDogleg2.uiCapability()->setUiReadOnly( true );
    CAF_PDM_InitScriptableField( &m_estimatedAzimuth, "EstimatedAzimuth", 0.0, "Est Azi(deg)" );
    m_estimatedAzimuth.uiCapability()->setUiReadOnly( true );
    CAF_PDM_InitScriptableField( &m_estimatedInclination, "EstimatedInclination", 0.0, "Est Inc(deg)" );
    m_estimatedInclination.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitScriptableField( &m_useFixedAzimuth, "UseFixedAzimuth", false, "Azi" );
    CAF_PDM_InitScriptableField( &m_useFixedInclination, "UseFixedInclination", false, "Inc" );

    CAF_PDM_InitScriptableField( &m_azimuth, "Azimuth", 0.0, "Azi(deg)" );
    CAF_PDM_InitScriptableField( &m_inclination, "Inclination", 0.0, "Inc(deg)" );

    CAF_PDM_InitFieldNoDefault( &m_targetType_OBSOLETE, "TargetType", "Type" );
    m_targetType_OBSOLETE.uiCapability()->setUiHidden( true );
    CAF_PDM_InitField( &m_hasTangentConstraintUiField_OBSOLETE, "HasTangentConstraint", false, "Dir" );
    m_hasTangentConstraintUiField_OBSOLETE.xmlCapability()->disableIO();
    m_hasTangentConstraintUiField_OBSOLETE.uiCapability()->setUiHidden( true );
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
void RimWellPathTarget::setPointXYZ( const cvf::Vec3d& point )
{
    m_targetPointXYD = { point.x(), point.y(), -point.z() };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathTarget::setAsPointTargetXYD( const cvf::Vec3d& point )
{
    m_targetType_OBSOLETE = TargetTypeEnum::POINT;

    m_targetPointXYD = point;

    m_useFixedAzimuth     = false;
    m_useFixedInclination = false;
    m_azimuth             = 0.0;
    m_inclination         = 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathTarget::setAsPointTargetXYZ( const cvf::Vec3d& point )
{
    m_targetType_OBSOLETE = TargetTypeEnum::POINT;

    m_targetPointXYD = cvf::Vec3d( point.x(), point.y(), -point.z() );

    m_useFixedAzimuth     = false;
    m_useFixedInclination = false;
    m_azimuth             = 0.0;
    m_inclination         = 0.0;
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
    m_targetType_OBSOLETE = TargetTypeEnum::POINT_AND_TANGENT;

    m_targetPointXYD = cvf::Vec3d( point.x(), point.y(), -point.z() );

    m_useFixedAzimuth     = true;
    m_useFixedInclination = true;
    m_azimuth             = cvf::Math::toDegrees( azimuth );
    m_inclination         = cvf::Math::toDegrees( inclination );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathTarget::setDerivedTangent( double azimuth, double inclination )
{
    m_estimatedAzimuth     = cvf::Math::toDegrees( azimuth );
    m_estimatedInclination = cvf::Math::toDegrees( inclination );

    if ( !m_useFixedAzimuth ) m_azimuth = cvf::Math::toDegrees( azimuth );
    if ( !m_useFixedInclination ) m_inclination = cvf::Math::toDegrees( inclination );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathTarget::updateFrom3DManipulator( const cvf::Vec3d& pointXYD )
{
    enableFullUpdate( false );
    m_targetPointXYD.setValueWithFieldChanged( pointXYD );
    enableFullUpdate( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaLineArcWellPathCalculator::WellTarget RimWellPathTarget::wellTargetData()
{
    RiaLineArcWellPathCalculator::WellTarget targetData;

    targetData.targetPointXYZ           = targetPointXYZ();
    targetData.isAzimuthConstrained     = m_useFixedAzimuth();
    targetData.isInclinationConstrained = m_useFixedInclination();
    targetData.azimuth                  = azimuth();
    targetData.inclination              = inclination();
    targetData.radius1                  = radius1();
    targetData.radius2                  = radius2();

    return targetData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimWellPathTarget::targetPointXYZ() const
{
    cvf::Vec3d xyzPoint( m_targetPointXYD() );
    xyzPoint.z() = -xyzPoint.z();
    return xyzPoint;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPathTarget::azimuth() const
{
    if ( m_useFixedAzimuth() )
    {
        return cvf::Math::toRadians( m_azimuth );
    }

    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPathTarget::inclination() const
{
    if ( m_useFixedInclination() )
    {
        return cvf::Math::toRadians( m_inclination );
    }

    return std::numeric_limits<double>::infinity();
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
void RimWellPathTarget::setRadius1Data( bool isEditable, bool isIncorrect, double actualRadius )
{
    double estimatedDogLeg = doglegFromRadius( actualRadius );
    m_estimatedDogleg1     = estimatedDogLeg;

    if ( isIncorrect )
    {
        if ( actualRadius < radius1() )
        {
            m_dogleg1.uiCapability()->setUiContentTextColor( Qt::red );
            m_dogleg1.uiCapability()->setUiToolTip( "Actual Dogleg: " + QString::number( estimatedDogLeg ) +
                                                    "\nThe dogleg constraint is not satisfied!" );
        }
        else
        {
            m_dogleg1.uiCapability()->setUiContentTextColor( Qt::darkGreen );
            m_dogleg1.uiCapability()->setUiToolTip( "Actual Dogleg: " + QString::number( estimatedDogLeg ) );
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
void RimWellPathTarget::setRadius2Data( bool isEditable, bool isIncorrect, double actualRadius )
{
    double estimatedDogLeg = doglegFromRadius( actualRadius );
    m_estimatedDogleg2     = estimatedDogLeg;

    if ( isIncorrect )
    {
        if ( actualRadius < radius2() )
        {
            m_dogleg2.uiCapability()->setUiContentTextColor( Qt::red );
            m_dogleg2.uiCapability()->setUiToolTip( "Actual Dogleg: " + QString::number( estimatedDogLeg ) +
                                                    "\nThe dogleg constraint is not satisfied!" );
        }
        else
        {
            m_dogleg2.uiCapability()->setUiContentTextColor( Qt::darkGreen );
            m_dogleg2.uiCapability()->setUiToolTip( "Actual Dogleg: " + QString::number( estimatedDogLeg ) );
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
std::vector<caf::PdmFieldHandle*> RimWellPathTarget::fieldsFor3dManipulator()
{
    return { &m_targetPointXYD, &m_azimuth, &m_inclination };
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
    if ( field == &m_targetPointXYD )
    {
        auto uiDisplayStringAttr = dynamic_cast<caf::PdmUiLineEditorAttributeUiDisplayString*>( attribute );

        if ( uiDisplayStringAttr )
        {
            uiDisplayStringAttr->m_displayString = QString::number( m_targetPointXYD()[0], 'f', 2 ) + " " +
                                                   QString::number( m_targetPointXYD()[1], 'f', 2 ) + " " +
                                                   QString::number( m_targetPointXYD()[2], 'f', 2 );
        }
    }
    else if ( field == &m_targetPointForDisplay )
    {
        auto uiDisplayStringAttr = dynamic_cast<caf::PdmUiLineEditorAttributeUiDisplayString*>( attribute );

        if ( uiDisplayStringAttr )
        {
            uiDisplayStringAttr->m_displayString = QString::number( m_targetPointForDisplay()[0], 'f', 2 ) + " " +
                                                   QString::number( m_targetPointForDisplay()[1], 'f', 2 ) + " " +
                                                   QString::number( m_targetPointForDisplay()[2], 'f', 2 );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimWellPathTarget::targetPointForDisplayXYD() const
{
    auto geoDef = geometryDefinition();
    if ( geoDef && geoDef->showAbsoluteCoordinates() )
    {
        auto offsetXYZ = geoDef->anchorPointXyz();

        auto coordXYD = targetPointXYZ() + offsetXYZ;
        coordXYD.z()  = -coordXYD.z();

        return coordXYD;
    }

    return m_targetPointXYD();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathTarget::setTargetPointFromDisplayCoord( const cvf::Vec3d& coordInXYD )
{
    cvf::Vec3d offsetXYD = cvf::Vec3d::ZERO;

    auto geoDef = geometryDefinition();
    if ( geoDef && geoDef->showAbsoluteCoordinates() )
    {
        offsetXYD = geoDef->anchorPointXyd();
    }

    auto newCoordInXYD = coordInXYD - offsetXYD;
    m_targetPointXYD   = newCoordInXYD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPathTarget::measuredDepth() const
{
    RimWellPath* wellPath = nullptr;
    this->firstAncestorOfType( wellPath );

    auto geoDef = geometryDefinition();

    if ( geoDef && wellPath && wellPath->wellPathGeometry() )
    {
        auto offsetXYZ = geoDef->anchorPointXyz();
        auto coordXYZ  = targetPointXYZ() + offsetXYZ;

        auto wellPathGeo = wellPath->wellPathGeometry();
        return wellPathGeo->closestMeasuredDepth( coordXYZ );
    }

    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathGeometryDef* RimWellPathTarget::geometryDefinition() const
{
    RimWellPathGeometryDef* geoDef = nullptr;
    this->firstAncestorOfType( geoDef );

    return geoDef;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathTarget::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                          const QVariant&            oldValue,
                                          const QVariant&            newValue )
{
    if ( changedField == &m_hasTangentConstraintUiField_OBSOLETE )
    {
        if ( m_hasTangentConstraintUiField_OBSOLETE )
            m_targetType_OBSOLETE = TargetTypeEnum::POINT_AND_TANGENT;
        else
            m_targetType_OBSOLETE = TargetTypeEnum::POINT;
    }

    moved.send( m_isFullUpdateEnabled );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathTarget::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( m_isEnabled() )
    {
        m_targetPointXYD.uiCapability()->setUiReadOnly( false );

        m_azimuth.uiCapability()->setUiReadOnly( !m_useFixedAzimuth() );
        m_inclination.uiCapability()->setUiReadOnly( !m_useFixedInclination() );
    }
    else
    {
        m_dogleg1.uiCapability()->setUiReadOnly( true );
        m_targetPointXYD.uiCapability()->setUiReadOnly( true );
        m_azimuth.uiCapability()->setUiReadOnly( true );
        m_inclination.uiCapability()->setUiReadOnly( true );
        m_dogleg2.uiCapability()->setUiReadOnly( true );
    }

    {
        bool showAbsCoords = false;
        auto geoDef        = geometryDefinition();
        if ( geoDef && geoDef->showAbsoluteCoordinates() )
        {
            showAbsCoords = true;
        }

        m_targetPointForDisplay.uiCapability()->setUiHidden( !showAbsCoords );
    }
}
