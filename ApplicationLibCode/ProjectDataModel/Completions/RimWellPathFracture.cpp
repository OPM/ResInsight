/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-2018 Statoil ASA
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

#include "RimWellPathFracture.h"

#include "RigWellPath.h"

#include "RimEllipseFractureTemplate.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathFractureCollection.h"

#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiDoubleSliderEditor.h"

CAF_PDM_SOURCE_INIT( RimWellPathFracture, "WellPathFracture" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathFracture::RimWellPathFracture()
{
    CAF_PDM_InitScriptableObject( "Fracture", ":/FractureSymbol16x16.png" );

    CAF_PDM_InitField( &m_measuredDepth, "MeasuredDepth", 0.0f, "Measured Depth Location" );
    m_measuredDepth.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathFracture::~RimWellPathFracture()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPathFracture::fractureMD() const
{
    return m_measuredDepth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathFracture::setMeasuredDepth( double mdValue )
{
    m_measuredDepth = mdValue;

    updatePositionFromMeasuredDepth();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathFracture::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimFracture::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_measuredDepth || changedField == &m_wellPathDepthAtFracture )
    {
        updatePositionFromMeasuredDepth();
        updateAzimuthBasedOnWellAzimuthAngle();

        RimProject::current()->reloadCompletionTypeResultsInAllViews();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathFracture::updateAzimuthBasedOnWellAzimuthAngle()
{
    if ( !fractureTemplate() ) return;

    if ( fractureTemplate()->orientationType() == RimFractureTemplate::ALONG_WELL_PATH ||
         fractureTemplate()->orientationType() == RimFractureTemplate::TRANSVERSE_WELL_PATH )
    {
        double wellPathAzimuth = wellAzimuthAtFracturePosition();

        if ( fractureTemplate()->orientationType() == RimFractureTemplate::ALONG_WELL_PATH )
        {
            m_azimuth = wellPathAzimuth;
        }
        if ( fractureTemplate()->orientationType() == RimFractureTemplate::TRANSVERSE_WELL_PATH )
        {
            if ( wellPathAzimuth + 90 < 360 )
                m_azimuth = wellPathAzimuth + 90;
            else
                m_azimuth = wellPathAzimuth - 90;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPathFracture::wellAzimuthAtFracturePosition() const
{
    auto wellPath = firstAncestorOrThisOfType<RimWellPath>();
    if ( !wellPath ) return cvf::UNDEFINED_DOUBLE;

    double wellPathAzimuth = 0.0;

    RigWellPath* wellPathGeometry = wellPath->wellPathGeometry();
    if ( wellPathGeometry )
    {
        wellPathAzimuth = wellPathGeometry->wellPathAzimuthAngle( fracturePosition() );
    }

    if ( wellPathAzimuth < 0 ) wellPathAzimuth += 360;

    return wellPathAzimuth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimWellPathFracture::computeFractureDirectionNormal() const
{
    auto wellPath = firstAncestorOrThisOfType<RimWellPath>();
    if ( !wellPath ) return cvf::Vec3d::UNDEFINED;

    RigWellPath* wellPathGeometry = wellPath->wellPathGeometry();

    // Find the well path points closest to the anchor position
    cvf::Vec3d p1;
    cvf::Vec3d p2;
    wellPathGeometry->twoClosestPoints( fracturePosition(), &p1, &p2 );

    // Create a well direction based on the two points
    cvf::Vec3d wellDirection = ( p2 - p1 ).getNormalized();

    cvf::Vec3d fractureDirectionNormal = wellDirection;
    if ( fractureTemplate()->orientationType() == RimFractureTemplate::ALONG_WELL_PATH )
    {
        cvf::Mat3d azimuthRotation = cvf::Mat3d::fromRotation( cvf::Vec3d::Z_AXIS, cvf::Math::toRadians( 90.0 ) );
        fractureDirectionNormal.transformVector( azimuthRotation );
    }
    else if ( fractureTemplate()->orientationType() == RimFractureTemplate::AZIMUTH )
    {
        // Azimuth angle of fracture is relative to north.
        double     wellAzimuth     = wellPathGeometry->wellPathAzimuthAngle( fracturePosition() );
        cvf::Mat3d azimuthRotation = cvf::Mat3d::fromRotation( cvf::Vec3d::Z_AXIS, cvf::Math::toRadians( wellAzimuth - m_azimuth - 90.0 ) );
        fractureDirectionNormal.transformVector( azimuthRotation );
    }

    return fractureDirectionNormal;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathFracture::loadDataAndUpdate()
{
    updatePositionFromMeasuredDepth();
    updateAzimuthBasedOnWellAzimuthAngle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> RimWellPathFracture::perforationLengthCenterLineCoords() const
{
    std::vector<cvf::Vec3d> wellPathCoords;

    auto wellPath = firstAncestorOrThisOfType<RimWellPath>();
    if ( wellPath && wellPath->wellPathGeometry() )
    {
        double startMd = m_measuredDepth - perforationLength() / 2.0;
        double endMd   = m_measuredDepth + perforationLength() / 2.0;

        auto coordsAndMd = wellPath->wellPathGeometry()->clippedPointSubset( startMd, endMd );

        wellPathCoords = coordsAndMd.first;
    }

    return wellPathCoords;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPathFracture::compareByWellPathNameAndMD( const RimWellPathFracture* lhs, const RimWellPathFracture* rhs )
{
    CVF_TIGHT_ASSERT( lhs && rhs );

    RimWellPath* lhsWellPath = lhs->firstAncestorOrThisOfType<RimWellPath>();
    RimWellPath* rhsWellPath = rhs->firstAncestorOrThisOfType<RimWellPath>();

    if ( lhsWellPath && rhsWellPath )
    {
        if ( lhsWellPath->name() != rhsWellPath->name() )
        {
            return lhsWellPath->name() < rhsWellPath->name();
        }
    }

    return lhs->fractureMD() < rhs->fractureMD();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPathFracture::isEnabled() const
{
    auto fractureCollection = firstAncestorOrThisOfTypeAsserted<RimWellPathFractureCollection>();

    return fractureCollection->isChecked() && isChecked();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathFracture::updatePositionFromMeasuredDepth()
{
    cvf::Vec3d positionAlongWellpath = cvf::Vec3d::ZERO;

    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>( this );
    if ( !objHandle ) return;

    auto wellPath = objHandle->firstAncestorOrThisOfType<RimWellPath>();
    if ( !wellPath ) return;

    RigWellPath* wellPathGeometry = wellPath->wellPathGeometry();
    if ( wellPathGeometry )
    {
        positionAlongWellpath = wellPathGeometry->interpolatedPointAlongWellPath( m_measuredDepth() );
    }

    setAnchorPosition( positionAlongWellpath );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathFracture::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimFracture::defineUiOrdering( uiConfigName, uiOrdering );

    if ( m_fractureTemplate() )
    {
        uiOrdering.add( nameField(), { .newRow = true, .totalColumnSpan = 3, .leftLabelColumnSpan = 1 } );
        uiOrdering.add( &m_fractureTemplate, { .totalColumnSpan = 2, .leftLabelColumnSpan = 1 } );
        uiOrdering.add( &m_editFractureTemplate, { .newRow = false, .totalColumnSpan = 1, .leftLabelColumnSpan = 0 } );
    }
    else
    {
        uiOrdering.add( nameField() );
        {
            if ( RimProject::current()->allFractureTemplates().empty() )
            {
                uiOrdering.add( &m_createEllipseFractureTemplate );
                uiOrdering.addRowAppend( &m_createStimPlanFractureTemplate );
            }
            else
            {
                uiOrdering.add( &m_fractureTemplate );
            }
        }
    }

    caf::PdmUiGroup* locationGroup = uiOrdering.addNewGroup( "Location / Orientation" );
    locationGroup->add( &m_measuredDepth );
    locationGroup->add( &m_azimuth );
    locationGroup->add( &m_uiWellPathAzimuth );
    locationGroup->add( &m_uiWellFractureAzimuthDiff );
    locationGroup->add( &m_wellFractureAzimuthAngleWarning );
    locationGroup->add( &m_dip );
    locationGroup->add( &m_tilt );

    caf::PdmUiGroup* propertyGroup = uiOrdering.addNewGroup( "Properties" );
    propertyGroup->add( &m_fractureUnit );
    propertyGroup->add( &m_stimPlanTimeIndexToPlot );
    propertyGroup->add( &m_perforationLength );
    propertyGroup->add( &m_perforationEfficiency );
    propertyGroup->add( &m_wellDiameter );

    caf::PdmUiGroup* fractureCenterGroup = uiOrdering.addNewGroup( "Fracture Center Info" );
    fractureCenterGroup->add( &m_uiAnchorPosition );

    uiOrdering.add( &m_autoUpdateWellPathDepthAtFractureFromTemplate );
    uiOrdering.add( &m_wellPathDepthAtFracture );
    m_wellPathDepthAtFracture.uiCapability()->setUiReadOnly( m_autoUpdateWellPathDepthAtFractureFromTemplate() );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathFracture::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    RimFracture::defineEditorAttribute( field, uiConfigName, attribute );

    if ( field == &m_measuredDepth )
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute );

        if ( myAttr )
        {
            auto wellPath = firstAncestorOrThisOfType<RimWellPath>();
            if ( !wellPath ) return;

            myAttr->m_minimum = wellPath->uniqueStartMD();
            myAttr->m_maximum = wellPath->uniqueEndMD();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathFracture::triangleGeometry( std::vector<cvf::Vec3f>* nodeCoords, std::vector<cvf::uint>* triangleIndices ) const
{
    m_fractureTemplate->fractureTriangleGeometry( nodeCoords, triangleIndices, m_wellPathDepthAtFracture );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathFracture::applyOffset( double offsetMD )
{
    m_measuredDepth = m_measuredDepth + offsetMD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathFracture::placeUsingTemplateData()
{
    if ( m_fractureTemplate )
    {
        m_fractureTemplate->placeFractureUsingTemplateData( this );
    }
}
