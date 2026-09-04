/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RimEnsembleJob.h"

#include "RiaFilePathTools.h"
#include "RiaLogging.h"
#include "RiaPreferencesOpm.h"

#include "JobCommands/RicRunJobFeature.h"
#include "JobCommands/RicStopJobFeature.h"

#include "RifOpmDeckFileTools.h"

#include "EnsembleFileSet/RimEnsembleFileSet.h"
#include "RimEclipseCase.h"
#include "RimJobWellSettings.h"
#include "RimOpmFlowJob.h"
#include "RimOpmFlowJobSettings.h"
#include "RimProject.h"
#include "RimReservoirGridEnsemble.h"
#include "RimTools.h"

#include "cafPdmUiButton.h"
#include "cafPdmUiTreeSelectionEditor.h"

#include <QFile>

CAF_PDM_SOURCE_INIT( RimEnsembleJob, "EnsembleJob" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleJob::RimEnsembleJob()
{
    CAF_PDM_InitObject( "Ensemble Job", ":/opm.png" );

    CAF_PDM_InitFieldNoDefault( &m_ensemble, "Ensemble", "Ensemble" );
    m_ensemble = nullptr;
    m_ensemble.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_selectedRealizations, "SelectedRealizations", "Selected Realizations" );
    m_selectedRealizations.uiCapability()->setUiEditorTypeName( caf::PdmUiTreeSelectionEditor::uiEditorTypeName() );
    m_selectedRealizations.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosition::HIDDEN );

    CAF_PDM_InitField( &m_outputIterationNumber, "OutputIterationNumber", 0, "Output Iteration Number" );
    CAF_PDM_InitFieldNoDefault( &m_subJobs, "SubJobs", "Jobs" );

    CAF_PDM_InitFieldNoDefault( &m_jobSettings, "JobSettings", "Opm Flow Settings" );
    m_jobSettings = RiaPreferencesOpm::current()->createDefaultJobSettings();
    m_jobSettings.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_jobWellSettings, "JobWellSettings", "Job Well Settings" );
    m_jobWellSettings = new RimJobWellSettings();
    m_jobWellSettings.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_wellGroupsInInputDeck, "WellGroupsInInputDeck", "Well Groups in Input Deck" );
    m_wellGroupsInInputDeck.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_datesInInputDeck, "DatesInInputDeck", "Dates in Input Deck" );
    m_datesInInputDeck.uiCapability()->setUiHidden( true );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleJob::~RimEnsembleJob()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleJob::stop()
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimEnsembleJob::percentageDone() const
{
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QStringList RimEnsembleJob::jobLog() const
{
    return QStringList();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleJob::matchesKeyValue( const QString& key, const QString& value ) const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleJob::setFinished( bool runOk )
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleJob::setStarted()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleJob::setEnsemble( RimReservoirGridEnsemble* ensemble )
{
    m_ensemble = ensemble;

    if ( ensemble == nullptr ) return;
    if ( ensemble->cases().empty() ) return;

    auto deckName = RiaFilePathTools::replaceFileExtension( ensemble->cases()[0]->gridFileName().toStdString(), "DATA" );

    m_datesInInputDeck = RifOpmDeckFileTools::datesInDeckFile( deckName );

    std::vector<QString> newWellGroups;
    for ( auto grp : RifOpmDeckFileTools::wellGroupsInFileDeck( deckName ) )
    {
        newWellGroups.push_back( QString::fromStdString( grp ) );
    }
    m_wellGroupsInInputDeck = newWellGroups;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimEnsembleJob::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_selectedRealizations )
    {
        for ( auto* realization : m_ensemble->cases() )
        {
            options.push_back( caf::PdmOptionItemInfo( realization->uiName(), realization ) );
        }
    }
    else if ( fieldNeedingOptions == &m_ensemble )
    {
        RimTools::reservoirGridEnsembleOptionItems( &options );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RimEnsembleJob::getSelectedRealizationFileNames() const
{
    std::vector<std::string> fileNames;
    for ( auto& realization : m_selectedRealizations.value() )
    {
        if ( realization.notNull() )
        {
            auto locationOnDisk = realization->gridFileName().toStdString();
            if ( QFile::exists( QString::fromStdString( locationOnDisk ) ) ) fileNames.push_back( locationOnDisk );
        }
    }
    return fileNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleJob::execute()
{
    auto realizations = getSelectedRealizations();

    m_subJobs.deleteChildren();
    updateAllRequiredEditors();

    for ( auto& real : realizations )
    {
        qDebug() << real.inputCase->uiName() << "Input Deck: " << QString::fromStdString( real.realizationInputDeckName )
                 << "Output Dir: " << QString::fromStdString( real.realizationOutputDir );

        RimOpmFlowJob* subJob = new RimOpmFlowJob();
        subJob->setEclipseCase( real.inputCase );
        subJob->setInputDataFile( QString::fromStdString( real.realizationInputDeckName ) );
        subJob->setWorkingDirectory( QString::fromStdString( real.realizationOutputDir ) );
        subJob->setName( real.inputCase->uiName() );
        subJob->setJobWellSettings( m_jobWellSettings.value() );
        subJob->setAutoLoadResults( false );
        m_subJobs.push_back( subJob );
    }

    updateAllRequiredEditors();

    for ( auto& subJob : m_subJobs() )
    {
        RicRunJobFeature::runJob( subJob );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEnsembleJob::RealizationInfo> RimEnsembleJob::getSelectedRealizations() const
{
    if ( !m_ensemble() || !m_ensemble()->ensembleFileSet() ) return {};

    auto [key1, key2] = m_ensemble()->ensembleFileSet()->nameKeys();

    std::vector<RealizationInfo> realizationInfos;
    for ( RimEclipseCase* realization : m_selectedRealizations.value() )
    {
        RealizationInfo info;

        // the case to use for well data extraction
        info.inputCase = realization;
        // the deck file to use as input when creating new input deck for job
        info.realizationInputDeckName = RiaFilePathTools::replaceFileExtension( realization->gridFileName().toStdString(), "DATA" );
        if ( !QFile::exists( QString::fromStdString( info.realizationInputDeckName ) ) )
        {
            RiaLogging::error( std::format( "Deck file {} does not exist, skipping realization.", info.realizationInputDeckName ) );
            continue;
        }

        // replace the output iteration folder (key2 for FMU) with the well planning iteration folder
        auto outputFileName = RiaFilePathTools::replaceSubFolderInPath( info.realizationInputDeckName, key2, outputIteration() );
        std::filesystem::path outputPath( outputFileName );
        info.realizationOutputDir = outputPath.parent_path().string();

        // make sure the output folder exists
        if ( !std::filesystem::exists( info.realizationOutputDir ) )
        {
            std::filesystem::create_directories( info.realizationOutputDir );
        }

        realizationInfos.push_back( info );
    }
    return realizationInfos;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimEnsembleJob::outputIteration() const
{
    return std::format( "wp-{}", m_outputIterationNumber.value() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleJob::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( isRunning() )
    {
        auto runGrp     = uiOrdering.addNewGroup( "Running" );
        auto stopButton = runGrp->addNewButton( "Stop", [this]() { RicStopJobFeature::stopJob( this ); } );
        stopButton->setUiIconFromResourceString( ":/stop.svg" );
        stopButton->setAlignment( Qt::AlignCenter );
        uiOrdering.skipRemainingFields();
        return;
    }

    auto genGrp = uiOrdering.addNewGroup( "General" );
    genGrp->add( nameField() );
    genGrp->add( &m_ensemble );

    auto realGrp = uiOrdering.addNewGroup( "Realizations" );
    realGrp->add( &m_selectedRealizations );

    m_jobWellSettings->useDateStrings( dateStrings() );
    m_jobWellSettings->useWellGroups( m_wellGroupsInInputDeck.value() );
    m_jobWellSettings->uiOrdering( realGrp );

    auto opmGrp = uiOrdering.addNewGroup( "OPM Flow" );

    auto runButton = opmGrp->addNewButton( "Run", [this]() { RicRunJobFeature::runJob( this ); } );
    runButton->setUiIconFromResourceString( ":/Play.svg" );
    runButton->setAlignment( Qt::AlignCenter );

    m_jobSettings->uiOrdering( opmGrp, false /* expand by default */ );

    auto advGrp = uiOrdering.addNewGroup( "Advanced" );
    advGrp->setCollapsedByDefault();
    advGrp->add( &m_outputIterationNumber );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimEnsembleJob::dateStrings() const
{
    std::vector<QString> dates;
    for ( auto& dt : m_datesInInputDeck.value() )
    {
        dates.push_back( dt.toString( "yyyy-MM-dd" ) );
    }
    return dates;
}
