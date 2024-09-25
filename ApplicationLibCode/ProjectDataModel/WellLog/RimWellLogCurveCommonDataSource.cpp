/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016      Statoil ASA
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

#include "RimWellLogCurveCommonDataSource.h"

#include "RiaSimWellBranchTools.h"
#include "RiaSummaryTools.h"

#include "RifReaderOpmRft.h"

#include "RimCase.h"
#include "RimDataSourceSteppingTools.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultCase.h"
#include "RimGeoMechCase.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimRftTools.h"
#include "RimRftTopologyCurve.h"
#include "RimSummaryCase.h"
#include "RimTools.h"
#include "RimWellFlowRateCurve.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogFile.h"
#include "RimWellLogLasFileCurve.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellLogRftCurve.h"
#include "RimWellLogTrack.h"
#include "RimWellLogWbsCurve.h"
#include "RimWellMeasurementCurve.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "RiuTools.h"

#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiCheckBoxTristateEditor.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiLineEditor.h"

#include <algorithm>

CAF_PDM_SOURCE_INIT( RimWellLogCurveCommonDataSource, "ChangeDataSourceFeatureUi" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogCurveCommonDataSource::DoubleComparator::DoubleComparator( double eps /*= 1.0e-8 */ )
    : m_eps( eps )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogCurveCommonDataSource::DoubleComparator::operator()( const double& lhs, const double& rhs ) const
{
    double diff = lhs - rhs;
    return diff < -m_eps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogCurveCommonDataSource::RimWellLogCurveCommonDataSource()
    : m_caseType( RiaDefines::CaseType::UNDEFINED_CASE )
{
    CAF_PDM_InitObject( "Change Data Source" );

    CAF_PDM_InitFieldNoDefault( &m_case, "CurveCase", "Case" );
    CAF_PDM_InitFieldNoDefault( &m_summaryCase, "SummaryCase", "Summary Case" );
    CAF_PDM_InitFieldNoDefault( &m_trajectoryType, "TrajectoryType", "Trajectory Type" );

    CAF_PDM_InitFieldNoDefault( &m_wellPath, "CurveWellPath", "Well Path" );

    CAF_PDM_InitFieldNoDefault( &m_simWellName, "SimulationWellName", "Well Name" );
    CAF_PDM_InitFieldNoDefault( &m_branchDetection,
                                "BranchDetection",
                                "Branch Detection",
                                "",
                                "Compute branches based on how simulation well cells are organized",
                                "" );
    CAF_PDM_InitField( &m_allow3DSelectionLink, "Allow3DSelectionLink", true, "Allow Well Selection from 3D View" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_allow3DSelectionLink );

    m_branchDetection.v() = caf::Tristate::State::PartiallyTrue;
    m_branchDetection.uiCapability()->setUiEditorTypeName( caf::PdmUiCheckBoxTristateEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_branchIndex, "Branch", -1, "Branch Index" );

    CAF_PDM_InitField( &m_timeStep, "CurveTimeStep", -1, "Time Step" );

    CAF_PDM_InitFieldNoDefault( &m_wbsSmoothing, "WBSSmoothing", "Smooth Curves" );
    m_wbsSmoothing.uiCapability()->setUiEditorTypeName( caf::PdmUiCheckBoxTristateEditor::uiEditorTypeName() );
    m_wbsSmoothing.v() = caf::Tristate::State::PartiallyTrue;

    CAF_PDM_InitField( &m_wbsSmoothingThreshold, "WBSSmoothingThreshold", -1.0, "Smoothing Threshold" );

    CAF_PDM_InitField( &m_maximumCurvePointInterval, "MaximumCurvePointInterval", std::make_pair( true, 10.0 ), "Maximum Curve Point Interval" );

    CAF_PDM_InitFieldNoDefault( &m_rftTimeStep, "RftTimeStep", "RFT Time Step" );
    CAF_PDM_InitFieldNoDefault( &m_rftWellName, "RftWellName", "RFT Well Name" );
    CAF_PDM_InitFieldNoDefault( &m_rftSegmentBranchIndex, "SegmentBranchIndex", "RFT Branch" );
    CAF_PDM_InitFieldNoDefault( &m_rftSegmentBranchType, "SegmentBranchType", "RFT Completion" );

    m_case     = nullptr;
    m_wellPath = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::setCaseType( RiaDefines::CaseType caseType )
{
    m_caseType = caseType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCase* RimWellLogCurveCommonDataSource::caseToApply() const
{
    return m_case;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::setSummaryCaseToApply( RimSummaryCase* val )
{
    m_summaryCase = val;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::setCaseToApply( RimCase* val )
{
    m_case = val;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimWellLogCurveCommonDataSource::summaryCaseToApply() const
{
    return m_summaryCase();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimWellLogCurveCommonDataSource::trajectoryTypeToApply() const
{
    return m_trajectoryType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::setTrajectoryTypeToApply( int val )
{
    m_trajectoryType = val;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RimWellLogCurveCommonDataSource::wellPathToApply() const
{
    return m_wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::setWellPathToApply( RimWellPath* val )
{
    m_wellPath = val;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimWellLogCurveCommonDataSource::branchIndexToApply() const
{
    return m_branchIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::setBranchIndexToApply( int val )
{
    m_branchIndex = val;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::Tristate RimWellLogCurveCommonDataSource::branchDetectionToApply() const
{
    return m_branchDetection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::setBranchDetectionToApply( caf::Tristate::State val )
{
    m_branchDetection.v() = val;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::Tristate RimWellLogCurveCommonDataSource::wbsSmoothingToApply() const
{
    return m_wbsSmoothing();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::setWbsSmoothingToApply( caf::Tristate::State val )
{
    m_wbsSmoothing.v() = val;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellLogCurveCommonDataSource::wbsSmoothingThreshold() const
{
    return m_wbsSmoothingThreshold;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::setWbsSmoothingThreshold( double smoothingThreshold )
{
    m_wbsSmoothingThreshold = smoothingThreshold;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogCurveCommonDataSource::simWellNameToApply() const
{
    return m_simWellName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::setSimWellNameToApply( const QString& val )
{
    m_simWellName = val;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimWellLogCurveCommonDataSource::timeStepToApply() const
{
    return m_timeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::setTimeStepToApply( int val )
{
    m_timeStep = val;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::resetSourceStepFields()
{
    setCaseToApply( nullptr );
    setSummaryCaseToApply( nullptr );
    setTrajectoryTypeToApply( -1 );
    setWellPathToApply( nullptr );
    setBranchIndexToApply( -1 );
    setBranchDetectionToApply( caf::Tristate::State::PartiallyTrue );
    setSimWellNameToApply( QString( "" ) );
    setTimeStepToApply( -1 );
    setWbsSmoothingToApply( caf::Tristate::State::PartiallyTrue );
    setWbsSmoothingThreshold( -1.0 );

    m_uniqueCases.clear();
    m_uniqueSummaryCases.clear();
    m_uniqueTrajectoryTypes.clear();
    m_uniqueWellPaths.clear();
    m_uniqueWellNames.clear();
    m_uniqueTimeSteps.clear();
    m_uniqueBranchIndices.clear();
    m_uniqueBranchDetection.clear();
    m_uniqueWbsSmoothing.clear();
    m_uniqueWbsSmoothingThreshold.clear();

    m_uniqueRftTimeSteps.clear();
    m_uniqueRftWellNames.clear();
    m_uniqueRftBranchIndices.clear();
    m_uniqueRftBranchTypes.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::analyseCurvesAndTracks( const std::vector<RimWellLogCurve*>& curves,
                                                              const std::vector<RimWellLogTrack*>& tracks )
{
    // Reset all source step fields in the UI
    resetSourceStepFields();

    // Check to see if the parameters are unique
    for ( RimWellLogCurve* curve : curves )
    {
        auto* extractionCurve = dynamic_cast<RimWellLogExtractionCurve*>( curve );
        auto* fileCurve       = dynamic_cast<RimWellLogLasFileCurve*>( curve );
        auto* flowRateCurve   = dynamic_cast<RimWellFlowRateCurve*>( curve );
        auto* rftCurve        = dynamic_cast<RimWellLogRftCurve*>( curve );

        if ( extractionCurve )
        {
            auto* wbsCurve = dynamic_cast<RimWellLogWbsCurve*>( extractionCurve );
            if ( wbsCurve )
            {
                m_uniqueWbsSmoothing.insert( wbsCurve->smoothCurve() );
                m_uniqueWbsSmoothingThreshold.insert( wbsCurve->smoothingThreshold() );
            }
            if ( extractionCurve->rimCase() )
            {
                m_uniqueCases.insert( extractionCurve->rimCase() );
            }
            m_uniqueTrajectoryTypes.insert( static_cast<int>( extractionCurve->trajectoryType() ) );
            if ( extractionCurve->trajectoryType() == RimWellLogExtractionCurve::WELL_PATH )
            {
                if ( extractionCurve->wellPath() )
                {
                    m_uniqueWellPaths.insert( extractionCurve->wellPath() );
                }
            }
            else if ( extractionCurve->trajectoryType() == RimWellLogExtractionCurve::SIMULATION_WELL )
            {
                if ( !extractionCurve->wellName().isEmpty() )
                {
                    m_uniqueWellNames.insert( extractionCurve->wellName() );
                }
            }

            m_uniqueTimeSteps.insert( extractionCurve->currentTimeStep() );
            m_uniqueBranchDetection.insert( extractionCurve->branchDetection() );
            m_uniqueBranchIndices.insert( extractionCurve->branchIndex() );
        }
        else if ( fileCurve )
        {
            m_uniqueWellPaths.insert( fileCurve->wellPath() );
            m_uniqueWellNames.insert( fileCurve->wellName() );
        }
        else if ( flowRateCurve )
        {
            m_uniqueTrajectoryTypes.insert( RimWellLogExtractionCurve::SIMULATION_WELL );
            m_uniqueWellNames.insert( flowRateCurve->wellName() );
            m_uniqueCases.insert( flowRateCurve->rimCase() );
            m_uniqueTimeSteps.insert( flowRateCurve->timeStep() );
        }
        else if ( rftCurve )
        {
            if ( rftCurve->summaryCase() ) m_uniqueSummaryCases.insert( rftCurve->summaryCase() );
            if ( rftCurve->eclipseCase() ) m_uniqueCases.insert( rftCurve->eclipseCase() );
            m_uniqueWellNames.insert( rftCurve->wellName() );

            auto adr = rftCurve->rftAddress();
            if ( adr.wellLogChannel() == RifEclipseRftAddress::RftWellLogChannelType::SEGMENT_VALUES && adr.segmentResultName() != "None" )
            {
                m_uniqueRftWellNames.insert( adr.wellName() );
                m_uniqueRftTimeSteps.insert( adr.timeStep() );
                m_uniqueRftBranchIndices.insert( adr.segmentBranchIndex() );
                m_uniqueRftBranchTypes.insert( adr.segmentBranchType() );
            }
        }
    }
    for ( RimWellLogTrack* track : tracks )
    {
        if ( track->showWellPathAttributes() )
        {
            m_uniqueTrajectoryTypes.insert( static_cast<int>( RimWellLogExtractionCurve::WELL_PATH ) );
            m_uniqueWellPaths.insert( track->wellPathAttributeSource() );
        }
        if ( track->showFormations() )
        {
            m_uniqueTrajectoryTypes.insert( track->formationTrajectoryType() );
            if ( track->formationTrajectoryType() == RimWellLogTrack::WELL_PATH )
            {
                m_uniqueWellPaths.insert( track->formationWellPath() );
            }
            else if ( track->formationTrajectoryType() == RimWellLogTrack::SIMULATION_WELL )
            {
                m_uniqueWellNames.insert( track->formationSimWellName() );
            }
            m_uniqueBranchDetection.insert( track->formationBranchDetection() );
            m_uniqueBranchIndices.insert( track->formationBranchIndex() );

            m_uniqueCases.insert( track->formationNamesCase() );
            m_uniqueWellPaths.insert( track->formationWellPath() );
        }
    }

    if ( m_uniqueCases.size() == 1u )
    {
        setCaseToApply( *m_uniqueCases.begin() );
    }

    if ( m_uniqueSummaryCases.size() == 1u )
    {
        setSummaryCaseToApply( *m_uniqueSummaryCases.begin() );
    }

    if ( m_uniqueTrajectoryTypes.size() == 1u )
    {
        m_trajectoryType = *m_uniqueTrajectoryTypes.begin();

        if ( m_uniqueWellPaths.size() == 1u )
        {
            setWellPathToApply( *m_uniqueWellPaths.begin() );
        }
        if ( m_uniqueBranchIndices.size() == 1u )
        {
            setBranchIndexToApply( *m_uniqueBranchIndices.begin() );
        }
        if ( m_uniqueBranchDetection.size() == 1u )
        {
            setBranchDetectionToApply( *m_uniqueBranchDetection.begin() ? caf::Tristate::State::True : caf::Tristate::State::False );
        }
        if ( m_uniqueWellNames.size() == 1u )
        {
            setSimWellNameToApply( *m_uniqueWellNames.begin() );
        }
    }

    if ( m_uniqueTimeSteps.size() == 1u )
    {
        setTimeStepToApply( *m_uniqueTimeSteps.begin() );
    }

    if ( m_uniqueWbsSmoothing.size() == 1u )
    {
        setWbsSmoothingToApply( *m_uniqueWbsSmoothing.begin() ? caf::Tristate::State::True : caf::Tristate::State::False );
    }

    if ( m_uniqueWbsSmoothingThreshold.size() == 1u )
    {
        setWbsSmoothingThreshold( *m_uniqueWbsSmoothingThreshold.begin() );
    }

    if ( m_uniqueRftWellNames.size() == 1u )
    {
        m_rftWellName = *( m_uniqueRftWellNames.begin() );
    }

    if ( m_uniqueRftTimeSteps.size() == 1u )
    {
        m_rftTimeStep = *( m_uniqueRftTimeSteps.begin() );
    }

    if ( m_uniqueRftBranchIndices.size() == 1u )
    {
        m_rftSegmentBranchIndex = *( m_uniqueRftBranchIndices.begin() );
    }

    if ( m_uniqueRftBranchTypes.size() == 1u )
    {
        m_rftSegmentBranchType = *( m_uniqueRftBranchTypes.begin() );
    }
    else
        m_rftSegmentBranchType = RiaDefines::RftBranchType::RFT_UNKNOWN;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::analyseCurvesAndTracks()
{
    auto parentPlot = firstAncestorOrThisOfType<RimWellLogPlot>();
    if ( parentPlot )
    {
        std::vector<RimWellLogCurve*> curves = parentPlot->descendantsIncludingThisOfType<RimWellLogCurve>();
        std::vector<RimWellLogTrack*> tracks = parentPlot->descendantsIncludingThisOfType<RimWellLogTrack>();

        analyseCurvesAndTracks( curves, tracks );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::applyDataSourceChanges( const std::vector<RimWellLogCurve*>& curves,
                                                              const std::vector<RimWellLogTrack*>& tracks )
{
    std::set<RimWellLogPlot*> plots;
    for ( RimWellLogCurve* curve : curves )
    {
        auto* fileCurve        = dynamic_cast<RimWellLogLasFileCurve*>( curve );
        auto* extractionCurve  = dynamic_cast<RimWellLogExtractionCurve*>( curve );
        auto* measurementCurve = dynamic_cast<RimWellMeasurementCurve*>( curve );
        auto* rftCurve         = dynamic_cast<RimWellLogRftCurve*>( curve );
        auto* topologyCurve    = dynamic_cast<RimRftTopologyCurve*>( curve );
        if ( fileCurve )
        {
            if ( wellPathToApply() != nullptr )
            {
                fileCurve->setWellPath( wellPathToApply() );
                if ( !fileCurve->wellLogChannelUiName().isEmpty() )
                {
                    RimWellLogFile* logFile = wellPathToApply()->firstWellLogFileMatchingChannelName( fileCurve->wellLogChannelUiName() );
                    fileCurve->setWellLog( logFile );
                    auto parentPlot = fileCurve->firstAncestorOrThisOfTypeAsserted<RimWellLogPlot>();
                    plots.insert( parentPlot );
                }
            }
        }
        else if ( extractionCurve )
        {
            bool updatedSomething = false;
            if ( caseToApply() != nullptr )
            {
                extractionCurve->setCase( caseToApply() );
                updatedSomething = true;
            }

            if ( wellPathToApply() != nullptr )
            {
                extractionCurve->setWellPath( wellPathToApply() );
                updatedSomething = true;
            }

            if ( m_trajectoryType() != -1 )
            {
                extractionCurve->setTrajectoryType( static_cast<RimWellLogExtractionCurve::TrajectoryType>( m_trajectoryType() ) );
                if ( m_trajectoryType() == (int)RimWellLogExtractionCurve::SIMULATION_WELL )
                {
                    if ( m_branchDetection().isTrue() )
                    {
                        extractionCurve->setBranchDetection( true );
                    }
                    else if ( m_branchDetection().isFalse() )
                    {
                        extractionCurve->setBranchDetection( false );
                    }

                    if ( m_branchIndex() != -1 )
                    {
                        extractionCurve->setBranchIndex( m_branchIndex() );
                    }
                    if ( m_simWellName() != QString( "" ) )
                    {
                        extractionCurve->setWellName( m_simWellName() );
                    }
                }
                updatedSomething = true;
            }

            if ( timeStepToApply() != -1 )
            {
                extractionCurve->setCurrentTimeStep( timeStepToApply() );
                updatedSomething = true;
            }

            auto* wbsCurve = dynamic_cast<RimWellLogWbsCurve*>( extractionCurve );
            if ( wbsCurve )
            {
                if ( !wbsSmoothingToApply().isPartiallyTrue() )
                {
                    wbsCurve->setSmoothCurve( wbsSmoothingToApply().isTrue() );
                }

                if ( wbsSmoothingThreshold() != 1.0 )
                {
                    wbsCurve->setSmoothingThreshold( wbsSmoothingThreshold() );
                }

                wbsCurve->enableMaximumCurvePointInterval( m_maximumCurvePointInterval().first );
                wbsCurve->setMaximumCurvePointInterval( m_maximumCurvePointInterval().second );

                // Always do an update for wbs plots
                updatedSomething = true;
            }

            if ( updatedSomething )
            {
                auto parentPlot = extractionCurve->firstAncestorOrThisOfTypeAsserted<RimWellLogPlot>();
                plots.insert( parentPlot );
            }
        }
        else if ( measurementCurve )
        {
            if ( wellPathToApply() != nullptr )
            {
                measurementCurve->setWellPath( wellPathToApply() );
            }
        }
        else if ( rftCurve )
        {
            if ( m_summaryCase() ) rftCurve->setSummaryCase( m_summaryCase() );
            rftCurve->setTimeStep( m_rftTimeStep() );
            rftCurve->setWellName( m_rftWellName() );
            rftCurve->setSegmentBranchIndex( m_rftSegmentBranchIndex() );

            if ( m_rftSegmentBranchType() != RiaDefines::RftBranchType::RFT_UNKNOWN )
                rftCurve->setSegmentBranchType( m_rftSegmentBranchType() );

            auto parentPlot = rftCurve->firstAncestorOrThisOfTypeAsserted<RimWellLogPlot>();
            plots.insert( parentPlot );
        }
        else if ( topologyCurve )
        {
            topologyCurve->setDataSource( m_summaryCase, m_rftTimeStep, m_rftWellName, m_rftSegmentBranchIndex );
        }

        curve->updateConnectedEditors();
    }

    for ( RimWellLogTrack* track : tracks )
    {
        bool updatedSomething = false;

        if ( track->showWellPathAttributes() )
        {
            if ( wellPathToApply() )
            {
                track->setWellPathAttributesSource( wellPathToApply() );
                updatedSomething = true;
            }
        }

        if ( track->showFormations() )
        {
            if ( caseToApply() != nullptr )
            {
                track->setFormationCase( caseToApply() );
                updatedSomething = true;
            }

            if ( wellPathToApply() != nullptr )
            {
                track->setFormationWellPath( wellPathToApply() );
                updatedSomething = true;
            }

            if ( !simWellNameToApply().isEmpty() )
            {
                track->setFormationSimWellName( simWellNameToApply() );
                updatedSomething = true;
            }

            if ( !branchDetectionToApply().isPartiallyTrue() )
            {
                track->setFormationSimWellName( simWellNameToApply() );
                updatedSomething = true;
            }

            if ( branchIndexToApply() >= 0 )
            {
                track->setFormationBranchIndex( branchIndexToApply() );
                updatedSomething = true;
            }
        }
        if ( updatedSomething )
        {
            auto parentPlot = track->firstAncestorOrThisOfTypeAsserted<RimWellLogPlot>();
            plots.insert( parentPlot );
            track->updateConnectedEditors();
        }
    }

    for ( RimWellLogPlot* plot : plots )
    {
        plot->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::applyDataSourceChanges()
{
    auto parentPlot = firstAncestorOrThisOfType<RimWellLogPlot>();
    if ( parentPlot )
    {
        auto curves = parentPlot->descendantsIncludingThisOfType<RimWellLogCurve>();
        auto tracks = parentPlot->descendantsIncludingThisOfType<RimWellLogTrack>();

        applyDataSourceChanges( curves, tracks );

        // plot->loadDataAndUpdate() has been called in applyDataSourceChanges(), and this is required before the visibility of tracks and
        // curves can be updated. However, if the visibility of curves changes, another loadDataAndUpdate() is required to calculate zoom
        // based on visible curves.

        for ( auto& track : tracks )
        {
            track->updateCheckStateBasedOnCurveData();
        }

        parentPlot->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::applyPrevCase()
{
    modifyCurrentIndex( &m_case, -1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::applyNextCase()
{
    modifyCurrentIndex( &m_case, 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::applyPrevWell()
{
    if ( m_trajectoryType() == RimWellLogExtractionCurve::WELL_PATH )
    {
        modifyCurrentIndex( &m_wellPath, -1 );
    }
    else if ( m_trajectoryType() == RimWellLogExtractionCurve::SIMULATION_WELL )
    {
        modifyCurrentIndex( &m_simWellName, -1 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::applyNextWell()
{
    if ( m_trajectoryType() == RimWellLogExtractionCurve::WELL_PATH )
    {
        modifyCurrentIndex( &m_wellPath, 1 );
    }
    else if ( m_trajectoryType() == RimWellLogExtractionCurve::SIMULATION_WELL )
    {
        modifyCurrentIndex( &m_simWellName, 1 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::applyPrevTimeStep()
{
    modifyCurrentIndex( &m_timeStep, -1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::applyNextTimeStep()
{
    modifyCurrentIndex( &m_timeStep, 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::PdmFieldHandle*> RimWellLogCurveCommonDataSource::fieldsToShowInToolbar()
{
    analyseCurvesAndTracks();

    std::vector<caf::PdmFieldHandle*> fieldsToDisplay;

    if ( !m_uniqueCases.empty() )
    {
        fieldsToDisplay.push_back( &m_case );
        if ( trajectoryTypeToApply() == RimWellLogExtractionCurve::WELL_PATH )
        {
            fieldsToDisplay.push_back( &m_wellPath );
        }
        else if ( trajectoryTypeToApply() == RimWellLogExtractionCurve::SIMULATION_WELL )
        {
            fieldsToDisplay.push_back( &m_simWellName );
        }
    }

    if ( m_uniqueRftWellNames.size() == 1u ) fieldsToDisplay.push_back( &m_rftWellName );
    if ( m_uniqueTimeSteps.size() == 1u ) fieldsToDisplay.push_back( &m_timeStep );
    if ( m_uniqueRftTimeSteps.size() == 1u ) fieldsToDisplay.push_back( &m_rftTimeStep );
    if ( m_uniqueRftBranchIndices.size() == 1u ) fieldsToDisplay.push_back( &m_rftSegmentBranchIndex );
    if ( m_uniqueRftBranchTypes.size() == 1u ) fieldsToDisplay.push_back( &m_rftSegmentBranchType );

    return fieldsToDisplay;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogCurveCommonDataSource::smoothingUiOrderinglabel()
{
    return "ApplySmoothing";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, double> RimWellLogCurveCommonDataSource::maximumCurvePointInterval() const
{
    return m_maximumCurvePointInterval();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_branchDetection && m_branchDetection().isPartiallyTrue() )
    {
        // The Tristate is cycled from false -> partially true -> true
        // Partially true is used for "Mixed state" and is not settable by the user so cycle on to true.
        m_branchDetection.v() = caf::Tristate::State::True;
    }
    if ( changedField == &m_wbsSmoothing && m_wbsSmoothing().isPartiallyTrue() )
    {
        m_wbsSmoothing.v() = caf::Tristate::State::True;
    }

    applyDataSourceChanges();

    if ( changedField == &m_rftWellName )
    {
        // The segment branch index is depending on the well name. Make sure that the combo box for branch index is
        // updated.
        m_rftSegmentBranchIndex.uiCapability()->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellLogCurveCommonDataSource::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    analyseCurvesAndTracks();

    if ( fieldNeedingOptions == &m_case )
    {
        if ( m_caseType == RiaDefines::CaseType::GEOMECH_ODB_CASE )
        {
            RimTools::geoMechCaseOptionItems( &options );
        }
        else if ( m_caseType == RiaDefines::CaseType::ECLIPSE_RESULT_CASE || m_caseType == RiaDefines::CaseType::ECLIPSE_INPUT_CASE ||
                  m_caseType == RiaDefines::CaseType::ECLIPSE_SOURCE_CASE || m_caseType == RiaDefines::CaseType::ECLIPSE_STAT_CASE )
        {
            RimTools::eclipseCaseOptionItems( &options );
        }
        else
        {
            RimTools::caseOptionItems( &options );
        }

        options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
    }
    if ( fieldNeedingOptions == &m_summaryCase )
    {
        options = RiaSummaryTools::optionsForAllSummaryCases();
    }
    else if ( fieldNeedingOptions == &m_trajectoryType )
    {
        if ( m_trajectoryType() == -1 )
        {
            if ( !m_uniqueTrajectoryTypes.empty() )
            {
                options.push_back( caf::PdmOptionItemInfo( "Mixed Trajectory Types", -1 ) );
            }
            else
            {
                options.push_back( caf::PdmOptionItemInfo( "No Trajectory Types", -1 ) );
            }
        }
        std::vector<RimWellLogExtractionCurve::TrajectoryType> trajectoryTypes = { RimWellLogExtractionCurve::WELL_PATH,
                                                                                   RimWellLogExtractionCurve::SIMULATION_WELL };
        for ( RimWellLogExtractionCurve::TrajectoryType trajectoryType : trajectoryTypes )
        {
            caf::PdmOptionItemInfo item( caf::AppEnum<RimWellLogExtractionCurve::TrajectoryType>::uiText( trajectoryType ),
                                         static_cast<int>( trajectoryType ) );
            options.push_back( item );
        }
    }
    else if ( fieldNeedingOptions == &m_wellPath )
    {
        RimTools::wellPathOptionItems( &options );
        if ( wellPathToApply() == nullptr )
        {
            if ( !m_uniqueWellPaths.empty() )
            {
                options.push_front( caf::PdmOptionItemInfo( "Mixed Well Paths", nullptr ) );
            }
            else
            {
                options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_timeStep )
    {
        if ( m_case() )
        {
            RimTools::timeStepsForCase( m_case, &options );

            if ( timeStepToApply() == -1 )
            {
                if ( !m_uniqueTimeSteps.empty() )
                {
                    options.push_front( caf::PdmOptionItemInfo( "Mixed Time Steps", -1 ) );
                }
                else
                {
                    options.push_front( caf::PdmOptionItemInfo( "No Time Steps", -1 ) );
                }
            }
        }
    }
    else if ( fieldNeedingOptions == &m_simWellName )
    {
        auto* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case() );
        if ( eclipseCase )
        {
            std::set<QString> sortedWellNames = eclipseCase->sortedSimWellNames();

            caf::IconProvider simWellIcon( ":/Well.svg" );
            for ( const QString& wname : sortedWellNames )
            {
                options.push_back( caf::PdmOptionItemInfo( wname, wname, false, simWellIcon ) );
            }

            if ( m_simWellName().isEmpty() )
            {
                if ( !m_uniqueWellNames.empty() )
                {
                    options.push_front( caf::PdmOptionItemInfo( "Mixed Well Names", "" ) );
                }
                else
                {
                    options.push_front( caf::PdmOptionItemInfo( "None", "None" ) );
                }
            }
        }
    }
    else if ( fieldNeedingOptions == &m_branchIndex )
    {
        bool hasCommonBranchDetection = !m_branchDetection().isPartiallyTrue();
        if ( hasCommonBranchDetection )
        {
            bool doBranchDetection = m_branchDetection().isTrue();
            auto branches          = RiaSimWellBranchTools::simulationWellBranches( m_simWellName, doBranchDetection );

            options = RiaSimWellBranchTools::valueOptionsForBranchIndexField( branches );

            if ( m_branchIndex() == -1 )
            {
                if ( !m_uniqueBranchIndices.empty() )
                {
                    options.push_front( caf::PdmOptionItemInfo( "Mixed Branches", -1 ) );
                }
                else
                {
                    options.push_front( caf::PdmOptionItemInfo( "No Branches", -1 ) );
                }
            }
        }
    }
    else if ( fieldNeedingOptions == &m_rftTimeStep )
    {
        if ( !m_uniqueRftWellNames.empty() )
            options = RimRftTools::segmentTimeStepOptions( rftReader(), *( m_uniqueRftWellNames.begin() ) );
    }
    else if ( fieldNeedingOptions == &m_rftWellName )
    {
        options = RimRftTools::wellNameOptions( rftReader() );
    }
    else if ( fieldNeedingOptions == &m_rftSegmentBranchIndex )
    {
        options = RimRftTools::segmentBranchIndexOptions( dynamic_cast<RifReaderOpmRft*>( rftReader() ),
                                                          m_rftWellName(),
                                                          m_rftTimeStep(),
                                                          m_rftSegmentBranchType() );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    analyseCurvesAndTracks();

    caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Data Source" );
    if ( m_case() ) group->add( &m_case );
    if ( m_summaryCase() ) group->add( &m_summaryCase );

    const auto* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case() );
    if ( eclipseCase )
    {
        group->add( &m_trajectoryType );
        if ( trajectoryTypeToApply() == RimWellLogExtractionCurve::WELL_PATH )
        {
            group->add( &m_wellPath );
        }
        else if ( trajectoryTypeToApply() == RimWellLogExtractionCurve::SIMULATION_WELL )
        {
            group->add( &m_simWellName );
            group->add( &m_allow3DSelectionLink );

            if ( RiaSimWellBranchTools::simulationWellBranches( m_simWellName(), true ).size() > 1 )
            {
                group->add( &m_branchDetection );
                bool hasCommonBranchDetection = !m_branchDetection().isPartiallyTrue();
                if ( hasCommonBranchDetection )
                {
                    bool doBranchDetection = m_branchDetection().isTrue();
                    if ( RiaSimWellBranchTools::simulationWellBranches( m_simWellName(), doBranchDetection ).size() > 1 )
                    {
                        group->add( &m_branchIndex );
                    }
                }
            }
        }

        group->add( &m_timeStep );
    }
    else
    {
        if ( m_wellPath() ) group->add( &m_wellPath );
    }

    if ( uiConfigName == smoothingUiOrderinglabel() )
    {
        group->add( &m_wbsSmoothing );
        group->add( &m_wbsSmoothingThreshold );
        group->add( &m_maximumCurvePointInterval );
    }

    if ( !m_uniqueRftTimeSteps.empty() ) group->add( &m_rftTimeStep );
    if ( !m_uniqueRftWellNames.empty() ) group->add( &m_rftWellName );
    if ( !m_uniqueRftBranchTypes.empty() ) group->add( &m_rftSegmentBranchType );
    if ( !m_uniqueRftBranchIndices.empty() ) group->add( &m_rftSegmentBranchIndex );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                             QString                    uiConfigName,
                                                             caf::PdmUiEditorAttribute* attribute )
{
    auto* myAttr = dynamic_cast<caf::PdmUiComboBoxEditorAttribute*>( attribute );
    if ( myAttr )
    {
        if ( field == &m_case || field == &m_summaryCase || field == &m_simWellName || field == &m_wellPath || field == &m_timeStep ||
             field == &m_rftTimeStep || field == &m_rftSegmentBranchIndex || field == &m_rftWellName )
        {
            RiuTools::enableUpDownArrowsForComboBox( attribute );
        }

        QString modifierText;

        if ( field == &m_case )
        {
            modifierText                  = ( "(Shift+" );
            myAttr->minimumContentsLength = 14;
        }
        else if ( field == &m_wellPath || field == &m_simWellName )
        {
            modifierText = ( "(Ctrl+" );
        }
        else if ( field == &m_timeStep )
        {
            modifierText                  = ( "(" );
            myAttr->minimumContentsLength = 12;
        }

        if ( !modifierText.isEmpty() )
        {
            myAttr->nextButtonText = "Next " + modifierText + "PgDown)";
            myAttr->prevButtonText = "Previous " + modifierText + "PgUp)";
        }
    }
    auto* uiDisplayStringAttr = dynamic_cast<caf::PdmUiLineEditorAttributeUiDisplayString*>( attribute );
    if ( uiDisplayStringAttr && ( wbsSmoothingThreshold() == -1.0 ) && ( field == &m_wbsSmoothingThreshold ) )
    {
        QString displayString = "Mixed";

        if ( m_uniqueWbsSmoothingThreshold.size() > 1u )
        {
            auto minmax_it = std::minmax_element( m_uniqueWbsSmoothingThreshold.begin(), m_uniqueWbsSmoothingThreshold.end() );
            displayString += QString( " [%1, %2]" ).arg( *( minmax_it.first ) ).arg( *( minmax_it.second ) );
        }

        uiDisplayStringAttr->m_displayString = displayString;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::modifyCurrentIndex( caf::PdmValueField* field, int indexOffset )
{
    QList<caf::PdmOptionItemInfo> options = calculateValueOptions( field );
    RimDataSourceSteppingTools::modifyCurrentIndex( field, options, indexOffset );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderRftInterface* RimWellLogCurveCommonDataSource::rftReader()
{
    auto eclipseCase = dynamic_cast<RimEclipseResultCase*>( m_case() );
    if ( eclipseCase && eclipseCase->rftReader() ) return eclipseCase->rftReader();

    if ( m_summaryCase() && m_summaryCase()->rftReader() ) return m_summaryCase->rftReader();

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::selectWell( QString wellName )
{
    if ( !m_allow3DSelectionLink() ) return;

    if ( m_trajectoryType() == RimWellLogExtractionCurve::WELL_PATH )
    {
        QList<caf::PdmOptionItemInfo> options;
        RimTools::wellPathOptionItems( &options );

        for ( auto& opt : options )
        {
            if ( opt.optionUiText() == wellName )
            {
                QVariant     oldPath  = m_wellPath.toQVariant();
                RimWellPath* wellPath = RimProject::current()->activeOilField()->wellPathCollection->wellPathByName( wellName );

                m_wellPath = wellPath;
                m_wellPath.uiCapability()->notifyFieldChanged( oldPath, opt.value() );
                break;
            }
        }
    }
    else if ( m_trajectoryType() == RimWellLogExtractionCurve::SIMULATION_WELL )
    {
        auto* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case() );
        if ( eclipseCase )
        {
            std::set<QString> sortedWellNames = eclipseCase->sortedSimWellNames();
            if ( std::count( sortedWellNames.begin(), sortedWellNames.end(), wellName ) > 0 )
            {
                m_simWellName.setValueWithFieldChanged( wellName );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogCurveCommonDataSource::rftWellName() const
{
    return m_rftWellName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QDateTime RimWellLogCurveCommonDataSource::rftTime() const
{
    return m_rftTimeStep();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimWellLogCurveCommonDataSource::rftBranchIndex() const
{
    if ( m_uniqueRftBranchIndices.size() == 1 ) return m_rftSegmentBranchIndex();

    return -1;
}
