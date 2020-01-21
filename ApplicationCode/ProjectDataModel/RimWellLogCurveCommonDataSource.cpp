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

#include "RimCase.h"
#include "RimDataSourceSteppingTools.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultCase.h"
#include "RimGeoMechCase.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimWellFlowRateCurve.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogFileCurve.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellLogTrack.h"
#include "RimWellLogWbsCurve.h"
#include "RimWellMeasurementCurve.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "RiaApplication.h"
#include "RiaSimWellBranchTools.h"

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
    : m_caseType( RiaDefines::UNDEFINED_CASE )
{
    CAF_PDM_InitObject( "Change Data Source", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_case, "CurveCase", "Case", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_trajectoryType, "TrajectoryType", "Trajectory Type", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_wellPath, "CurveWellPath", "Well Name", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_simWellName, "SimulationWellName", "Well Name", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_branchDetection,
                                "BranchDetection",
                                "Branch Detection",
                                "",
                                "Compute branches based on how simulation well cells are organized",
                                "" );
    m_branchDetection.v() = caf::Tristate::State::PartiallyTrue;
    m_branchDetection.uiCapability()->setUiEditorTypeName( caf::PdmUiCheckBoxTristateEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_branchIndex, "Branch", -1, "Branch Index", "", "", "" );

    CAF_PDM_InitField( &m_timeStep, "CurveTimeStep", -1, "Time Step", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_wbsSmoothing, "WBSSmoothing", "Smooth Curves", "", "", "" );
    m_wbsSmoothing.uiCapability()->setUiEditorTypeName( caf::PdmUiCheckBoxTristateEditor::uiEditorTypeName() );
    m_wbsSmoothing.v() = caf::Tristate::State::PartiallyTrue;

    CAF_PDM_InitField( &m_wbsSmoothingThreshold, "WBSSmoothingThreshold", -1.0, "Smoothing Threshold", "", "", "" );

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
void RimWellLogCurveCommonDataSource::setCaseToApply( RimCase* val )
{
    m_case = val;
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
void RimWellLogCurveCommonDataSource::resetDefaultOptions()
{
    setCaseToApply( nullptr );
    setTrajectoryTypeToApply( -1 );
    setWellPathToApply( nullptr );
    setBranchIndexToApply( -1 );
    setBranchDetectionToApply( caf::Tristate::State::PartiallyTrue );
    setSimWellNameToApply( QString( "" ) );
    setTimeStepToApply( -1 );
    setWbsSmoothingToApply( caf::Tristate::State::PartiallyTrue );
    setWbsSmoothingThreshold( -1.0 );

    m_uniqueCases.clear();
    m_uniqueTrajectoryTypes.clear();
    m_uniqueWellPaths.clear();
    m_uniqueWellNames.clear();
    m_uniqueTimeSteps.clear();
    m_uniqueBranchIndices.clear();
    m_uniqueBranchDetection.clear();
    m_uniqueWbsSmoothing.clear();
    m_uniqueWbsSmoothingThreshold.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::updateDefaultOptions( const std::vector<RimWellLogCurve*>& curves,
                                                            const std::vector<RimWellLogTrack*>& tracks )
{
    // Reset all options in the UI
    resetDefaultOptions();

    // Check to see if the parameters are unique
    for ( RimWellLogCurve* curve : curves )
    {
        if ( !curve->isCurveVisible() )
        {
            continue;
        }
        RimWellLogExtractionCurve* extractionCurve = dynamic_cast<RimWellLogExtractionCurve*>( curve );
        RimWellLogFileCurve*       fileCurve       = dynamic_cast<RimWellLogFileCurve*>( curve );
        RimWellFlowRateCurve*      flowRateCurve   = dynamic_cast<RimWellFlowRateCurve*>( curve );
        if ( extractionCurve )
        {
            RimWellLogWbsCurve* wbsCurve = dynamic_cast<RimWellLogWbsCurve*>( extractionCurve );
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
            setBranchDetectionToApply( *m_uniqueBranchDetection.begin() == true ? caf::Tristate::State::True
                                                                                : caf::Tristate::State::False );
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
        setWbsSmoothingToApply( *m_uniqueWbsSmoothing.begin() == true ? caf::Tristate::State::True
                                                                      : caf::Tristate::State::False );
    }

    if ( m_uniqueWbsSmoothingThreshold.size() == 1u )
    {
        setWbsSmoothingThreshold( *m_uniqueWbsSmoothingThreshold.begin() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::updateDefaultOptions()
{
    RimWellLogPlot* parentPlot = nullptr;
    this->firstAncestorOrThisOfType( parentPlot );
    if ( parentPlot )
    {
        std::vector<RimWellLogCurve*> curves;
        parentPlot->descendantsIncludingThisOfType( curves );

        std::vector<RimWellLogTrack*> tracks;
        parentPlot->descendantsIncludingThisOfType( tracks );

        this->updateDefaultOptions( curves, tracks );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::updateCurvesAndTracks( const std::vector<RimWellLogCurve*>& curves,
                                                             const std::vector<RimWellLogTrack*>& tracks )
{
    std::set<RimWellLogPlot*> plots;
    for ( RimWellLogCurve* curve : curves )
    {
        if ( !curve->isCurveVisible() )
        {
            continue;
        }
        RimWellLogFileCurve*       fileCurve        = dynamic_cast<RimWellLogFileCurve*>( curve );
        RimWellLogExtractionCurve* extractionCurve  = dynamic_cast<RimWellLogExtractionCurve*>( curve );
        RimWellMeasurementCurve*   measurementCurve = dynamic_cast<RimWellMeasurementCurve*>( curve );
        if ( fileCurve )
        {
            if ( wellPathToApply() != nullptr )
            {
                fileCurve->setWellPath( wellPathToApply() );
                if ( !fileCurve->wellLogChannelName().isEmpty() )
                {
                    RimWellLogFile* logFile = wellPathToApply()->firstWellLogFileMatchingChannelName(
                        fileCurve->wellLogChannelName() );
                    fileCurve->setWellLogFile( logFile );
                    RimWellLogPlot* parentPlot = nullptr;
                    fileCurve->firstAncestorOrThisOfTypeAsserted( parentPlot );
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
                extractionCurve->setTrajectoryType(
                    static_cast<RimWellLogExtractionCurve::TrajectoryType>( m_trajectoryType() ) );
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

            RimWellLogWbsCurve* wbsCurve = dynamic_cast<RimWellLogWbsCurve*>( extractionCurve );
            if ( wbsCurve )
            {
                if ( !wbsSmoothingToApply().isPartiallyTrue() )
                {
                    wbsCurve->setSmoothCurve( wbsSmoothingToApply().isTrue() );
                    updatedSomething = true;
                }

                if ( wbsSmoothingThreshold() != 1.0 )
                {
                    wbsCurve->setSmoothingThreshold( wbsSmoothingThreshold() );
                    updatedSomething = true;
                }
            }
            if ( updatedSomething )
            {
                RimWellLogPlot* parentPlot = nullptr;
                extractionCurve->firstAncestorOrThisOfTypeAsserted( parentPlot );
                plots.insert( parentPlot );
                curve->updateConnectedEditors();
            }
        }
        else if ( measurementCurve )
        {
            if ( wellPathToApply() != nullptr )
            {
                measurementCurve->setWellPath( wellPathToApply() );
            }
        }
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
            RimWellLogPlot* parentPlot = nullptr;
            track->firstAncestorOrThisOfTypeAsserted( parentPlot );
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
void RimWellLogCurveCommonDataSource::updateCurvesAndTracks()
{
    RimWellLogPlot* parentPlot = nullptr;
    this->firstAncestorOrThisOfType( parentPlot );
    if ( parentPlot )
    {
        std::vector<RimWellLogCurve*> curves;
        parentPlot->descendantsIncludingThisOfType( curves );

        std::vector<RimWellLogTrack*> tracks;
        parentPlot->descendantsIncludingThisOfType( tracks );

        this->updateCurvesAndTracks( curves, tracks );
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
    updateDefaultOptions();

    std::vector<caf::PdmFieldHandle*> fieldsToDisplay;
    fieldsToDisplay.push_back( &m_case );
    if ( trajectoryTypeToApply() == RimWellLogExtractionCurve::WELL_PATH )
    {
        fieldsToDisplay.push_back( &m_wellPath );
    }
    else if ( trajectoryTypeToApply() == RimWellLogExtractionCurve::SIMULATION_WELL )
    {
        fieldsToDisplay.push_back( &m_simWellName );
    }
    fieldsToDisplay.push_back( &m_timeStep );
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
void RimWellLogCurveCommonDataSource::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                        const QVariant&            oldValue,
                                                        const QVariant&            newValue )
{
    RimWellLogPlot* parentPlot = nullptr;
    this->firstAncestorOrThisOfType( parentPlot );

    if ( changedField == &m_branchDetection )
    {
        if ( m_branchDetection().isPartiallyTrue() )
        {
            // The Tristate is cycled from false -> partially true -> true
            // Partially true is used for "Mixed state" and is not settable by the user so cycle on to true.
            m_branchDetection.v() = caf::Tristate::State::True;
        }
    }
    if ( changedField == &m_wbsSmoothing )
    {
        if ( m_wbsSmoothing().isPartiallyTrue() )
        {
            m_wbsSmoothing.v() = caf::Tristate::State::True;
        }
    }

    this->updateCurvesAndTracks();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RimWellLogCurveCommonDataSource::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                            bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    this->updateDefaultOptions();

    if ( fieldNeedingOptions == &m_case )
    {
        if ( m_caseType == RiaDefines::GEOMECH_ODB_CASE )
        {
            RimTools::geoMechCaseOptionItems( &options );
        }
        else if ( m_caseType == RiaDefines::ECLIPSE_RESULT_CASE || m_caseType == RiaDefines::ECLIPSE_INPUT_CASE ||
                  m_caseType == RiaDefines::ECLIPSE_SOURCE_CASE || m_caseType == RiaDefines::ECLIPSE_STAT_CASE )
        {
            RimTools::eclipseCaseOptionItems( &options );
        }
        else
        {
            RimTools::caseOptionItems( &options );
        }

        if ( caseToApply() == nullptr )
        {
            if ( !m_uniqueCases.empty() )
            {
                options.push_front( caf::PdmOptionItemInfo( "Mixed Cases", nullptr ) );
            }
            else
            {
                options.push_front( caf::PdmOptionItemInfo( "None", nullptr ) );
            }
        }
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
        std::vector<RimWellLogExtractionCurve::TrajectoryType> trajectoryTypes =
            {RimWellLogExtractionCurve::WELL_PATH, RimWellLogExtractionCurve::SIMULATION_WELL};
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
        QStringList timeStepNames;

        if ( m_case )
        {
            timeStepNames = m_case->timeStepStrings();
        }

        for ( int i = 0; i < timeStepNames.size(); i++ )
        {
            options.push_back( caf::PdmOptionItemInfo( timeStepNames[i], i ) );
        }

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
    else if ( fieldNeedingOptions == &m_simWellName )
    {
        RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case() );
        if ( eclipseCase )
        {
            std::set<QString> sortedWellNames = eclipseCase->sortedSimWellNames();

            caf::QIconProvider simWellIcon( ":/Well.png" );
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

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    updateDefaultOptions();

    caf::PdmUiGroup* group = uiOrdering.addNewGroup( "Data Source" );
    group->add( &m_case );

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( m_case() );
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
    }
    else
    {
        group->add( &m_wellPath );
    }
    group->add( &m_timeStep );

    if ( uiConfigName == smoothingUiOrderinglabel() )
    {
        group->add( &m_wbsSmoothing );
        group->add( &m_wbsSmoothingThreshold );
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurveCommonDataSource::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                             QString                    uiConfigName,
                                                             caf::PdmUiEditorAttribute* attribute )
{
    caf::PdmUiComboBoxEditorAttribute* myAttr = dynamic_cast<caf::PdmUiComboBoxEditorAttribute*>( attribute );
    if ( myAttr )
    {
        if ( field == &m_case || field == &m_simWellName || field == &m_wellPath || field == &m_timeStep )
        {
            myAttr->showPreviousAndNextButtons = true;
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
    caf::PdmUiLineEditorAttributeUiDisplayString* uiDisplayStringAttr =
        dynamic_cast<caf::PdmUiLineEditorAttributeUiDisplayString*>( attribute );
    if ( uiDisplayStringAttr && wbsSmoothingThreshold() == -1.0 )
    {
        QString displayString = "Mixed";

        if ( m_uniqueWbsSmoothingThreshold.size() > 1u )
        {
            auto minmax_it = std::minmax_element( m_uniqueWbsSmoothingThreshold.begin(),
                                                  m_uniqueWbsSmoothingThreshold.end() );
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
    bool                          useOptionsOnly;
    QList<caf::PdmOptionItemInfo> options = calculateValueOptions( field, &useOptionsOnly );
    RimDataSourceSteppingTools::modifyCurrentIndex( field, options, indexOffset );
}
