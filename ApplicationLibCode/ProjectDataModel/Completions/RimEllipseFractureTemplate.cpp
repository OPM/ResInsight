/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RimEllipseFractureTemplate.h"

#include "RiaApplication.h"
#include "RiaCompletionTypeCalculationScheduler.h"
#include "RiaEclipseUnitTools.h"
#include "RiaFractureDefines.h"
#include "RiaLogging.h"

#include "RigCellGeometryTools.h"
#include "RigFractureCell.h"
#include "RigFractureGrid.h"
#include "RigStatisticsMath.h"
#include "RigTesselatorTools.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimFracture.h"
#include "RimFractureContainment.h"
#include "RimFractureTemplate.h"
#include "RimProject.h"
#include "RimStimPlanColors.h"

#include "cafPdmObject.h"

#include "cvfGeometryTools.h"
#include "cvfVector3.h"

CAF_PDM_SOURCE_INIT( RimEllipseFractureTemplate, "RimEllipseFractureTemplate" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEllipseFractureTemplate::RimEllipseFractureTemplate()
{
    CAF_PDM_InitObject( "Fracture Template", ":/FractureTemplate16x16.png", "", "" );

    CAF_PDM_InitField( &m_halfLength, "HalfLength", 0.0, "Half Length X<sub>f</sub>", "", "", "" );
    CAF_PDM_InitField( &m_height, "Height", 0.0, "Height", "", "", "" );
    CAF_PDM_InitField( &m_width, "Width", 0.0, "Width", "", "", "" );
    CAF_PDM_InitField( &m_permeability, "Permeability", 0.0, "Permeability [mD]", "", "", "" );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEllipseFractureTemplate::~RimEllipseFractureTemplate()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEllipseFractureTemplate::loadDataAndUpdate()
{
    for ( RimFracture* fracture : fracturesUsingThisTemplate() )
    {
        fracture->updateFractureGrid();
    }

    // TODO: is this necessary? Strange responsibility for a fracture template.
    RimEclipseView* activeView = dynamic_cast<RimEclipseView*>( RiaApplication::instance()->activeReservoirView() );
    if ( activeView ) activeView->loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEllipseFractureTemplate::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                   const QVariant&            oldValue,
                                                   const QVariant&            newValue )
{
    RimFractureTemplate::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_halfLength || changedField == &m_height || changedField == &m_width ||
         changedField == &m_permeability || changedField == &m_scaleApplyButton )
    {
        m_scaleApplyButton = false;

        // Changes to one of these parameters should change all fractures with this fracture template attached.
        onLoadDataAndUpdateGeometryHasChanged();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEllipseFractureTemplate::fractureTriangleGeometry( std::vector<cvf::Vec3f>* nodeCoords,
                                                           std::vector<cvf::uint>*  triangleIndices,
                                                           double                   wellPathDepthAtFracture ) const
{
    RigEllipsisTesselator tesselator( 20 );

    float a = m_halfLength * m_halfLengthScaleFactor;
    float b = m_height / 2.0f * m_heightScaleFactor;

    tesselator.tesselateEllipsis( a, b, triangleIndices, nodeCoords );

    for ( cvf::Vec3f& v : *nodeCoords )
    {
        // Y is depth in fracture coordinate system
        v.y() += computeHeightOffset( wellPathDepthAtFracture );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3f> RimEllipseFractureTemplate::fractureBorderPolygon( double wellPathDepthAtFracture ) const
{
    std::vector<cvf::Vec3f> polygon;

    std::vector<cvf::Vec3f> nodeCoords;
    std::vector<cvf::uint>  triangleIndices;

    fractureTriangleGeometry( &nodeCoords, &triangleIndices, wellPathDepthAtFracture );

    for ( size_t i = 1; i < nodeCoords.size(); i++ )
    {
        polygon.push_back( nodeCoords[i] );
    }

    return polygon;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEllipseFractureTemplate::changeUnits()
{
    if ( fractureTemplateUnit() == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
    {
        convertToUnitSystem( RiaDefines::EclipseUnitSystem::UNITS_FIELD );
    }
    else if ( fractureTemplateUnit() == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
    {
        convertToUnitSystem( RiaDefines::EclipseUnitSystem::UNITS_METRIC );
    }

    this->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::cref<RigFractureGrid> RimEllipseFractureTemplate::createFractureGrid( double wellPathDepthAtFracture ) const
{
    cvf::ref<RigFractureGrid>    fractureGrid = new RigFractureGrid();
    std::vector<RigFractureCell> fractureCells;

    int numberOfCellsI = 35;
    int numberOfCellsJ = 35;

    double height     = m_height * m_heightScaleFactor;
    double halfLength = m_halfLength * m_halfLengthScaleFactor;

    double cellSizeX = ( halfLength * 2 ) / numberOfCellsI * m_halfLengthScaleFactor;
    double cellSizeZ = height / numberOfCellsJ * m_heightScaleFactor;

    double cellArea                     = cellSizeX * cellSizeZ;
    double areaTresholdForIncludingCell = 0.5 * cellArea;

    std::pair<size_t, size_t> wellCenterFractureCellIJ = std::make_pair( numberOfCellsI / 2, numberOfCellsJ / 2 );

    for ( int i = 0; i < numberOfCellsI; i++ )
    {
        for ( int j = 0; j < numberOfCellsJ; j++ )
        {
            double X1 = -halfLength + i * cellSizeX;
            double X2 = -halfLength + ( i + 1 ) * cellSizeX;
            double Y1 = -height / 2 + j * cellSizeZ + computeHeightOffset( wellPathDepthAtFracture );
            double Y2 = -height / 2 + ( j + 1 ) * cellSizeZ + computeHeightOffset( wellPathDepthAtFracture );

            std::vector<cvf::Vec3d> cellPolygon;
            cellPolygon.push_back( cvf::Vec3d( X1, Y1, 0.0 ) );
            cellPolygon.push_back( cvf::Vec3d( X2, Y1, 0.0 ) );
            cellPolygon.push_back( cvf::Vec3d( X2, Y2, 0.0 ) );
            cellPolygon.push_back( cvf::Vec3d( X1, Y2, 0.0 ) );

            double cond = conductivity();

            std::vector<cvf::Vec3f> ellipseFracPolygon = fractureBorderPolygon( wellPathDepthAtFracture );
            std::vector<cvf::Vec3d> ellipseFracPolygonDouble;
            for ( const auto& v : ellipseFracPolygon )
                ellipseFracPolygonDouble.push_back( static_cast<cvf::Vec3d>( v ) );
            std::vector<std::vector<cvf::Vec3d>> clippedFracturePolygons =
                RigCellGeometryTools::intersectionWithPolygon( cellPolygon, ellipseFracPolygonDouble );
            if ( !clippedFracturePolygons.empty() )
            {
                for ( const auto& clippedFracturePolygon : clippedFracturePolygons )
                {
                    double areaCutPolygon = cvf::GeometryTools::polygonAreaNormal3D( clippedFracturePolygon ).length();
                    if ( areaCutPolygon < areaTresholdForIncludingCell )
                    {
                        cond = 0.0; // Cell is excluded from calculation, cond is set to zero. Must be included for
                                    // indexing to be correct
                    }
                }
            }
            else
                cond = 0.0;

            RigFractureCell fractureCell( cellPolygon, i, j );
            fractureCell.setConductivityValue( cond );
            fractureCells.push_back( fractureCell );

            // The well path is intersecting the fracture at origo in the fracture coordinate system
            // Find the fracture cell where the well path is intersecting
            if ( ( cellPolygon[0].x() <= 0.0 && cellPolygon[1].x() >= 0.0 ) &&
                 ( cellPolygon[1].y() <= 0.0 && cellPolygon[2].y() >= 0.0 ) )
            {
                wellCenterFractureCellIJ = std::make_pair( fractureCell.getI(), fractureCell.getJ() );
                RiaLogging::debug(
                    QString( "Setting wellCenterStimPlanCell at cell %1, %2" )
                        .arg( QString::number( fractureCell.getI() ), QString::number( fractureCell.getJ() ) ) );
            }
        }
    }

    fractureGrid->setFractureCells( fractureCells );

    // Set well intersection to center of ellipse
    fractureGrid->setWellCenterFractureCellIJ( wellCenterFractureCellIJ );

    fractureGrid->setICellCount( numberOfCellsI );
    fractureGrid->setJCellCount( numberOfCellsJ );
    return cvf::cref<RigFractureGrid>( fractureGrid.p() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
WellFractureIntersectionData
    RimEllipseFractureTemplate::wellFractureIntersectionData( const RimFracture* fractureInstance ) const
{
    WellFractureIntersectionData values;
    values.m_width        = m_width;
    values.m_permeability = m_permeability;

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimEllipseFractureTemplate::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_fractureWidthType )
    {
        options.push_back(
            caf::PdmOptionItemInfo( caf::AppEnum<WidthEnum>::uiText( USER_DEFINED_WIDTH ), USER_DEFINED_WIDTH ) );
        options.push_back(
            caf::PdmOptionItemInfo( caf::AppEnum<WidthEnum>::uiText( WIDTH_FROM_FRACTURE ), WIDTH_FROM_FRACTURE ) );
    }

    if ( fieldNeedingOptions == &m_betaFactorType )
    {
        options.push_back( caf::PdmOptionItemInfo( caf::AppEnum<BetaFactorEnum>::uiText( USER_DEFINED_BETA_FACTOR ),
                                                   USER_DEFINED_BETA_FACTOR ) );

        if ( isBetaFactorAvailableOnFile() )
        {
            options.push_back( caf::PdmOptionItemInfo( caf::AppEnum<BetaFactorEnum>::uiText( BETA_FACTOR_FROM_FRACTURE ),
                                                       BETA_FACTOR_FROM_FRACTURE ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEllipseFractureTemplate::setDefaultValuesFromUnit()
{
    if ( fractureTemplateUnit() == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
    {
        m_width        = 0.5;
        m_permeability = 80000.0;
        m_halfLength   = 300.0;
        m_height       = 225.0;
    }
    else
    {
        m_width        = 0.01;
        m_permeability = 100000.0;
        m_halfLength   = 100.0;
        m_height       = 75.0;
    }

    // Default to 1/3 of height
    m_wellPathDepthAtFracture = m_height / 3.0;

    this->setDefaultWellDiameterFromUnit();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimEllipseFractureTemplate::conductivity() const
{
    double cond = cvf::UNDEFINED_DOUBLE;
    if ( fractureTemplateUnit() == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
    {
        // Conductivity should be md-m, width is in m
        cond = m_permeability * m_width;
    }
    else if ( fractureTemplateUnit() == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
    {
        // Conductivity should be md-ft, but width is in inches
        cond = m_permeability * RiaEclipseUnitTools::inchToFeet( m_width );
    }

    return m_conductivityScaleFactor * cond;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimEllipseFractureTemplate::halfLength() const
{
    return m_halfLength;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimEllipseFractureTemplate::height() const
{
    return m_height;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimEllipseFractureTemplate::width() const
{
    return m_width;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEllipseFractureTemplate::appendDataToResultStatistics( const QString&     uiResultName,
                                                               const QString&     unit,
                                                               MinMaxAccumulator& minMaxAccumulator,
                                                               PosNegAccumulator& posNegAccumulator ) const
{
    if ( uiResultName == RiaDefines::conductivityResultName() )
    {
        minMaxAccumulator.addValue( conductivity() );
        posNegAccumulator.addValue( conductivity() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<QString, QString>> RimEllipseFractureTemplate::uiResultNamesWithUnit() const
{
    std::vector<std::pair<QString, QString>> propertyNamesAndUnits;

    QString condUnit = RiaDefines::unitStringConductivity( fractureTemplateUnit() );
    propertyNamesAndUnits.push_back( std::make_pair( RiaDefines::conductivityResultName(), condUnit ) );

    return propertyNamesAndUnits;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEllipseFractureTemplate::onLoadDataAndUpdateGeometryHasChanged()
{
    loadDataAndUpdate();

    RimEclipseCase* eclipseCase = nullptr;
    this->firstAncestorOrThisOfType( eclipseCase );
    if ( eclipseCase )
    {
        RiaCompletionTypeCalculationScheduler::instance()->scheduleRecalculateCompletionTypeAndRedrawAllViews( eclipseCase );
    }
    else
    {
        RiaCompletionTypeCalculationScheduler::instance()->scheduleRecalculateCompletionTypeAndRedrawAllViews();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEllipseFractureTemplate::convertToUnitSystem( RiaDefines::EclipseUnitSystem neededUnit )
{
    if ( m_fractureTemplateUnit() == neededUnit ) return;

    setUnitSystem( neededUnit );
    RimFractureTemplate::convertToUnitSystem( neededUnit );

    if ( neededUnit == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
    {
        m_halfLength = RiaEclipseUnitTools::meterToFeet( m_halfLength );
        m_height     = RiaEclipseUnitTools::meterToFeet( m_height );
        m_width      = RiaEclipseUnitTools::meterToInch( m_width );

        // Convert here instead of base class to avoid interfering with StimPlan template
        // which handles units differently for this property.
        m_wellPathDepthAtFracture = RiaEclipseUnitTools::meterToFeet( m_wellPathDepthAtFracture );
    }
    else if ( neededUnit == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
    {
        m_halfLength = RiaEclipseUnitTools::feetToMeter( m_halfLength );
        m_height     = RiaEclipseUnitTools::feetToMeter( m_height );
        m_width      = RiaEclipseUnitTools::inchToMeter( m_width );

        m_wellPathDepthAtFracture = RiaEclipseUnitTools::feetToMeter( m_wellPathDepthAtFracture );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEllipseFractureTemplate::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( fractureTemplateUnit() == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
    {
        m_halfLength.uiCapability()->setUiName( "Halflength X<sub>f</sub> [m]" );
        m_height.uiCapability()->setUiName( "Height [m]" );
        m_width.uiCapability()->setUiName( "Width [m]" );
    }
    else if ( fractureTemplateUnit() == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
    {
        m_halfLength.uiCapability()->setUiName( "Halflength X<sub>f</sub> [ft]" );
        m_height.uiCapability()->setUiName( "Height [ft]" );
        m_width.uiCapability()->setUiName( "Width [inches]" );
    }

    if ( conductivityType() == FINITE_CONDUCTIVITY )
    {
        m_permeability.uiCapability()->setUiHidden( false );
        m_width.uiCapability()->setUiHidden( false );
    }
    else if ( conductivityType() == INFINITE_CONDUCTIVITY )
    {
        m_permeability.uiCapability()->setUiHidden( true );
        m_width.uiCapability()->setUiHidden( true );
    }

    uiOrdering.add( &m_name );
    uiOrdering.add( &m_id );

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Geometry" );
        group->add( &m_halfLength );
        group->add( &m_height );
        group->add( &m_orientationType );
        group->add( &m_azimuthAngle );
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Fracture Truncation" );
        group->setCollapsedByDefault( true );

        m_fractureContainment()->uiOrdering( uiConfigName, *group );
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Properties" );
        group->add( &m_conductivityType );
        group->add( &m_permeability );
        group->add( &m_width );
        group->add( &m_skinFactor );
        group->add( &m_perforationLength );
        group->add( &m_perforationEfficiency );
        group->add( &m_wellDiameter );
    }

    uiOrdering.add( &m_wellPathDepthAtFracture );

    RimFractureTemplate::defineUiOrdering( uiConfigName, uiOrdering );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RimEllipseFractureTemplate::wellPathDepthAtFractureRange() const
{
    double scaledHeight = height() * m_heightScaleFactor;
    return std::make_pair( 0.0, scaledHeight );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimEllipseFractureTemplate::computeHeightOffset( double wellPathDepthAtFractureRange ) const
{
    return ( height() * m_heightScaleFactor / 2 ) - wellPathDepthAtFractureRange;
}
