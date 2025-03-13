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

#include "JobCommands/RicRunJobFeature.h"

#include "RifOpmFlowDeckFile.h"

#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimProject.h"
#include "RimReloadCaseTools.h"
#include "RimTools.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiPushButtonEditor.h"

#include <QFileInfo>

CAF_PDM_SOURCE_INIT( RimOpmFlowJob, "OpmFlowJob" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimOpmFlowJob::RimOpmFlowJob()
{
    CAF_PDM_InitObject( "Opm Flow Simulation", ":/gear.svg" );

    CAF_PDM_InitFieldNoDefault( &m_eclipseCase, "EclipseCase", "Eclipse Case" );
    CAF_PDM_InitFieldNoDefault( &m_workDir, "WorkDirectory", "Working Folder" );

    CAF_PDM_InitField( &m_runButton, "runButton", false, "" );
    caf::PdmUiPushButtonEditor::configureEditorLabelHidden( &m_runButton );
    m_runButton.xmlCapability()->disableIO();

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
QList<caf::PdmOptionItemInfo> RimOpmFlowJob::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_eclipseCase )
    {
        RimTools::eclipseCaseOptionItems( &options );
        if ( options.isEmpty() )
        {
            options.push_back( caf::PdmOptionItemInfo( "None", nullptr ) );
        }
    }

    return options;
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJob::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( nameField() );
    uiOrdering.add( &m_eclipseCase );
    uiOrdering.add( &m_workDir );
    uiOrdering.add( &m_runButton );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJob::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_eclipseCase )
    {
        m_deckName = "";
    }
    else if ( changedField == &m_runButton )
    {
        m_runButton = false;
        RicRunJobFeature::runJob( this );
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
    m_eclipseCase = eCase;
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
        if ( m_eclipseCase() != nullptr )
        {
            QString   gridFile = m_eclipseCase->gridFileName();
            QFileInfo fi( gridFile );
            m_deckName = fi.completeBaseName();
        }
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
    if ( m_eclipseCase() == nullptr ) return false;

    QString gridFile = m_eclipseCase->gridFileName();

    QFileInfo fi( gridFile );
    QString   dataFile = fi.absolutePath() + "/" + fi.completeBaseName() + deckExtension();
    if ( !QFile::exists( dataFile ) ) return false;

    RifOpmFlowDeckFile deckFile;
    if ( !deckFile.loadDeck( dataFile.toStdString() ) ) return false;

    return deckFile.saveDeck( m_workDir().path().toStdString(), deckName().toStdString() + deckExtension().toStdString() );
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
        QStringList                              files( outputEgrid );
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
