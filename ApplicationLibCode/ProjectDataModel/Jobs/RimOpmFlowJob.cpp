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

#include "RiaImportEclipseCaseTools.h"
#include "RiaLogging.h"
#include "RiaPreferencesOpm.h"
#include "RiaWslTools.h"

#include "CompletionExportCommands/RicExportCompletionDataSettingsUi.h"
#include "CompletionExportCommands/RicWellPathExportCompletionDataFeatureImpl.h"
#include "EclipseCommands/RicCreateGridCaseEnsemblesFromFilesFeature.h"
#include "JobCommands/RicRunJobFeature.h"
#include "JobCommands/RicStopJobFeature.h"

#include "RifOpmFlowDeckFile.h"

#include "Ensemble/RimSummaryFileSetEnsemble.h"
#include "EnsembleFileSet/RimEnsembleFileSet.h"
#include "EnsembleFileSet/RimEnsembleFileSetTools.h"
#include "RimCase.h"
#include "RimDeckPositionDlg.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseCaseEnsemble.h"
#include "RimFishbones.h"
#include "RimKeywordFactory.h"
#include "RimKeywordWconinje.h"
#include "RimKeywordWconprod.h"
#include "RimOilField.h"
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
#include "cafPdmUiPushButtonEditor.h"
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
void caf::AppEnum<RimOpmFlowJob::WellOpenType>::setUp()
{
    addItem( RimOpmFlowJob::WellOpenType::OPEN_BY_POSITION, "OpenByPosition", "By Position in File" );
    addItem( RimOpmFlowJob::WellOpenType::OPEN_AT_DATE, "AtSelectedDate", "By Date" );

    setDefault( RimOpmFlowJob::WellOpenType::OPEN_AT_DATE );
}

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
{
    CAF_PDM_InitObject( "Opm Flow Simulation", ":/opm.png" );

    CAF_PDM_InitFieldNoDefault( &m_deckFileName, "DeckFile", "Input Data File" );
    m_deckFileName.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_workDir, "WorkDirectory", "Working Folder" );
    CAF_PDM_InitFieldNoDefault( &m_wellPath, "WellPath", "Well Path for New Well" );
    CAF_PDM_InitFieldNoDefault( &m_eclipseCase, "EclipseCase", "Eclipse Case" );
    CAF_PDM_InitFieldNoDefault( &m_gridEnsemble, "GridEnsemble", "Grid Ensemble" );
    CAF_PDM_InitFieldNoDefault( &m_summaryEnsemble, "SummaryEnsemble", "Summary Ensemble" );

    CAF_PDM_InitField( &m_pauseBeforeRun, "PauseBeforeRun", false, "Pause before running OPM Flow" );
    CAF_PDM_InitField( &m_addNewWell, "AddNewWell", true, "Add New Well" );
    CAF_PDM_InitField( &m_openWellDeckPosition, "OpenWellDeckPosition", -1, "Open Well at Keyword Index" );
    CAF_PDM_InitField( &m_includeMSWData, "IncludeMswData", false, "Include MSW Data (experimental)" );
    CAF_PDM_InitField( &m_addToEnsemble, "AddToEnsemble", false, "Add Runs to Ensemble" );
    CAF_PDM_InitField( &m_useRestart, "UseRestart", false, "Restart Simulation at Well Open Date" );
    CAF_PDM_InitField( &m_currentRunId, "CurrentRunID", 0, "Current Run ID" );

    CAF_PDM_InitFieldNoDefault( &m_wellGroupName, "WellGroupName", "Well Group Name" );
    m_wellGroupName.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_wellOpenType, "WellOpenType", caf::AppEnum<WellOpenType>( WellOpenType::OPEN_AT_DATE ), "Open Well" );
    CAF_PDM_InitField( &m_wellOpenKeyword, "WellOpenKeyword", QString( "WCONPROD" ), "Open Well Keyword" );
    m_wellOpenKeyword.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    m_wellOpenKeyword.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_wconprodKeyword, "WconprodKeyword", "WCONPROD Settings" );
    m_wconprodKeyword = new RimKeywordWconprod();
    m_wconprodKeyword.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_wconinjeKeyword, "WconinjeKeyword", "WCONINJE Settings" );
    m_wconinjeKeyword = new RimKeywordWconinje();
    m_wconinjeKeyword.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitField( &m_openTimeStep, "OpenTimeStep", 0, " " );
    CAF_PDM_InitField( &m_endTimeStep, "EndTimeStep", 0, " " );
    CAF_PDM_InitField( &m_endTimeStepEnabled, "EndTimeStepEnabled", false, "Stop Simulation at Date" );

    CAF_PDM_InitField( &m_appendNewDates, "AppendNewDates", false, "Append Extra Dates" );
    CAF_PDM_InitField( &m_newDatesInterval, "NewDatesInterval", 1, "Interval" );
    CAF_PDM_InitField( &m_numberOfNewDates, "NumberOfNewDates", 12, "Number of Dates to Append" );
    CAF_PDM_InitField( &m_dateAppendType, "DateAppendType", caf::AppEnum<DateAppendType>( DateAppendType::ADD_MONTHS ), " " );

    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_addToEnsemble );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_pauseBeforeRun );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_useRestart );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_includeMSWData );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_addNewWell );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_endTimeStepEnabled );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_appendNewDates );

    setDeletable( true );
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
    openDeckFile();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJob::decodeProgress( const QString& logLine )
{
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJob::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_workDir )
    {
        if ( auto myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>( attribute ) )
        {
            myAttr->m_selectDirectory = true;
        }
    }
    else if ( field == &m_wellGroupName )
    {
        auto attr = dynamic_cast<caf::PdmUiComboBoxEditorAttribute*>( attribute );
        if ( attr )
        {
            attr->enableEditableContent  = true;
            attr->enableAutoComplete     = false;
            attr->adjustWidthToContents  = true;
            attr->notifyWhenTextIsEdited = false;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJob::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( isRunning() )
    {
        auto runGrp = uiOrdering.addNewGroup( "Running" );
        m_workDir.uiCapability()->setUiReadOnly( true );
        runGrp->add( &m_workDir );
        auto stopButton = runGrp->addNewButton( "Stop", [this]() { RicStopJobFeature::stopJob( this ); } );
        stopButton->setUiIconFromResourceString( ":/stop.svg" );
        stopButton->setAlignment( Qt::AlignCenter );
        uiOrdering.skipRemainingFields();
        return;
    }
    m_workDir.uiCapability()->setUiReadOnly( false );

    auto genGrp = uiOrdering.addNewGroup( "General" );
    genGrp->add( nameField() );
    genGrp->add( &m_deckFileName );
    genGrp->add( &m_workDir );
    genGrp->add( &m_eclipseCase );

    if ( m_eclipseCase() == nullptr )
    {
        m_addNewWell = false;
    }
    else
    {
        auto wellGrp = uiOrdering.addNewGroup( "New Well Settings" );
        wellGrp->add( &m_addNewWell );

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
                if ( m_wellOpenType() == WellOpenType::OPEN_AT_DATE )
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
                else if ( m_wellOpenType == WellOpenType::OPEN_BY_POSITION )
                {
                    createOpenPostionButton = true;
                }
                m_wellOpenType.uiCapability()->setUiReadOnly( false );
            }
            else
            {
                createOpenPostionButton = true;

                m_wellOpenType = WellOpenType::OPEN_BY_POSITION;
                m_wellOpenType.uiCapability()->setUiReadOnly( true );
            }

            if ( createOpenPostionButton )
            {
                auto openSelectButton = wellGrp->addNewButton( "Select Open Keyword Position", [this]() { selectOpenWellPosition(); } );
                openSelectButton->setAlignment( Qt::AlignRight );
            }
        }
    }

    if ( m_fileDeckHasDates )
    {
        auto dateGrp = uiOrdering.addNewGroup( "Date Settings" );
        dateGrp->setCollapsedByDefault();
        dateGrp->add( &m_appendNewDates );
        if ( m_appendNewDates() )
        {
            dateGrp->add( &m_numberOfNewDates );
            dateGrp->add( &m_newDatesInterval );
            dateGrp->appendToRow( &m_dateAppendType );
        }
    }

    auto opmGrp = uiOrdering.addNewGroup( "OPM Flow" );

    auto runButton = opmGrp->addNewButton( "Run", [this]() { RicRunJobFeature::runJob( this ); } );
    runButton->setUiIconFromResourceString( ":/Play.svg" );
    runButton->setAlignment( Qt::AlignCenter );

    opmGrp->add( &m_pauseBeforeRun );
    if ( m_fileDeckHasDates )
    {
        opmGrp->add( &m_endTimeStepEnabled );
        if ( m_endTimeStepEnabled() )
        {
            opmGrp->add( &m_endTimeStep );
        }
    }
    opmGrp->add( &m_addToEnsemble );
    if ( m_addToEnsemble() )
    {
        auto advOpmGrp = opmGrp->addNewGroup( "Advanced" );
        advOpmGrp->setCollapsedByDefault();

        auto resetRunIdButton = advOpmGrp->addNewButton( "Reset Ensemble Run Id", [this]() { resetEnsembleRunId(); } );
        resetRunIdButton->setAlignment( Qt::AlignRight );
    }

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
    if ( ( changedField == &m_deckFileName ) || ( changedField == &m_addToEnsemble ) )
    {
        m_deckName = "";
    }
    else if ( changedField == &m_wellPath )
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJob::setWorkingDirectory( QString workDir )
{
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
    m_deckName = "";
    if ( eCase == nullptr )
    {
        m_deckFileName.setValue( QString() );
        closeDeckFile();
        return;
    }

    QFileInfo fi( eCase->gridFileName() );
    m_deckFileName.setValue( fi.absolutePath() + "/" + fi.completeBaseName() + deckExtension() );
    m_eclipseCase = eCase;
    closeDeckFile();
    openDeckFile();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJob::setInputDataFile( QString filename )
{
    m_deckName = "";
    m_deckFileName.setValue( filename );
    closeDeckFile();
    openDeckFile();
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
            deckLoadOk = m_deckFile->loadDeck( m_deckFileName().path().toStdString() );
            if ( deckLoadOk )
            {
                m_fileDeckHasDates  = m_deckFile->hasDatesKeyword();
                m_fileDeckIsRestart = m_deckFile->isRestartFile();
            }
        }
        catch ( std::filesystem::filesystem_error& )
        {
            deckLoadOk = false;
            RiaLogging::error( QString( "Failed to open %1, possibly unsupported or incorrect format." ).arg( m_deckFileName().path() ) );
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
        QString dstName = workingDirectory() + "/" + restartDeckName() + ".UNRST";
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
    if ( !m_addToEnsemble() )
    {
        return m_workDir().path();
    }
    else
    {
        return QString( "%1/run-%2" ).arg( m_workDir().path() ).arg( m_currentRunId() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimOpmFlowJob::deckName()
{
    if ( m_deckName.isEmpty() )
    {
        m_deckName = baseDeckName();
        if ( m_addToEnsemble() )
        {
            m_deckName = m_deckName + "-" + QString::number( m_currentRunId() );
        }
    }

    return m_deckName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimOpmFlowJob::baseDeckName() const
{
    QFileInfo fi( m_deckFileName().path() );
    return fi.completeBaseName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimOpmFlowJob::restartDeckName() const
{
    return baseDeckName() + "_RST";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimOpmFlowJob::deckExtension() const
{
    return ".DATA";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimOpmFlowJob::wellTempFile( int timeStep, bool includeMSW, bool includeLGR ) const
{
    QString postfix = "";
    if ( timeStep >= 0 )
    {
        postfix = QString( "_%1" ).arg( timeStep );
    }
    if ( includeLGR )
    {
        postfix = postfix + "_LGR";
    }
    if ( includeMSW )
    {
        postfix = postfix + "_MSW";
    }

    return workingDirectory() + "/ri_new_well" + postfix + deckExtension();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimOpmFlowJob::command()
{
    QStringList cmd;

    QString workDir = workingDirectory();
    if ( workDir.isEmpty() ) return cmd;

    QString dataFile = workDir + "/" + deckName();
    if ( !QFile::exists( dataFile + deckExtension() ) ) return cmd;

    auto opmPref = RiaPreferencesOpm::current();
    if ( opmPref->useWsl() )
    {
        cmd.append( RiaWslTools::wslCommand() );
        cmd.append( opmPref->wslOptions() );

        workDir  = RiaWslTools::convertToWslPath( workDir );
        dataFile = RiaWslTools::convertToWslPath( dataFile );
    }

    if ( opmPref->useMpi() )
    {
        cmd.append( opmPref->mpirunCommand() );
        cmd.append( QString( "-np" ) );
        cmd.append( QString( "%1" ).arg( opmPref->mpiProcesses() ) );
    }

    cmd.append( opmPref->opmFlowCommand() );
    cmd.append( QString( "--output-dir=%1" ).arg( workDir ) );
    cmd.append( QString( "--ecl-deck-file-name=%1" ).arg( dataFile ) );
    cmd.append( QString( "--enable-esmry=true" ) );

    return cmd;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString, QString> RimOpmFlowJob::environment()
{
    auto opmPref = RiaPreferencesOpm::current();
    if ( opmPref->useWsl() )
    {
        return RiaWslTools::wslEnvironmentVariables();
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimOpmFlowJob::onPrepare()
{
    // reload file deck to make sure we start with the original
    closeDeckFile();
    if ( !openDeckFile() )
    {
        RiaLogging::error( "Unable to open input DATA file " + m_deckFileName().path() );
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

        int mergePosition = mergeBasicWellSettings();
        if ( mergePosition < 0 )
        {
            RiaLogging::error( "Unable to merge new well data into DATA file. Please check file format." );
            return false;
        }

        if ( ( m_includeMSWData ) && ( m_wellOpenType == WellOpenType::OPEN_AT_DATE ) )
        {
            mergePosition = mergeMswData( mergePosition );
            if ( mergePosition < 0 )
            {
                RiaLogging::error( "Failed to merge MSW data into file deck." );
                return false;
            }
        }

        Opm::DeckKeyword openKeyword = ( m_wellOpenKeyword() == "WCONPROD" ) ? m_wconprodKeyword->keyword( wellNameInDeck )
                                                                             : m_wconinjeKeyword->keyword( wellNameInDeck );

        // open new well at selected timestep
        if ( m_wellOpenType == WellOpenType::OPEN_AT_DATE )
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

    if ( m_useRestart() )
    {
        if ( !copyUnrstFileToWorkDir() )
        {
            RiaLogging::error( "Unable to locate UNRST file from input case." );
            return false;
        }

        if ( !m_deckFile->restartAtTimeStep( m_openTimeStep(), restartDeckName().toStdString() ) )
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

    caf::ProgressInfo saveProgress( 1, "Saving updated information to DATA file.", false );

    // save DATA file to working folder
    bool saveOk = m_deckFile->saveDeck( workingDirectory().toStdString(), deckName().toStdString() + deckExtension().toStdString() );

    closeDeckFile();

    return saveOk;
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

        auto reply = QMessageBox::information( nullptr, "Opm Flow simulation", infoText, QMessageBox::Ok | QMessageBox::Cancel );

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
void RimOpmFlowJob::onCompleted( bool success )
{
    if ( !success ) return;

    QString outputEgridFileName = workingDirectory() + "/" + deckName() + ".EGRID";
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
            QString pattern   = m_workDir().path() + "/run-*/" + baseDeckName() + "-*";
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
        m_deckName     = "";
    }
    else
    {
        if ( auto existingCase = findExistingCase( outputEgridFileName ) )
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
                if ( auto newCase = findExistingCase( outputEgridFileName ) )
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
RimEclipseCase* RimOpmFlowJob::findExistingCase( QString filename )
{
    if ( RimProject* proj = RimProject::current() )
    {
        for ( RimCase* c : proj->allGridCases() )
        {
            RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( c );
            if ( eclipseCase && ( eclipseCase->gridFileName() == filename ) )
            {
                return eclipseCase;
            }
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///  Returns value < 0 if failure to merge
//--------------------------------------------------------------------------------------------------
int RimOpmFlowJob::mergeBasicWellSettings()
{
    const int failure = -1;

    int mergePosition = m_openWellDeckPosition();

    auto compdatKw  = RimKeywordFactory::compdatKeyword( m_eclipseCase(), m_wellPath() );
    auto welspecsKw = RimKeywordFactory::welspecsKeyword( m_wellGroupName().toStdString(), m_eclipseCase(), m_wellPath() );

    if ( welspecsKw.size() < 1 || compdatKw.size() < 1 )
    {
        RiaLogging::error( "Failed to create WELSPECS and COMPDAT keywords for selected well path. Do you have a valid case selected?" );
        return failure;
    }

    if ( m_wellOpenType == WellOpenType::OPEN_AT_DATE )
    {
        // reverse order for correct insertion order
        if ( !m_deckFile->mergeKeywordAtTimeStep( m_openTimeStep(), compdatKw ) ) return failure;
        if ( !m_deckFile->mergeKeywordAtTimeStep( m_openTimeStep(), welspecsKw ) ) return failure;
        mergePosition = 0;
    }
    else
    {
        mergePosition = m_deckFile->mergeKeywordAtPosition( mergePosition, welspecsKw );
        if ( mergePosition < 0 ) return failure;
        mergePosition = m_deckFile->mergeKeywordAtPosition( mergePosition, compdatKw );
        if ( mergePosition < 0 ) return failure;
    }

    // increase wells and connections in welldims to make sure they are big enough
    auto additionalConnections = (int)compdatKw.size();
    auto welldims              = m_deckFile->welldims();
    if ( ( welldims.size() < 4 ) ||
         !m_deckFile->setWelldims( (int)welldims[0] + 1, (int)( welldims[1] + additionalConnections ), (int)welldims[2] + 1, (int)welldims[3] + 1 ) )
    {
        RiaLogging::error( "Failed to update WELLDIMS keyword in DATA file, is it missing?" );
        return failure;
    }
    return mergePosition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimOpmFlowJob::exportMswWellSettings( int timeStep )
{
    RicExportCompletionDataSettingsUi exportSettings;

    QString customName    = wellTempFile( timeStep );
    QString customMswName = wellTempFile( timeStep, true /*include msw*/ );

    // this file is not used, but the export generates the file anyways, so we need to remove it
    QString customMswLgrName = wellTempFile( timeStep, true /*include msw*/, true /*include LGR*/ );

    exportSettings.fileSplit   = RicExportCompletionDataSettingsUi::ExportSplit::UNIFIED_FILE;
    exportSettings.caseToApply = m_eclipseCase();
    exportSettings.setCustomFileName( customName );
    exportSettings.includeMsw = true;
    exportSettings.setExportDataSourceAsComment( false );
    exportSettings.timeStep = timeStep;

    exportSettings.folder = workingDirectory();

    auto topLevelWell = m_wellPath->topLevelWellPath();

    RicWellPathExportCompletionDataFeatureImpl::exportCompletions( { topLevelWell }, exportSettings );

    QString fileContent = readFileContent( customName );
    fileContent += readFileContent( customMswName );

    QFile::remove( customName );
    QFile::remove( customMswName );
    QFile::remove( customMswLgrName );

    return fileContent.toStdString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimOpmFlowJob::mergeMswData( int mergePosition )
{
    const int failure = -1;

    auto mswData = exportMswWellSettings( 0 );

    auto welsegsKw  = RimKeywordFactory::welsegsKeyword( m_eclipseCase(), m_wellPath(), mswData );
    auto compsegsKw = RimKeywordFactory::compsegsKeyword( m_eclipseCase(), m_wellPath(), mswData );
    auto wsegvalvKw = RimKeywordFactory::wsegvalvKeyword( m_eclipseCase(), m_wellPath(), mswData );
    auto wsegaicdKw = RimKeywordFactory::wsegaicdKeyword( m_eclipseCase(), m_wellPath(), mswData );

    if ( welsegsKw.size() < 1 || compsegsKw.size() < 1 )
    {
        RiaLogging::error( "Failed to create WELSEGS or COMPSEGS keyword from MSW data." );
        return failure;
    }

    if ( m_wellOpenType == WellOpenType::OPEN_AT_DATE )
    {
        // make sure we insert after COMPDAT kw
        if ( !m_deckFile->mergeKeywordAtTimeStep( m_openTimeStep(), welsegsKw, "COMPDAT" ) ) return failure;
        if ( !m_deckFile->mergeKeywordAtTimeStep( m_openTimeStep(), compsegsKw, welsegsKw.name() ) ) return failure;
        if ( wsegvalvKw.size() >= 1 )
        {
            if ( !m_deckFile->mergeKeywordAtTimeStep( m_openTimeStep(), wsegvalvKw, compsegsKw.name() ) ) return failure;
        }
        if ( wsegaicdKw.size() >= 1 )
        {
            if ( !m_deckFile->mergeKeywordAtTimeStep( m_openTimeStep(), wsegaicdKw, compsegsKw.name() ) ) return failure;
        }

        mergePosition = 0;
    }
    else
    {
        mergePosition = m_deckFile->mergeKeywordAtPosition( mergePosition, welsegsKw );
        if ( mergePosition < 0 ) return failure;
        mergePosition++;
        mergePosition = m_deckFile->mergeKeywordAtPosition( mergePosition, compsegsKw );
        if ( mergePosition < 0 ) return failure;
        mergePosition++;
        if ( wsegvalvKw.size() >= 1 )
        {
            mergePosition = m_deckFile->mergeKeywordAtPosition( mergePosition, wsegvalvKw );
            if ( mergePosition < 0 ) return failure;
            mergePosition++;
        }
        if ( wsegaicdKw.size() >= 1 )
        {
            mergePosition = m_deckFile->mergeKeywordAtPosition( mergePosition, wsegaicdKw );
            if ( mergePosition < 0 ) return failure;
            mergePosition++;
        }
    }

    int branches = (int)m_wellPath->allWellPathLaterals().size();

    // increase wells and connections in welldims to make sure they are big enough
    auto additionalSegments = (int)welsegsKw.size() - 1;
    auto wsegdims           = m_deckFile->wsegdims();
    auto maxBranches        = std::max( branches, wsegdims[2] );
    if ( ( wsegdims.size() < 3 ) || !m_deckFile->setWsegdims( wsegdims[0] + 1, wsegdims[1] + additionalSegments, maxBranches + 1 ) )
    {
        RiaLogging::error( "Failed to update WSEGDIMS keyword in DATA file." );
        return failure;
    }
    return mergePosition;
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

    m_openWellDeckPosition = RimDeckPositionDlg::askForPosition( nullptr, kwVec, "--- Open New Well HERE ---", m_openWellDeckPosition );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJob::resetEnsembleRunId()
{
    if ( QMessageBox::information( nullptr, "Opm Flow Job", "Do you want to reset the ensemble run ID to 0?", QMessageBox::Yes | QMessageBox::No ) ==
         QMessageBox::Yes )
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
QString RimOpmFlowJob::readFileContent( QString filename )
{
    QFile file( filename );
    if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        QTextStream in( &file );
        QString     fileContent = in.readAll();
        file.close();
        return fileContent;
    }
    return "";
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
