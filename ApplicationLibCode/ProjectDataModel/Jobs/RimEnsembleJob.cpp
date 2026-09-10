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

#include "RiaColorTools.h"
#include "RiaFilePathTools.h"
#include "RiaLogging.h"
#include "RiaPreferencesOpm.h"
#include "Summary/RiaSummaryTools.h"

#include "JobCommands/RicRunJobFeature.h"
#include "JobCommands/RicStopJobFeature.h"

#include "RifOpmDeckFileTools.h"

#include "Ensemble/RimSummaryFileSetEnsemble.h"
#include "EnsembleFileSet/RimEnsembleFileSet.h"
#include "EnsembleFileSet/RimEnsembleFileSetTools.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimJobWellSettings.h"
#include "RimOilField.h"
#include "RimOpmFlowJob.h"
#include "RimOpmFlowJobSettings.h"
#include "RimProject.h"
#include "RimReservoirGridEnsemble.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryEnsembleTools.h"
#include "RimTools.h"

#include "RiuGuiTheme.h"

#include "cafPdmUiButton.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiTreeAttributes.h"
#include "cafPdmUiTreeSelectionEditor.h"

#include <QFile>

CAF_PDM_SOURCE_INIT( RimEnsembleJob, "EnsembleJob" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleJob::RimEnsembleJob()
    : m_subJobsCompleted( 0 )
{
    CAF_PDM_InitObject( "Ensemble Job", ":/opm.png" );

    CAF_PDM_InitFieldNoDefault( &m_inputEnsemble, "Ensemble", "Input Ensemble" );
    m_inputEnsemble = nullptr;
    m_inputEnsemble.uiCapability()->setUiReadOnly( true );

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

    CAF_PDM_InitField( &m_createGridEnsemble, "CreateGridEnsemble", false, "Create Grid Ensemble (if missing)" );
    CAF_PDM_InitField( &m_createSummaryEnsemble, "CreateSummaryEnsemble", true, "Create Summary Ensemble (if missing)" );

    CAF_PDM_InitFieldNoDefault( &m_outputEnsembleFileSet, "OutputEnsembleFileSet", "Output Ensemble File Set" );
    m_outputEnsembleFileSet = nullptr;
    m_outputEnsembleFileSet.uiCapability()->setUiTreeChildrenHidden( true );

    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_createGridEnsemble );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_createSummaryEnsemble );

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
    if ( !isRunning() ) return false;

    for ( auto& subJob : m_subJobs() )
    {
        subJob->stop();
    }
    return true;
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
void RimEnsembleJob::setStarted()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleJob::setInputEnsemble( RimReservoirGridEnsemble* ensemble )
{
    m_inputEnsemble = ensemble;

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
        for ( auto* realization : m_inputEnsemble->cases() )
        {
            options.push_back( caf::PdmOptionItemInfo( realization->uiName(), realization ) );
        }
    }
    else if ( fieldNeedingOptions == &m_inputEnsemble )
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

    m_expectedOutputFiles.clear();
    m_subJobs.deleteChildren();
    m_subJobsCompleted = 0;

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
        subJob->setIsChildJob( true );
        m_subJobs.push_back( subJob );

        m_expectedOutputFiles.push_back( real.realizationOutputDir + "/" + real.outputDeckName + ".DATA" );
    }

    updateAllRequiredEditors();

    for ( auto& subJob : m_subJobs() )
    {
        subJob->jobCompleted.connect( this, &RimEnsembleJob::subJobCompleted );
        RicRunJobFeature::runJob( subJob );
    }

    m_inputEnsemble->reloadMetaDataIfNeeded();

    setState( RimGenericJob::Running );
    updateConnectedEditors();

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleJob::subJobCompleted( const caf::SignalEmitter* emitter, bool runOk )
{
    m_subJobsCompleted++;
    if ( m_subJobsCompleted >= m_subJobs.size() )
    {
        setFinished( true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleJob::setFinished( bool runOk )
{
    setState( RimGenericJob::Completed );
    updateConnectedEditors();

    if ( runOk )
    {
        if ( m_outputEnsembleFileSet() == nullptr )
        {
            auto fileSets = RimEnsembleFileSetTools::createEnsembleFileSets( m_expectedOutputFiles );
            if ( fileSets.empty() ) return;
            m_outputEnsembleFileSet = fileSets[0];

            if ( m_createGridEnsemble() )
            {
                RimEnsembleFileSetTools::createGridEnsemblesFromFileSets( { m_outputEnsembleFileSet() } );
            }

            if ( m_createSummaryEnsemble() )
            {
                RimEnsembleFileSetTools::createSummaryEnsemblesFromFileSets( { m_outputEnsembleFileSet() } );
            }
        }
        else
        {
            m_outputEnsembleFileSet()->setRealizationSubSet( "*" );
            m_outputEnsembleFileSet()->reload();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEnsembleJob::RealizationInfo> RimEnsembleJob::getSelectedRealizations() const
{
    if ( !m_inputEnsemble() || !m_inputEnsemble()->ensembleFileSet() ) return {};

    auto [key1, key2] = m_inputEnsemble()->ensembleFileSet()->nameKeys();

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
        info.outputDeckName       = outputPath.stem().string();

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
    genGrp->add( &m_inputEnsemble );
    genGrp->add( &m_createSummaryEnsemble );
    genGrp->add( &m_createGridEnsemble );

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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleJob::defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    static auto warnColor = QColor( RiaColorTools::toQColor( cvf::Color3f( cvf::Color3f::DARK_YELLOW ) ) );
    static auto contrastWarnColor =
        QColor( RiaColorTools::toQColor( RiaColorTools::contrastColor( cvf::Color3f( cvf::Color3f::DARK_YELLOW ) ) ) );

    static auto waitColor = QColor( RiaColorTools::toQColor( cvf::Color3f( cvf::Color3f::LIGHT_GRAY ) ) );
    static auto contrastWaitColor =
        QColor( RiaColorTools::toQColor( RiaColorTools::contrastColor( cvf::Color3f( cvf::Color3f::LIGHT_GRAY ) ) ) );

    if ( auto* treeItemAttribute = dynamic_cast<caf::PdmUiTreeViewItemAttribute*>( attribute ) )
    {
        if ( state() == JobState::Failed )
        {
            auto txt = "Failed";
            auto tag =
                caf::PdmUiTreeViewItemAttribute::createTag( QColor( Qt::red ), RiuGuiTheme::getColorByVariableName( "backgroundColor1" ), txt );
            treeItemAttribute->tags.push_back( std::move( tag ) );
        }
        else if ( ( state() == JobState::Running ) || ( state() == JobState::Completed ) )
        {
            auto tag = caf::PdmUiTreeViewItemAttribute::createTag();

            if ( state() == JobState::Running )
            {
                tag->text = "Running";
            }
            else
            {
                tag->text = "Done";
            }

            cvf::Color3f viewColor     = cvf::Color3f( cvf::Color3f::GREEN );
            cvf::Color3f viewTextColor = RiaColorTools::contrastColor( viewColor );
            tag->bgColor               = QColor( RiaColorTools::toQColor( viewColor ) );
            tag->fgColor               = QColor( RiaColorTools::toQColor( viewTextColor ) );
            treeItemAttribute->tags.push_back( std::move( tag ) );
        }
    }
}
