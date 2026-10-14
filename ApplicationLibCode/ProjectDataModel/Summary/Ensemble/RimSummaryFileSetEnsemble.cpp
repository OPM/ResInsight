/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RimSummaryFileSetEnsemble.h"

#include "Ensemble/RiaEnsembleImportTools.h"
#include "RiaFilePathTools.h"
#include "RiaLogging.h"
#include "Summary/RiaSummaryTools.h"

#include "EnsembleFileSet/RimEnsembleFileSet.h"
#include "EnsembleFileSet/RimEnsembleFileSetCollection.h"
#include "EnsembleFileSet/RimEnsembleFileSetTools.h"
#include "RimFileSummaryCase.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryEnsembleTools.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

#include <algorithm>
#include <map>

CAF_PDM_SOURCE_INIT( RimSummaryFileSetEnsemble, "RimSummaryFileSetEnsemble" );

namespace internal
{
const QString pathPatternPlaceholder = "*";

// Longest common folder (ending with '/') of the summary header files, used to make the stored header paths independent of the
// absolute location of the ensemble data.
QString commonHeaderFolder( const std::vector<RimFileSummaryCase*>& fileCases )
{
    if ( fileCases.empty() ) return {};

    QString first  = fileCases.front()->summaryHeaderFilename();
    QString folder = first.left( first.lastIndexOf( '/' ) + 1 );
    for ( auto fileCase : fileCases )
    {
        const QString header = fileCase->summaryHeaderFilename();
        while ( !folder.isEmpty() && !header.startsWith( folder ) )
        {
            folder = folder.left( folder.lastIndexOf( '/', -2 ) + 1 );
        }
    }

    return folder;
}
} // namespace internal

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryFileSetEnsemble::RimSummaryFileSetEnsemble()
{
    CAF_PDM_InitObject( "Path Pattern Ensemble", ":/SummaryCase.svg" );

    CAF_PDM_InitFieldNoDefault( &m_ensembleFileSet, "EnsembleFileSet", "Ensemble File Set" );

    CAF_PDM_InitFieldNoDefault( &m_additionalSummaryFiles, "AdditionalSummaryFiles", "Additional Summary Files" );
    m_additionalSummaryFiles.uiCapability()->setUiHidden( true );

    m_cases.xmlCapability()->disableIO();

    setAsEnsemble( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleFileSet* RimSummaryFileSetEnsemble::ensembleFileSet()
{
    return m_ensembleFileSet();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryFileSetEnsemble::setEnsembleFileSet( RimEnsembleFileSet* ensembleFileSet )
{
    m_ensembleFileSet = ensembleFileSet;
    setGroupingMode( m_ensembleFileSet->groupingMode() );
    connectSignals();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryFileSetEnsemble::updateName( const std::set<QString>& existingEnsembleNames )
{
    QString candidateName = m_ensembleFileSet() ? m_ensembleFileSet()->name() : "Path Pattern Ensemble";

    if ( m_name == candidateName ) return;

    m_name = candidateName;
    caseNameChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryFileSetEnsemble::cleanupBeforeDelete()
{
    if ( m_ensembleFileSet() )
    {
        m_ensembleFileSet()->fileSetChanged.disconnect( this );
        m_ensembleFileSet()->nameChanged.disconnect( this );

        auto fileSet      = m_ensembleFileSet();
        m_ensembleFileSet = nullptr;

        if ( auto coll = fileSet->firstAncestorOrThisOfType<RimEnsembleFileSetCollection>() )
        {
            coll->deleteFileSetIfPossible( fileSet );
        }
    }
    RimSummaryEnsemble::cleanupBeforeDelete();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryFileSetEnsemble::reloadCases()
{
    if ( m_ensembleFileSet() )
    {
        m_ensembleFileSet()->reload();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSummaryFileSetEnsemble::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    if ( fieldNeedingOptions == &m_ensembleFileSet )
    {
        return RimEnsembleFileSetTools::ensembleFileSetOptions();
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryFileSetEnsemble::onFileSetChanged( const caf::SignalEmitter* emitter )
{
    bool notifyChange = true;
    createSummaryCasesFromEnsembleFileSet( notifyChange );
    RiaSummaryTools::updateConnectedPlots( this );
    buildChildNodes();
    updateAllRequiredEditors();

    RimSummaryEnsembleTools::updateDependentDeltaEnsembles( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryFileSetEnsemble::onFileSetNameChanged( const caf::SignalEmitter* emitter )
{
    updateName( {} );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryFileSetEnsemble::createSummaryCasesFromEnsembleFileSet( bool notifyChange )
{
    // Keep user-defined summary vectors across a recreation of the cases
    storeAdditionalSummaryFiles();

    m_cases.deleteChildren();

    if ( m_ensembleFileSet() )
    {
        auto paths = m_ensembleFileSet()->createPaths( ".SMSPEC" );

        RiaDefines::FileType fileType = RiaDefines::FileType::SMSPEC;

        RiaEnsembleImportTools::CreateConfig createConfig{ .fileType              = fileType,
                                                           .ensembleOrGroup       = false,
                                                           .allowDialogs          = false,
                                                           .buildSummaryAddresses = false };

        auto newCases = RiaEnsembleImportTools::createSummaryCasesFromFiles( paths, createConfig );
        if ( newCases.empty() )
        {
            RiaLogging::warning( "No new cases are created." );
            return;
        }

        restoreAdditionalSummaryFiles( newCases );

        replaceCases( newCases, notifyChange );

        // Update name of cases and ensemble after all cases are added
        for ( auto summaryCase : newCases )
        {
            summaryCase->setShowTreeNodes( false );
            summaryCase->setDisplayNameOption( RimCaseDisplayNameTools::DisplayName::SHORT_CASE_NAME );
            summaryCase->updateAutoShortName();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryFileSetEnsemble::onProjectBeingSaved()
{
    storeAdditionalSummaryFiles();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryFileSetEnsemble::storeAdditionalSummaryFiles()
{
    if ( m_cases.empty() ) return;

    // Keep the project file portable and the entries short: the summary header files are identified by the path relative to the common
    // folder of all header files in the ensemble, and the additional summary files by file name only. The additional files are located
    // in the project folder (or the temp folder before the project is saved).
    std::vector<RimFileSummaryCase*> fileCases;
    for ( auto summaryCase : m_cases.childrenByType() )
    {
        if ( auto fileSummaryCase = dynamic_cast<RimFileSummaryCase*>( summaryCase ) )
        {
            fileCases.push_back( fileSummaryCase );
        }
    }

    const QString commonFolder = internal::commonHeaderFolder( fileCases );

    QStringList lines;
    for ( auto fileSummaryCase : fileCases )
    {
        QString additionalFile = fileSummaryCase->additionalSummaryFilePath();
        if ( additionalFile.isEmpty() ) continue;

        lines.push_back( fileSummaryCase->summaryHeaderFilename().mid( commonFolder.size() ) + "|" + QFileInfo( additionalFile ).fileName() );
    }

    m_additionalSummaryFiles = lines.join( '\n' );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryFileSetEnsemble::restoreAdditionalSummaryFiles( const std::vector<RimSummaryCase*>& summaryCases )
{
    if ( m_additionalSummaryFiles().isEmpty() ) return;

    std::map<QString, QString> additionalFileForHeader;
    for ( const auto& line : m_additionalSummaryFiles().split( '\n', Qt::SkipEmptyParts ) )
    {
        // The additional file name is generated and does not contain separator characters
        auto separatorIndex = line.lastIndexOf( '|' );
        if ( separatorIndex > 0 )
        {
            additionalFileForHeader[line.left( separatorIndex )] = line.mid( separatorIndex + 1 );
        }
    }

    std::vector<RimFileSummaryCase*> fileCases;
    for ( auto summaryCase : summaryCases )
    {
        if ( auto fileSummaryCase = dynamic_cast<RimFileSummaryCase*>( summaryCase ) )
        {
            fileCases.push_back( fileSummaryCase );
        }
    }

    const QString commonFolder = internal::commonHeaderFolder( fileCases );

    std::vector<QString> searchFolders;
    if ( RimProject::current() && !RimProject::current()->fileName().isEmpty() )
    {
        searchFolders.push_back( QFileInfo( RimProject::current()->fileName() ).absolutePath() );
    }
    searchFolders.push_back( QDir::tempPath() );

    // The common folder changes if realizations are added to or removed from the file set. Fall back to a unique suffix match of the
    // stored path to avoid losing the user-defined vectors.
    auto findAdditionalFile = [&]( const QString& header ) -> QString
    {
        auto it = additionalFileForHeader.find( header.mid( commonFolder.size() ) );
        if ( it != additionalFileForHeader.end() ) return it->second;

        QString match;
        for ( const auto& [storedHeader, additionalFile] : additionalFileForHeader )
        {
            if ( !header.endsWith( "/" + storedHeader ) ) continue;

            const auto matchCount = std::count_if( fileCases.begin(),
                                                   fileCases.end(),
                                                   [&]( auto c ) { return c->summaryHeaderFilename().endsWith( "/" + storedHeader ); } );
            if ( matchCount == 1 ) match = additionalFile;
        }
        return match;
    };

    for ( auto fileSummaryCase : fileCases )
    {
        const QString additionalFileName = findAdditionalFile( fileSummaryCase->summaryHeaderFilename() );
        if ( additionalFileName.isEmpty() ) continue;

        for ( const auto& folder : searchFolders )
        {
            QString filePath = QDir( folder ).absoluteFilePath( additionalFileName );
            if ( QFile::exists( filePath ) )
            {
                fileSummaryCase->setAdditionalSummaryFilePath( filePath );
                break;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryFileSetEnsemble::createDerivedObjects()
{
    if ( m_cases.empty() )
    {
        bool notifyChange = false;
        createSummaryCasesFromEnsembleFileSet( notifyChange );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryFileSetEnsemble::initAfterRead()
{
    RimSummaryEnsemble::initAfterRead();

    connectSignals();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryFileSetEnsemble::connectSignals()
{
    if ( m_ensembleFileSet() )
    {
        m_ensembleFileSet()->fileSetChanged.connect( this, &RimSummaryFileSetEnsemble::onFileSetChanged );
        m_ensembleFileSet()->nameChanged.connect( this, &RimSummaryFileSetEnsemble::onFileSetNameChanged );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryFileSetEnsemble::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( m_ensembleFileSet() )
    {
        auto group = uiOrdering.addNewGroup( "Ensemble Definition" );
        m_ensembleFileSet()->uiOrdering( uiConfigName, *group );
    }
    uiOrdering.add( &m_includeInAutoReload );

    auto developersGroup = uiOrdering.addNewGroup( " -- Developers --" );
    developersGroup->setCollapsedByDefault();
    developersGroup->add( &m_ensembleFileSet );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryFileSetEnsemble::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    RimSummaryEnsemble::fieldChangedByUi( changedField, oldValue, newValue );

    if ( changedField == &m_ensembleFileSet )
    {
        connectSignals();
        bool notifyChange = true;
        createSummaryCasesFromEnsembleFileSet( notifyChange );
        RiaSummaryTools::updateConnectedPlots( this );
        caseNameChanged.send();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryFileSetEnsemble::onLoadDataAndUpdate()
{
    createDerivedObjects();

    RimSummaryEnsemble::onLoadDataAndUpdate();
}
