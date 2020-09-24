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

#include "RiaCompletionTypeCalculationScheduler.h"
#include "RiaEclipseUnitTools.h"
#include "RiaFractureDefines.h"
#include "RiaFractureModelDefines.h"
#include "RiaLogging.h"

#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigSimulationWellCoordsAndMD.h"
#include "RigWellPath.h"
#include "RigWellPathIntersectionTools.h"

#include "Rim3dView.h"
#include "RimAnnotationCollection.h"
#include "RimAnnotationInViewCollection.h"
#include "RimColorLegend.h"
#include "RimColorLegendCollection.h"
#include "RimColorLegendItem.h"
#include "RimCompletionTemplateCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimFractureModelPlot.h"
#include "RimFractureModelTemplate.h"
#include "RimFractureModelTemplateCollection.h"
#include "RimModeledWellPath.h"
#include "RimOilField.h"
#include "RimPolylineTarget.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimUserDefinedPolylinesAnnotation.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathGeometryDef.h"
#include "RimWellPathTarget.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmFieldCvfVec3d.h"
#include "cafPdmFieldScriptingCapabilityCvfVec3d.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiDoubleValueEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiToolButtonEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include "cvfBoundingBox.h"
#include "cvfGeometryTools.h"
#include "cvfMath.h"
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

template <>
void caf::AppEnum<RimFractureModel::FractureOrientation>::setUp()
{
    addItem( RimFractureModel::FractureOrientation::ALONG_WELL_PATH, "ALONG_WELL_PATH", "Along Well Path" );
    addItem( RimFractureModel::FractureOrientation::TRANSVERSE_WELL_PATH,
             "TRANSVERSE_WELL_PATH",
             "Transverse (normal) to Well Path" );

    setDefault( RimFractureModel::FractureOrientation::TRANSVERSE_WELL_PATH );
}
}; // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModel::RimFractureModel()
{
    CAF_PDM_InitScriptableObject( "FractureModel", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_fractureModelTemplate, "FractureModelTemplate", "Fracture Model Template", "", "", "" );
    CAF_PDM_InitField( &m_editFractureModelTemplate, "EditModelTemplate", false, "Edit", "", "", "" );
    m_editFractureModelTemplate.uiCapability()->setUiEditorTypeName( caf::PdmUiToolButtonEditor::uiEditorTypeName() );
    m_editFractureModelTemplate.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

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

    // Stress unit: bar
    // Stress gradient unit: bar/m
    // Depth is meter
    double defaultStressGradient = 0.238;
    double defaultStressDepth    = computeDefaultStressDepth();
    double defaultStress         = defaultStressDepth * defaultStressGradient;

    CAF_PDM_InitScriptableField( &m_verticalStress, "VerticalStress", defaultStress, "Vertical Stress", "", "", "" );
    m_verticalStress.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );
    CAF_PDM_InitScriptableField( &m_verticalStressGradient,
                                 "VerticalStressGradient",
                                 defaultStressGradient,
                                 "Vertical Stress Gradient",
                                 "",
                                 "",
                                 "" );
    CAF_PDM_InitScriptableField( &m_stressDepth, "StressDepth", defaultStressDepth, "Stress Depth", "", "", "" );
    m_stressDepth.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

    CAF_PDM_InitScriptableField( &m_referenceTemperature, "ReferenceTemperature", 70.0, "Temperature [C]", "", "", "" );
    CAF_PDM_InitScriptableField( &m_referenceTemperatureGradient,
                                 "ReferenceTemperatureGradient",
                                 0.025,
                                 "Temperature Gradient [C/m]",
                                 "",
                                 "",
                                 "" );
    CAF_PDM_InitScriptableField( &m_referenceTemperatureDepth,
                                 "ReferenceTemperatureDepth",
                                 2500.0,
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
                                 "ThermalExpansionCoefficient",
                                 0.0,
                                 "Thermal Expansion Coefficient [1/C]",
                                 "",
                                 "",
                                 "" );

    CAF_PDM_InitScriptableField( &m_perforationLength, "PerforationLength", 10.0, "Perforation Length [m]", "", "", "" );
    CAF_PDM_InitScriptableField( &m_fractureOrientation,
                                 "FractureOrientation",
                                 caf::AppEnum<FractureOrientation>( FractureOrientation::ALONG_WELL_PATH ),
                                 "Fracture Orientation",
                                 "",
                                 "",
                                 "" );

    CAF_PDM_InitScriptableField( &m_formationDip, "FormationDip", 0.0, "Formation Dip", "", "", "" );
    m_formationDip.uiCapability()->setUiReadOnly( true );
    m_formationDip.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

    CAF_PDM_InitScriptableField( &m_autoComputeBarrier, "AutoComputeBarrier", true, "Auto Compute Barrier", "", "", "" );
    CAF_PDM_InitScriptableField( &m_hasBarrier, "Barrier", true, "Barrier", "", "", "" );
    CAF_PDM_InitScriptableField( &m_distanceToBarrier, "DistanceToBarrier", 0.0, "Distance To Barrier [m]", "", "", "" );
    m_distanceToBarrier.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );
    m_distanceToBarrier.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitScriptableField( &m_barrierDip, "BarrierDip", 0.0, "Barrier Dip", "", "", "" );
    m_barrierDip.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );
    m_barrierDip.uiCapability()->setUiReadOnly( true );
    CAF_PDM_InitScriptableField( &m_wellPenetrationLayer, "WellPenetrationLayer", 0, "Well Penetration Layer", "", "", "" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_barrierAnnotation, "BarrierAnnotation", "Barrier Annotation", "", "", "" );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModel::~RimFractureModel()
{
    clearBarrierAnnotation();

    RimWellPath*           wellPath           = m_thicknessDirectionWellPath.value();
    RimWellPathCollection* wellPathCollection = RimTools::wellPathCollection();

    if ( wellPath && wellPathCollection )
    {
        wellPathCollection->removeWellPath( wellPath );
        delete wellPath;

        wellPathCollection->uiCapability()->updateConnectedEditors();
        wellPathCollection->scheduleRedrawAffectedViews();
    }
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
         changedField == &m_boundingBoxHorizontal || changedField == &m_fractureOrientation ||
         changedField == &m_autoComputeBarrier )
    {
        updateThicknessDirection();

        if ( m_autoComputeBarrier )
        {
            updateDistanceToBarrierAndDip();
        }
        else
        {
            clearBarrierAnnotation();
        }
    }

    if ( changedField == &m_autoComputeBarrier || changedField == &m_hasBarrier )
    {
        m_barrierDip.uiCapability()->setUiReadOnly( m_autoComputeBarrier || !m_hasBarrier );
        m_distanceToBarrier.uiCapability()->setUiReadOnly( m_autoComputeBarrier || !m_hasBarrier );
    }

    if ( changedField == &m_extractionType || changedField == &m_thicknessDirectionWellPath )
    {
        updateThicknessDirectionWellPathName();
        m_thicknessDirectionWellPath()->updateConnectedEditors();
    }

    if ( changedField == &m_useDetailedFluidLoss )
    {
        m_relativePermeabilityFactorDefault.uiCapability()->setUiReadOnly( !m_useDetailedFluidLoss );
        m_poroElasticConstantDefault.uiCapability()->setUiReadOnly( !m_useDetailedFluidLoss );
        m_thermalExpansionCoeffientDefault.uiCapability()->setUiReadOnly( !m_useDetailedFluidLoss );

        m_referenceTemperature.uiCapability()->setUiReadOnly( !m_useDetailedFluidLoss );
        m_referenceTemperatureGradient.uiCapability()->setUiReadOnly( !m_useDetailedFluidLoss );
        m_referenceTemperatureDepth.uiCapability()->setUiReadOnly( !m_useDetailedFluidLoss );
    }

    if ( changedField == &m_editFractureModelTemplate )
    {
        m_editFractureModelTemplate = false;
        if ( m_fractureModelTemplate != nullptr )
        {
            Riu3DMainWindowTools::selectAsCurrentItem( m_fractureModelTemplate() );
        }
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

    if ( fieldNeedingOptions == &m_fractureModelTemplate )
    {
        RimOilField* oilField = RimProject::current()->activeOilField();
        if ( oilField && oilField->completionTemplateCollection() )
        {
            RimFractureModelTemplateCollection* fracDefColl =
                oilField->completionTemplateCollection()->fractureModelTemplateCollection();

            for ( RimFractureModelTemplate* fracDef : fracDefColl->fractureModelTemplates() )
            {
                QString displayText = fracDef->name();
                options.push_back( caf::PdmOptionItemInfo( displayText, fracDef ) );
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
    m_formationDip       = calculateFormationDip( direction );

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

    RigEclipseCaseData* eclipseCaseData = getEclipseCaseData();
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
void RimFractureModel::updateDistanceToBarrierAndDip()
{
    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>( this );
    if ( !objHandle ) return;

    RimWellPath* wellPath = nullptr;
    objHandle->firstAncestorOrThisOfType( wellPath );
    if ( !wellPath ) return;

    RigEclipseCaseData* eclipseCaseData = getEclipseCaseData();
    if ( !eclipseCaseData ) return;

    const cvf::Vec3d& position = anchorPosition();

    RiaLogging::info( "Computing distance to barrier." );
    RiaLogging::info( QString( "Anchor position: %1" ).arg( RimFractureModel::vecToString( position ) ) );

    RigWellPath* wellPathGeometry = wellPath->wellPathGeometry();

    // Find the well path points closest to the anchor position
    cvf::Vec3d p1;
    cvf::Vec3d p2;
    wellPathGeometry->twoClosestPoints( position, &p1, &p2 );
    RiaLogging::info( QString( "Closest points on well path: %1 %2" )
                          .arg( RimFractureModel::vecToString( p1 ) )
                          .arg( RimFractureModel::vecToString( p2 ) ) );

    // Create a well direction based on the two points
    cvf::Vec3d wellDirection = ( p2 - p1 ).getNormalized();
    RiaLogging::info( QString( "Well direction: %1" ).arg( RimFractureModel::vecToString( wellDirection ) ) );

    cvf::Vec3d fractureDirection = wellDirection;
    if ( m_fractureOrientation == FractureOrientation::ALONG_WELL_PATH )
    {
        cvf::Mat3d azimuthRotation = cvf::Mat3d::fromRotation( cvf::Vec3d::Z_AXIS, cvf::Math::toRadians( 90.0 ) );
        fractureDirection.transformVector( azimuthRotation );
    }

    // The direction to the barrier is normal to the TST
    cvf::Vec3d directionToBarrier = ( thicknessDirection() ^ fractureDirection ).getNormalized();
    RiaLogging::info( QString( "Direction to barrier: %1" ).arg( RimFractureModel::vecToString( directionToBarrier ) ) );

    std::vector<WellPathCellIntersectionInfo> intersections =
        generateBarrierIntersections( eclipseCaseData, position, directionToBarrier );

    RiaLogging::info( QString( "Intersections: %1" ).arg( intersections.size() ) );

    double shortestDistance = std::numeric_limits<double>::max();

    RigMainGrid* mainGrid   = eclipseCaseData->mainGrid();
    bool         foundFault = false;
    cvf::Vec3d   barrierPosition;
    double       barrierDip = 0.0;
    for ( const WellPathCellIntersectionInfo& intersection : intersections )
    {
        // Find the closest cell face which is a fault
        double          distance = position.pointDistance( intersection.startPoint );
        const RigFault* fault    = mainGrid->findFaultFromCellIndexAndCellFace( intersection.globCellIndex,
                                                                             intersection.intersectedCellFaceIn );
        if ( fault && distance < shortestDistance )
        {
            foundFault       = true;
            shortestDistance = distance;
            barrierPosition  = intersection.startPoint;

            const RigCell& cell       = mainGrid->globalCellArray()[intersection.globCellIndex];
            cvf::Vec3d     faceNormal = cell.faceNormalWithAreaLength( intersection.intersectedCellFaceIn );
            barrierDip                = calculateFormationDip( faceNormal );
        }
    }

    if ( foundFault )
    {
        RiaLogging::info( QString( "Found barrier distance: %1 Dip: %2" ).arg( shortestDistance ).arg( barrierDip ) );
        clearBarrierAnnotation();
        addBarrierAnnotation( position, barrierPosition );

        m_hasBarrier        = true;
        m_barrierDip        = barrierDip;
        m_distanceToBarrier = shortestDistance;
    }
    else
    {
        RiaLogging::info( "No barrier found." );
        clearBarrierAnnotation();
        m_hasBarrier        = false;
        m_barrierDip        = 0.0;
        m_distanceToBarrier = 0.0;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<WellPathCellIntersectionInfo>
    RimFractureModel::generateBarrierIntersections( RigEclipseCaseData* eclipseCaseData,
                                                    const cvf::Vec3d&   position,
                                                    const cvf::Vec3d&   directionToBarrier )
{
    double                                    randoDistance    = 10000.0;
    cvf::Vec3d                                forwardPosition  = position + ( directionToBarrier * randoDistance );
    cvf::Vec3d                                backwardPosition = position + ( directionToBarrier * -randoDistance );
    std::vector<WellPathCellIntersectionInfo> intersections =
        generateBarrierIntersectionsBetweenPoints( eclipseCaseData, position, forwardPosition );
    std::vector<WellPathCellIntersectionInfo> backwardIntersections =
        generateBarrierIntersectionsBetweenPoints( eclipseCaseData, position, backwardPosition );

    // Merge the intersections for the search for closest
    intersections.insert( intersections.end(), backwardIntersections.begin(), backwardIntersections.end() );
    return intersections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<WellPathCellIntersectionInfo>
    RimFractureModel::generateBarrierIntersectionsBetweenPoints( RigEclipseCaseData* eclipseCaseData,
                                                                 const cvf::Vec3d&   startPosition,
                                                                 const cvf::Vec3d&   endPosition )
{
    // Create a fake well path from the anchor point to
    // a point far away in the direction barrier direction
    std::vector<cvf::Vec3d> pathCoords;
    pathCoords.push_back( startPosition );
    pathCoords.push_back( endPosition );

    RigSimulationWellCoordsAndMD helper( pathCoords );
    return RigWellPathIntersectionTools::findCellIntersectionInfosAlongPath( eclipseCaseData,
                                                                             helper.wellPathPoints(),
                                                                             helper.measuredDepths() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModel::clearBarrierAnnotation()
{
    auto existingAnnotation = m_barrierAnnotation.value();
    if ( existingAnnotation )
    {
        delete existingAnnotation;
        m_barrierAnnotation = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModel::addBarrierAnnotation( const cvf::Vec3d& startPosition, const cvf::Vec3d& endPosition )
{
    RimAnnotationCollection* coll = annotationCollection();
    if ( !coll ) return;

    auto newAnnotation = new RimUserDefinedPolylinesAnnotation();

    RimPolylineTarget* startTarget = new RimPolylineTarget();
    startTarget->setAsPointXYZ( startPosition );
    newAnnotation->insertTarget( nullptr, startTarget );

    RimPolylineTarget* endTarget = new RimPolylineTarget();
    endTarget->setAsPointXYZ( endPosition );
    newAnnotation->insertTarget( nullptr, endTarget );

    m_barrierAnnotation = newAnnotation;

    coll->addAnnotation( newAnnotation );
    coll->scheduleRedrawOfRelevantViews();
    coll->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnnotationCollection* RimFractureModel::annotationCollection()
{
    const auto project  = RimProject::current();
    auto       oilField = project->activeOilField();
    return oilField ? oilField->annotationCollection() : nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModel::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    m_thicknessDirectionWellPath.uiCapability()->setUiHidden( true );
    m_barrierAnnotation.uiCapability()->setUiHidden( true );

    uiOrdering.add( nameField(), caf::PdmUiOrdering::LayoutOptions( true, 3, 1 ) );
    uiOrdering.add( &m_fractureModelTemplate, {true, 2, 1} );
    uiOrdering.add( &m_editFractureModelTemplate, {false, 1, 0} );

    uiOrdering.add( &m_MD );
    uiOrdering.add( &m_extractionType );
    uiOrdering.add( &m_anchorPosition );
    uiOrdering.add( &m_thicknessDirection );

    caf::PdmUiOrdering* boundingBoxGroup = uiOrdering.addNewGroup( "Bounding Box" );
    boundingBoxGroup->add( &m_boundingBoxHorizontal );
    boundingBoxGroup->add( &m_boundingBoxVertical );

    caf::PdmUiOrdering* referenceStressGroup = uiOrdering.addNewGroup( "Reference Stress" );
    referenceStressGroup->add( &m_verticalStress );
    referenceStressGroup->add( &m_verticalStressGradient );
    referenceStressGroup->add( &m_stressDepth );

    caf::PdmUiOrdering* detailedFluidLossGroup = uiOrdering.addNewGroup( "Detailed Fluid Loss" );
    detailedFluidLossGroup->add( &m_useDetailedFluidLoss );
    detailedFluidLossGroup->add( &m_relativePermeabilityFactorDefault );
    detailedFluidLossGroup->add( &m_poroElasticConstantDefault );
    detailedFluidLossGroup->add( &m_thermalExpansionCoeffientDefault );

    caf::PdmUiOrdering* temperatureGroup = detailedFluidLossGroup->addNewGroup( "Temperature" );
    temperatureGroup->add( &m_referenceTemperature );
    temperatureGroup->add( &m_referenceTemperatureGradient );
    temperatureGroup->add( &m_referenceTemperatureDepth );

    caf::PdmUiOrdering* perforationGroup = uiOrdering.addNewGroup( "Perforation" );
    perforationGroup->add( &m_perforationLength );
    perforationGroup->add( &m_fractureOrientation );

    caf::PdmUiOrdering* asymmetricGroup = uiOrdering.addNewGroup( "Asymmetric" );
    asymmetricGroup->add( &m_formationDip );
    asymmetricGroup->add( &m_hasBarrier );
    asymmetricGroup->add( &m_autoComputeBarrier );
    asymmetricGroup->add( &m_distanceToBarrier );
    asymmetricGroup->add( &m_barrierDip );
    asymmetricGroup->add( &m_wellPenetrationLayer );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModel::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                              QString                    uiConfigName,
                                              caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_stressDepth || field == &m_verticalStress || field == &m_formationDip || field == &m_barrierDip ||
         field == &m_distanceToBarrier )
    {
        auto doubleAttr = dynamic_cast<caf::PdmUiDoubleValueEditorAttribute*>( attribute );
        if ( doubleAttr )
        {
            doubleAttr->m_decimals     = 2;
            doubleAttr->m_numberFormat = caf::PdmUiDoubleValueEditorAttribute::NumberFormat::FIXED;
        }
    }
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
    RigEclipseCaseData* eclipseCaseData = getEclipseCaseData();
    if ( !eclipseCaseData ) return;

    const cvf::Vec3d& position  = anchorPosition();
    const cvf::Vec3d& direction = thicknessDirection();

    // Create a "fake" well path which from top to bottom of formation
    // passing through the point and with the given direction

    const cvf::BoundingBox& geometryBoundingBox = eclipseCaseData->mainGrid()->boundingBox();

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
double RimFractureModel::calculateFormationDip( const cvf::Vec3d& direction )
{
    // Formation dip is inclination of a plane from horizontal.
    return cvf::Math::toDegrees( cvf::GeometryTools::getAngle( direction, -cvf::Vec3d::Z_AXIS ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModel::loadDataAndUpdate()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::defaultPorosity() const
{
    return m_fractureModelTemplate() ? m_fractureModelTemplate()->defaultPorosity() : 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::defaultPermeability() const
{
    return m_fractureModelTemplate() ? m_fractureModelTemplate()->defaultPermeability() : 0.0;
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
        if ( !m_fractureModelTemplate )
        {
            return 0.0;
        }
        return m_fractureModelTemplate()->overburdenFluidDensity() * 9.81 * 1000.0 / 1.0e5;
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
        if ( !m_fractureModelTemplate )
        {
            return 0.0;
        }

        return m_fractureModelTemplate()->underburdenFluidDensity() * 9.81 * 1000.0 / 1.0e5;
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
    else if ( curveProperty == RiaDefines::CurveProperty::THERMAL_EXPANSION_COEFFICIENT )
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
                         RiaDefines::CurveProperty::THERMAL_EXPANSION_COEFFICIENT};
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
    return m_fractureModelTemplate() ? m_fractureModelTemplate()->overburdenHeight() : 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::underburdenHeight() const
{
    return m_fractureModelTemplate() ? m_fractureModelTemplate()->underburdenHeight() : 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::defaultOverburdenPorosity() const
{
    return m_fractureModelTemplate() ? m_fractureModelTemplate()->defaultOverburdenPorosity() : 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::defaultUnderburdenPorosity() const
{
    return m_fractureModelTemplate() ? m_fractureModelTemplate()->defaultUnderburdenPorosity() : 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::defaultOverburdenPermeability() const
{
    return m_fractureModelTemplate() ? m_fractureModelTemplate()->defaultOverburdenPermeability() : 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::defaultUnderburdenPermeability() const
{
    return m_fractureModelTemplate() ? m_fractureModelTemplate()->defaultUnderburdenPermeability() : 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFractureModel::overburdenFormation() const
{
    return m_fractureModelTemplate() ? m_fractureModelTemplate()->overburdenFormation() : "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFractureModel::overburdenFacies() const
{
    return m_fractureModelTemplate() ? m_fractureModelTemplate()->overburdenFacies() : "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFractureModel::underburdenFormation() const
{
    return m_fractureModelTemplate() ? m_fractureModelTemplate()->underburdenFormation() : "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFractureModel::underburdenFacies() const
{
    return m_fractureModelTemplate() ? m_fractureModelTemplate()->underburdenFacies() : "";
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimFractureModel::getEclipseCase()
{
    // Find an eclipse case
    RimProject* proj = RimProject::current();
    if ( proj->eclipseCases().empty() ) return nullptr;

    return proj->eclipseCases()[0];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseCaseData* RimFractureModel::getEclipseCaseData()
{
    // Find an eclipse case
    RimEclipseCase* eclipseCase = getEclipseCase();
    if ( !eclipseCase ) return nullptr;

    return eclipseCase->eclipseCaseData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::computeDefaultStressDepth()
{
    const double stressDepth = 1000.0;

    RimEclipseCase* eclipseCase = getEclipseCase();
    if ( !eclipseCase ) return stressDepth;

    // Use top of active cells as reference stress depth
    return -eclipseCase->activeCellsBoundingBox().max().z();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::perforationLength() const
{
    return m_perforationLength();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModel::FractureOrientation RimFractureModel::fractureOrientation() const
{
    return m_fractureOrientation();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::formationDip() const
{
    return m_formationDip;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimFractureModel::hasBarrier() const
{
    return m_hasBarrier;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::distanceToBarrier() const
{
    return m_distanceToBarrier;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimFractureModel::barrierDip() const
{
    return m_barrierDip;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimFractureModel::wellPenetrationLayer() const
{
    return m_wellPenetrationLayer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFractureModel::setFractureModelTemplate( RimFractureModelTemplate* fractureModelTemplate )
{
    m_fractureModelTemplate = fractureModelTemplate;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModelTemplate* RimFractureModel::fractureModelTemplate() const
{
    return m_fractureModelTemplate;
}
