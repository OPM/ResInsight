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
#include "RiaPreferencesOpm.h"
#include "RiaWslTools.h"

#include "CompletionExportCommands/RicExportCompletionDataSettingsUi.h"
#include "CompletionExportCommands/RicWellPathExportCompletionDataFeatureImpl.h"
#include "JobCommands/RicRunJobFeature.h"

#include "RifOpmFlowDeckFile.h"

#include "RimCase.h"
#include "RimDeckPositionDlg.h"
#include "RimEclipseCase.h"
#include "RimFishbones.h"
#include "RimPerforationInterval.h"
#include "RimProject.h"
#include "RimReloadCaseTools.h"
#include "RimTools.h"
#include "RimWellPath.h"
#include "RimWellPathCompletionSettings.h"
#include "RimWellPathFracture.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiPushButtonEditor.h"

#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextStream>

CAF_PDM_SOURCE_INIT( RimOpmFlowJob, "OpmFlowJob" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimOpmFlowJob::RimOpmFlowJob()
    : m_fileDeckHasDates( false )
{
    CAF_PDM_InitObject( "Opm Flow Simulation", ":/gear.svg" );

    CAF_PDM_InitFieldNoDefault( &m_deckFileName, "DeckFile", "Input Data File" );
    m_deckFileName.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_workDir, "WorkDirectory", "Working Folder" );
    CAF_PDM_InitFieldNoDefault( &m_wellPath, "WellPath", "Well Path for New Well" );
    CAF_PDM_InitFieldNoDefault( &m_eclipseCase, "EclipseCase", "Eclipse Case" );
    CAF_PDM_InitField( &m_pauseBeforeRun, "PauseBeforeRun", true, "Pause before running OPM Flow" );
    CAF_PDM_InitField( &m_delayOpenWell, "DelayOpenWell", false, "Keep well shut until selected time step" );
    CAF_PDM_InitField( &m_addNewWell, "AddNewWell", true, "Add New Well" );
    CAF_PDM_InitField( &m_openWellDeckPosition, "OpenWellDeckPosition", -1, "Open Well at Keyword Index" );

    CAF_PDM_InitField( &m_wellOpenKeyword, "WellOpenKeyword", QString( "WCONPROD" ), "Open Well Keyword" );
    m_wellOpenKeyword.uiCapability()->setUiEditorTypeName( caf::PdmUiComboBoxEditor::uiEditorTypeName() );
    m_wellOpenKeyword.xmlCapability()->disableIO();

    CAF_PDM_InitField( &m_wellOpenText, "WellOpenText", QString( "'GRUP' 5000 4* 100 20 5" ), "Open Well Parameters" );

    CAF_PDM_InitField( &m_openTimeStep, "OpenTimeStep", 0, "Open Well at Time Step" );

    CAF_PDM_InitField( &m_runButton, "runButton", false, "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_runButton );
    m_runButton.xmlCapability()->disableIO();

    CAF_PDM_InitField( &m_openSelectButton, "openSelectButton", false, "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_openSelectButton );
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
    genGrp->add( &m_addNewWell );

    if ( m_addNewWell() )
    {
        auto wellGrp = uiOrdering.addNewGroup( "New Well Settings" );

        wellGrp->add( &m_wellPath );
        wellGrp->add( &m_wellOpenKeyword );
        wellGrp->add( &m_wellOpenText );

        if ( m_fileDeckHasDates )
        {
            wellGrp->add( &m_delayOpenWell );
            if ( m_delayOpenWell() )
            {
                wellGrp->add( &m_openTimeStep );
            }
            else
            {
                wellGrp->add( &m_openSelectButton );
            }
        }
        else
        {
            wellGrp->add( &m_openSelectButton );
        }
    }

    uiOrdering.add( &m_pauseBeforeRun );
    uiOrdering.add( &m_runButton );

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
    if ( changedField == &m_deckFileName )
    {
        m_deckName = "";
    }
    else if ( changedField == &m_runButton )
    {
        m_runButton = false;
        RicRunJobFeature::runJob( this );
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
    m_fileDeck.reset();
    openDeckFile();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJob::setInputDataFile( QString filename )
{
    m_deckName = "";
    m_deckFileName.setValue( filename );
    m_fileDeck.reset();
    openDeckFile();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimOpmFlowJob::openDeckFile()
{
    if ( m_fileDeck == nullptr )
    {
        m_fileDeck = std::make_unique<RifOpmFlowDeckFile>();
        if ( m_fileDeck->loadDeck( m_deckFileName().path().toStdString() ) )
        {
            m_fileDeckHasDates = m_fileDeck->hasDatesKeyword();
        }
        else
        {
            m_fileDeckHasDates = false;
            m_fileDeck.reset();
        }
    }

    return m_fileDeck != nullptr;
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
QString RimOpmFlowJob::workingDirectory()
{
    return m_workDir().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimOpmFlowJob::deckName()
{
    if ( m_deckName.isEmpty() )
    {
        QFileInfo fi( m_deckFileName().path() );
        m_deckName = fi.completeBaseName();
    }

    return m_deckName;
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
QString RimOpmFlowJob::wellTempFile() const
{
    return m_workDir().path() + "/ri_new_well" + deckExtension();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimOpmFlowJob::openWellTempFile() const
{
    return m_workDir().path() + "/ri_open_well" + deckExtension();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimOpmFlowJob::command()
{
    QStringList cmd;

    QString workDir = m_workDir().path();
    if ( workDir.isEmpty() ) return QStringList();

    QString dataFile = workDir + "/" + deckName();
    if ( !QFile::exists( dataFile + deckExtension() ) ) return QStringList();

    auto opmPref = RiaPreferencesOpm::current();
    if ( opmPref->useWsl() )
    {
        cmd.append( RiaWslTools::wslCommand() );
        cmd.append( opmPref->wslOptions() );

        workDir  = RiaWslTools::convertToWslPath( workDir );
        dataFile = RiaWslTools::convertToWslPath( dataFile );
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
    if ( m_wellPath == nullptr ) return false;

    // reload file deck to make sure we start with the original
    m_fileDeck.reset();
    if ( !openDeckFile() ) return false;

    // add a new well?
    if ( m_addNewWell() )
    {
        // export new well settings from resinsight
        prepareWellSettings();
        if ( !QFile::exists( wellTempFile() ) ) return false;

        QString openWellText = prepareOpenWellText();

        // merge new well settings from resinsight into DATA deck
        if ( !m_fileDeck->mergeWellDeck( wellTempFile().toStdString() ) ) return false;

        // open new well at selected timestep
        if ( m_fileDeckHasDates && m_delayOpenWell )
        {
            if ( !m_fileDeck->openWellAtTimeStep( m_openTimeStep(), openWellText.toStdString() ) )
            {
                return false;
            }
        }
        else
        {
            if ( !m_fileDeck->openWellAtDeckPosition( m_openWellDeckPosition, openWellText.toStdString() ) )
            {
                return false;
            }
        }
        QFile::remove( wellTempFile() );
        QFile::remove( openWellTempFile() );
    }

    // save DATA file to working folder
    bool saveOk = m_fileDeck->saveDeck( m_workDir().path().toStdString(), deckName().toStdString() + deckExtension().toStdString() );
    m_fileDeck.reset();

    if ( m_pauseBeforeRun() )
    {
        QString infoText = "Input parameter files can now be found in the working folder:";
        infoText += " \"" + m_workDir().path() + "\"\n";
        infoText += "\nClick OK to start the Opm Flow simulation or Cancel to stop.";

        auto reply = QMessageBox::information( nullptr, "Opm Flow simulation", infoText, QMessageBox::Ok | QMessageBox::Cancel );

        if ( reply != QMessageBox::Ok ) return false;
    }

    return saveOk;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJob::onCompleted( bool success )
{
    if ( !success ) return;

    QString outputEgrid = m_workDir().path() + "/" + deckName() + ".EGRID";
    if ( !QFile::exists( outputEgrid ) ) return;

    if ( auto existingCase = findExistingCase( outputEgrid ) )
    {
        RimReloadCaseTools::reloadEclipseGridAndSummary( existingCase );
        existingCase->setCustomCaseName( name() );
        existingCase->updateConnectedEditors();
    }
    else
    {
        QStringList files( outputEgrid );

        RiaImportEclipseCaseTools::FileCaseIdMap newCaseFiles;
        if ( RiaImportEclipseCaseTools::openEclipseCasesFromFile( files, true /*create view*/, &newCaseFiles, false /* dialog */ ) )
        {
            if ( auto newCase = findExistingCase( outputEgrid ) )
            {
                newCase->setCustomCaseName( name() );
                newCase->updateConnectedEditors();
                Riu3DMainWindowTools::selectAsCurrentItem( newCase );
                Riu3DMainWindowTools::setExpanded( newCase, true );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimOpmFlowJob::findExistingCase( QString filename )
{
    RimProject* proj = RimProject::current();
    if ( proj )
    {
        std::vector<RimCase*> cases = proj->allGridCases();
        for ( RimCase* c : cases )
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
void RimOpmFlowJob::prepareWellSettings()
{
    RicExportCompletionDataSettingsUi exportSettings;

    exportSettings.fileSplit   = RicExportCompletionDataSettingsUi::ExportSplit::UNIFIED_FILE;
    exportSettings.caseToApply = m_eclipseCase();
    exportSettings.setCustomFileName( wellTempFile() );
    exportSettings.includeMsw = false;
    exportSettings.setExportDataSourceAsComment( false );

    exportSettings.folder = m_workDir().path();

    std::vector<RimWellPathFracture*>    wellPathFractures;
    std::vector<RimFishbones*>           wellPathFishbones;
    std::vector<RimPerforationInterval*> wellPathPerforations;

    std::vector<RimWellPath*> topLevelWells;
    topLevelWells.push_back( m_wellPath->topLevelWellPath() );

    std::vector<RimWellPath*> allLaterals;
    {
        std::set<RimWellPath*> lateralSet;

        for ( auto t : topLevelWells )
        {
            auto laterals = t->allWellPathLaterals();
            for ( auto l : laterals )
            {
                lateralSet.insert( l );
            }
        }

        allLaterals.assign( lateralSet.begin(), lateralSet.end() );
    }

    for ( auto w : allLaterals )
    {
        auto fractures = w->descendantsIncludingThisOfType<RimWellPathFracture>();
        wellPathFractures.insert( wellPathFractures.end(), fractures.begin(), fractures.end() );

        auto fishbones = w->descendantsIncludingThisOfType<RimFishbones>();
        wellPathFishbones.insert( wellPathFishbones.end(), fishbones.begin(), fishbones.end() );

        auto perforations = w->descendantsIncludingThisOfType<RimPerforationInterval>();
        wellPathPerforations.insert( wellPathPerforations.end(), perforations.begin(), perforations.end() );
    }

    RicWellPathExportCompletionDataFeatureImpl::exportCompletions( topLevelWells, exportSettings );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimOpmFlowJob::prepareOpenWellText()
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

    auto kws = m_fileDeck->keywords();
    int  i   = 0;
    for ( auto& kw : kws )
    {
        kwVec.push_back( std::make_pair( i++, QString::fromStdString( kw ) ) );
    }

    m_openWellDeckPosition = RimDeckPositionDlg::askForPosition( nullptr, kwVec, "--- Open New Well HERE ---", m_openWellDeckPosition );
}