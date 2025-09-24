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

#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafProgressInfo.h"

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
    addItem( RimOpmFlowJob::WellOpenType::OPEN_BY_POSITION, "OpenByPosition", "Select Keyword Position in File" );
    addItem( RimOpmFlowJob::WellOpenType::OPEN_AT_DATE, "AtSelectedDate", "Select a Date" );

    setDefault( RimOpmFlowJob::WellOpenType::OPEN_AT_DATE );
}
} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimOpmFlowJob::RimOpmFlowJob()
    : m_fileDeckHasDates( false )
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
    CAF_PDM_InitField( &m_includeMSWData, "IncludeMswData", false, "Add MSW Data" );
    CAF_PDM_InitField( &m_addToEnsemble, "AddToEnsemble", false, "Add Runs to Ensemble" );
    CAF_PDM_InitField( &m_useRestart, "UseRestart", false, "Restart Simulation at Well Open Date" );
    CAF_PDM_InitField( &m_currentRunId, "CurrentRunID", 0, "Current Run ID" );

    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_addToEnsemble );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_pauseBeforeRun );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_useRestart );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_includeMSWData );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_addNewWell );

    CAF_PDM_InitField( &m_wellOpenType, "WellOpenType", caf::AppEnum<WellOpenType>( WellOpenType::OPEN_AT_DATE ), "Open Well" );
    CAF_PDM_InitField( &m_wellOpenKeyword, "WellOpenKeyword", QString( "WCONPROD" ), "Open Well Keyword" );
    m_wellOpenKeyword.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    m_wellOpenKeyword.xmlCapability()->disableIO();

    CAF_PDM_InitField( &m_wellOpenText, "WellOpenText", QString( "'GRUP' 5000 4* 100 20 5" ), "Open Well Parameters" );

    CAF_PDM_InitField( &m_openTimeStep, "OpenTimeStep", 0, " " );

    CAF_PDM_InitField( &m_runButton, "runButton", false, "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_runButton );
    m_runButton.xmlCapability()->disableIO();

    CAF_PDM_InitField( &m_resetRunIdButton, "resetRunIdButton", false, " " );
    caf::PdmUiPushButtonEditor::configureEditorLabelLeft( &m_resetRunIdButton );
    m_resetRunIdButton.xmlCapability()->disableIO();

    CAF_PDM_InitField( &m_openSelectButton, "openSelectButton", false, " " );
    caf::PdmUiPushButtonEditor::configureEditorLabelLeft( &m_openSelectButton );
    m_openSelectButton.xmlCapability()->disableIO();

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
void RimOpmFlowJob::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_workDir )
    {
        if ( auto myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>( attribute ) )
        {
            myAttr->m_selectDirectory = true;
        }
    }
    else if ( field == &m_runButton )
    {
        auto* pbAttribute = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( pbAttribute )
        {
            pbAttribute->m_buttonText = "Run Simulation";
            pbAttribute->m_buttonIcon = QIcon( ":/opm.png" );
        }
    }
    else if ( field == &m_openSelectButton )
    {
        auto* pbAttribute = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( pbAttribute )
        {
            pbAttribute->m_buttonText = "Select Open Keyword Position";
        }
    }
    else if ( field == &m_resetRunIdButton )
    {
        auto* pbAttribute = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>( attribute );
        if ( pbAttribute )
        {
            pbAttribute->m_buttonText = "Reset Ensemble Run Id";
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJob::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto genGrp = uiOrdering.addNewGroup( "General" );
    genGrp->add( nameField() );
    genGrp->add( &m_deckFileName );
    genGrp->add( &m_workDir );
    if ( m_eclipseCase() == nullptr )
    {
        m_addNewWell = false;
    }
    else
    {
        genGrp->add( &m_addNewWell );
    }

    if ( m_addNewWell() )
    {
        auto wellGrp = uiOrdering.addNewGroup( "New Well Settings" );

        wellGrp->add( &m_wellPath );
        wellGrp->add( &m_wellOpenKeyword );
        wellGrp->add( &m_wellOpenText );

        wellGrp->add( &m_wellOpenType );

        if ( m_fileDeckHasDates )
        {
            if ( m_wellOpenType() == WellOpenType::OPEN_AT_DATE )
            {
                wellGrp->add( &m_openTimeStep );
                wellGrp->add( &m_useRestart );
                if ( !m_useRestart() )
                {
                    wellGrp->add( &m_includeMSWData );
                }
                else
                {
                    m_includeMSWData = false;
                }
            }
            else if ( m_wellOpenType == WellOpenType::OPEN_BY_POSITION )
            {
                wellGrp->add( &m_openSelectButton );
            }
        }
        else
        {
            wellGrp->add( &m_openSelectButton );
        }
    }

    auto opmGrp = uiOrdering.addNewGroup( "Opm Flow" );
    opmGrp->add( &m_runButton );
    opmGrp->add( &m_pauseBeforeRun );
    opmGrp->add( &m_addToEnsemble );
    if ( m_addToEnsemble() )
    {
        auto advOpmGrp = opmGrp->addNewGroup( "Advanced" );
        advOpmGrp->setCollapsedByDefault();
        advOpmGrp->add( &m_resetRunIdButton );
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
    else if ( fieldNeedingOptions == &m_openTimeStep )
    {
        if ( auto ec = m_eclipseCase() )
        {
            RimTools::timeStepsForCase( ec, &options );
        }
    }
    else if ( fieldNeedingOptions == &m_wellOpenKeyword )
    {
        options.push_back( caf::PdmOptionItemInfo( "WCONPROD", QVariant::fromValue( QString( "WCONPROD" ) ) ) );
        options.push_back( caf::PdmOptionItemInfo( "WCONINJE", QVariant::fromValue( QString( "WCONINJE" ) ) ) );
    }

    return options;
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
    else if ( changedField == &m_runButton )
    {
        m_runButton = false;
        RicRunJobFeature::runJob( this );
    }
    else if ( changedField == &m_resetRunIdButton )
    {
        m_resetRunIdButton = false;
        auto reply         = QMessageBox::information( nullptr,
                                               "Opm Flow Job",
                                               "Do you want to reset the ensemble run ID to 0?",
                                               QMessageBox::Yes | QMessageBox::No );
        if ( reply == QMessageBox::Yes )
        {
            m_currentRunId = 0;
        }
    }
    else if ( changedField == &m_wellOpenKeyword )
    {
        if ( newValue.toString() == "WCONPROD" )
        {
            m_wellOpenText.setValueWithFieldChanged( "'GRUP' 5000 4* 100 20 5" );
        }
        else
        {
            m_wellOpenText.setValueWithFieldChanged( "'GRUP' 6500 1* 350" );
        }
    }
    else if ( changedField == &m_openSelectButton )
    {
        m_openSelectButton = false;
        selectOpenWellPosition();
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
void RimOpmFlowJob::setEclipseCase( RimEclipseCase* eCase )
{
    m_deckName = "";
    if ( eCase == nullptr )
    {
        m_deckFileName.setValue( QString() );
        return;
    }

    QFileInfo fi( eCase->gridFileName() );
    m_deckFileName.setValue( fi.absolutePath() + "/" + fi.completeBaseName() + deckExtension() );
    m_eclipseCase = eCase;
    m_deckFile.reset();
    openDeckFile();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJob::setInputDataFile( QString filename )
{
    m_deckName = "";
    m_deckFileName.setValue( filename );
    m_deckFile.reset();
    openDeckFile();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimOpmFlowJob::openDeckFile()
{
    if ( m_deckFile == nullptr )
    {
        m_deckFile      = std::make_unique<RifOpmFlowDeckFile>();
        bool deckLoadOk = false;
        try
        {
            deckLoadOk = m_deckFile->loadDeck( m_deckFileName().path().toStdString() );
        }
        catch ( std::filesystem::filesystem_error& )
        {
            RiaLogging::error( QString( "Failed to open %1, possibly unsupported or incorrect format." ).arg( m_deckFileName().path() ) );
        }

        if ( deckLoadOk )
        {
            m_fileDeckHasDates = m_deckFile->hasDatesKeyword();
        }
        else
        {
            m_fileDeckHasDates = false;
            m_deckFile.reset();
        }
    }

    return m_deckFile != nullptr;
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
QString RimOpmFlowJob::title()
{
    return name();
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
QString RimOpmFlowJob::openWellTempFile() const
{
    return workingDirectory() + "/ri_open_well" + deckExtension();
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

    return cmd;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimOpmFlowJob::onPrepare()
{
    // reload file deck to make sure we start with the original
    m_deckFile.reset();
    if ( !openDeckFile() )
    {
        RiaLogging::error( "Unable to open input DATA file " + m_deckFileName().path() );
        return false;
    }

    // add a new well?
    if ( m_addNewWell() )
    {
        if ( m_wellPath == nullptr )
        {
            RiaLogging::error( "Please select a well path to use." );
            return false;
        }

        if ( m_wellPath->completionSettings()->wellName().isEmpty() )
        {
            RiaLogging::error( "Selected Well Path does not have a WELL NAME set, please check the completion settings." );
            return false;
        }

        if ( m_wellPath->completionSettings()->groupName().isEmpty() )
        {
            RiaLogging::error( "Selected Well Path does not have a GROUP NAME set, please check the completion settings." );
            return false;
        }

        if ( m_includeMSWData && m_wellOpenType == WellOpenType::OPEN_AT_DATE )
        {
            if ( m_eclipseCase() )
            {
                std::vector<std::string> mswData;
                int                      i = 0;
                for ( auto& date : m_eclipseCase->timeStepDates() )
                {
                    mswData.push_back( exportMswWellSettings( date, i++ ) );
                }

                if ( !m_deckFile->mergeMswData( mswData ) )
                {
                    RiaLogging::error( "Failed to merge MSW data into file deck." );
                    return false;
                }
            }
        }
        else
        {
            // export new well settings from resinsight
            exportBasicWellSettings();
            if ( !QFile::exists( wellTempFile() ) )
            {
                RiaLogging::error( "Could not find exported well data from ResInsight: " + wellTempFile() );
                return false;
            }

            int fallbackPosition = ( m_fileDeckHasDates && m_wellOpenType == WellOpenType::OPEN_AT_DATE ) ? -1 : m_openWellDeckPosition() - 1;

            // merge new well settings from resinsight into DATA deck
            if ( !m_deckFile->mergeWellDeck( m_openTimeStep(), wellTempFile().toStdString(), fallbackPosition ) )
            {
                RiaLogging::error( "Unable to merge new well data into DATA file. Are there WELSPECS and COMPDAT keywords?" );
                return false;
            }
            QFile::remove( wellTempFile() );
        }

        QString openWellText = generateBasicOpenWellText();

        // open new well at selected timestep
        if ( m_fileDeckHasDates && m_wellOpenType == WellOpenType::OPEN_AT_DATE )
        {
            if ( !m_deckFile->openWellAtTimeStep( m_openTimeStep(), openWellText.toStdString() ) )
            {
                RiaLogging::error( "Unable to open new well in DATA file." );
                return false;
            }

            if ( m_useRestart() )
            {
                if ( !copyUnrstFileToWorkDir() )
                {
                    RiaLogging::error( "Unable to locate UNRST file from input case." );
                    return false;
                }

                if ( !m_deckFile->restartAtTimeStep( std::max( m_openTimeStep() - 1, 0 ), restartDeckName().toStdString() ) )
                {
                    RiaLogging::error( "Unable to insert restart keywords in DATA file." );
                    return false;
                }
            }
        }
        else
        {
            if ( !m_deckFile->openWellAtDeckPosition( m_openWellDeckPosition, openWellText.toStdString() ) )
            {
                RiaLogging::error( "Unable to open new well at selected position in DATA file." );
                return false;
            }
        }
    }

    caf::ProgressInfo saveProgress( 1, "Saving updated information to DATA file.", false );

    // save DATA file to working folder
    bool saveOk = m_deckFile->saveDeck( workingDirectory().toStdString(), deckName().toStdString() + deckExtension().toStdString() );
    m_deckFile.reset();

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
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJob::exportBasicWellSettings()
{
    RicExportCompletionDataSettingsUi exportSettings;

    exportSettings.fileSplit   = RicExportCompletionDataSettingsUi::ExportSplit::UNIFIED_FILE;
    exportSettings.caseToApply = m_eclipseCase();
    exportSettings.setCustomFileName( wellTempFile() );
    exportSettings.includeMsw = false;
    exportSettings.setExportDataSourceAsComment( false );

    exportSettings.folder = workingDirectory();

    std::vector<RimWellPathFracture*>    wellPathFractures;
    std::vector<RimFishbones*>           wellPathFishbones;
    std::vector<RimPerforationInterval*> wellPathPerforations;

    auto topLevelWell = m_wellPath->topLevelWellPath();

    std::set<RimWellPath*> uniquePaths;
    for ( auto w : topLevelWell->allWellPathLaterals() )
    {
        uniquePaths.insert( w );
    }

    for ( auto w : uniquePaths )
    {
        auto fractures = w->descendantsIncludingThisOfType<RimWellPathFracture>();
        wellPathFractures.insert( wellPathFractures.end(), fractures.begin(), fractures.end() );

        auto fishbones = w->descendantsIncludingThisOfType<RimFishbones>();
        wellPathFishbones.insert( wellPathFishbones.end(), fishbones.begin(), fishbones.end() );

        auto perforations = w->descendantsIncludingThisOfType<RimPerforationInterval>();
        wellPathPerforations.insert( wellPathPerforations.end(), perforations.begin(), perforations.end() );
    }

    RicWellPathExportCompletionDataFeatureImpl::exportCompletions( { topLevelWell }, exportSettings );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimOpmFlowJob::exportMswWellSettings( QDateTime date, int timeStep )
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

    std::vector<RimWellPathFracture*>    wellPathFractures;
    std::vector<RimFishbones*>           wellPathFishbones;
    std::vector<RimPerforationInterval*> wellPathPerforations;

    auto topLevelWell = m_wellPath->topLevelWellPath();

    std::set<RimWellPath*> uniquePaths;
    for ( auto w : topLevelWell->allWellPathLaterals() )
    {
        uniquePaths.insert( w );
    }

    for ( auto w : uniquePaths )
    {
        auto fractures = w->descendantsIncludingThisOfType<RimWellPathFracture>();
        wellPathFractures.insert( wellPathFractures.end(), fractures.begin(), fractures.end() );

        auto fishbones = w->descendantsIncludingThisOfType<RimFishbones>();
        wellPathFishbones.insert( wellPathFishbones.end(), fishbones.begin(), fishbones.end() );

        auto perforations = w->descendantsIncludingThisOfType<RimPerforationInterval>();
        wellPathPerforations.insert( wellPathPerforations.end(), perforations.begin(), perforations.end() );
    }

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
QString RimOpmFlowJob::generateBasicOpenWellText()
{
    auto cs = m_wellPath->completionSettings();

    QString wellText = m_wellOpenKeyword() + "\n";

    if ( m_wellOpenKeyword() == "WCONPROD" )
    {
        wellText += QString( "'%1' 'OPEN' %2 /\n" ).arg( cs->wellNameForExport() ).arg( m_wellOpenText );
    }
    else if ( m_wellOpenKeyword() == "WCONINJE" )
    {
        wellText += QString( "'%1' 'WATER' 'OPEN' %2 /\n" ).arg( cs->wellNameForExport() ).arg( m_wellOpenText );
    }
    wellText += "/\n";

    return wellText;
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