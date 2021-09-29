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
#include "RiaFractureDefines.h"
#include "RiaLogging.h"
#include "RiaStimPlanModelDefines.h"

#include "RigEclipseCaseData.h"
#include "RigFault.h"
#include "RigMainGrid.h"
#include "RigStimPlanModelTools.h"
#include "RigWellPath.h"

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
#include "RimExtractionConfiguration.h"
#include "RimFaciesProperties.h"
#include "RimFaultInView.h"
#include "RimFaultInViewCollection.h"
#include "RimModeledWellPath.h"
#include "RimNonNetLayers.h"
#include "RimOilField.h"
#include "RimPolylineTarget.h"
#include "RimPressureTable.h"
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
#include "cvfMath.h"
#include "cvfPlane.h"

#include <cmath>
#include <limits>

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

    CAF_PDM_InitScriptableFieldNoDefault( &m_eclipseCase, "EclipseCase", "Dynamic Case", "", "", "" );
    m_eclipseCase.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitScriptableField( &m_timeStep, "TimeStep", 0, "Time Step", "", "", "" );
    m_timeStep.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitScriptableFieldNoDefault( &m_initialPressureEclipseCase,
                                          "InitialPressureEclipseCase",
                                          "Initial Pressure Case",
                                          "",
                                          "",
                                          "" );
    m_initialPressureEclipseCase.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitScriptableFieldNoDefault( &m_staticEclipseCase, "StaticEclipseCase", "Static Case", "", "", "" );
    m_staticEclipseCase.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitScriptableField( &m_MD, "MeasuredDepth", 0.0, "Measured Depth", "", "", "" );
    m_MD.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );

    CAF_PDM_InitScriptableField( &m_extractionOffsetTop, "ExtractionOffsetTop", -1.0, "Top Offset", "", "", "" );
    m_extractionOffsetTop.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

    CAF_PDM_InitScriptableField( &m_extractionOffsetBottom, "ExtractionOffsetBottom", -1.0, "Bottom Offset", "", "", "" );
    m_extractionOffsetBottom.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );

    CAF_PDM_InitScriptableField( &m_extractionDepthTop, "ExtractionDepthTop", -1.0, "Depth", "", "", "" );
    m_extractionDepthTop.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );
    m_extractionDepthTop.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitScriptableField( &m_extractionDepthBottom, "ExtractionDepthBottom", -1.0, "Depth", "", "", "" );
    m_extractionDepthBottom.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );
    m_extractionDepthBottom.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitScriptableField( &m_extractionType,
                                 "ExtractionType",
                                 caf::AppEnum<ExtractionType>( ExtractionType::TRUE_STRATIGRAPHIC_THICKNESS ),
                                 "Extraction Type",
                                 "",
                                 "",
                                 "" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_anchorPosition, "AnchorPosition", "Anchor Position", "", "", "" );
    m_anchorPosition.uiCapability()->setUiReadOnly( true );
    m_anchorPosition.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_anchorPositionForUi, "AnchorPositionForUi", "Anchor Position", "", "", "" );
    m_anchorPositionForUi.registerGetMethod( this, &RimStimPlanModel::anchorPositionForUi );
    m_anchorPositionForUi.uiCapability()->setUiReadOnly( true );
    m_anchorPositionForUi.xmlCapability()->disableIO();

    CAF_PDM_InitScriptableFieldNoDefault( &m_thicknessDirection, "ThicknessDirection", "Thickness Direction", "", "", "" );
    m_thicknessDirection.uiCapability()->setUiReadOnly( true );
    m_thicknessDirection.xmlCapability()->disableIO();

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
        updateExtractionDepthBoundaries();
    }

    if ( changedField == &m_extractionOffsetTop || changedField == &m_extractionOffsetBottom )
    {
        updateExtractionDepthBoundaries();
    }

    if ( changedField == &m_MD || changedField == &m_extractionType || changedField == &m_boundingBoxVertical ||
         changedField == &m_boundingBoxHorizontal || changedField == &m_fractureOrientation ||
         changedField == &m_autoComputeBarrier || changedField == &m_azimuthAngle ||
         changedField == &m_showOnlyBarrierFault || changedField == &m_eclipseCase ||
         changedField == &m_extractionDepthTop || changedField == &m_extractionDepthBottom ||
         changedField == &m_extractionOffsetTop || changedField == &m_extractionOffsetBottom )
    {
        updateThicknessDirection();
        updateBarrierProperties();
    }

    if ( changedField == &m_eclipseCase )
    {
        if ( m_eclipseCase )
        {
            // Set a valid default time step
            const int timeStepCount = m_eclipseCase->timeStepStrings().size();
            if ( timeStepCount > 0 )
            {
                m_timeStep = timeStepCount - 1;
            }
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
        m_calculator->clearCache();
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
    else if ( fieldNeedingOptions == &m_eclipseCase || fieldNeedingOptions == &m_staticEclipseCase ||
              fieldNeedingOptions == &m_initialPressureEclipseCase )
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
void RimStimPlanModel::applyOffset( double offsetMD )
{
    // Nothing to do here, this operation is inteded for well path completions
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
cvf::Vec3d RimStimPlanModel::anchorPositionForUi() const
{
    cvf::Vec3d v = m_anchorPosition;
    v.z()        = -v.z();
    return v;
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

    RimWellPath* wp = wellPath();
    if ( wp && wp->wellPathGeometry() )
    {
        positionAlongWellpath = wp->wellPathGeometry()->interpolatedPointAlongWellPath( m_MD() );
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
        direction = RigStimPlanModelTools::calculateTSTDirection( getEclipseCaseData(),
                                                                  m_anchorPosition(),
                                                                  m_boundingBoxHorizontal,
                                                                  m_boundingBoxVertical );
    }

    m_thicknessDirection = direction;

    if ( m_thicknessDirectionWellPath )
    {
        RimWellPathGeometryDef* wellGeomDef = m_thicknessDirectionWellPath->geometryDefinition();
        wellGeomDef->deleteAllTargets();
        // Disable auto-generated target at sea level: this will usually be outside the
        // bounding box, and will introduce problems when the project is reloaded.
        wellGeomDef->setUseAutoGeneratedTargetAtSeaLevel( false );

        cvf::Vec3d topPosition;
        cvf::Vec3d bottomPosition;

        if ( RigStimPlanModelTools::findThicknessTargetPoints( getEclipseCaseData(),
                                                               m_anchorPosition(),
                                                               m_thicknessDirection(),
                                                               m_extractionDepthTop(),
                                                               m_extractionDepthBottom(),
                                                               topPosition,
                                                               bottomPosition ) )
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
void RimStimPlanModel::updateExtractionDepthBoundaries()
{
    RigEclipseCaseData* eclipseCaseData = getEclipseCaseData();
    if ( eclipseCaseData )
    {
        const cvf::BoundingBox& boundingBox = eclipseCaseData->mainGrid()->boundingBox();

        double depth = -m_anchorPosition().z();
        if ( m_extractionOffsetTop() < 0.0 ) m_extractionOffsetTop = boundingBox.extent().z();
        if ( m_extractionOffsetBottom() < 0.0 ) m_extractionOffsetBottom = boundingBox.extent().z();

        m_extractionDepthTop    = std::max( depth - m_extractionOffsetTop, -boundingBox.max().z() );
        m_extractionDepthBottom = std::min( depth + m_extractionOffsetBottom, -boundingBox.min().z() );
        updateConnectedEditors();
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
    if ( !wellPath() ) return;

    RigEclipseCaseData* eclipseCaseData = getEclipseCaseData();
    if ( !eclipseCaseData ) return;

    const cvf::Vec3d& position = anchorPosition();

    RiaLogging::info( "Computing distance to barrier." );
    RiaLogging::info( QString( "Anchor position: %1" ).arg( RigStimPlanModelTools::vecToString( position ) ) );

    RigWellPath* wellPathGeometry = wellPath()->wellPathGeometry();

    // Find the well path points closest to the anchor position
    cvf::Vec3d p1;
    cvf::Vec3d p2;
    wellPathGeometry->twoClosestPoints( position, &p1, &p2 );
    RiaLogging::info( QString( "Closest points on well path: %1 %2" )
                          .arg( RigStimPlanModelTools::vecToString( p1 ) )
                          .arg( RigStimPlanModelTools::vecToString( p2 ) ) );

    // Create a well direction based on the two points
    cvf::Vec3d wellDirection = ( p2 - p1 ).getNormalized();
    RiaLogging::info( QString( "Well direction: %1" ).arg( RigStimPlanModelTools::vecToString( wellDirection ) ) );

    cvf::Vec3d fractureDirectionNormal = wellDirection;
    if ( m_fractureOrientation == FractureOrientation::ALONG_WELL_PATH )
    {
        cvf::Mat3d azimuthRotation = cvf::Mat3d::fromRotation( cvf::Vec3d::Z_AXIS, cvf::Math::toRadians( 90.0 ) );
        fractureDirectionNormal.transformVector( azimuthRotation );
    }
    else if ( m_fractureOrientation == FractureOrientation::AZIMUTH )
    {
        // Azimuth angle of fracture is relative to north.
        double     wellAzimuth = wellPathGeometry->wellPathAzimuthAngle( position );
        cvf::Mat3d azimuthRotation =
            cvf::Mat3d::fromRotation( cvf::Vec3d::Z_AXIS, cvf::Math::toRadians( wellAzimuth - m_azimuthAngle() - 90.0 ) );
        fractureDirectionNormal.transformVector( azimuthRotation );
    }

    // Create a fracture plane
    cvf::Plane fracturePlane;
    if ( !fracturePlane.setFromPointAndNormal( position, fractureDirectionNormal ) )
    {
        RiaLogging::error( "Unable to create fracture plane" );
        return;
    }

    // The direction to the barrier must be in the fracture plane.
    // Project the TST onto the fracture plane.
    cvf::Vec3d tstInPlane;
    if ( !fracturePlane.projectVector( thicknessDirection(), &tstInPlane ) )
    {
        RiaLogging::error( "Unable to project thickess vector into fracture plane" );
        return;
    }
    RiaLogging::info(
        QString( "Thickness direction: %1" ).arg( RigStimPlanModelTools::vecToString( thicknessDirection() ) ) );
    RiaLogging::info(
        QString( "Thickness direction in fracture plane: %1" ).arg( RigStimPlanModelTools::vecToString( tstInPlane ) ) );

    // The direction to the barrier is normal to the TST project into the fracture plane
    cvf::Vec3d directionToBarrier = ( tstInPlane ^ fractureDirectionNormal ).getNormalized();
    RiaLogging::info(
        QString( "Direction to barrier: %1" ).arg( RigStimPlanModelTools::vecToString( directionToBarrier ) ) );

    // Update formation dip. The direction for the barrier search follows the
    // inclination of the formation, and is in effect the formation dip in the
    // fracture plane. -90 to convert from horizontal to vertical.
    m_formationDip = std::abs( RigStimPlanModelTools::calculateFormationDip( directionToBarrier ) - 90.0 );

    auto [foundFault, shortestDistance, barrierPosition, barrierDip] =
        RigStimPlanModelTools::findClosestFaultBarrier( eclipseCaseData, position, directionToBarrier );

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

    uiOrdering.add( &m_staticEclipseCase );

    caf::PdmUiOrdering* pressureDataSourceGroup = uiOrdering.addNewGroup( "Pressure Data Source" );
    pressureDataSourceGroup->add( &m_eclipseCase );
    pressureDataSourceGroup->add( &m_timeStep );
    pressureDataSourceGroup->add( &m_initialPressureEclipseCase );

    uiOrdering.add( &m_MD );
    uiOrdering.add( &m_extractionType );
    uiOrdering.add( &m_anchorPositionForUi );
    uiOrdering.add( &m_thicknessDirection );

    caf::PdmUiOrdering* extractionBoundariesGroup = uiOrdering.addNewGroup( "Extraction Depth Boundaries" );
    extractionBoundariesGroup->add( &m_extractionOffsetTop, caf::PdmUiOrdering::LayoutOptions( true, 3, 1 ) );
    extractionBoundariesGroup->add( &m_extractionDepthTop, { false, 2, 1 } );

    extractionBoundariesGroup->add( &m_extractionOffsetBottom, caf::PdmUiOrdering::LayoutOptions( true, 3, 1 ) );
    extractionBoundariesGroup->add( &m_extractionDepthBottom, { false, 2, 1 } );

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
            if ( !wellPath() ) return;

            myAttr->m_minimum = wellPath()->startMD();
            myAttr->m_maximum = wellPath()->endMD();
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
void RimStimPlanModel::loadDataAndUpdate()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModel::resetAnchorPositionAndThicknessDirection()
{
    // Always recompute thickness direction as MD in project file might have been changed
    updatePositionFromMeasuredDepth();
    updateExtractionDepthBoundaries();
    updateThicknessDirection();
    updateBarrierProperties();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModel::defaultPorosity() const
{
    return m_stimPlanModelTemplate() ? m_stimPlanModelTemplate()->defaultPorosity() : RiaDefines::defaultPorosity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimStimPlanModel::defaultPermeability() const
{
    return m_stimPlanModelTemplate() ? m_stimPlanModelTemplate()->defaultPermeability()
                                     : RiaDefines::defaultPermeability();
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
    else
    {
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
    else
    {
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
std::deque<RimStimPlanModel::MissingValueStrategy>
    RimStimPlanModel::missingValueStrategies( RiaDefines::CurveProperty curveProperty ) const
{
    if ( curveProperty == RiaDefines::CurveProperty::PRESSURE )
        return { RimStimPlanModel::MissingValueStrategy::OTHER_CURVE_PROPERTY };
    else if ( curveProperty == RiaDefines::CurveProperty::EQLNUM )
        return { RimStimPlanModel::MissingValueStrategy::DEFAULT_VALUE,
                 RimStimPlanModel::MissingValueStrategy::CELLS_ABOVE,
                 RimStimPlanModel::MissingValueStrategy::CELLS_BELOW,
                 RimStimPlanModel::MissingValueStrategy::LINEAR_INTERPOLATION };
    else
        return { RimStimPlanModel::MissingValueStrategy::DEFAULT_VALUE };
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
    updateExtractionDepthBoundaries();
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
    updateExtractionDepthBoundaries();
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
bool RimStimPlanModel::useStaticEclipseCase( RiaDefines::CurveProperty curveProperty )
{
    std::vector<RiaDefines::CurveProperty> matching = {
        RiaDefines::CurveProperty::POROSITY,
        RiaDefines::CurveProperty::POROSITY_UNSCALED,
        RiaDefines::CurveProperty::PERMEABILITY_X,
        RiaDefines::CurveProperty::PERMEABILITY_Z,
        RiaDefines::CurveProperty::FACIES,
        RiaDefines::CurveProperty::NET_TO_GROSS,
        RiaDefines::CurveProperty::EQLNUM,
    };

    return std::find( matching.begin(), matching.end(), curveProperty ) != matching.end();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimStimPlanModel::eclipseCaseForProperty( RiaDefines::CurveProperty curveProperty ) const
{
    if ( m_initialPressureEclipseCase && ( curveProperty == RiaDefines::CurveProperty::INITIAL_PRESSURE ||
                                           curveProperty == RiaDefines::CurveProperty::EQLNUM ) )
    {
        return m_initialPressureEclipseCase;
    }
    else if ( m_staticEclipseCase && useStaticEclipseCase( curveProperty ) )
    {
        return m_staticEclipseCase;
    }
    else
    {
        return m_eclipseCase;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimStimPlanModel::setEclipseCase( RimEclipseCase* eclipseCase )
{
    if ( m_stimPlanModelTemplate )
    {
        m_stimPlanModelTemplate->setDynamicEclipseCase( eclipseCase );
        m_stimPlanModelTemplate->setInitialPressureEclipseCase( eclipseCase );
        m_stimPlanModelTemplate->setStaticEclipseCase( eclipseCase );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseCaseData* RimStimPlanModel::getEclipseCaseData() const
{
    // Find an eclipse case
    RimEclipseCase* eclCase = eclipseCaseForProperty( RiaDefines::CurveProperty::FACIES );
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
    stimPlanModelTemplateChanged( nullptr );

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
    m_calculator->clearCache();

    if ( m_stimPlanModelTemplate() )
    {
        m_eclipseCase                = m_stimPlanModelTemplate()->dynamicEclipseCase();
        m_timeStep                   = m_stimPlanModelTemplate()->timeStep();
        m_initialPressureEclipseCase = m_stimPlanModelTemplate()->initialPressureEclipseCase();
        m_staticEclipseCase          = m_stimPlanModelTemplate()->staticEclipseCase();
        updateExtractionDepthBoundaries();

        updateThicknessDirection();
        updateBarrierProperties();
    }

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
    else if ( curveProperty == RiaDefines::CurveProperty::EQLNUM )
        return "EQLNUM";
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimStimPlanModel::pressureDate() const
{
    if ( !m_stimPlanModelTemplate ) return QString();

    if ( m_stimPlanModelTemplate->usePressureTableForProperty( RiaDefines::CurveProperty::PRESSURE ) )
        return m_stimPlanModelTemplate->pressureTable()->pressureDate();
    else if ( m_eclipseCase && m_timeStep >= 0 && m_timeStep < m_eclipseCase->timeStepStrings().size() )
        return m_eclipseCase->timeStepStrings()[m_timeStep];
    else
        return QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::deque<RimExtractionConfiguration>
    RimStimPlanModel::extractionConfigurations( RiaDefines::CurveProperty curveProperty ) const
{
    if ( curveProperty == RiaDefines::CurveProperty::EQLNUM )
    {
        return {
            RimExtractionConfiguration( "EQLNUM_1",
                                        RiaDefines::ResultCatType::INPUT_PROPERTY,
                                        RimExtractionConfiguration::EclipseCaseType::INITIAL_PRESSURE_CASE ),
            RimExtractionConfiguration( "EQLNUM",
                                        RiaDefines::ResultCatType::INPUT_PROPERTY,
                                        RimExtractionConfiguration::EclipseCaseType::INITIAL_PRESSURE_CASE ),
            RimExtractionConfiguration( "EQLNUM_1",
                                        RiaDefines::ResultCatType::INPUT_PROPERTY,
                                        RimExtractionConfiguration::EclipseCaseType::DYNAMIC_CASE ),
            RimExtractionConfiguration( "EQLNUM",
                                        RiaDefines::ResultCatType::INPUT_PROPERTY,
                                        RimExtractionConfiguration::EclipseCaseType::DYNAMIC_CASE ),
            RimExtractionConfiguration( "EQLNUM_1",
                                        RiaDefines::ResultCatType::INPUT_PROPERTY,
                                        RimExtractionConfiguration::EclipseCaseType::STATIC_CASE ),
            RimExtractionConfiguration( "EQLNUM",
                                        RiaDefines::ResultCatType::INPUT_PROPERTY,
                                        RimExtractionConfiguration::EclipseCaseType::STATIC_CASE ),
            RimExtractionConfiguration( "EQLNUM",
                                        RiaDefines::ResultCatType::STATIC_NATIVE,
                                        RimExtractionConfiguration::EclipseCaseType::STATIC_CASE ),
            RimExtractionConfiguration( "EQLNUM",
                                        RiaDefines::ResultCatType::STATIC_NATIVE,
                                        RimExtractionConfiguration::EclipseCaseType::INITIAL_PRESSURE_CASE ),
            RimExtractionConfiguration( "EQLNUM",
                                        RiaDefines::ResultCatType::STATIC_NATIVE,
                                        RimExtractionConfiguration::EclipseCaseType::DYNAMIC_CASE ),

        };
    }
    else if ( curveProperty == RiaDefines::CurveProperty::POROSITY ||
              curveProperty == RiaDefines::CurveProperty::POROSITY_UNSCALED ||
              curveProperty == RiaDefines::CurveProperty::PERMEABILITY_X ||
              curveProperty == RiaDefines::CurveProperty::PERMEABILITY_Z )
    {
        QString resultName = eclipseResultVariable( curveProperty );
        return {
            RimExtractionConfiguration( QString( "%1_1" ).arg( resultName ),
                                        RiaDefines::ResultCatType::INPUT_PROPERTY,
                                        RimExtractionConfiguration::EclipseCaseType::STATIC_CASE ),
            RimExtractionConfiguration( resultName,
                                        RiaDefines::ResultCatType::INPUT_PROPERTY,
                                        RimExtractionConfiguration::EclipseCaseType::STATIC_CASE ),
            RimExtractionConfiguration( resultName,
                                        RiaDefines::ResultCatType::STATIC_NATIVE,
                                        RimExtractionConfiguration::EclipseCaseType::DYNAMIC_CASE ),
        };
    }

    return std::deque<RimExtractionConfiguration>();
}

RimEclipseCase* RimStimPlanModel::eclipseCaseForType( RimExtractionConfiguration::EclipseCaseType caseType ) const
{
    if ( caseType == RimExtractionConfiguration::EclipseCaseType::STATIC_CASE ) return m_staticEclipseCase;
    if ( caseType == RimExtractionConfiguration::EclipseCaseType::DYNAMIC_CASE ) return m_eclipseCase;
    if ( caseType == RimExtractionConfiguration::EclipseCaseType::INITIAL_PRESSURE_CASE )
        return m_initialPressureEclipseCase;

    return nullptr;
}
