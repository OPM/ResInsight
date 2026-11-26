/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RimOpmFlowJob.h"

#include "RiaGuiApplication.h"
#include "RiaImportEclipseCaseTools.h"
#include "RiaLogging.h"
#include "RiaPreferencesOpm.h"
#include "RiaQStringFormatter.h"
#include "RiaWslTools.h"

#include "EclipseCommands/RicCreateGridCaseEnsemblesFromFilesFeature.h"
#include "ExportCommands/RicEclipseCellResultToFileImpl.h"
#include "JobCommands/RicRunJobFeature.h"
#include "JobCommands/RicStopJobFeature.h"

#include "RifOpmFlowDeckFile.h"

#include "RigWellRegionResultTools.h"

#include "Ensemble/RimSummaryFileSetEnsemble.h"
#include "EnsembleFileSet/RimEnsembleFileSet.h"
#include "EnsembleFileSet/RimEnsembleFileSetTools.h"
#include "Polygons/RimPolygon.h"
#include "RimCase.h"
#include "RimDeckPositionDlg.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseCaseEnsemble.h"
#include "RimFishbones.h"
#include "RimJobWellSettings.h"
#include "RimKeywordFactory.h"
#include "RimKeywordWconinje.h"
#include "RimKeywordWconprod.h"
#include "RimOilField.h"
#include "RimOpmFlowJobFileHelper.h"
#include "RimOpmFlowJobSettings.h"
#include "RimOpmFlowJobTools.h"
#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimProject.h"
#include "RimReloadCaseTools.h"
#include "RimSummaryEnsemble.h"
#include "RimTools.h"
#include "RimWellPath.h"
#include "RimWellPathCompletionSettings.h"
#include "RimWellPathFracture.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmUiButton.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiFilePathEditor.h"
#include "cafProgressInfo.h"

#include "opm/input/eclipse/Deck/DeckKeyword.hpp"

#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextStream>

CAF_PDM_SOURCE_INIT( RimOpmFlowJob, "OpmFlowJob" );

namespace caf
{
template <>
void caf::AppEnum<RimOpmFlowJob::DateAppendType>::setUp()
{
    addItem( RimOpmFlowJob::DateAppendType::ADD_DAYS, "AppendDays", "Day(s)" );
    addItem( RimOpmFlowJob::DateAppendType::ADD_MONTHS, "AppendMonths", "Month(s)" );

    setDefault( RimOpmFlowJob::DateAppendType::ADD_MONTHS );
}

} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimOpmFlowJob::RimOpmFlowJob()
    : m_fileDeckHasDates( false )
    , m_fileDeckIsRestart( false )
    , m_startStepForProgress( -1 )
    , jobCompleted( this )
    , m_fileHelper( std::make_unique<RimOpmFlowJobFileHelper>() )
{
    CAF_PDM_InitObject( "Opm Flow Simulation", ":/opm.png" );

    CAF_PDM_InitFieldNoDefault( &m_deckFileName, "DeckFile", "Input Data File" );
    m_deckFileName.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_workDir, "WorkDirectory", "Working Folder" );
    m_workDir.uiCapability()->setAttribute( caf::PdmUiFilePathEditor::Keys::SELECT_DIRECTORY, true );
    CAF_PDM_InitFieldNoDefault( &m_wellPath, "WellPath", "Well Path for New Well" );
    CAF_PDM_InitFieldNoDefault( &m_eclipseCase, "EclipseCase", "Eclipse Case for Well Data", "", "Eclipse Case to use for Well Data" );
    CAF_PDM_InitFieldNoDefault( &m_gridEnsemble, "GridEnsemble", "Grid Ensemble" );
    CAF_PDM_InitFieldNoDefault( &m_summaryEnsemble, "SummaryEnsemble", "Summary Ensemble" );
    CAF_PDM_InitFieldNoDefault( &m_sectorPolygon, "SectorPolygon", "Sector Polygon" );

    CAF_PDM_InitField( &m_pauseBeforeRun, "PauseBeforeRun", false, "Pause before running OPM Flow" );
    CAF_PDM_InitField( &m_addNewWell, "AddNewWell", true, "Add New Well" );
    CAF_PDM_InitField( &m_openWellDeckPosition, "OpenWellDeckPosition", -1, "Open Well at Keyword Index" );
    CAF_PDM_InitField( &m_includeMSWData, "IncludeMswData", false, "Include MSW Data" );
    CAF_PDM_InitField( &m_addToEnsemble, "AddToEnsemble", false, "Add Runs to Ensemble" );
    CAF_PDM_InitField( &m_useRestart, "UseRestart", false, "Restart Simulation at Well Open Date" );
    CAF_PDM_InitField( &m_currentRunId, "CurrentRunID", 0, "Current Ensemble Run ID" );
    m_currentRunId.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_wellGroupName, "WellGroupName", "Well Group Name" );
    m_wellGroupName.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    m_wellGroupName.uiCapability()->setAttribute( caf::PdmUiComboBoxEditor::Keys::ENABLE_EDITABLE_CONTENT, true );
    m_wellGroupName.uiCapability()->setAttribute( caf::PdmUiComboBoxEditor::Keys::ENABLE_AUTO_COMPLETE, false );
    m_wellGroupName.uiCapability()->setAttribute( caf::PdmUiComboBoxEditor::Keys::ADJUST_WIDTH_TO_CONTENTS, true );

    CAF_PDM_InitField( &m_wellOpenType,
                       "WellOpenType",
                       caf::AppEnum<RimJobWellSettings::WellOpenType>( RimJobWellSettings::WellOpenType::OPEN_AT_DATE ),
                       "Open Well" );
    CAF_PDM_InitField( &m_wellOpenKeyword, "WellOpenKeyword", QString( "WCONPROD" ), "Open Well Keyword" );
    m_wellOpenKeyword.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    m_wellOpenKeyword.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_wconprodKeyword, "WconprodKeyword", "WCONPROD Settings" );
    m_wconprodKeyword = new RimKeywordWconprod();
    m_wconprodKeyword.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_wconinjeKeyword, "WconinjeKeyword", "WCONINJE Settings" );
    m_wconinjeKeyword = new RimKeywordWconinje();
    m_wconinjeKeyword.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_jobSettings, "JobSettings", "Opm Flow Settings" );
    m_jobSettings = RiaPreferencesOpm::current()->createDefaultJobSettings();
    m_jobSettings.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitField( &m_openTimeStep, "OpenTimeStep", 0, " " );
    CAF_PDM_InitField( &m_endTimeStep, "EndTimeStep", 0, " " );
    CAF_PDM_InitField( &m_endTimeStepEnabled, "EndTimeStepEnabled", false, "Stop Simulation at Date" );

    CAF_PDM_InitField( &m_appendNewDates, "AppendNewDates", false, "Append Extra Dates" );
    CAF_PDM_InitField( &m_newDatesInterval, "NewDatesInterval", 1, "Interval" );
    CAF_PDM_InitField( &m_numberOfNewDates, "NumberOfNewDates", 12, "Number of Dates to Append" );
    CAF_PDM_InitField( &m_dateAppendType, "DateAppendType", caf::AppEnum<DateAppendType>( DateAppendType::ADD_MONTHS ), " " );

    CAF_PDM_InitField( &m_autoLoadResults, "AutoLoadResults", true, "Automatic loading of results from job" );
    m_autoLoadResults.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_insertFluxall, "InsertFluxall", false, "Insert FLUXALL Keyword" );
    CAF_PDM_InitField( &m_createSectorModel, "CreateSectorModel", false, "Create Sector Model" );

    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_addToEnsemble );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_pauseBeforeRun );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_useRestart );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_includeMSWData );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_addNewWell );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_endTimeStepEnabled );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_appendNewDates );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_insertFluxall );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_createSectorModel );

    setDeletable( true );

    m_fileHelper->setUseEnsembleNames( m_addToEnsemble() ? m_currentRunId() : -1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimOpmFlowJob::~RimOpmFlowJob()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJob::initAfterRead()
{
    m_fileHelper->setName( name() );
    m_fileHelper->setUseEnsembleNames( m_addToEnsemble() ? m_currentRunId() : -1 );
    m_fileHelper->setWorkingDirectory( m_workDir().path() );

    openDeckFile();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJob::setName( const QString& name )
{
    RimNamedObject::setName( name );
    m_fileHelper->setName( name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJob::processLogOutput( const QString& logLine )
{
    // progress output parsing
    // Example log lines:
    // Report step 757/773 at day 9466/10958, date = 01-Dec-2025
    // Report step 758/773 at day 9497/10958, date = 01-Jan-2026
    if ( logLine.startsWith( "Report step" ) )
    {
        auto parts = logLine.split( ' ', Qt::SkipEmptyParts );
        if ( parts.size() >= 4 )
        {
            auto stepPart  = parts[2]; // 756/773
            auto stepParts = stepPart.split( '/' );
            if ( stepParts.size() == 2 )
            {
                bool ok1         = false;
                bool ok2         = false;
                int  currentStep = stepParts[0].toInt( &ok1 );
                if ( ok1 && ( m_startStepForProgress < 0 ) )
                {
                    m_startStepForProgress = currentStep;
                }

                int totalSteps = stepParts[1].toInt( &ok2 );
                if ( ok1 && ok2 )
                {
                    auto noSteps = totalSteps - m_startStepForProgress;
                    if ( noSteps > 0 )
                    {
                        double perc = ( (double)( currentStep - m_startStepForProgress ) / (double)( noSteps ) ) * 100.0;
                        perc        = std::max( 0.0, perc );
                        perc        = std::min( perc, 100.0 );
                        onProgress( perc );
                    }
                }
            }
        }
    }
    else if ( logLine.startsWith( "Warning" ) )
    {
        m_warningsDetected++;
    }
    else if ( logLine.startsWith( "Problem" ) )
    {
        m_warningsDetected++;
    }
    else if ( logLine.startsWith( "Error" ) )
    {
        m_errorsDetected++;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimOpmFlowJob::matchesKeyValue( const QString& key, const QString& value ) const
{
    if ( key == jobInputFileKey() )
    {
        return ( m_deckFileName() == value );
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJob::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( isRunning() )
    {
        m_workDir.uiCapability()->setUiReadOnly( true );
        auto runGrp = uiOrdering.addNewGroup( "Running" );
        runGrp->add( &m_workDir );
        auto stopButton = runGrp->addNewButton( "Stop", [this]() { RicStopJobFeature::stopJob( this ); } );
        stopButton->setUiIconFromResourceString( ":/stop.svg" );
        stopButton->setAlignment( Qt::AlignCenter );
        uiOrdering.skipRemainingFields();
        return;
    }

    if ( isChildJob() )
    {
        m_workDir.uiCapability()->setUiReadOnly( true );
        uiOrdering.add( &m_workDir );
        uiOrdering.skipRemainingFields();
        return;
    }

    m_workDir.uiCapability()->setUiReadOnly( false );
    auto genGrp = uiOrdering.addNewGroup( "General" );
    genGrp->add( nameField() );
    genGrp->add( &m_deckFileName );
    genGrp->add( &m_workDir );
    genGrp->add( &m_addToEnsemble );

    auto sectorGrp = uiOrdering.addNewGroup( "Sector Model" );
    sectorGrp->add( &m_createSectorModel );
    if ( m_createSectorModel() )
    {
        sectorGrp->add( &m_sectorPolygon );
    }

    auto runButton = genGrp->addNewButton( "Run", [this]() { RicRunJobFeature::runJob( this ); } );
    runButton->setUiIconFromResourceString( ":/Play.svg" );
    runButton->setAlignment( Qt::AlignCenter );

    if ( m_eclipseCase() == nullptr )
    {
        m_addNewWell = false;
    }
    else
    {
        auto wellGrp = uiOrdering.addNewGroup( "New Well Settings" );
        wellGrp->add( &m_addNewWell );
        wellGrp->add( &m_eclipseCase );

        if ( m_addNewWell() )
        {
            wellGrp->add( &m_wellPath );
            wellGrp->add( &m_wellGroupName );
            wellGrp->add( &m_wellOpenKeyword );
            if ( m_wellOpenKeyword() == "WCONPROD" )
            {
                auto wconGrp = wellGrp->addNewGroup( "WCONPROD Settings" );
                m_wconprodKeyword->uiOrdering( wconGrp );
                wconGrp->setCollapsedByDefault();
            }
            else
            {
                auto wconGrp = wellGrp->addNewGroup( "WCONINJE Settings" );
                m_wconinjeKeyword->uiOrdering( wconGrp );
                wconGrp->setCollapsedByDefault();
            }

            wellGrp->add( &m_wellOpenType );

            bool createOpenPostionButton = false;

            if ( m_fileDeckHasDates )
            {
                if ( m_wellOpenType() == RimJobWellSettings::WellOpenType::OPEN_AT_DATE )
                {
                    wellGrp->add( &m_openTimeStep );
                    if ( !m_fileDeckIsRestart )
                    {
                        wellGrp->add( &m_useRestart );
                    }
                    else
                    {
                        m_useRestart = false;
                    }
                    if ( m_eclipseCase() ) wellGrp->add( &m_includeMSWData );
                }
                else if ( m_wellOpenType == RimJobWellSettings::WellOpenType::OPEN_BY_POSITION )
                {
                    createOpenPostionButton = true;
                }
                m_wellOpenType.uiCapability()->setUiReadOnly( false );
            }
            else
            {
                createOpenPostionButton = true;

                m_wellOpenType = RimJobWellSettings::WellOpenType::OPEN_BY_POSITION;
                m_wellOpenType.uiCapability()->setUiReadOnly( true );
            }

            if ( createOpenPostionButton )
            {
                wellGrp->addNewButton( "Select Open Keyword Position", [this]() { selectOpenWellPosition(); } );
            }
        }
    }

    if ( m_fileDeckHasDates )
    {
        auto dateGrp = uiOrdering.addNewGroup( "Date Settings" );
        dateGrp->setCollapsedByDefault();

        dateGrp->add( &m_endTimeStepEnabled );
        if ( m_endTimeStepEnabled() )
        {
            dateGrp->add( &m_endTimeStep );
        }

        dateGrp->add( &m_appendNewDates );
        if ( m_appendNewDates() )
        {
            dateGrp->add( &m_numberOfNewDates );
            dateGrp->add( &m_newDatesInterval );
            dateGrp->appendToRow( &m_dateAppendType );
        }
    }

    auto opmGrp = uiOrdering.addNewGroup( "OPM Flow" );
    opmGrp->setCollapsedByDefault();
    opmGrp->add( &m_pauseBeforeRun );

    m_jobSettings->uiOrdering( opmGrp, false /* expand by default */ );

    auto advGrp = uiOrdering.addNewGroup( "Advanced" );
    advGrp->setCollapsedByDefault();
    advGrp->add( &m_insertFluxall );
    advGrp->add( &m_currentRunId );
    advGrp->addNewButton( "Reset Ensemble Run Id", [this]() { resetEnsembleRunId(); } );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimOpmFlowJob::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_wellPath )
    {
        RimTools::wellPathOptionItems( &options );
    }
    else if ( ( fieldNeedingOptions == &m_openTimeStep ) || ( fieldNeedingOptions == &m_endTimeStep ) )
    {
        if ( openDeckFile() )
        {
            auto timeStepNames = dateStrings();
            for ( int i = 0; i < static_cast<int>( timeStepNames.size() - 1 ); ++i )
            {
                options.push_back( caf::PdmOptionItemInfo( timeStepNames[i], QVariant::fromValue( i ) ) );
            }
        }
    }
    else if ( fieldNeedingOptions == &m_wellOpenKeyword )
    {
        options.push_back( caf::PdmOptionItemInfo( "WCONPROD", QVariant::fromValue( QString( "WCONPROD" ) ) ) );
        options.push_back( caf::PdmOptionItemInfo( "WCONINJE", QVariant::fromValue( QString( "WCONINJE" ) ) ) );
    }
    else if ( fieldNeedingOptions == &m_wellGroupName )
    {
        for ( auto& grp : wellgroupsInFileDeck() )
        {
            options.push_back( caf::PdmOptionItemInfo( grp, QVariant::fromValue( grp ) ) );
        }
    }
    else if ( fieldNeedingOptions == &m_eclipseCase )
    {
        RimTools::eclipseCaseOptionItems( &options );
    }
    else if ( fieldNeedingOptions == &m_sectorPolygon )
    {
        RimTools::polygonOptionItems( &options );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimOpmFlowJob::dateStrings()
{
    std::vector<QString> dateStrs;

    for ( auto& dt : dateTimes() )
    {
        dateStrs.push_back( dt.toString( "dd-MMM-yyyy  (HH:mm)" ) );
    }

    return dateStrs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RimOpmFlowJob::dateTimes()
{
    auto dates    = datesInFileDeck();
    auto newDates = addedDateTimes();
    dates.insert( dates.end(), newDates.begin(), newDates.end() );

    return dates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RimOpmFlowJob::addedDateTimes()
{
    if ( !m_appendNewDates() || !m_fileDeckHasDates ) return {};

    auto  existingDates = datesInFileDeck();
    auto& startDate     = existingDates.back();

    std::vector<QDateTime> newDates;
    for ( auto dt = startDate; newDates.size() < static_cast<size_t>( m_numberOfNewDates() ); )
    {
        if ( m_dateAppendType() == DateAppendType::ADD_DAYS )
        {
            dt = dt.addDays( m_newDatesInterval() );
        }
        else if ( m_dateAppendType() == DateAppendType::ADD_MONTHS )
        {
            dt = dt.addMonths( m_newDatesInterval() );
        }
        newDates.push_back( dt );
    }

    return newDates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJob::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_wellPath )
    {
        if ( ( m_wellPath() != nullptr ) && ( m_wellPath->completionSettings() != nullptr ) )
        {
            auto wellSettings = m_wellPath->completionSettings();
            if ( !wellSettings->groupName().isEmpty() && m_wellGroupName().isEmpty() )
            {
                m_wellGroupName = wellSettings->groupName();
            }
        }
    }
    else if ( changedField == nameField() )
    {
        m_fileHelper->setName( name() );
    }
    else if ( changedField == &m_workDir )
    {
        m_fileHelper->setWorkingDirectory( m_workDir().path() );
    }
    else if ( changedField == &m_addToEnsemble )
    {
        if ( m_addToEnsemble() )
            m_fileHelper->setUseEnsembleNames( m_currentRunId() );
        else
            m_fileHelper->setUseEnsembleNames( -1 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJob::setWorkingDirectory( QString workDir )
{
    m_fileHelper->setWorkingDirectory( workDir );
    m_workDir = workDir;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimOpmFlowJob::mainWorkingDirectory() const
{
    return m_workDir().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJob::setEclipseCase( RimEclipseCase* eCase )
{
    if ( eCase == nullptr )
    {
        m_deckFileName.setValue( QString() );
        closeDeckFile();
        return;
    }

    QFileInfo fi( eCase->gridFileName() );
    m_deckFileName.setValue( fi.absolutePath() + "/" + fi.completeBaseName() + m_fileHelper->deckExtension() );
    m_eclipseCase = eCase;
    closeDeckFile();
    openDeckFile();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJob::setInputDataFile( QString filename )
{
    m_deckFileName.setValue( filename );
    closeDeckFile();
    openDeckFile();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJob::setJobSettings( RimOpmFlowJobSettings* jobSettings )
{
    m_jobSettings = jobSettings->copyObject<RimOpmFlowJobSettings>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimOpmFlowJob::openDeckFile()
{
    if ( m_deckFile.get() == nullptr )
    {
        m_deckFile      = std::make_unique<RifOpmFlowDeckFile>();
        bool deckLoadOk = false;
        try
        {
            auto loadResult = m_deckFile->loadDeck( m_deckFileName().path().toStdString() );
            if ( loadResult.has_value() )
            {
                m_fileDeckHasDates  = m_deckFile->hasDatesKeyword();
                m_fileDeckIsRestart = m_deckFile->isRestartFile();
                deckLoadOk          = true;
            }
            else
            {
                RiaLogging::error( QString( "Failed to open %1. Error: %2." )
                                       .arg( m_deckFileName().path() )
                                       .arg( QString::fromStdString( loadResult.error() ) )
                                       .toStdString() );
                deckLoadOk = false;
            }
        }
        catch ( std::filesystem::filesystem_error& )
        {
            deckLoadOk = false;
            RiaLogging::error( std::format( "Failed to open {}, possibly unsupported or incorrect format.", m_deckFileName().path() ) );
        }

        if ( !deckLoadOk )
        {
            m_fileDeckHasDates  = false;
            m_fileDeckIsRestart = false;
            closeDeckFile();
        }
    }

    return m_deckFile.get() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJob::closeDeckFile()
{
    m_deckFile.reset();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimOpmFlowJob::copyUnrstFileToWorkDir()
{
    QFileInfo fi( m_deckFileName().path() );

    QString unrstName = fi.absolutePath() + "/" + fi.baseName() + ".UNRST";
    if ( QFile::exists( unrstName ) )
    {
        QString dstName = workingDirectory() + "/" + m_fileHelper->restartDeckName( inputDeckName() ) + ".UNRST";
        if ( dstName != unrstName )
        {
            QFile::copy( unrstName, dstName );
        }
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimOpmFlowJob::workingDirectory() const
{
    return m_fileHelper->workingDirectory();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimOpmFlowJob::inputDeckName() const
{
    QFileInfo fi( m_deckFileName().path() );
    return fi.completeBaseName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimOpmFlowJob::numberOfProcesses()
{
    if ( !RiaPreferencesOpm::current()->useMpi() ) return 1;
    return m_jobSettings()->mpiProcesses();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QStringList> RimOpmFlowJob::commands()
{
    std::vector<QStringList> commands;

    auto opmPref = RiaPreferencesOpm::current();

    QString workDir = workingDirectory();
    if ( workDir.isEmpty() ) return commands;

    QString dataFile = workDir + "/" + m_fileHelper->deckName();
    if ( !QFile::exists( dataFile + m_fileHelper->deckExtension() ) ) return commands;

    // optional make flux
    if ( m_createSectorModel() )
    {
        QStringList cmd;

        QString parent  = m_deckFileName().path();
        QString fluxnum = m_fileHelper->fluxnumFileName( false /*not relative*/ );
        QString output  = m_fileHelper->fluxFileName();

        if ( opmPref->useWsl() )
        {
            parent  = RiaWslTools::convertToWslPath( parent );
            fluxnum = RiaWslTools::convertToWslPath( fluxnum );
            output  = RiaWslTools::convertToWslPath( output );
        }

        QStringList arguments;

        cmd.append( opmPref->makeFluxCommand() );
        cmd << "--parent=" + parent;
        cmd << "--fluxnum=" + fluxnum;
        cmd << "--regions=1";
        cmd << "--output=" + output;

        commands.push_back( cmd );
    }

    // opm flow
    QStringList cmd;

    if ( opmPref->useWsl() )
    {
        workDir  = RiaWslTools::convertToWslPath( workDir );
        dataFile = RiaWslTools::convertToWslPath( dataFile );
    }

    if ( opmPref->useMpi() )
    {
        cmd.append( opmPref->mpirunCommand() );
        cmd.append( QString( "-np" ) );
        cmd.append( QString( "%1" ).arg( numberOfProcesses() ) );
    }

    cmd.append( opmPref->opmFlowCommand() );
    cmd.append( QString( "--output-dir=%1" ).arg( workDir ) );
    cmd.append( QString( "--ecl-deck-file-name=%1" ).arg( dataFile ) );

    cmd.append( m_jobSettings->commandLineOptions( workingDirectory(), workDir ) );

    commands.push_back( cmd );

    return commands;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimOpmFlowJob::onPrepare()
{
    if ( name().isEmpty() )
    {
        RiaLogging::error( "Please set a name for the OPM Flow Job." );
        return false;
    }

    // reload file deck to make sure we start with the original
    closeDeckFile();
    if ( !openDeckFile() )
    {
        RiaLogging::error( "Unable to open input DATA file " + m_deckFileName().path().toStdString() );
        return false;
    }

    // add extra date keywords
    if ( m_appendNewDates() && m_fileDeckHasDates )
    {
        std::vector<std::time_t> newDates;
        for ( auto& dt : addedDateTimes() )
        {
            newDates.push_back( dt.toSecsSinceEpoch() );
        }
        m_deckFile->appendDateKeywords( newDates );
    }

    if ( !addWellIfNeeded() ) return false;

    if ( !setupSectorModelIfNeeded() ) return false;

    if ( m_useRestart() )
    {
        if ( !copyUnrstFileToWorkDir() )
        {
            RiaLogging::error( "Unable to locate UNRST file from input case." );
            return false;
        }

        if ( !m_deckFile->restartAtTimeStep( m_openTimeStep(), m_fileHelper->restartDeckName( inputDeckName() ).toStdString() ) )
        {
            RiaLogging::error( "Unable to insert restart keywords in DATA file." );
            return false;
        }
    }

    if ( m_endTimeStepEnabled() )
    {
        if ( !m_deckFile->stopAtTimeStep( m_endTimeStep() ) )
        {
            RiaLogging::error( "Unable to insert END keyword in DATA file." );
            return false;
        }
    }

    if ( m_insertFluxall() )
    {
        if ( !m_deckFile->findKeyword( "FLUXALL" ).has_value() )
        {
            if ( !m_deckFile->insertKeywordInSection( "SUMMARY", RimKeywordFactory::fluxallKeyword() ) )
            {
                RiaLogging::error( "Unable to insert FLUXALL keyword in DATA file." );
                return false;
            }
        }
    }

    caf::ProgressInfo saveProgress( 1, "Saving updated information to DATA file.", false );

    // save DATA file to working folder
    bool saveOk = m_deckFile->saveDeck( workingDirectory().toStdString(),
                                        m_fileHelper->deckName().toStdString() + m_fileHelper->deckExtension().toStdString() );

    closeDeckFile();

    return saveOk;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimOpmFlowJob::addWellIfNeeded()
{
    // add a new well?
    if ( m_addNewWell() )
    {
        if ( m_wellPath == nullptr )
        {
            RiaLogging::error( "Please select a well path to use." );
            return false;
        }

        if ( m_wellPath->completionSettings() == nullptr )
        {
            RiaLogging::error( "The selected well path has no completions defined." );
            return false;
        }

        auto wellNameInDeck = m_wellPath->completionSettings()->wellName();
        if ( wellNameInDeck.isEmpty() )
        {
            RiaLogging::error( "Selected Well Path does not have a WELL NAME set, please check the completion settings." );
            return false;
        }

        if ( m_wellGroupName().isEmpty() )
        {
            RiaLogging::error( "Please set the well group name." );
            return false;
        }
        m_wellPath->completionSettings()->setGroupName( m_wellGroupName() );

        // well data needs the case to be open, this is not necessarily true for ensemble
        bool needToOpenCase = !m_eclipseCase->isReservoirCaseOpen();
        if ( needToOpenCase )
        {
            m_eclipseCase->ensureReservoirCaseIsOpen();
        }

        int mergePosition = RimOpmFlowJobTools::mergeBasicWellSettings( m_deckFile.get(),
                                                                        m_openWellDeckPosition(),
                                                                        m_eclipseCase(),
                                                                        m_wellPath(),
                                                                        m_wellOpenType(),
                                                                        m_openTimeStep(),
                                                                        m_wellOpenKeyword() );
        if ( mergePosition < 0 )
        {
            if ( needToOpenCase ) m_eclipseCase->closeReservoirCase();
            RiaLogging::error( "Unable to merge new well data into DATA file. Please check file format." );
            return false;
        }

        if ( ( m_includeMSWData ) && ( m_wellOpenType == RimJobWellSettings::WellOpenType::OPEN_AT_DATE ) )
        {
            mergePosition = RimOpmFlowJobTools::mergeMswData( m_deckFile.get(),
                                                              mergePosition,
                                                              m_eclipseCase(),
                                                              m_wellPath(),
                                                              m_wellOpenType(),
                                                              m_openTimeStep() );
            if ( mergePosition < 0 )
            {
                if ( needToOpenCase ) m_eclipseCase->closeReservoirCase();
                RiaLogging::error( "Failed to merge MSW data into file deck." );
                return false;
            }
        }

        // close cases that was previously closed to save memory
        if ( needToOpenCase ) m_eclipseCase->closeReservoirCase();

        Opm::DeckKeyword openKeyword = ( m_wellOpenKeyword() == "WCONPROD" ) ? m_wconprodKeyword->keyword( wellNameInDeck )
                                                                             : m_wconinjeKeyword->keyword( wellNameInDeck );

        // open new well at selected timestep
        if ( m_wellOpenType == RimJobWellSettings::WellOpenType::OPEN_AT_DATE )
        {
            if ( !m_deckFile->openWellAtTimeStep( m_openTimeStep(), openKeyword ) )
            {
                RiaLogging::error( "Unable to open new well at selected timestep in DATA file." );
                return false;
            }
        }
        else
        {
            if ( !m_deckFile->openWellAtDeckPosition( mergePosition, openKeyword ) )
            {
                RiaLogging::error( "Unable to open new well at selected position in DATA file." );
                return false;
            }
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimOpmFlowJob::onRun()
{
    if ( m_pauseBeforeRun() )
    {
        QString infoText = "Input parameter files can now be found in the working folder:";
        infoText += " \"" + workingDirectory() + "\"\n";
        infoText += "\nClick OK to run the Opm Flow simulation.";

        auto parent = RiaGuiApplication::widgetToUseAsParent();

        auto reply = QMessageBox::information( parent, "Opm Flow simulation", infoText, QMessageBox::Ok | QMessageBox::Cancel );

        if ( reply != QMessageBox::Ok ) return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJob::onProgress( double percentageDone )
{
    m_percentageDone = percentageDone;
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimOpmFlowJob::shouldUseWsl()
{
    return RiaPreferencesOpm::current()->useWsl();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJob::onCompleted( bool success )
{
    if ( success && m_autoLoadResults() )
    {
        loadResults();
    }

    jobCompleted.send( success );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJob::loadResults()
{
    QString outputEgridFileName = workingDirectory() + "/" + m_fileHelper->deckName() + ".EGRID";
    if ( !QFile::exists( outputEgridFileName ) ) return;

    if ( m_addToEnsemble() )
    {
        // grid ensemble
        if ( m_gridEnsemble() == nullptr )
        {
            m_gridEnsemble = RicCreateGridCaseEnsemblesFromFilesFeature::importSingleGridCaseEnsemble( QStringList( outputEgridFileName ) );
        }
        else
        {
            if ( auto rimResultCase = m_gridEnsemble->findByFileName( outputEgridFileName ) )
            {
                rimResultCase->reloadEclipseGridFile();
            }
            else
            {
                rimResultCase = RicCreateGridCaseEnsemblesFromFilesFeature::importSingleGridCase( outputEgridFileName );
                m_gridEnsemble->addCase( rimResultCase );
            }

            for ( auto gridCase : m_gridEnsemble->cases() )
            {
                gridCase->updateAutoShortName();
            }

            RimProject::current()->activeOilField()->analysisModels()->updateConnectedEditors();
        }

        // summary ensemble
        if ( m_summaryEnsemble() == nullptr )
        {
            QString pattern   = m_workDir().path() + "/run-*/" + m_fileHelper->baseDeckName() + "-*";
            auto    fileSet   = RimEnsembleFileSetTools::createEnsembleFileSetFromOpm( pattern, name() );
            auto    ensembles = RimEnsembleFileSetTools::createSummaryEnsemblesFromFileSets( { fileSet } );
            if ( !ensembles.empty() )
            {
                m_summaryEnsemble = ensembles[0];
            }
        }
        else
        {
            m_summaryEnsemble->reloadCases();
        }

        m_currentRunId = m_currentRunId + 1;
    }
    else
    {
        if ( auto existingCase = RimOpmFlowJobTools::findExistingCase( outputEgridFileName ) )
        {
            RimReloadCaseTools::reloadEclipseGridAndSummary( existingCase );
            existingCase->setCustomCaseName( name() );
            existingCase->updateConnectedEditors();
        }
        else
        {
            QStringList files( outputEgridFileName );

            RiaImportEclipseCaseTools::FileCaseIdMap newCaseFiles;
            if ( RiaImportEclipseCaseTools::openEclipseCasesFromFile( files, true /*create view*/, &newCaseFiles, false /* dialog */ ) )
            {
                if ( auto newCase = RimOpmFlowJobTools::findExistingCase( outputEgridFileName ) )
                {
                    newCase->setCustomCaseName( name() );
                    newCase->updateConnectedEditors();
                    if ( m_eclipseCase() == nullptr )
                    {
                        m_eclipseCase = newCase;
                    }

                    Riu3DMainWindowTools::selectAsCurrentItem( newCase );
                    Riu3DMainWindowTools::setExpanded( newCase, true );
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJob::selectOpenWellPosition()
{
    if ( !openDeckFile() ) return;

    std::vector<std::pair<int, QString>> kwVec;

    auto kws = m_deckFile->keywords();
    int  i   = 0;
    for ( auto& kw : kws )
    {
        kwVec.push_back( std::make_pair( i++, QString::fromStdString( kw ) ) );
    }

    m_openWellDeckPosition = RimDeckPositionDlg::askForPosition( RiaGuiApplication::widgetToUseAsParent(),
                                                                 kwVec,
                                                                 "--- Open New Well HERE ---",
                                                                 m_openWellDeckPosition );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJob::resetEnsembleRunId()
{
    if ( QMessageBox::information( RiaGuiApplication::widgetToUseAsParent(),
                                   "Opm Flow Job",
                                   "Do you want to reset the ensemble run ID to 0?",
                                   QMessageBox::Yes | QMessageBox::No ) == QMessageBox::Yes )
    {
        m_currentRunId = 0;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJob::initAfterCopy()
{
    m_currentRunId    = 0;
    m_gridEnsemble    = nullptr;
    m_summaryEnsemble = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RimOpmFlowJob::datesInFileDeck()
{
    std::vector<QDateTime> dates;

    if ( openDeckFile() )
    {
        for ( auto tt : m_deckFile->dates() )
        {
            dates.push_back( QDateTime::fromSecsSinceEpoch( tt ) );
        }
    }

    return dates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimOpmFlowJob::wellgroupsInFileDeck()
{
    std::vector<QString> groups;

    if ( openDeckFile() )
    {
        for ( auto& grp : m_deckFile->wellGroupsInFile() )
        {
            groups.push_back( QString::fromStdString( grp ) );
        }
    }
    return groups;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimOpmFlowJob::jobInputFileKey()
{
    return "OpmFlowInputFile";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJob::setJobWellSettings( RimJobWellSettings* jobWellSettings )
{
    if ( jobWellSettings == nullptr ) return;

    m_addNewWell      = jobWellSettings->addNewWell();
    m_wellPath        = jobWellSettings->wellPath();
    m_wellGroupName   = jobWellSettings->wellGroupName();
    m_openTimeStep    = jobWellSettings->openTimeStep();
    m_wellOpenType    = jobWellSettings->wellOpenType();
    m_wellOpenKeyword = jobWellSettings->wellOpenKeyword();
    m_wconinjeKeyword = jobWellSettings->wconinjeKeyword();
    m_wconprodKeyword = jobWellSettings->wconprodKeyword();
    m_includeMSWData  = jobWellSettings->includeMSWData();

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJob::setAutoLoadResults( bool autoLoadResults )
{
    m_autoLoadResults = autoLoadResults;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJob::setSectorPolygon( RimPolygon* polygon )
{
    m_sectorPolygon     = polygon;
    m_createSectorModel = ( polygon != nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimOpmFlowJob::setupSectorModelIfNeeded()
{
    if ( !m_createSectorModel() ) return true;
    if ( ( m_eclipseCase() == nullptr ) || ( m_sectorPolygon() == nullptr ) ) return false;

    // we need the case to be open, this is not necessarily true for ensembles
    bool needToOpenCase = !m_eclipseCase->isReservoirCaseOpen();
    if ( needToOpenCase )
    {
        m_eclipseCase->ensureReservoirCaseIsOpen();
    }

    auto   timeSteps   = m_eclipseCase()->timeStepDates();
    size_t timeStepIdx = timeSteps.size() - 1; // use last available timestep

    // calculcate fluxnum result using RigWellRegionResultTools::createWellRegionResult

    RigWellPath*              wellPathGeo = nullptr;
    std::optional<RigMdRange> mdRange;
    if ( m_wellPath() != nullptr )
    {
        wellPathGeo = m_wellPath()->wellPathGeometry();

        mdRange = m_wellPath->perforationIntervalCollection()->activePerforationMdRange( timeSteps[timeStepIdx] );
    }

    if ( !RigWellRegionResultTools::createWellRegionResult( m_eclipseCase()->eclipseCaseData(),
                                                            m_sectorPolygon()->pointsInDomainCoords(),
                                                            wellPathGeo,
                                                            mdRange,
                                                            timeStepIdx,
                                                            "FLUXNUM",
                                                            2 ) )
    {
        if ( needToOpenCase ) m_eclipseCase->closeReservoirCase();
        RiaLogging::error( "Failed to create FLUXNUM result for sector model." );
        return false;
    }

    // export result to grdecl file in output working folder using RicEclipseCellResultToFileImpl::writePropertyToTextFile
    QString errorMsg;
    QString outputFluxFileName = m_fileHelper->fluxnumFileName( false /* use full path */ );
    if ( !RicEclipseCellResultToFileImpl::writePropertyToTextFile( outputFluxFileName,
                                                                   m_eclipseCase()->eclipseCaseData(),
                                                                   0, // result is available in timestep 0
                                                                   "FLUXNUM",
                                                                   "FLUXNUM",
                                                                   0, // undef value is 0
                                                                   false /*no echo */,
                                                                   &errorMsg ) )
    {
        if ( needToOpenCase ) m_eclipseCase->closeReservoirCase();
        RiaLogging::error( "Failed to export FLUXNUM result to file: " + outputFluxFileName.toStdString() + "\n" + errorMsg.toStdString() );
        return false;
    }

    // close case to save memory
    if ( needToOpenCase ) m_eclipseCase->closeReservoirCase();

    // add new keywords to deck file
    if ( !m_deckFile->insertKeywordInSection( "GRID", RimKeywordFactory::fluxregKeyword( 1 ) ) )
    {
        RiaLogging::error( "Unable to insert FLUXREG keyword in DATA file." );
        return false;
    }
    if ( !m_deckFile->insertKeywordInSection( "GRID", RimKeywordFactory::usefluxKeyword( m_fileHelper->deckName().toStdString() ) ) )
    {
        RiaLogging::error( "Unable to insert USEFLUX keyword in DATA file." );
        return false;
    }
    if ( !m_deckFile->addIncludeKeyword( "GRID", m_fileHelper->fluxnumFileName().toStdString() ) )
    {
        RiaLogging::error( "Unable to insert INCLUDE keyword for FLUXNUM in DATA file." );
        return false;
    }

    return true;
}