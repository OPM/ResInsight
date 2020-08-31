/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RimWellLogExtractionCurve.h"

#include "RiaColorTables.h"
#include "RiaLogging.h"
#include "RiaSimWellBranchTools.h"

#include "RiaWellLogUnitTools.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseWellLogExtractor.h"
#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"
#include "RigGeoMechWellLogExtractor.h"
#include "RigResultAccessorFactory.h"
#include "RigSimWellData.h"
#include "RigSimulationWellCenterLineCalculator.h"
#include "RigSimulationWellCoordsAndMD.h"
#include "RigWellLogCurveData.h"
#include "RigWellPath.h"

#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechResultDefinition.h"
#include "RimGeoMechView.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimWellBoreStabilityPlot.h"
#include "RimWellLogCurve.h"
#include "RimWellLogFile.h"
#include "RimWellLogFileChannel.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPlotTools.h"

#include "RiuPlotMainWindowTools.h"
#include "RiuQwtPlotCurve.h"
#include "RiuQwtPlotWidget.h"

#include "cafPdmUiTreeOrdering.h"
#include "cafUtils.h"

#include <cmath>

//==================================================================================================
///
///
//==================================================================================================

CAF_PDM_SOURCE_INIT( RimWellLogExtractionCurve, "RimWellLogExtractionCurve" );

namespace caf
{
template <>
void AppEnum<RimWellLogExtractionCurve::TrajectoryType>::setUp()
{
    addItem( RimWellLogExtractionCurve::WELL_PATH, "WELL_PATH", "Well Path" );
    addItem( RimWellLogExtractionCurve::SIMULATION_WELL, "SIMULATION_WELL", "Simulation Well" );
    setDefault( RimWellLogExtractionCurve::WELL_PATH );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogExtractionCurve::RimWellLogExtractionCurve()
{
    CAF_PDM_InitObject( "Well Log Curve", RimWellLogCurve::wellLogCurveIconName(), "", "" );

    CAF_PDM_InitFieldNoDefault( &m_trajectoryType, "TrajectoryType", "Trajectory Type", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_wellPath, "CurveWellPath", "Well Name", "", "", "" );
    m_wellPath.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitField( &m_simWellName, "SimulationWellName", QString( "" ), "Well Name", "", "", "" );
    CAF_PDM_InitField( &m_branchDetection,
                       "BranchDetection",
                       true,
                       "Branch Detection",
                       "",
                       "Compute branches based on how simulation well cells are organized",
                       "" );
    CAF_PDM_InitField( &m_branchIndex, "Branch", 0, "Branch Index", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_case, "CurveCase", "Case", "", "", "" );
    m_case.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_eclipseResultDefinition, "CurveEclipseResult", "", "", "", "" );
    m_eclipseResultDefinition.uiCapability()->setUiHidden( true );
    m_eclipseResultDefinition.uiCapability()->setUiTreeChildrenHidden( true );
    m_eclipseResultDefinition = new RimEclipseResultDefinition;
    m_eclipseResultDefinition->findField( "MResultType" )->uiCapability()->setUiName( "Result Type" );

    CAF_PDM_InitFieldNoDefault( &m_geomResultDefinition, "CurveGeomechResult", "", "", "", "" );
    m_geomResultDefinition.uiCapability()->setUiHidden( true );
    m_geomResultDefinition.uiCapability()->setUiTreeChildrenHidden( true );
    m_geomResultDefinition = new RimGeoMechResultDefinition;
    m_geomResultDefinition->setAddWellPathDerivedResults( true );

    CAF_PDM_InitField( &m_timeStep, "CurveTimeStep", 0, "Time Step", "", "", "" );

    // Add some space before name to indicate these belong to the Auto Name field
    CAF_PDM_InitField( &m_addCaseNameToCurveName, "AddCaseNameToCurveName", true, "   Case Name", "", "", "" );
    CAF_PDM_InitField( &m_addPropertyToCurveName, "AddPropertyToCurveName", true, "   Property", "", "", "" );
    CAF_PDM_InitField( &m_addWellNameToCurveName, "AddWellNameToCurveName", true, "   Well Name", "", "", "" );
    CAF_PDM_InitField( &m_addTimestepToCurveName, "AddTimestepToCurveName", false, "   Timestep", "", "", "" );
    CAF_PDM_InitField( &m_addDateToCurveName, "AddDateToCurveName", true, "   Date", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogExtractionCurve::~RimWellLogExtractionCurve()
{
    clearGeneratedSimWellPaths();

    delete m_geomResultDefinition;
    delete m_eclipseResultDefinition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::setWellPath( RimWellPath* wellPath )
{
    m_wellPath = wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RimWellLogExtractionCurve::wellPath() const
{
    return m_wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::setFromSimulationWellName( const QString& simWellName, int branchIndex, bool branchDetection )
{
    m_trajectoryType  = SIMULATION_WELL;
    m_simWellName     = simWellName;
    m_branchIndex     = branchIndex;
    m_branchDetection = branchDetection;

    clearGeneratedSimWellPaths();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::setCase( RimCase* rimCase )
{
    disconnectCaseSignals( m_case.value() );

    m_case = rimCase;

    connectCaseSignals( rimCase );
    clearGeneratedSimWellPaths();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCase* RimWellLogExtractionCurve::rimCase() const
{
    return m_case;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::setPropertiesFromView( Rim3dView* view )
{
    if ( view )
    {
        disconnectCaseSignals( m_case.value() );
        m_case = view->ownerCase();
        connectCaseSignals( m_case );
    }

    RimGeoMechCase* geomCase    = dynamic_cast<RimGeoMechCase*>( m_case.value() );
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case.value() );
    m_eclipseResultDefinition->setEclipseCase( eclipseCase );
    m_geomResultDefinition->setGeoMechCase( geomCase );

    RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>( view );
    if ( eclipseView )
    {
        m_eclipseResultDefinition->simpleCopy( eclipseView->cellResult() );

        m_timeStep = eclipseView->currentTimeStep();
    }
    else if ( eclipseCase )
    {
        m_eclipseResultDefinition->setResultType( RiaDefines::ResultCatType::STATIC_NATIVE );
        m_eclipseResultDefinition->setResultVariable( "PORO" );
    }

    RimGeoMechView* geoMechView = dynamic_cast<RimGeoMechView*>( view );
    if ( geoMechView )
    {
        m_geomResultDefinition->setResultAddress( geoMechView->cellResultResultDefinition()->resultAddress() );
        m_geomResultDefinition->setNormalizationAirGap( geoMechView->cellResultResultDefinition()->normalizationAirGap() );
        m_timeStep = geoMechView->currentTimeStep();
    }
    else if ( geomCase )
    {
        m_geomResultDefinition->setResultAddress( RigFemResultAddress( RIG_ELEMENT_NODAL, "ST", "S33" ) );
    }

    clearGeneratedSimWellPaths();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogExtractionCurve::TrajectoryType RimWellLogExtractionCurve::trajectoryType() const
{
    return m_trajectoryType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::clampTimestep()
{
    if ( m_timeStep > 0 && m_case )
    {
        if ( m_timeStep > m_case->timeStepStrings().size() - 1 )
        {
            m_timeStep = m_case->timeStepStrings().size() - 1;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::clampBranchIndex()
{
    int branchCount =
        static_cast<int>( RiaSimWellBranchTools::simulationWellBranches( m_simWellName, m_branchDetection ).size() );
    if ( branchCount > 0 )
    {
        if ( m_branchIndex >= branchCount )
            m_branchIndex = branchCount - 1;
        else if ( m_branchIndex < 0 )
            m_branchIndex = 0;
    }
    else
    {
        m_branchIndex = -1;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                  const QVariant&            oldValue,
                                                  const QVariant&            newValue )
{
    RimWellLogCurve::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_case )
    {
        clampTimestep();

        auto wellNameSet = sortedSimWellNames();
        if ( !wellNameSet.count( m_simWellName() ) ) m_simWellName = "";

        clearGeneratedSimWellPaths();

        this->loadDataAndUpdate( true );
    }
    else if ( changedField == &m_wellPath )
    {
        this->loadDataAndUpdate( true );
    }
    else if ( changedField == &m_simWellName )
    {
        clearGeneratedSimWellPaths();

        this->loadDataAndUpdate( true );
    }
    else if ( changedField == &m_trajectoryType )
    {
        this->loadDataAndUpdate( true );
    }
    else if ( changedField == &m_branchDetection || changedField == &m_branchIndex )
    {
        clearGeneratedSimWellPaths();

        this->loadDataAndUpdate( true );
    }
    else if ( changedField == &m_timeStep )
    {
        this->loadDataAndUpdate( true );
    }

    if ( changedField == &m_addCaseNameToCurveName || changedField == &m_addPropertyToCurveName ||
         changedField == &m_addWellNameToCurveName || changedField == &m_addTimestepToCurveName ||
         changedField == &m_addDateToCurveName )
    {
        this->uiCapability()->updateConnectedEditors();
        updateCurveNameAndUpdatePlotLegendAndTitle();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::onLoadDataAndUpdate( bool updateParentPlot )
{
    if ( isCurveVisible() )
    {
        bool isUsingPseudoLength = false;
        performDataExtraction( &isUsingPseudoLength );

        RimDepthTrackPlot* wellLogPlot;
        firstAncestorOrThisOfType( wellLogPlot );
        if ( !wellLogPlot ) return;

        RiaDefines::DepthTypeEnum depthType   = wellLogPlot->depthType();
        RiaDefines::DepthUnitType displayUnit = wellLogPlot->depthUnit();
        if ( depthType == RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH ||
             depthType == RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH_RKB )
        {
            isUsingPseudoLength = false;
        }

        std::vector<double> xPlotValues     = curveData()->xPlotValues();
        std::vector<double> depthPlotValues = curveData()->depthPlotValues( depthType, displayUnit );
        CAF_ASSERT( xPlotValues.size() == depthPlotValues.size() );
        m_qwtPlotCurve->setSamples( xPlotValues.data(), depthPlotValues.data(), static_cast<int>( xPlotValues.size() ) );

        m_qwtPlotCurve->setLineSegmentStartStopIndices( curveData()->polylineStartStopIndices() );

        this->RimPlotCurve::updateCurvePresentation( updateParentPlot );

        if ( isUsingPseudoLength )
        {
            RimWellLogTrack* wellLogTrack;
            firstAncestorOrThisOfType( wellLogTrack );
            CVF_ASSERT( wellLogTrack );

            RiuQwtPlotWidget* viewer = wellLogTrack->viewer();
            if ( viewer )
            {
                viewer->setAxisTitleText( QwtPlot::yLeft, "PL/" + wellLogPlot->depthAxisTitle() );
            }
        }

        if ( updateParentPlot )
        {
            updateZoomInParentPlot();
        }

        setLogScaleFromSelectedResult();

        if ( m_parentQwtPlot )
        {
            m_parentQwtPlot->replot();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::performDataExtraction( bool* isUsingPseudoLength )
{
    extractData( isUsingPseudoLength );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::extractData( bool*  isUsingPseudoLength,
                                             bool   performDataSmoothing /*= false*/,
                                             double smoothingThreshold /*= -1.0 */ )
{
    CAF_ASSERT( isUsingPseudoLength );

    // Make sure we have set correct case data into the result definitions.
    RimGeoMechCase* geomCase    = dynamic_cast<RimGeoMechCase*>( m_case.value() );
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case.value() );
    m_eclipseResultDefinition->setEclipseCase( eclipseCase );
    m_geomResultDefinition->setGeoMechCase( geomCase );

    clampBranchIndex();

    RimMainPlotCollection* mainPlotCollection;
    this->firstAncestorOrThisOfTypeAsserted( mainPlotCollection );

    RimWellLogPlotCollection* wellLogCollection = mainPlotCollection->wellLogPlotCollection();

    cvf::ref<RigEclipseWellLogExtractor> eclExtractor;

    if ( eclipseCase )
    {
        if ( m_trajectoryType == WELL_PATH )
        {
            eclExtractor = wellLogCollection->findOrCreateExtractor( m_wellPath, eclipseCase );
        }
        else
        {
            std::vector<const RigWellPath*> simWellBranches =
                RiaSimWellBranchTools::simulationWellBranches( m_simWellName, m_branchDetection );
            if ( m_branchIndex >= 0 && m_branchIndex < static_cast<int>( simWellBranches.size() ) )
            {
                auto wellBranch = simWellBranches[m_branchIndex];
                eclExtractor    = wellLogCollection->findOrCreateSimWellExtractor( m_simWellName,
                                                                                eclipseCase->caseUserDescription(),
                                                                                wellBranch,
                                                                                eclipseCase->eclipseCaseData() );
                if ( eclExtractor.notNull() )
                {
                    m_wellPathsWithExtractors.push_back( wellBranch );
                }

                *isUsingPseudoLength = true;
            }
        }
    }
    cvf::ref<RigGeoMechWellLogExtractor> geomExtractor = wellLogCollection->findOrCreateExtractor( m_wellPath, geomCase );

    std::vector<double> values;
    std::vector<double> measuredDepthValues;
    std::vector<double> tvDepthValues;
    double              rkbDiff = 0.0;

    RiaDefines::DepthUnitType depthUnit = RiaDefines::DepthUnitType::UNIT_METER;
    QString                   xUnits    = RiaWellLogUnitTools<double>::noUnitString();

    if ( eclExtractor.notNull() && eclipseCase )
    {
        measuredDepthValues = eclExtractor->cellIntersectionMDs();
        tvDepthValues       = eclExtractor->cellIntersectionTVDs();
        rkbDiff             = eclExtractor->wellPathData()->rkbDiff();

        m_eclipseResultDefinition->loadResult();

        cvf::ref<RigResultAccessor> resAcc =
            RigResultAccessorFactory::createFromResultDefinition( eclipseCase->eclipseCaseData(),
                                                                  0,
                                                                  m_timeStep,
                                                                  m_eclipseResultDefinition );

        if ( resAcc.notNull() )
        {
            eclExtractor->curveData( resAcc.p(), &values );
        }

        RiaEclipseUnitTools::UnitSystem eclipseUnitsType = eclipseCase->eclipseCaseData()->unitsType();
        if ( eclipseUnitsType == RiaEclipseUnitTools::UnitSystem::UNITS_FIELD )
        {
            // See https://github.com/OPM/ResInsight/issues/538

            depthUnit = RiaDefines::DepthUnitType::UNIT_FEET;
        }
    }
    else if ( geomExtractor.notNull() ) // geomExtractor
    {
        measuredDepthValues = geomExtractor->cellIntersectionMDs();
        tvDepthValues       = geomExtractor->cellIntersectionTVDs();
        rkbDiff             = geomExtractor->wellPathData()->rkbDiff();

        if ( measuredDepthValues.empty() )
        {
            return;
        }

        findAndLoadWbsParametersFromLasFiles( m_wellPath(), geomExtractor.p() );
        RimWellBoreStabilityPlot* wbsPlot;
        this->firstAncestorOrThisOfType( wbsPlot );
        if ( wbsPlot )
        {
            wbsPlot->applyWbsParametersToExtractor( geomExtractor.p() );
        }

        m_geomResultDefinition->loadResult();
        xUnits = geomExtractor->curveData( m_geomResultDefinition->resultAddress(), m_timeStep, &values );
        if ( performDataSmoothing )
        {
            geomExtractor->performCurveDataSmoothing( m_timeStep,
                                                      &measuredDepthValues,
                                                      &tvDepthValues,
                                                      &values,
                                                      smoothingThreshold );
        }
    }

    if ( !values.empty() && !measuredDepthValues.empty() )
    {
        if ( tvDepthValues.empty() )
        {
            this->setValuesAndDepths( values,
                                      measuredDepthValues,
                                      RiaDefines::DepthTypeEnum::MEASURED_DEPTH,
                                      0.0,
                                      depthUnit,
                                      !performDataSmoothing,
                                      xUnits );
        }
        else
        {
            this->setValuesWithMdAndTVD( values,
                                         measuredDepthValues,
                                         tvDepthValues,
                                         rkbDiff,
                                         depthUnit,
                                         !performDataSmoothing,
                                         xUnits );
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Search well path for LAS-files containing Well Bore Stability data and set them in the extractor.
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::findAndLoadWbsParametersFromLasFiles( const RimWellPath*          wellPath,
                                                                      RigGeoMechWellLogExtractor* geomExtractor )
{
    auto allParams = RigWbsParameter::allParameters();
    for ( const RigWbsParameter& parameter : allParams )
    {
        QString lasAddress = parameter.addressString( RigWbsParameter::LAS_FILE );

        QString                                lasUnits;
        std::vector<std::pair<double, double>> lasFileValues =
            RimWellLogFile::findMdAndChannelValuesForWellPath( wellPath, lasAddress, &lasUnits );
        if ( !lasFileValues.empty() )
        {
            QString extractorUnits = geomExtractor->parameterInputUnits( parameter );

            if ( RiaWellLogUnitTools<double>::convertValues( &lasFileValues,
                                                             lasUnits,
                                                             extractorUnits,
                                                             wellPath->wellPathGeometry() ) )
            {
                geomExtractor->setWbsLasValues( parameter, lasFileValues );
            }
            else
            {
                QString errMsg = QString( "Could not convert units of LAS-channel %1 from %2 to %3" )
                                     .arg( lasAddress )
                                     .arg( lasUnits )
                                     .arg( extractorUnits );
                RiaLogging::error( errMsg );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::setAutoNameComponents( bool addCaseName,
                                                       bool addProperty,
                                                       bool addWellname,
                                                       bool addTimeStep,
                                                       bool addDate )
{
    m_addCaseNameToCurveName = addCaseName;
    m_addPropertyToCurveName = addProperty;
    m_addWellNameToCurveName = addWellname;
    m_addTimestepToCurveName = addTimeStep;
    m_addDateToCurveName     = addDate;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::PhaseType RimWellLogExtractionCurve::phaseType() const
{
    auto phase = RiaDefines::PhaseType::PHASE_NOT_APPLICABLE;

    if ( m_eclipseResultDefinition )
    {
        phase = m_eclipseResultDefinition->resultPhaseType();
    }
    return phase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString> RimWellLogExtractionCurve::sortedSimWellNames()
{
    std::set<QString> sortedWellNames;
    RimEclipseCase*   eclipseCase = dynamic_cast<RimEclipseCase*>( m_case.value() );
    if ( eclipseCase )
    {
        sortedWellNames = eclipseCase->sortedSimWellNames();
    }

    return sortedWellNames;
}

//--------------------------------------------------------------------------------------------------
/// Clean up existing generated well paths
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::clearGeneratedSimWellPaths()
{
    RimWellLogPlotCollection* wellLogCollection = nullptr;

    // Need to use this approach, and not firstAnchestor because the curve might not be inside the hierarchy when deleted.

    RimProject* proj = RimProject::current();
    if ( proj && proj->mainPlotCollection() ) wellLogCollection = proj->mainPlotCollection()->wellLogPlotCollection();

    if ( !wellLogCollection ) return;

    for ( auto wellPath : m_wellPathsWithExtractors )
    {
        wellLogCollection->removeExtractors( wellPath );
    }

    m_wellPathsWithExtractors.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimWellLogExtractionCurve::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    options = RimWellLogCurve::calculateValueOptions( fieldNeedingOptions, useOptionsOnly );
    if ( options.size() > 0 ) return options;

    if ( fieldNeedingOptions == &m_wellPath )
    {
        RimTools::wellPathOptionItems( &options );
    }
    else if ( fieldNeedingOptions == &m_case )
    {
        RimTools::caseOptionItems( &options );
    }
    else if ( fieldNeedingOptions == &m_timeStep )
    {
        RimTools::timeStepsForCase( m_case, &options );
    }
    else if ( fieldNeedingOptions == &m_simWellName )
    {
        std::set<QString> sortedWellNames = this->sortedSimWellNames();

        caf::IconProvider simWellIcon( ":/Well.png" );
        for ( const QString& wname : sortedWellNames )
        {
            options.push_back( caf::PdmOptionItemInfo( wname, wname, false, simWellIcon ) );
        }
    }
    else if ( fieldNeedingOptions == &m_branchIndex )
    {
        auto branches = RiaSimWellBranchTools::simulationWellBranches( m_simWellName, m_branchDetection );

        options = RiaSimWellBranchTools::valueOptionsForBranchIndexField( branches );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimPlotCurve::updateOptionSensitivity();

    caf::PdmUiGroup* curveDataGroup = uiOrdering.addNewGroupWithKeyword( "Data Source", dataSourceGroupKeyword() );

    curveDataGroup->add( &m_case );

    RimGeoMechCase* geomCase    = dynamic_cast<RimGeoMechCase*>( m_case.value() );
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case.value() );

    if ( eclipseCase )
    {
        curveDataGroup->add( &m_trajectoryType );
        if ( m_trajectoryType() == WELL_PATH )
        {
            curveDataGroup->add( &m_wellPath );
        }
        else
        {
            curveDataGroup->add( &m_simWellName );

            RiaSimWellBranchTools::appendSimWellBranchFieldsIfRequiredFromSimWellName( curveDataGroup,
                                                                                       m_simWellName,
                                                                                       m_branchDetection,
                                                                                       m_branchIndex );
        }
        m_eclipseResultDefinition->uiOrdering( uiConfigName, *curveDataGroup );
    }
    else if ( geomCase )
    {
        curveDataGroup->add( &m_wellPath );
        m_geomResultDefinition->uiOrdering( uiConfigName, *curveDataGroup );
    }

    if ( ( eclipseCase && m_eclipseResultDefinition->hasDynamicResult() ) || geomCase )
    {
        curveDataGroup->add( &m_timeStep );
    }

    caf::PdmUiGroup* stackingGroup = uiOrdering.addNewGroup( "Stacking" );
    RimStackablePlotCurve::stackingUiOrdering( *stackingGroup );

    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup( "Appearance" );
    RimPlotCurve::appearanceUiOrdering( *appearanceGroup );

    caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup( "Curve Name" );
    nameGroup->setCollapsedByDefault( true );
    nameGroup->add( &m_showLegend );
    RimPlotCurve::curveNameUiOrdering( *nameGroup );

    if ( m_isUsingAutoName )
    {
        nameGroup->add( &m_addWellNameToCurveName );
        nameGroup->add( &m_addCaseNameToCurveName );
        nameGroup->add( &m_addPropertyToCurveName );
        nameGroup->add( &m_addDateToCurveName );
        nameGroup->add( &m_addTimestepToCurveName );
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::initAfterRead()
{
    RimWellLogCurve::initAfterRead();

    RimGeoMechCase* geomCase    = dynamic_cast<RimGeoMechCase*>( m_case.value() );
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case.value() );

    m_eclipseResultDefinition->setEclipseCase( eclipseCase );
    m_geomResultDefinition->setGeoMechCase( geomCase );

    connectCaseSignals( m_case.value() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogExtractionCurve::dataSourceGroupKeyword()
{
    return "DataSource";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering,
                                                      QString                 uiConfigName /*= ""*/ )
{
    uiTreeOrdering.skipRemainingChildren( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::setLogScaleFromSelectedResult()
{
    QString resVar = m_eclipseResultDefinition->resultVariable();

    if ( resVar.toUpper().contains( "PERM" ) )
    {
        RimWellLogTrack* track = nullptr;
        this->firstAncestorOrThisOfType( track );
        if ( track )
        {
            if ( track->curveCount() == 1 )
            {
                track->setLogarithmicScale( true );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogExtractionCurve::createCurveAutoName()
{
    RimGeoMechCase* geomCase    = dynamic_cast<RimGeoMechCase*>( m_case.value() );
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case.value() );

    QStringList generatedCurveName;

    if ( m_addWellNameToCurveName )
    {
        if ( !wellName().isEmpty() )
        {
            generatedCurveName += wellName();
            if ( m_trajectoryType == SIMULATION_WELL &&
                 RiaSimWellBranchTools::simulationWellBranches( m_simWellName, m_branchDetection ).size() > 1 )
            {
                generatedCurveName.push_back( " Br" + QString::number( m_branchIndex + 1 ) );
            }
        }
    }

    if ( m_addCaseNameToCurveName && m_case() )
    {
        generatedCurveName.push_back( m_case->caseUserDescription() );
    }

    if ( m_addPropertyToCurveName && !wellLogChannelUiName().isEmpty() )
    {
        generatedCurveName.push_back( wellLogChannelUiName() );
    }

    if ( m_addTimestepToCurveName || m_addDateToCurveName )
    {
        size_t maxTimeStep = 0;

        if ( eclipseCase )
        {
            if ( eclipseCase->eclipseCaseData() )
            {
                maxTimeStep =
                    eclipseCase->eclipseCaseData()->results( m_eclipseResultDefinition->porosityModel() )->maxTimeStepCount();
            }
        }
        else if ( geomCase )
        {
            if ( geomCase->geoMechData() )
            {
                maxTimeStep = geomCase->geoMechData()->femPartResults()->frameCount();
            }
        }

        if ( m_addDateToCurveName )
        {
            QString dateString = wellDate();
            if ( !dateString.isEmpty() )
            {
                generatedCurveName.push_back( dateString );
            }
        }

        if ( m_addTimestepToCurveName )
        {
            generatedCurveName.push_back( QString( "[%1/%2]" ).arg( m_timeStep() + 1 ).arg( maxTimeStep ) );
        }
    }

    return generatedCurveName.join( ", " );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogExtractionCurve::wellLogChannelUiName() const
{
    RimGeoMechCase* geoMechCase = dynamic_cast<RimGeoMechCase*>( m_case.value() );
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case.value() );

    QString name;
    if ( eclipseCase )
    {
        name = caf::Utils::makeValidFileBasename( m_eclipseResultDefinition->resultVariableUiShortName() );
    }
    else if ( geoMechCase )
    {
        name = m_geomResultDefinition->resultVariableUiName();
    }

    return name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogExtractionCurve::wellLogChannelName() const
{
    RimGeoMechCase* geoMechCase = dynamic_cast<RimGeoMechCase*>( m_case.value() );
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case.value() );

    QString name;
    if ( eclipseCase )
    {
        name = caf::Utils::makeValidFileBasename( m_eclipseResultDefinition->resultVariableUiShortName() );
    }
    else if ( geoMechCase )
    {
        name = caf::Utils::makeValidFileBasename( m_geomResultDefinition->resultVariableName() );
    }

    return name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogExtractionCurve::wellLogChannelUnits() const
{
    RimGeoMechCase* geoMechCase = dynamic_cast<RimGeoMechCase*>( m_case.value() );
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case.value() );

    QString name;
    if ( eclipseCase )
    {
        name = RiaWellLogUnitTools<double>::noUnitString();
    }
    else if ( geoMechCase )
    {
        name = m_geomResultDefinition->defaultLasUnits();
    }

    return name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogExtractionCurve::wellName() const
{
    if ( m_trajectoryType() == WELL_PATH )
    {
        if ( m_wellPath )
        {
            return m_wellPath->name();
        }
        else
        {
            return QString();
        }
    }
    else
    {
        return m_simWellName;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogExtractionCurve::wellDate() const
{
    RimGeoMechCase* geomCase    = dynamic_cast<RimGeoMechCase*>( m_case.value() );
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case.value() );

    QStringList timeStepNames;

    if ( eclipseCase )
    {
        if ( eclipseCase->eclipseCaseData() )
        {
            timeStepNames = eclipseCase->timeStepStrings();
        }
    }
    else if ( geomCase )
    {
        if ( geomCase->geoMechData() )
        {
            timeStepNames = geomCase->timeStepStrings();
        }
    }

    return ( m_timeStep >= 0 && m_timeStep < timeStepNames.size() ) ? timeStepNames[m_timeStep] : "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimWellLogExtractionCurve::branchIndex() const
{
    return m_branchIndex();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogExtractionCurve::branchDetection() const
{
    return m_branchDetection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogExtractionCurve::isEclipseCurve() const
{
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case.value() );
    if ( eclipseCase )
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogExtractionCurve::caseName() const
{
    if ( m_case )
    {
        return m_case->caseUserDescription();
    }

    return QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimWellLogExtractionCurve::currentTimeStep() const
{
    return m_timeStep();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::setCurrentTimeStep( int timeStep )
{
    m_timeStep = timeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::setEclipseResultVariable( const QString& resVarname )
{
    m_eclipseResultDefinition->setResultVariable( resVarname );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogExtractionCurve::eclipseResultVariable() const
{
    return m_eclipseResultDefinition->resultVariable();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::setGeoMechResultAddress( const RigFemResultAddress& resAddr )
{
    m_geomResultDefinition->setResultAddress( resAddr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::setTrajectoryType( TrajectoryType trajectoryType )
{
    m_trajectoryType = trajectoryType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::setWellName( QString wellName )
{
    m_simWellName = wellName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::setBranchDetection( bool branchDetection )
{
    m_branchDetection = branchDetection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::setBranchIndex( int index )
{
    m_branchIndex = index;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::connectCaseSignals( RimCase* rimCase )
{
    if ( rimCase )
    {
        rimCase->settingsChanged.connect( this, &RimWellLogExtractionCurve::onCaseSettingsChanged );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::disconnectCaseSignals( RimCase* rimCase )
{
    if ( rimCase != nullptr )
    {
        rimCase->settingsChanged.disconnect( this );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurve::onCaseSettingsChanged( const caf::SignalEmitter* emitter )
{
    loadDataAndUpdate( true );
}
