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

#include "RimStimPlanModel.h"

#include "RiaApplication.h"
#include "RiaCompletionTypeCalculationScheduler.h"
#include "RiaEclipseUnitTools.h"
#include "RiaFractureDefines.h"
#include "RiaLogging.h"
#include "RiaStimPlanModelDefines.h"

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
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimFaciesProperties.h"
#include "RimFaultInView.h"
#include "RimFaultInViewCollection.h"
#include "RimFracture.h"
#include "RimModeledWellPath.h"
#include "RimNonNetLayers.h"
#include "RimOilField.h"
#include "RimPolylineTarget.h"
#include "RimProject.h"
#include "RimStimPlanModelCalculator.h"
#include "RimStimPlanModelPlot.h"
#include "RimStimPlanModelTemplate.h"
#include "RimStimPlanModelTemplateCollection.h"
#include "RimTextAnnotation.h"
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

CAF_PDM_SOURCE_INIT( RimStimPlanModel, "StimPlanModel" );

namespace caf
{
template <>
void caf::AppEnum<RimStimPlanModel::ExtractionType>::setUp()
{
    addItem( RimStimPlanModel::ExtractionType::TRUE_VERTICAL_THICKNESS, "TVT", "True Vertical Thickness" );
    addItem( RimStimPlanModel::ExtractionType::TRUE_STRATIGRAPHIC_THICKNESS, "TST", "True Stratigraphic Thickness" );

    setDefault( RimStimPlanModel::ExtractionType::TRUE_VERTICAL_THICKNESS );
}

template <>
void caf::AppEnum<RimStimPlanModel::FractureOrientation>::setUp()
{
    addItem( RimStimPlanModel::FractureOrientation::ALONG_WELL_PATH, "ALONG_WELL_PATH", "Along Well Path" );
    addItem( RimStimPlanModel::FractureOrientation::TRANSVERSE_WELL_PATH,
             "TRANSVERSE_WELL_PATH",
             "Transverse (normal) to Well Path" );
    addItem( RimStimPlanModel::FractureOrientation::AZIMUTH, "AZIMUTH", "Azimuth" );

    setDefault( RimStimPlanModel::FractureOrientation::TRANSVERSE_WELL_PATH );
}

template <>
void caf::AppEnum<RimStimPlanModel::MissingValueStrategy>::setUp()
{
    addItem( RimStimPlanModel::MissingValueStrategy::DEFAULT_VALUE, "DEFAULT_VALUE", "Default value" );
    addItem( RimStimPlanModel::MissingValueStrategy::LINEAR_INTERPOLATION, "LINEAR_INTERPOLATION", "Linear interpolation" );
    addItem( RimStimPlanModel::MissingValueStrategy::OTHER_CURVE_PROPERTY, "OTHER_CURVE_PROPERTY", "Other Curve Property" );

    setDefault( RimStimPlanModel::MissingValueStrategy::DEFAULT_VALUE );
}

template <>
void caf::AppEnum<RimStimPlanModel::BurdenStrategy>::setUp()
{
    addItem( RimStimPlanModel::BurdenStrategy::DEFAULT_VALUE, "DEFAULT_VALUE", "Default value" );
    addItem( RimStimPlanModel::BurdenStrategy::GRADIENT, "GRADIENT", "Gradient" );

    setDefault( RimStimPlanModel::BurdenStrategy::DEFAULT_VALUE );
}
}; // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModel::RimStimPlanModel()
{
    CAF_PDM_InitScriptableObject( "StimPlanModel", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_stimPlanModelTemplate, "StimPlanModelTemplate", "StimPlan Model Template", "", "", "" );
    CAF_PDM_InitField( &m_editStimPlanModelTemplate, "EditModelTemplate", false, "Edit", "", "", "" );
    m_editStimPlanModelTemplate.uiCapability()->setUiEditorTypeName( caf::PdmUiToolButtonEditor::uiEditorTypeName() );
    m_editStimPlanModelTemplate.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitScriptableFieldNoDefault( &m_eclipseCase, "EclipseCase", "Case", "", "", "" );
    CAF_PDM_InitScriptableField( &m_timeStep, "TimeStep", 0, "Time Step", "", "", "" );

    CAF_PDM_InitScriptableField( &m_MD, "MeasuredDepth", 0.0, "Measured Depth", "", "", "" );
    m_MD.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitScriptableField( &m_extractionDepthTop, "ExtractionDepthTop", -1.0, "Top", "", "", "" );
    m_extractionDepthTop.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

    CAF_PDM_InitScriptableField( &m_extractionDepthBottom, "ExtractionDepthBottom", -1.0, "Bottom", "", "", "" );
    m_extractionDepthBottom.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

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
    CAF_PDM_InitScriptableField( &m_azimuthAngle, "AzimuthAngle", 0.0, "Azimuth Angle", "", "", "" );

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
    CAF_PDM_InitScriptableField( &m_wellPenetrationLayer, "WellPenetrationLayer", 2, "Well Penetration Layer", "", "", "" );

    CAF_PDM_InitScriptableField( &m_showOnlyBarrierFault, "ShowOnlyBarrierFault", false, "Show Only Barrier Fault", "", "", "" );
    CAF_PDM_InitScriptableField( &m_showAllFaults, "ShowAllFaults", false, "Show All Faults", "", "", "" );
    m_showAllFaults.uiCapability()->setUiEditorTypeName( caf::PdmUiToolButtonEditor::uiEditorTypeName() );
    m_showAllFaults.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::HIDDEN );

    CAF_PDM_InitScriptableField( &m_barrierFaultName, "BarrierFaultName", QString( "" ), "Barrier Fault", "", "", "" );
    m_barrierFaultName.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitScriptableFieldNoDefault( &m_barrierAnnotation, "BarrierAnnotation", "Barrier Annotation", "", "", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_barrierTextAnnotation,
                                          "BarrierTextAnnotation",
                                          "Barrier Text Annotation",
                                          "",
                                          "",
                                          "" );

    m_calculator = std::shared_ptr<RimStimPlanModelCalculator>( new RimStimPlanModelCalculator );
    m_calculator->setStimPlanModel( this );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModel::~RimStimPlanModel()
{
    RimWellPath*           wellPath           = m_thicknessDirectionWellPath.value();
    RimWellPathCollection* wellPathCollection = RimTools::wellPathCollection();

    if ( wellPath && wellPathCollection )
    {
        wellPathCollection->removeWellPath( wellPath );
        wellPathCollection->uiCapability()->updateConnectedEditors();
        wellPathCollection->scheduleRedrawAffectedViews();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanModel::isEnabled() const
{
    return isChecked();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanModel::useDetailedFluidLoss() const
{
    return m_useDetailedFluidLoss();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModel::initAfterRead()
{
    if ( m_stimPlanModelTemplate )
    {
        m_stimPlanModelTemplate->changed.connect( this, &RimStimPlanModel::stimPlanModelTemplateChanged );
    }

    if ( m_extractionDepthTop() < 0.0 || m_extractionDepthBottom < 0.0 )
    {
        updateExtractionDepthBoundaries();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModel::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                         const QVariant&            oldValue,
                                         const QVariant&            newValue )
{
    if ( changedField == &m_MD )
    {
        updatePositionFromMeasuredDepth();
    }

    if ( changedField == &m_MD || changedField == &m_extractionType || changedField == &m_boundingBoxVertical ||
         changedField == &m_boundingBoxHorizontal || changedField == &m_fractureOrientation ||
         changedField == &m_autoComputeBarrier || changedField == &m_azimuthAngle ||
         changedField == &m_showOnlyBarrierFault || changedField == &m_eclipseCase ||
         changedField == &m_extractionDepthTop || changedField == &m_extractionDepthBottom )
    {
        updateThicknessDirection();
        updateBarrierProperties();
    }

    if ( changedField == &m_eclipseCase )
    {
        // Set a valid default time step
        const int timeStepCount = m_eclipseCase->timeStepStrings().size();
        if ( timeStepCount > 0 )
        {
            m_timeStep = timeStepCount - 1;
        }

        updateExtractionDepthBoundaries();
    }

    if ( changedField == &m_showAllFaults )
    {
        m_showAllFaults        = false;
        m_showOnlyBarrierFault = false;
        showAllFaults();
    }

    if ( changedField == &m_autoComputeBarrier || changedField == &m_hasBarrier )
    {
        m_barrierDip.uiCapability()->setUiReadOnly( m_autoComputeBarrier || !m_hasBarrier );
        m_distanceToBarrier.uiCapability()->setUiReadOnly( m_autoComputeBarrier || !m_hasBarrier );
        m_showOnlyBarrierFault.uiCapability()->setUiReadOnly( !m_hasBarrier );
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
    }

    if ( changedField == &m_stimPlanModelTemplate )
    {
        setStimPlanModelTemplate( m_stimPlanModelTemplate() );
    }

    if ( changedField == &m_editStimPlanModelTemplate )
    {
        m_editStimPlanModelTemplate = false;
        if ( m_stimPlanModelTemplate != nullptr )
        {
            Riu3DMainWindowTools::selectAsCurrentItem( m_stimPlanModelTemplate() );
        }
    }
    else
    {
        updateViewsAndPlots();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimStimPlanModel::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                       bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_stimPlanModelTemplate )
    {
        RimOilField* oilField = RimProject::current()->activeOilField();
        if ( oilField && oilField->completionTemplateCollection() )
        {
            RimStimPlanModelTemplateCollection* fracDefColl =
                oilField->completionTemplateCollection()->stimPlanModelTemplateCollection();

            for ( RimStimPlanModelTemplate* fracDef : fracDefColl->stimPlanModelTemplates() )
            {
                QString displayText = fracDef->name();
                options.push_back( caf::PdmOptionItemInfo( displayText, fracDef ) );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_eclipseCase )
    {
        RimTools::eclipseCaseOptionItems( &options );
    }
    else if ( fieldNeedingOptions == &m_timeStep )
    {
        RimTools::timeStepsForCase( m_eclipseCase(), &options );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimStimPlanModel::fracturePosition() const
{
    return m_anchorPosition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::WellPathComponentType RimStimPlanModel::componentType() const
{
    return RiaDefines::WellPathComponentType::FRACTURE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStimPlanModel::componentLabel() const
{
    return name();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStimPlanModel::componentTypeLabel() const
{
    return "StimPlan Model";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimStimPlanModel::defaultComponentColor() const
{
    return cvf::Color3f::RED; // RiaColorTables::wellPathComponentColors()[componentType()];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModel::startMD() const
{
    return m_MD();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModel::endMD() const
{
    return m_MD();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimStimPlanModel::anchorPosition() const
{
    return m_anchorPosition();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimStimPlanModel::thicknessDirection() const
{
    return m_thicknessDirection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModel::updatePositionFromMeasuredDepth()
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
void RimStimPlanModel::updateThicknessDirection()
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
        RimWellPathGeometryDef* wellGeomDef = m_thicknessDirectionWellPath->geometryDefinition();
        wellGeomDef->deleteAllTargets();
        // Disable auto-generated target at sea level: this will usually be outside the
        // bounding box, and will introduce problems when the project is reloaded.
        wellGeomDef->setUseAutoGeneratedTargetAtSeaLevel( false );

        cvf::Vec3d topPosition;
        cvf::Vec3d bottomPosition;

        if ( findThicknessTargetPoints( topPosition, bottomPosition ) )
        {
            topPosition.z() *= -1.0;
            bottomPosition.z() *= -1.0;

            RimWellPathTarget* topPathTarget = new RimWellPathTarget();
            topPathTarget->setAsPointTargetXYD( topPosition );

            RimWellPathTarget* bottomPathTarget = new RimWellPathTarget();
            bottomPathTarget->setAsPointTargetXYD( bottomPosition );

            wellGeomDef->insertTarget( nullptr, topPathTarget );
            wellGeomDef->insertTarget( nullptr, bottomPathTarget );
        }

        wellGeomDef->updateConnectedEditors();
        wellGeomDef->updateWellPathVisualization( false );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimStimPlanModel::calculateTSTDirection() const
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

    direction = ( direction / static_cast<double>( numContributingCells ) ).getNormalized();

    // A surface has normals in both directions: if the normal points upwards we flip it to
    // make it point downwards. This necessary when finding the TST start and end points later.
    if ( direction.z() > 0.0 )
    {
        direction *= -1.0;
    }

    return direction;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModel::updateExtractionDepthBoundaries()
{
    RigEclipseCaseData* eclipseCaseData = getEclipseCaseData();
    if ( eclipseCaseData )
    {
        const cvf::BoundingBox& boundingBox = eclipseCaseData->mainGrid()->boundingBox();
        m_extractionDepthTop                = -boundingBox.max().z();
        m_extractionDepthBottom             = -boundingBox.min().z();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModel::updateBarrierProperties()
{
    if ( m_autoComputeBarrier )
    {
        updateDistanceToBarrierAndDip();
    }
    else
    {
        clearBarrierAnnotation();
    }

    if ( m_showOnlyBarrierFault() && m_hasBarrier() )
    {
        hideOtherFaults( m_barrierFaultName() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModel::updateDistanceToBarrierAndDip()
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
    RiaLogging::info( QString( "Anchor position: %1" ).arg( RimStimPlanModel::vecToString( position ) ) );

    RigWellPath* wellPathGeometry = wellPath->wellPathGeometry();

    // Find the well path points closest to the anchor position
    cvf::Vec3d p1;
    cvf::Vec3d p2;
    wellPathGeometry->twoClosestPoints( position, &p1, &p2 );
    RiaLogging::info( QString( "Closest points on well path: %1 %2" )
                          .arg( RimStimPlanModel::vecToString( p1 ) )
                          .arg( RimStimPlanModel::vecToString( p2 ) ) );

    // Create a well direction based on the two points
    cvf::Vec3d wellDirection = ( p2 - p1 ).getNormalized();
    RiaLogging::info( QString( "Well direction: %1" ).arg( RimStimPlanModel::vecToString( wellDirection ) ) );

    cvf::Vec3d fractureDirection = wellDirection;
    if ( m_fractureOrientation == FractureOrientation::ALONG_WELL_PATH )
    {
        cvf::Mat3d azimuthRotation = cvf::Mat3d::fromRotation( cvf::Vec3d::Z_AXIS, cvf::Math::toRadians( 90.0 ) );
        fractureDirection.transformVector( azimuthRotation );
    }
    else if ( m_fractureOrientation == FractureOrientation::AZIMUTH )
    {
        // Azimuth angle of fracture is relative to north.
        double     wellAzimuth = wellPathGeometry->wellPathAzimuthAngle( position );
        cvf::Mat3d azimuthRotation =
            cvf::Mat3d::fromRotation( cvf::Vec3d::Z_AXIS, cvf::Math::toRadians( wellAzimuth - m_azimuthAngle() - 90.0 ) );
        fractureDirection.transformVector( azimuthRotation );
    }

    // The direction to the barrier is normal to the TST
    cvf::Vec3d directionToBarrier = ( thicknessDirection() ^ fractureDirection ).getNormalized();
    RiaLogging::info( QString( "Direction to barrier: %1" ).arg( RimStimPlanModel::vecToString( directionToBarrier ) ) );

    std::vector<WellPathCellIntersectionInfo> intersections =
        generateBarrierIntersections( eclipseCaseData, position, directionToBarrier );

    RiaLogging::info( QString( "Intersections: %1" ).arg( intersections.size() ) );

    double shortestDistance = std::numeric_limits<double>::max();

    RigMainGrid*    mainGrid = eclipseCaseData->mainGrid();
    cvf::Vec3d      barrierPosition;
    double          barrierDip = 0.0;
    const RigFault* foundFault = nullptr;
    for ( const WellPathCellIntersectionInfo& intersection : intersections )
    {
        // Find the closest cell face which is a fault
        double          distance = position.pointDistance( intersection.startPoint );
        const RigFault* fault    = mainGrid->findFaultFromCellIndexAndCellFace( intersection.globCellIndex,
                                                                             intersection.intersectedCellFaceIn );
        if ( fault && distance < shortestDistance )
        {
            foundFault       = fault;
            shortestDistance = distance;
            barrierPosition  = intersection.startPoint;

            const RigCell& cell       = mainGrid->globalCellArray()[intersection.globCellIndex];
            cvf::Vec3d     faceNormal = cell.faceNormalWithAreaLength( intersection.intersectedCellFaceIn );
            barrierDip                = calculateFormationDip( faceNormal );
        }
    }

    if ( foundFault )
    {
        RiaLogging::info( QString( "Found barrier distance: %1. Dip: %2. Fault: %3" )
                              .arg( shortestDistance )
                              .arg( barrierDip )
                              .arg( foundFault->name() ) );
        QString barrierText =
            QString( "Barrier Fault: %1\nDistance: %2m" ).arg( foundFault->name() ).arg( shortestDistance );

        clearBarrierAnnotation();
        addBarrierAnnotation( position, barrierPosition, barrierText );

        m_hasBarrier        = true;
        m_barrierDip        = barrierDip;
        m_distanceToBarrier = shortestDistance;
        m_barrierFaultName  = foundFault->name();
    }
    else
    {
        RiaLogging::info( "No barrier found." );
        clearBarrierAnnotation();
        m_hasBarrier        = false;
        m_barrierDip        = 0.0;
        m_distanceToBarrier = 0.0;
        m_barrierFaultName  = "";
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<WellPathCellIntersectionInfo>
    RimStimPlanModel::generateBarrierIntersections( RigEclipseCaseData* eclipseCaseData,
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
    RimStimPlanModel::generateBarrierIntersectionsBetweenPoints( RigEclipseCaseData* eclipseCaseData,
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
                                                                             "",
                                                                             helper.wellPathPoints(),
                                                                             helper.measuredDepths() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModel::clearBarrierAnnotation()
{
    auto existingAnnotation = m_barrierAnnotation.value();
    if ( existingAnnotation )
    {
        delete existingAnnotation;
        m_barrierAnnotation = nullptr;
    }

    auto existingTextAnnotation = m_barrierTextAnnotation.value();
    if ( existingTextAnnotation )
    {
        delete existingTextAnnotation;
        m_barrierTextAnnotation = nullptr;
    }

    RimAnnotationCollectionBase* coll = annotationCollection();
    if ( coll )
    {
        coll->onAnnotationDeleted();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModel::addBarrierAnnotation( const cvf::Vec3d& startPosition,
                                             const cvf::Vec3d& endPosition,
                                             const QString&    text )
{
    RimAnnotationCollectionBase* coll = annotationCollection();
    if ( !coll ) return;

    {
        auto newAnnotation = new RimUserDefinedPolylinesAnnotation();

        RimPolylineTarget* startTarget = new RimPolylineTarget();
        startTarget->setAsPointXYZ( startPosition );
        newAnnotation->insertTarget( nullptr, startTarget );

        RimPolylineTarget* endTarget = new RimPolylineTarget();
        endTarget->setAsPointXYZ( endPosition );
        newAnnotation->insertTarget( nullptr, endTarget );

        m_barrierAnnotation = newAnnotation;
        dynamic_cast<RimAnnotationCollection*>( coll )->addAnnotation( newAnnotation );
    }

    {
        auto newAnnotation = new RimTextAnnotation();
        newAnnotation->setText( text );
        newAnnotation->setLabelPoint( endPosition );
        newAnnotation->setAnchorPoint( endPosition );

        m_barrierTextAnnotation = newAnnotation;
        coll->addAnnotation( newAnnotation );
    }

    coll->scheduleRedrawOfRelevantViews();
    coll->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnnotationCollectionBase* RimStimPlanModel::annotationCollection()
{
    const auto project  = RimProject::current();
    auto       oilField = project->activeOilField();
    return oilField ? oilField->annotationCollection() : nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModel::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    m_thicknessDirectionWellPath.uiCapability()->setUiHidden( true );
    m_barrierAnnotation.uiCapability()->setUiHidden( true );
    m_barrierTextAnnotation.uiCapability()->setUiHidden( true );
    m_azimuthAngle.uiCapability()->setUiHidden( m_fractureOrientation() != RimStimPlanModel::FractureOrientation::AZIMUTH );

    uiOrdering.add( nameField(), caf::PdmUiOrdering::LayoutOptions( true, 3, 1 ) );
    uiOrdering.add( &m_stimPlanModelTemplate, { true, 2, 1 } );
    uiOrdering.add( &m_editStimPlanModelTemplate, { false, 1, 0 } );

    uiOrdering.add( &m_eclipseCase );
    uiOrdering.add( &m_timeStep );
    uiOrdering.add( &m_MD );
    uiOrdering.add( &m_extractionType );
    uiOrdering.add( &m_anchorPosition );
    uiOrdering.add( &m_thicknessDirection );

    caf::PdmUiOrdering* extractionBoundariesGroup = uiOrdering.addNewGroup( "Extraction Depth Boundaries" );
    extractionBoundariesGroup->add( &m_extractionDepthTop );
    extractionBoundariesGroup->add( &m_extractionDepthBottom );

    caf::PdmUiOrdering* boundingBoxGroup = uiOrdering.addNewGroup( "Bounding Box" );
    boundingBoxGroup->add( &m_boundingBoxHorizontal );
    boundingBoxGroup->add( &m_boundingBoxVertical );

    caf::PdmUiOrdering* detailedFluidLossGroup = uiOrdering.addNewGroup( "Detailed Fluid Loss" );
    detailedFluidLossGroup->add( &m_useDetailedFluidLoss );
    detailedFluidLossGroup->add( &m_relativePermeabilityFactorDefault );
    detailedFluidLossGroup->add( &m_poroElasticConstantDefault );
    detailedFluidLossGroup->add( &m_thermalExpansionCoeffientDefault );

    caf::PdmUiOrdering* perforationGroup = uiOrdering.addNewGroup( "Perforation" );
    perforationGroup->add( &m_perforationLength );
    perforationGroup->add( &m_fractureOrientation );
    perforationGroup->add( &m_azimuthAngle );

    caf::PdmUiOrdering* asymmetricGroup = uiOrdering.addNewGroup( "Asymmetric" );
    asymmetricGroup->add( &m_formationDip );
    asymmetricGroup->add( &m_hasBarrier );
    asymmetricGroup->add( &m_autoComputeBarrier );
    asymmetricGroup->add( &m_distanceToBarrier );
    asymmetricGroup->add( &m_barrierDip );
    asymmetricGroup->add( &m_barrierFaultName );

    asymmetricGroup->add( &m_showOnlyBarrierFault, caf::PdmUiOrdering::LayoutOptions( true, 2, 1 ) );
    asymmetricGroup->add( &m_showAllFaults, { false, 1, 0 } );

    asymmetricGroup->add( &m_wellPenetrationLayer );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModel::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                              QString                    uiConfigName,
                                              caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_formationDip || field == &m_barrierDip || field == &m_distanceToBarrier )
    {
        auto doubleAttr = dynamic_cast<caf::PdmUiDoubleValueEditorAttribute*>( attribute );
        if ( doubleAttr )
        {
            doubleAttr->m_decimals     = 2;
            doubleAttr->m_numberFormat = caf::PdmUiDoubleValueEditorAttribute::NumberFormat::FIXED;
        }
    }

    if ( field == &m_MD )
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute );

        if ( myAttr )
        {
            RimWellPath* wellPath = nullptr;
            this->firstAncestorOrThisOfType( wellPath );
            if ( !wellPath ) return;

            myAttr->m_minimum = wellPath->startMD();
            myAttr->m_maximum = wellPath->endMD();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RimStimPlanModel::wellPath() const
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
RimModeledWellPath* RimStimPlanModel::thicknessDirectionWellPath() const
{
    return m_thicknessDirectionWellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModel::setThicknessDirectionWellPath( RimModeledWellPath* thicknessDirectionWellPath )
{
    m_thicknessDirectionWellPath = thicknessDirectionWellPath;
    updateThicknessDirectionWellPathName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModel::updateThicknessDirectionWellPathName()
{
    QString wellNameFormat( "%1 for %2" );
    m_thicknessDirectionWellPath()->setName( wellNameFormat.arg( m_extractionType().text() ).arg( name() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStimPlanModel::vecToString( const cvf::Vec3d& vec )
{
    return QString( "[%1, %2, %3]" ).arg( vec.x() ).arg( vec.y() ).arg( vec.z() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanModel::findThicknessTargetPoints( cvf::Vec3d& topPosition, cvf::Vec3d& bottomPosition )
{
    RigEclipseCaseData* eclipseCaseData = getEclipseCaseData();
    if ( !eclipseCaseData ) return false;

    const cvf::Vec3d& position  = anchorPosition();
    const cvf::Vec3d& direction = thicknessDirection();

    RiaLogging::info( QString( "Position:  %1" ).arg( RimStimPlanModel::vecToString( position ) ) );
    RiaLogging::info( QString( "Direction: %1" ).arg( RimStimPlanModel::vecToString( direction ) ) );

    // Create a "fake" well path which from top to bottom of formation
    // passing through the point and with the given direction

    const cvf::BoundingBox& allCellsBoundingBox = eclipseCaseData->mainGrid()->boundingBox();

    RiaLogging::info( QString( "All cells bounding box: %1 %2" )
                          .arg( RimStimPlanModel::vecToString( allCellsBoundingBox.min() ) )
                          .arg( RimStimPlanModel::vecToString( allCellsBoundingBox.max() ) ) );
    cvf::BoundingBox geometryBoundingBox( allCellsBoundingBox );

    // Use smaller depth bounding box for extraction if configured
    if ( m_extractionDepthTop > 0.0 && m_extractionDepthBottom > 0.0 && m_extractionDepthTop < m_extractionDepthBottom )
    {
        cvf::Vec3d bbMin( allCellsBoundingBox.min().x(), allCellsBoundingBox.min().y(), -m_extractionDepthBottom );
        cvf::Vec3d bbMax( allCellsBoundingBox.max().x(), allCellsBoundingBox.max().y(), -m_extractionDepthTop );
        geometryBoundingBox = cvf::BoundingBox( bbMin, bbMax );
    }

    if ( !geometryBoundingBox.contains( position ) )
    {
        //
        RiaLogging::error( "Anchor position is outside the grid bounding box. Unable to compute direction." );
        return false;
    }

    // Create plane on top and bottom of formation
    cvf::Plane topPlane;
    topPlane.setFromPointAndNormal( geometryBoundingBox.max(), cvf::Vec3d::Z_AXIS );

    cvf::Plane bottomPlane;
    bottomPlane.setFromPointAndNormal( geometryBoundingBox.min(), cvf::Vec3d::Z_AXIS );

    // Find and add point on top plane
    cvf::Vec3d abovePlane = position + ( direction * -10000.0 );
    if ( !topPlane.intersect( position, abovePlane, &topPosition ) )
    {
        RiaLogging::error( "Unable to compute top position of thickness direction vector." );
        return false;
    }

    RiaLogging::info( QString( "Top: %1" ).arg( RimStimPlanModel::vecToString( topPosition ) ) );

    // Find and add point on bottom plane
    cvf::Vec3d belowPlane = position + ( direction * 10000.0 );
    if ( !bottomPlane.intersect( position, belowPlane, &bottomPosition ) )
    {
        RiaLogging::error( "Unable to compute bottom position of thickness direction vector." );
        return false;
    }

    RiaLogging::info( QString( "Bottom: %1" ).arg( RimStimPlanModel::vecToString( bottomPosition ) ) );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModel::calculateFormationDip( const cvf::Vec3d& direction )
{
    // Formation dip is inclination of a plane from horizontal.
    return cvf::Math::toDegrees( cvf::GeometryTools::getAngle( direction, -cvf::Vec3d::Z_AXIS ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModel::loadDataAndUpdate()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModel::defaultPorosity() const
{
    return m_stimPlanModelTemplate() ? m_stimPlanModelTemplate()->defaultPorosity() : 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModel::defaultPermeability() const
{
    return m_stimPlanModelTemplate() ? m_stimPlanModelTemplate()->defaultPermeability() : 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModel::getDefaultForMissingValue( RiaDefines::CurveProperty curveProperty ) const
{
    if ( curveProperty == RiaDefines::CurveProperty::POROSITY ||
         curveProperty == RiaDefines::CurveProperty::POROSITY_UNSCALED )
    {
        return defaultPorosity();
    }
    else if ( curveProperty == RiaDefines::CurveProperty::PERMEABILITY_X ||
              curveProperty == RiaDefines::CurveProperty::PERMEABILITY_Z )
    {
        return defaultPermeability();
    }
    else if ( curveProperty == RiaDefines::CurveProperty::NET_TO_GROSS )
    {
        return 1.0;
    }
    else
    {
        RiaLogging::error( QString( "Missing default value for %1." )
                               .arg( caf::AppEnum<RiaDefines::CurveProperty>( curveProperty ).uiText() ) );
        return std::numeric_limits<double>::infinity();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::CurveProperty RimStimPlanModel::getDefaultPropertyForMissingValues( RiaDefines::CurveProperty curveProperty ) const
{
    if ( curveProperty == RiaDefines::CurveProperty::PRESSURE )
    {
        return RiaDefines::CurveProperty::INITIAL_PRESSURE;
    }

    return RiaDefines::CurveProperty::UNDEFINED;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModel::getDefaultForMissingOverburdenValue( RiaDefines::CurveProperty curveProperty ) const
{
    if ( curveProperty == RiaDefines::CurveProperty::POROSITY ||
         curveProperty == RiaDefines::CurveProperty::POROSITY_UNSCALED )
    {
        return defaultOverburdenPorosity();
    }
    else if ( curveProperty == RiaDefines::CurveProperty::PERMEABILITY_X ||
              curveProperty == RiaDefines::CurveProperty::PERMEABILITY_Z )
    {
        return defaultOverburdenPermeability();
    }
    else if ( curveProperty == RiaDefines::CurveProperty::FACIES )
    {
        RimColorLegend* faciesColorLegend = getFaciesColorLegend();
        if ( !faciesColorLegend ) return std::numeric_limits<double>::infinity();
        return findFaciesValue( *faciesColorLegend, overburdenFacies() );
    }
    else if ( curveProperty == RiaDefines::CurveProperty::NET_TO_GROSS )
    {
        return 1.0;
    }
    else if ( curveProperty == RiaDefines::CurveProperty::PRESSURE )
    {
        return std::numeric_limits<double>::infinity();
    }
    else
    {
        RiaLogging::error( QString( "Missing default overburden value for %1." )
                               .arg( caf::AppEnum<RiaDefines::CurveProperty>( curveProperty ).uiText() ) );
        return std::numeric_limits<double>::infinity();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModel::getDefaultForMissingUnderburdenValue( RiaDefines::CurveProperty curveProperty ) const
{
    if ( curveProperty == RiaDefines::CurveProperty::POROSITY ||
         curveProperty == RiaDefines::CurveProperty::POROSITY_UNSCALED )
    {
        return defaultUnderburdenPorosity();
    }
    else if ( curveProperty == RiaDefines::CurveProperty::PERMEABILITY_X ||
              curveProperty == RiaDefines::CurveProperty::PERMEABILITY_Z )
    {
        return defaultUnderburdenPermeability();
    }
    else if ( curveProperty == RiaDefines::CurveProperty::FACIES )
    {
        RimColorLegend* faciesColorLegend = getFaciesColorLegend();
        if ( !faciesColorLegend ) return std::numeric_limits<double>::infinity();
        return findFaciesValue( *faciesColorLegend, underburdenFacies() );
    }
    else if ( curveProperty == RiaDefines::CurveProperty::NET_TO_GROSS )
    {
        return 1.0;
    }
    else if ( curveProperty == RiaDefines::CurveProperty::PRESSURE )
    {
        return std::numeric_limits<double>::infinity();
    }
    else
    {
        RiaLogging::error( QString( "Missing default underburden value for %1." )
                               .arg( caf::AppEnum<RiaDefines::CurveProperty>( curveProperty ).uiText() ) );
        return std::numeric_limits<double>::infinity();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModel::getOverburdenGradient( RiaDefines::CurveProperty curveProperty ) const
{
    if ( curveProperty == RiaDefines::CurveProperty::PRESSURE ||
         curveProperty == RiaDefines::CurveProperty::INITIAL_PRESSURE )
    {
        if ( !m_stimPlanModelTemplate )
        {
            return 0.0;
        }
        return m_stimPlanModelTemplate()->overburdenFluidDensity() * 9.81 * 1000.0 / 1.0e5;
    }
    else
    {
        RiaLogging::error( QString( "Missing overburden gradient for %1." )
                               .arg( caf::AppEnum<RiaDefines::CurveProperty>( curveProperty ).uiText() ) );
        return std::numeric_limits<double>::infinity();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModel::getUnderburdenGradient( RiaDefines::CurveProperty curveProperty ) const
{
    if ( curveProperty == RiaDefines::CurveProperty::PRESSURE ||
         curveProperty == RiaDefines::CurveProperty::INITIAL_PRESSURE )
    {
        if ( !m_stimPlanModelTemplate )
        {
            return 0.0;
        }

        return m_stimPlanModelTemplate()->underburdenFluidDensity() * 9.81 * 1000.0 / 1.0e5;
    }
    else
    {
        RiaLogging::error( QString( "Missing underburden gradient for %1." )
                               .arg( caf::AppEnum<RiaDefines::CurveProperty>( curveProperty ).uiText() ) );
        return std::numeric_limits<double>::infinity();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModel::getDefaultValueForProperty( RiaDefines::CurveProperty curveProperty ) const
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
RimStimPlanModel::MissingValueStrategy RimStimPlanModel::missingValueStrategy( RiaDefines::CurveProperty curveProperty ) const
{
    if ( curveProperty == RiaDefines::CurveProperty::INITIAL_PRESSURE )
        return RimStimPlanModel::MissingValueStrategy::LINEAR_INTERPOLATION;
    else if ( curveProperty == RiaDefines::CurveProperty::PRESSURE )
        return RimStimPlanModel::MissingValueStrategy::OTHER_CURVE_PROPERTY;
    else
        return RimStimPlanModel::MissingValueStrategy::DEFAULT_VALUE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModel::BurdenStrategy RimStimPlanModel::burdenStrategy( RiaDefines::CurveProperty curveProperty ) const
{
    if ( curveProperty == RiaDefines::CurveProperty::INITIAL_PRESSURE )
        return RimStimPlanModel::BurdenStrategy::GRADIENT;

    return RimStimPlanModel::BurdenStrategy::DEFAULT_VALUE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanModel::hasDefaultValueForProperty( RiaDefines::CurveProperty curveProperty ) const
{
    auto withDefaults = { RiaDefines::CurveProperty::RELATIVE_PERMEABILITY_FACTOR,
                          RiaDefines::CurveProperty::PORO_ELASTIC_CONSTANT,
                          RiaDefines::CurveProperty::THERMAL_EXPANSION_COEFFICIENT };
    return std::find( withDefaults.begin(), withDefaults.end(), curveProperty ) != withDefaults.end();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModel::verticalStress() const
{
    return m_stimPlanModelTemplate() ? m_stimPlanModelTemplate()->verticalStress() : 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModel::verticalStressGradient() const
{
    return m_stimPlanModelTemplate() ? m_stimPlanModelTemplate()->verticalStressGradient() : 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModel::stressDepth() const
{
    return m_stimPlanModelTemplate() ? m_stimPlanModelTemplate()->stressDepth() : 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModel::overburdenHeight() const
{
    return m_stimPlanModelTemplate() ? m_stimPlanModelTemplate()->overburdenHeight() : 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModel::underburdenHeight() const
{
    return m_stimPlanModelTemplate() ? m_stimPlanModelTemplate()->underburdenHeight() : 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModel::defaultOverburdenPorosity() const
{
    return m_stimPlanModelTemplate() ? m_stimPlanModelTemplate()->defaultOverburdenPorosity() : 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModel::defaultUnderburdenPorosity() const
{
    return m_stimPlanModelTemplate() ? m_stimPlanModelTemplate()->defaultUnderburdenPorosity() : 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModel::defaultOverburdenPermeability() const
{
    return m_stimPlanModelTemplate() ? m_stimPlanModelTemplate()->defaultOverburdenPermeability() : 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModel::defaultUnderburdenPermeability() const
{
    return m_stimPlanModelTemplate() ? m_stimPlanModelTemplate()->defaultUnderburdenPermeability() : 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStimPlanModel::overburdenFormation() const
{
    return m_stimPlanModelTemplate() ? m_stimPlanModelTemplate()->overburdenFormation() : "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStimPlanModel::overburdenFacies() const
{
    return m_stimPlanModelTemplate() ? m_stimPlanModelTemplate()->overburdenFacies() : "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStimPlanModel::underburdenFormation() const
{
    return m_stimPlanModelTemplate() ? m_stimPlanModelTemplate()->underburdenFormation() : "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStimPlanModel::underburdenFacies() const
{
    return m_stimPlanModelTemplate() ? m_stimPlanModelTemplate()->underburdenFacies() : "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModel::updateReferringPlots()
{
    // Update plots referring to this fracture model
    std::vector<RimStimPlanModelPlot*> referringObjects;
    objectsWithReferringPtrFieldsOfType( referringObjects );

    for ( auto modelPlot : referringObjects )
    {
        if ( modelPlot ) modelPlot->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModel::setMD( double md )
{
    m_MD = md;
    updatePositionFromMeasuredDepth();
    updateThicknessDirection();
    updateBarrierProperties();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModel::setEclipseCaseAndTimeStep( RimEclipseCase* eclipseCase, int timeStep )
{
    setEclipseCase( eclipseCase );
    setTimeStep( timeStep );
    updateThicknessDirection();
    updateBarrierProperties();
    updateViewsAndPlots();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimStimPlanModel::timeStep() const
{
    return m_timeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModel::setTimeStep( int timeStep )
{
    m_timeStep = timeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModel::referenceTemperature() const
{
    return m_stimPlanModelTemplate() ? m_stimPlanModelTemplate()->referenceTemperature() : 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModel::referenceTemperatureGradient() const
{
    return m_stimPlanModelTemplate() ? m_stimPlanModelTemplate()->referenceTemperatureGradient() : 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModel::referenceTemperatureDepth() const
{
    return m_stimPlanModelTemplate() ? m_stimPlanModelTemplate()->referenceTemperatureDepth() : 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimStimPlanModel::eclipseCase() const
{
    return m_eclipseCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModel::setEclipseCase( RimEclipseCase* eclipseCase )
{
    m_eclipseCase = eclipseCase;
    updateExtractionDepthBoundaries();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseCaseData* RimStimPlanModel::getEclipseCaseData() const
{
    // Find an eclipse case
    RimEclipseCase* eclCase = eclipseCase();
    if ( !eclCase ) return nullptr;

    return eclCase->eclipseCaseData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModel::perforationLength() const
{
    return m_perforationLength();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModel::FractureOrientation RimStimPlanModel::fractureOrientation() const
{
    return m_fractureOrientation();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModel::formationDip() const
{
    return m_formationDip;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanModel::hasBarrier() const
{
    return m_hasBarrier;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModel::distanceToBarrier() const
{
    return m_distanceToBarrier;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModel::barrierDip() const
{
    return m_barrierDip;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimStimPlanModel::wellPenetrationLayer() const
{
    return m_wellPenetrationLayer;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModel::setStimPlanModelTemplate( RimStimPlanModelTemplate* stimPlanModelTemplate )
{
    if ( m_stimPlanModelTemplate )
    {
        m_stimPlanModelTemplate->changed.disconnect( this );
    }

    m_stimPlanModelTemplate = stimPlanModelTemplate;

    if ( m_stimPlanModelTemplate )
    {
        m_stimPlanModelTemplate->changed.connect( this, &RimStimPlanModel::stimPlanModelTemplateChanged );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModelTemplate* RimStimPlanModel::stimPlanModelTemplate() const
{
    return m_stimPlanModelTemplate;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModel::stimPlanModelTemplateChanged( const caf::SignalEmitter* emitter )
{
    updateViewsAndPlots();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModel::updateViewsAndPlots()
{
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

    RimProject::current()->scheduleCreateDisplayModelAndRedrawAllViews();

    updateReferringPlots();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModel::hideOtherFaults( const QString& targetFaultName )
{
    RimEclipseView* view = dynamic_cast<RimEclipseView*>( RiaApplication::instance()->activeReservoirView() );
    if ( !view ) return;

    RimFaultInViewCollection* faultCollection = view->faultCollection();
    if ( !faultCollection ) return;

    for ( RimFaultInView* rimFault : faultCollection->faults() )
    {
        rimFault->showFault = ( rimFault->name() == targetFaultName );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModel::showAllFaults()
{
    RimEclipseView* view = dynamic_cast<RimEclipseView*>( RiaApplication::instance()->activeReservoirView() );
    if ( !view ) return;

    RimFaultInViewCollection* faultCollection = view->faultCollection();
    if ( !faultCollection ) return;

    for ( RimFaultInView* rimFault : faultCollection->faults() )
    {
        rimFault->showFault = true;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<RimStimPlanModelCalculator> RimStimPlanModel::calculator() const
{
    return m_calculator;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::ResultCatType RimStimPlanModel::eclipseResultCategory( RiaDefines::CurveProperty curveProperty ) const
{
    if ( curveProperty == RiaDefines::CurveProperty::PRESSURE ||
         curveProperty == RiaDefines::CurveProperty::INITIAL_PRESSURE )
    {
        return RiaDefines::ResultCatType::DYNAMIC_NATIVE;
    }
    else if ( curveProperty == RiaDefines::CurveProperty::FACIES )
    {
        if ( !m_stimPlanModelTemplate ) return RiaDefines::ResultCatType::STATIC_NATIVE;

        RimFaciesProperties* faciesProperties = m_stimPlanModelTemplate->faciesProperties();
        if ( !faciesProperties ) return RiaDefines::ResultCatType::STATIC_NATIVE;

        const RimEclipseResultDefinition* faciesDefinition = faciesProperties->faciesDefinition();
        if ( !faciesDefinition ) return RiaDefines::ResultCatType::STATIC_NATIVE;

        return faciesDefinition->resultType();
    }
    else if ( curveProperty == RiaDefines::CurveProperty::NET_TO_GROSS )
    {
        if ( !m_stimPlanModelTemplate ) return RiaDefines::ResultCatType::STATIC_NATIVE;

        RimNonNetLayers* nonNetLayers = m_stimPlanModelTemplate->nonNetLayers();
        if ( !nonNetLayers ) return RiaDefines::ResultCatType::STATIC_NATIVE;

        const RimEclipseResultDefinition* resultDef = nonNetLayers->resultDefinition();
        if ( !resultDef ) return RiaDefines::ResultCatType::STATIC_NATIVE;

        return resultDef->resultType();
    }
    else
    {
        return RiaDefines::ResultCatType::STATIC_NATIVE;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStimPlanModel::eclipseResultVariable( RiaDefines::CurveProperty curveProperty ) const
{
    if ( curveProperty == RiaDefines::CurveProperty::PRESSURE ||
         curveProperty == RiaDefines::CurveProperty::INITIAL_PRESSURE )
        return "PRESSURE";
    else if ( curveProperty == RiaDefines::CurveProperty::PERMEABILITY_X )
        return "PERMX";
    else if ( curveProperty == RiaDefines::CurveProperty::PERMEABILITY_Z )
        return "PERMZ";
    else if ( curveProperty == RiaDefines::CurveProperty::POROSITY ||
              curveProperty == RiaDefines::CurveProperty::POROSITY_UNSCALED )
        return "PORO";
    else if ( curveProperty == RiaDefines::CurveProperty::FACIES )
    {
        if ( !m_stimPlanModelTemplate ) return "";

        RimFaciesProperties* faciesProperties = m_stimPlanModelTemplate->faciesProperties();
        if ( !faciesProperties ) return "";

        const RimEclipseResultDefinition* faciesDefinition = faciesProperties->faciesDefinition();
        if ( !faciesDefinition ) return "";

        return faciesDefinition->resultVariable();
    }
    else if ( curveProperty == RiaDefines::CurveProperty::NET_TO_GROSS )
    {
        if ( !m_stimPlanModelTemplate ) return "";

        RimNonNetLayers* nonNetLayers = m_stimPlanModelTemplate->nonNetLayers();
        if ( !nonNetLayers ) return "";

        const RimEclipseResultDefinition* resultDef = nonNetLayers->resultDefinition();
        if ( !resultDef ) return "";

        return resultDef->resultVariable();
    }
    else
        return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimColorLegend* RimStimPlanModel::getFaciesColorLegend() const
{
    if ( !m_stimPlanModelTemplate ) return nullptr;

    RimFaciesProperties* faciesProperties = m_stimPlanModelTemplate->faciesProperties();
    if ( !faciesProperties ) return nullptr;

    return faciesProperties->colorLegend();
}

//-------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModel::findFaciesValue( const RimColorLegend& colorLegend, const QString& name )
{
    for ( auto item : colorLegend.colorLegendItems() )
    {
        if ( item->categoryName() == name ) return item->categoryValue();
    }

    return std::numeric_limits<double>::infinity();
}

//-------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimStimPlanModel::isScaledByNetToGross( RiaDefines::CurveProperty curveProperty ) const
{
    std::vector<RiaDefines::CurveProperty> matching = { RiaDefines::CurveProperty::POROSITY,
                                                        RiaDefines::CurveProperty::PERMEABILITY_X,
                                                        RiaDefines::CurveProperty::PERMEABILITY_Z,
                                                        RiaDefines::CurveProperty::YOUNGS_MODULUS,
                                                        RiaDefines::CurveProperty::POISSONS_RATIO,
                                                        RiaDefines::CurveProperty::BIOT_COEFFICIENT,
                                                        RiaDefines::CurveProperty::K0,
                                                        RiaDefines::CurveProperty::K_IC,
                                                        RiaDefines::CurveProperty::PROPPANT_EMBEDMENT,
                                                        RiaDefines::CurveProperty::FLUID_LOSS_COEFFICIENT,
                                                        RiaDefines::CurveProperty::SPURT_LOSS,
                                                        RiaDefines::CurveProperty::RELATIVE_PERMEABILITY_FACTOR,
                                                        RiaDefines::CurveProperty::PORO_ELASTIC_CONSTANT,
                                                        RiaDefines::CurveProperty::THERMAL_EXPANSION_COEFFICIENT,
                                                        RiaDefines::CurveProperty::IMMOBILE_FLUID_SATURATION };

    return std::find( matching.begin(), matching.end(), curveProperty ) != matching.end();
}
