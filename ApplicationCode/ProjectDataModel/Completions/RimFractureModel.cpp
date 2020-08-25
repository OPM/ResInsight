/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RimFractureModel.h"

#include "RiaColorTables.h"
#include "RiaCompletionTypeCalculationScheduler.h"
#include "RiaEclipseUnitTools.h"
#include "RiaFractureDefines.h"
#include "RiaFractureModelDefines.h"
#include "RiaLogging.h"

#include "Riu3DMainWindowTools.h"

#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigWellPath.h"

#include "Rim3dView.h"
#include "RimColorLegend.h"
#include "RimColorLegendCollection.h"
#include "RimColorLegendItem.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimElasticProperties.h"
#include "RimEllipseFractureTemplate.h"
#include "RimFractureModelPlot.h"
#include "RimModeledWellPath.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimStimPlanColors.h"
#include "RimStimPlanFractureTemplate.h"
#include "RimWellPath.h"
#include "RimWellPathGeometryDef.h"
#include "RimWellPathTarget.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapabilityCvfVec3d.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiToolButtonEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include "cvfBoundingBox.h"
#include "cvfGeometryTools.h"
#include "cvfMath.h"
#include "cvfMatrix4.h"
#include "cvfPlane.h"

#include <cmath>

CAF_PDM_SOURCE_INIT( RimFractureModel, "RimFractureModel" );

namespace caf
{
template <>
void caf::AppEnum<RimFractureModel::ExtractionType>::setUp()
{
    addItem( RimFractureModel::ExtractionType::TRUE_VERTICAL_THICKNESS, "TVT", "True Vertical Thickness" );
    addItem( RimFractureModel::ExtractionType::TRUE_STRATIGRAPHIC_THICKNESS, "TST", "True Stratigraphic Thickness" );

    setDefault( RimFractureModel::ExtractionType::TRUE_VERTICAL_THICKNESS );
}
}; // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModel::RimFractureModel()
{
    CAF_PDM_InitScriptableObject( "FractureModel", "", "", "" );

    CAF_PDM_InitScriptableField( &m_MD, "MD", 0.0, "MD", "", "", "" );

    CAF_PDM_InitScriptableField( &m_extractionType,
                                 "ExtractionType",
                                 caf::AppEnum<ExtractionType>( ExtractionType::TRUE_STRATIGRAPHIC_THICKNESS ),
                                 "Extraction Type",
                                 "",
                                 "",
                                 "" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_anchorPosition, "AnchorPosition", "Anchor Position", "", "", "" );
    m_anchorPosition.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitScriptableFieldNoDefault( &m_thicknessDirection, "ThicknessDirection", "Thickness Direction", "", "", "" );
    m_thicknessDirection.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitScriptableFieldNoDefault( &m_thicknessDirectionWellPath,
                                          "ThicknessDirectionWellPath",
                                          "Thickness Direction Well Path",
                                          "",
                                          "",
                                          "" );

    CAF_PDM_InitScriptableField( &m_boundingBoxHorizontal, "BoundingBoxHorizontal", 50.0, "Bounding Box Horizontal", "", "", "" );
    CAF_PDM_InitScriptableField( &m_boundingBoxVertical, "BoundingBoxVertical", 100.0, "Bounding Box Vertical", "", "", "" );

    CAF_PDM_InitScriptableField( &m_defaultPorosity, "DefaultPorosity", 0.0, "Default Porosity", "", "", "" );
    CAF_PDM_InitScriptableField( &m_defaultPermeability, "DefaultPermeability", 10.0e-6, "Default Permeability", "", "", "" );

    // Stress unit: bar
    // Stress gradient unit: bar/m
    // Depth is meter
    CAF_PDM_InitScriptableField( &m_verticalStress, "VerticalStress", 879.0, "Vertical Stress", "", "", "" );
    CAF_PDM_InitScriptableField( &m_verticalStressGradient,
                                 "VerticalStressGradient",
                                 0.238,
                                 "Vertical Stress Gradient",
                                 "",
                                 "",
                                 "" );
    CAF_PDM_InitScriptableField( &m_stressDepth, "StressDepth", 1000.0, "Stress Depth", "", "", "" );

    CAF_PDM_InitScriptableField( &m_overburdenHeight, "OverburdenHeight", 50.0, "Overburden Height", "", "", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_overburdenFormation, "OverburdenFormation", "Overburden Formation", "", "", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_overburdenFacies, "OverburdenFacies", "Overburden Facies", "", "", "" );
    CAF_PDM_InitScriptableField( &m_overburdenPorosity, "OverburdenPorosity", 0.0, "Overburden Porosity", "", "", "" );
    CAF_PDM_InitScriptableField( &m_overburdenPermeability,
                                 "OverburdenPermeability",
                                 10.0e-6,
                                 "Overburden Permeability",
                                 "",
                                 "",
                                 "" );
    CAF_PDM_InitScriptableField( &m_overburdenFluidDensity,
                                 "OverburdenFluidDensity",
                                 1.03,
                                 "Overburden Fluid Density [g/cm^3]",
                                 "",
                                 "",
                                 "" );

    CAF_PDM_InitScriptableField( &m_underburdenHeight, "UnderburdenHeight", 50.0, "Underburden Height", "", "", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_underburdenFormation, "UnderburdenFormation", "Underburden Formation", "", "", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_underburdenFacies, "UnderburdenFacies", "Underburden Facies", "", "", "" );
    CAF_PDM_InitScriptableField( &m_underburdenPorosity, "UnderburdenPorosity", 0.0, "Underburden Porosity", "", "", "" );
    CAF_PDM_InitScriptableField( &m_underburdenPermeability,
                                 "UnderburdenPermeability",
                                 10.0e-6,
                                 "Underburden Permeability",
                                 "",
                                 "",
                                 "" );
    CAF_PDM_InitScriptableField( &m_underburdenFluidDensity,
                                 "UnderburdenFluidDensity",
                                 1.03,
                                 "Underburden Fluid Density [g/cm^3]",
                                 "",
                                 "",
                                 "" );

    CAF_PDM_InitScriptableField( &m_referenceTemperature, "ReferenceTemperature", 20.0, "Temperature [C]", "", "", "" );
    CAF_PDM_InitScriptableField( &m_referenceTemperatureGradient,
                                 "ReferenceTemperatureGradient",
                                 0.025,
                                 "Temperature Gradient [C/m]",
                                 "",
                                 "",
                                 "" );
    CAF_PDM_InitScriptableField( &m_referenceTemperatureDepth,
                                 "ReferenceTemperatureDepth",
                                 1000.0,
                                 "Temperature Depth [m]",
                                 "",
                                 "",
                                 "" );

    CAF_PDM_InitScriptableField( &m_useDetailedFluidLoss, "UseDetailedFluidLoss", true, "Use Detailed Fluid Loss", "", "", "" );

    CAF_PDM_InitScriptableField( &m_relativePermeabilityFactorDefault,
                                 "RelativePermeabilityFactor",
                                 0.5,
                                 "Relative Permeability Factor",
                                 "",
                                 "",
                                 "" );
    CAF_PDM_InitScriptableField( &m_poroElasticConstantDefault, "PoroElasticConstant", 0.0, "Poro-Elastic Constant", "", "", "" );
    CAF_PDM_InitScriptableField( &m_thermalExpansionCoeffientDefault,
                                 "ThermalExpansionCoeffisient",
                                 0.0,
                                 "Thermal Expansion Coeffisient",
                                 "",
                                 "",
                                 "" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_elasticProperties, "ElasticProperties", "Elastic Properties", "", "", "" );
    m_elasticProperties.uiCapability()->setUiHidden( true );
    m_elasticProperties.uiCapability()->setUiTreeHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModel::~RimFractureModel()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFractureModel::isEnabled() const
{
    return isChecked();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFractureModel::useDetailedFluidLoss() const
{
    return m_useDetailedFluidLoss();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModel::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                         const QVariant&            oldValue,
                                         const QVariant&            newValue )
{
    if ( changedField == &m_MD )
    {
        updatePositionFromMeasuredDepth();
    }

    if ( changedField == &m_MD || changedField == &m_extractionType || changedField == &m_boundingBoxVertical ||
         changedField == &m_boundingBoxHorizontal )
    {
        updateThicknessDirection();
    }

    if ( changedField == &m_extractionType || changedField == &m_thicknessDirectionWellPath )
    {
        updateThicknessDirectionWellPathName();
        m_thicknessDirectionWellPath()->updateConnectedEditors();
    }

    {
        RimEclipseCase* eclipseCase = nullptr;
        this->firstAncestorOrThisOfType( eclipseCase );
        if ( eclipseCase )
        {
            RiaCompletionTypeCalculationScheduler::instance()->scheduleRecalculateCompletionTypeAndRedrawAllViews(
                eclipseCase );
        }
        else
        {
            RiaCompletionTypeCalculationScheduler::instance()->scheduleRecalculateCompletionTypeAndRedrawAllViews();
        }

        RimProject::current()->scheduleCreateDisplayModelAndRedrawAllViews();

        updateReferringPlots();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimFractureModel::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                       bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_overburdenFormation || fieldNeedingOptions == &m_underburdenFormation )
    {
        // Find an eclipse case
        RimProject* proj = RimProject::current();
        if ( proj->eclipseCases().empty() ) return options;

        RimEclipseCase* eclipseCase = proj->eclipseCases()[0];
        if ( !eclipseCase ) return options;

        RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();
        if ( !eclipseCaseData ) return options;

        std::vector<QString> formationNames = eclipseCase->eclipseCaseData()->formationNames();
        for ( const QString& formationName : formationNames )
        {
            options.push_back( caf::PdmOptionItemInfo( formationName, formationName ) );
        }
    }
    else if ( fieldNeedingOptions == &m_overburdenFacies || fieldNeedingOptions == &m_underburdenFacies )
    {
        RimColorLegend* faciesColors =
            RimProject::current()->colorLegendCollection()->findByName( RiaDefines::faciesColorLegendName() );
        if ( faciesColors )
        {
            for ( RimColorLegendItem* item : faciesColors->colorLegendItems() )
            {
                options.push_back( caf::PdmOptionItemInfo( item->categoryName(), item->categoryName() ) );
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimFractureModel::fracturePosition() const
{
    return m_anchorPosition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::WellPathComponentType RimFractureModel::componentType() const
{
    return RiaDefines::WellPathComponentType::FRACTURE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFractureModel::componentLabel() const
{
    return name();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFractureModel::componentTypeLabel() const
{
    return "Fracture Model";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimFractureModel::defaultComponentColor() const
{
    return cvf::Color3f::RED; // RiaColorTables::wellPathComponentColors()[componentType()];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::startMD() const
{
    return m_MD();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::endMD() const
{
    return m_MD();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimFractureModel::anchorPosition() const
{
    return m_anchorPosition();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimFractureModel::thicknessDirection() const
{
    return m_thicknessDirection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModel::updatePositionFromMeasuredDepth()
{
    cvf::Vec3d positionAlongWellpath = cvf::Vec3d::ZERO;

    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>( this );
    if ( !objHandle ) return;

    RimWellPath* wellPath = nullptr;
    objHandle->firstAncestorOrThisOfType( wellPath );
    if ( !wellPath ) return;

    RigWellPath* wellPathGeometry = wellPath->wellPathGeometry();
    if ( wellPathGeometry )
    {
        positionAlongWellpath = wellPathGeometry->interpolatedPointAlongWellPath( m_MD() );
    }

    m_anchorPosition = positionAlongWellpath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModel::updateThicknessDirection()
{
    // True vertical thickness: just point straight up
    cvf::Vec3d direction( 0.0, 0.0, -1.0 );

    if ( m_extractionType() == ExtractionType::TRUE_STRATIGRAPHIC_THICKNESS )
    {
        direction = calculateTSTDirection();
    }

    m_thicknessDirection = direction;

    if ( m_thicknessDirectionWellPath )
    {
        cvf::Vec3d topPosition;
        cvf::Vec3d bottomPosition;

        findThicknessTargetPoints( topPosition, bottomPosition );
        topPosition.z() *= -1.0;
        bottomPosition.z() *= -1.0;

        RimWellPathGeometryDef* wellGeomDef = m_thicknessDirectionWellPath->geometryDefinition();
        wellGeomDef->deleteAllTargets();

        RimWellPathTarget* topPathTarget = new RimWellPathTarget();

        topPathTarget->setAsPointTargetXYD( topPosition );
        RimWellPathTarget* bottomPathTarget = new RimWellPathTarget();
        bottomPathTarget->setAsPointTargetXYD( bottomPosition );

        wellGeomDef->insertTarget( nullptr, topPathTarget );
        wellGeomDef->insertTarget( nullptr, bottomPathTarget );

        wellGeomDef->updateConnectedEditors();
        wellGeomDef->updateWellPathVisualization();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimFractureModel::calculateTSTDirection() const
{
    cvf::Vec3d defaultDirection = cvf::Vec3d( 0.0, 0.0, -1.0 );

    // TODO: find a better way?
    // Find an eclipse case
    RimProject* proj = RimProject::current();
    if ( proj->eclipseCases().empty() ) return defaultDirection;

    RimEclipseCase* eclipseCase = proj->eclipseCases()[0];
    if ( !eclipseCase ) return defaultDirection;

    RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();
    if ( !eclipseCaseData ) return defaultDirection;

    RigMainGrid* mainGrid = eclipseCaseData->mainGrid();
    if ( !mainGrid ) return defaultDirection;

    cvf::Vec3d boundingBoxSize( m_boundingBoxHorizontal, m_boundingBoxHorizontal, m_boundingBoxVertical );

    // Find upper face of cells close to the anchor point
    cvf::BoundingBox    boundingBox( m_anchorPosition() - boundingBoxSize, m_anchorPosition() + boundingBoxSize );
    std::vector<size_t> closeCells;
    mainGrid->findIntersectingCells( boundingBox, &closeCells );

    // The stratigraphic thickness is the averge of normals of the top face
    cvf::Vec3d direction = cvf::Vec3d::ZERO;

    int numContributingCells = 0;
    for ( size_t globalCellIndex : closeCells )
    {
        const RigCell& cell = mainGrid->globalCellArray()[globalCellIndex];

        if ( !cell.isInvalid() )
        {
            direction += cell.faceNormalWithAreaLength( cvf::StructGridInterface::NEG_K ).getNormalized();
            numContributingCells++;
        }
    }

    RiaLogging::info( QString( "TST contributing cells: %1/%2" ).arg( numContributingCells ).arg( closeCells.size() ) );
    if ( numContributingCells == 0 )
    {
        // No valid close cells found: just point straight up
        return defaultDirection;
    }

    return ( direction / static_cast<double>( numContributingCells ) ).getNormalized();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModel::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    m_thicknessDirectionWellPath.uiCapability()->setUiHidden( true );
    m_elasticProperties.uiCapability()->setUiHidden( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RimFractureModel::wellPath() const
{
    const caf::PdmObjectHandle* objHandle = dynamic_cast<const caf::PdmObjectHandle*>( this );
    if ( !objHandle ) return nullptr;

    RimWellPath* wellPath = nullptr;
    objHandle->firstAncestorOrThisOfType( wellPath );
    return wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimModeledWellPath* RimFractureModel::thicknessDirectionWellPath() const
{
    return m_thicknessDirectionWellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModel::setThicknessDirectionWellPath( RimModeledWellPath* thicknessDirectionWellPath )
{
    m_thicknessDirectionWellPath = thicknessDirectionWellPath;
    updateThicknessDirectionWellPathName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModel::updateThicknessDirectionWellPathName()
{
    QString wellNameFormat( "%1 for %2" );
    m_thicknessDirectionWellPath()->setName( wellNameFormat.arg( m_extractionType().text() ).arg( name() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFractureModel::vecToString( const cvf::Vec3d& vec )
{
    return QString( "[%1, %2, %3]" ).arg( vec.x() ).arg( vec.y() ).arg( vec.z() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModel::findThicknessTargetPoints( cvf::Vec3d& topPosition, cvf::Vec3d& bottomPosition )
{
    // TODO: duplicated and ugly!
    RimProject* proj = RimProject::current();
    if ( proj->eclipseCases().empty() ) return;

    RimEclipseCase* eclipseCase = proj->eclipseCases()[0];
    if ( !eclipseCase ) return;

    RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();
    if ( !eclipseCaseData ) return;

    const cvf::Vec3d& position  = anchorPosition();
    const cvf::Vec3d& direction = thicknessDirection();

    // Create a "fake" well path which from top to bottom of formation
    // passing through the point and with the given direction

    const cvf::BoundingBox& geometryBoundingBox = eclipseCase->mainGrid()->boundingBox();

    RiaLogging::info( QString( "All cells bounding box: %1 %2" )
                          .arg( RimFractureModel::vecToString( geometryBoundingBox.min() ) )
                          .arg( RimFractureModel::vecToString( geometryBoundingBox.max() ) ) );
    RiaLogging::info( QString( "Position:  %1" ).arg( RimFractureModel::vecToString( position ) ) );
    RiaLogging::info( QString( "Direction: %1" ).arg( RimFractureModel::vecToString( direction ) ) );

    // Create plane on top and bottom of formation
    cvf::Plane topPlane;
    topPlane.setFromPointAndNormal( geometryBoundingBox.max(), cvf::Vec3d::Z_AXIS );
    cvf::Plane bottomPlane;
    bottomPlane.setFromPointAndNormal( geometryBoundingBox.min(), cvf::Vec3d::Z_AXIS );

    // Find and add point on top plane
    cvf::Vec3d abovePlane = position + ( direction * -10000.0 );
    topPlane.intersect( position, abovePlane, &topPosition );
    RiaLogging::info( QString( "Top: %1" ).arg( RimFractureModel::vecToString( topPosition ) ) );

    // Find and add point on bottom plane
    cvf::Vec3d belowPlane = position + ( direction * 10000.0 );
    bottomPlane.intersect( position, belowPlane, &bottomPosition );
    RiaLogging::info( QString( "Bottom: %1" ).arg( RimFractureModel::vecToString( bottomPosition ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimElasticProperties* RimFractureModel::elasticProperties() const
{
    return m_elasticProperties;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModel::setElasticProperties( RimElasticProperties* elasticProperties )
{
    m_elasticProperties = elasticProperties;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModel::loadDataAndUpdate()
{
    if ( m_elasticProperties )
    {
        m_elasticProperties->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::defaultPorosity() const
{
    return m_defaultPorosity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::defaultPermeability() const
{
    return m_defaultPermeability();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::getDefaultForMissingValue( const QString& keyword ) const
{
    if ( keyword == QString( "PORO" ) )
    {
        return defaultPorosity();
    }
    else if ( keyword == QString( "PERMX" ) || keyword == QString( "PERMZ" ) )
    {
        return defaultPermeability();
    }
    else
    {
        RiaLogging::error( QString( "Missing default value for %1." ).arg( keyword ) );
        return std::numeric_limits<double>::infinity();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::CurveProperty RimFractureModel::getDefaultPropertyForMissingValues( const QString& keyword ) const
{
    if ( keyword == QString( "PRESSURE" ) )
    {
        return RiaDefines::CurveProperty::INITIAL_PRESSURE;
    }

    return RiaDefines::CurveProperty::UNDEFINED;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::getDefaultForMissingOverburdenValue( const QString& keyword ) const
{
    if ( keyword == QString( "PORO" ) )
    {
        return defaultOverburdenPorosity();
    }
    else if ( keyword == QString( "PERMX" ) || keyword == QString( "PERMZ" ) )
    {
        return defaultOverburdenPermeability();
    }
    else
    {
        RiaLogging::error( QString( "Missing default overburden value for %1." ).arg( keyword ) );
        return std::numeric_limits<double>::infinity();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::getDefaultForMissingUnderburdenValue( const QString& keyword ) const
{
    if ( keyword == QString( "PORO" ) )
    {
        return defaultUnderburdenPorosity();
    }
    else if ( keyword == QString( "PERMX" ) || keyword == QString( "PERMZ" ) )
    {
        return defaultUnderburdenPermeability();
    }
    else
    {
        RiaLogging::error( QString( "Missing default underburden value for %1." ).arg( keyword ) );
        return std::numeric_limits<double>::infinity();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::getOverburdenGradient( const QString& keyword ) const
{
    if ( keyword == QString( "PRESSURE" ) )
    {
        return m_overburdenFluidDensity * 9.81 * 1000.0 / 1.0e5;
    }
    else
    {
        RiaLogging::error( QString( "Missing overburden gradient for %1." ).arg( keyword ) );
        return std::numeric_limits<double>::infinity();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::getUnderburdenGradient( const QString& keyword ) const
{
    if ( keyword == QString( "PRESSURE" ) )
    {
        return m_underburdenFluidDensity * 9.81 * 1000.0 / 1.0e5;
    }
    else
    {
        RiaLogging::error( QString( "Missing underburden gradient for %1." ).arg( keyword ) );
        return std::numeric_limits<double>::infinity();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::getDefaultValueForProperty( RiaDefines::CurveProperty curveProperty ) const
{
    if ( curveProperty == RiaDefines::CurveProperty::RELATIVE_PERMEABILITY_FACTOR )
    {
        return m_relativePermeabilityFactorDefault;
    }
    else if ( curveProperty == RiaDefines::CurveProperty::PORO_ELASTIC_CONSTANT )
    {
        return m_poroElasticConstantDefault;
    }
    else if ( curveProperty == RiaDefines::CurveProperty::THERMAL_EXPANSION_COEFFISIENT )
    {
        return m_thermalExpansionCoeffientDefault;
    }
    else
    {
        RiaLogging::error(
            QString( "Missing default for %1." ).arg( caf::AppEnum<RiaDefines::CurveProperty>( curveProperty ).uiText() ) );
        return std::numeric_limits<double>::infinity();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFractureModel::hasDefaultValueForProperty( RiaDefines::CurveProperty curveProperty ) const
{
    auto withDefaults = {RiaDefines::CurveProperty::RELATIVE_PERMEABILITY_FACTOR,
                         RiaDefines::CurveProperty::PORO_ELASTIC_CONSTANT,
                         RiaDefines::CurveProperty::THERMAL_EXPANSION_COEFFISIENT};
    return std::find( withDefaults.begin(), withDefaults.end(), curveProperty ) != withDefaults.end();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::verticalStress() const
{
    return m_verticalStress;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::verticalStressGradient() const
{
    return m_verticalStressGradient;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::stressDepth() const
{
    return m_stressDepth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::overburdenHeight() const
{
    return m_overburdenHeight;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::underburdenHeight() const
{
    return m_underburdenHeight;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::defaultOverburdenPorosity() const
{
    return m_overburdenPorosity;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::defaultUnderburdenPorosity() const
{
    return m_underburdenPorosity;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::defaultOverburdenPermeability() const
{
    return m_overburdenPermeability;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::defaultUnderburdenPermeability() const
{
    return m_underburdenPermeability;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFractureModel::overburdenFormation() const
{
    return m_overburdenFormation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFractureModel::overburdenFacies() const
{
    return m_overburdenFacies;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFractureModel::underburdenFormation() const
{
    return m_underburdenFormation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFractureModel::underburdenFacies() const
{
    return m_underburdenFacies;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModel::updateReferringPlots()
{
    // Update plots referring to this fracture model
    std::vector<RimFractureModelPlot*> referringObjects;
    objectsWithReferringPtrFieldsOfType( referringObjects );

    for ( auto modelPlot : referringObjects )
    {
        if ( modelPlot ) modelPlot->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModel::setMD( double md )
{
    m_MD = md;
    updatePositionFromMeasuredDepth();
    updateThicknessDirection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::referenceTemperature() const
{
    return m_referenceTemperature;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::referenceTemperatureGradient() const
{
    return m_referenceTemperatureGradient;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::referenceTemperatureDepth() const
{
    return m_referenceTemperatureDepth;
}
