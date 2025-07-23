/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RimSummaryCaseMainCollection.h"

#include "RiaEclipseFileNameTools.h"
#include "RiaEnsembleNameTools.h"
#include "RiaLogging.h"
#include "RiaPreferencesSummary.h"
#include "RiaPreferencesSystem.h"
#include "Summary/RiaSummaryTools.h"

#include "RifCaseRealizationParametersReader.h"
#include "RifEclipseSummaryTools.h"
#include "RifOpmCommonSummary.h"
#include "RifOpmSummaryTools.h"
#include "RifSummaryCaseRestartSelector.h"
#include "Sumo/RimSummaryCaseSumo.h"

#ifdef USE_HDF5
#include "RifHdf5SummaryExporter.h"
#endif

#include "RimCaseDisplayNameTools.h"
#include "RimCsvSummaryCase.h"
#include "RimDeltaSummaryEnsemble.h"
#include "RimEclipseResultCase.h"
#include "RimFileSummaryCase.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimRftPlotCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryMultiPlotCollection.h"
#include "RimSummaryPlot.h"
#include "RimWellRftPlot.h"

#include "cafPdmFieldReorderCapability.h"
#include "cafProgressInfo.h"

#include <QCoreApplication>
#include <QDir>
#include <memory>

CAF_PDM_SOURCE_INIT( RimSummaryCaseMainCollection, "SummaryCaseCollection" );

//--------------------------------------------------------------------------------------------------
/// Internal function
//--------------------------------------------------------------------------------------------------
void addCaseRealizationParametersIfFound( RimSummaryCase& sumCase, const QString modelFolderOrFile, const QString& filePathCandidate )
{
    std::shared_ptr<RigCaseRealizationParameters> parameters;

    QString parametersFile = filePathCandidate.isEmpty() ? RifCaseRealizationParametersFileLocator::locate( modelFolderOrFile )
                                                         : filePathCandidate;
    if ( !parametersFile.isEmpty() )
    {
        auto reader = RifCaseRealizationReader::createReaderFromFileName( parametersFile );
        if ( reader )
        {
            // Try parse case realization parameters
            try
            {
                reader->parse();
                parameters = reader->parameters();
            }
            catch ( ... )
            {
            }
        }
    }
    else
    {
        parameters = std::make_shared<RigCaseRealizationParameters>();
    }

    if ( dynamic_cast<RimSummaryCaseSumo*>( &sumCase ) == nullptr )
    {
        int realizationNumber = RifCaseRealizationParametersFileLocator::realizationNumber( modelFolderOrFile );
        parameters->setRealizationNumber( realizationNumber );
        parameters->addParameter( RiaDefines::summaryRealizationNumber(), realizationNumber );

        sumCase.setCaseRealizationParameters( parameters );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCaseMainCollection::RimSummaryCaseMainCollection()
    : dataSourceHasChanged( this )
{
    CAF_PDM_InitObject( "Summary Cases", ":/SummaryCases16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_cases, "SummaryCases", "" );
    caf::PdmFieldReorderCapability::addToField( &m_cases );

    CAF_PDM_InitFieldNoDefault( &m_ensembles, "SummaryCaseCollections", "" );
    caf::PdmFieldReorderCapability::addToField( &m_ensembles );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCaseMainCollection::~RimSummaryCaseMainCollection()
{
    m_cases.deleteChildrenAsync();
    m_ensembles.deleteChildrenAsync();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimSummaryCaseMainCollection::findTopLevelSummaryCaseFromFileName( const QString& fileName ) const
{
    for ( const auto& summaryCase : topLevelSummaryCases() )
    {
        if ( summaryCase->summaryHeaderFilename() == fileName )
        {
            return summaryCase;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::addCases( const std::vector<RimSummaryCase*> cases )
{
    for ( RimSummaryCase* sumCase : cases )
    {
        m_cases.push_back( sumCase );
        sumCase->nameChanged.connect( this, &RimSummaryCaseMainCollection::onCaseNameChanged );
    }
    dataSourceHasChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::addCase( RimSummaryCase* summaryCase )
{
    m_cases.push_back( summaryCase );
    summaryCase->nameChanged.connect( this, &RimSummaryCaseMainCollection::onCaseNameChanged );

    dataSourceHasChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::removeCase( RimSummaryCase* summaryCase, bool notifyChange )
{
    std::vector<RimDeltaSummaryEnsemble*> derivedEnsembles;

    // Build a list of derived ensembles that must be updated after delete
    for ( auto ensemble : summaryEnsembles() )
    {
        auto derEnsemble = dynamic_cast<RimDeltaSummaryEnsemble*>( ensemble );
        if ( derEnsemble )
        {
            if ( derEnsemble->hasCaseReference( summaryCase ) )
            {
                derivedEnsembles.push_back( derEnsemble );
            }
        }
    }

    m_cases.removeChild( summaryCase );

    for ( RimSummaryEnsemble* ensemble : m_ensembles )
    {
        ensemble->removeCase( summaryCase, notifyChange );
    }

    // Update derived ensemble cases (if any)
    for ( auto derEnsemble : derivedEnsembles )
    {
        derEnsemble->createDerivedEnsembleCases();
    }

    if ( notifyChange ) dataSourceHasChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::removeCases( std::vector<RimSummaryCase*>& cases )
{
    for ( auto sumCase : cases )
    {
        removeCase( sumCase, false );
    }

    for ( RimSummaryEnsemble* ensemble : m_ensembles )
    {
        ensemble->updateReferringCurveSetsZoomAll();
    }

    dataSourceHasChanged.send();
}

//--------------------------------------------------------------------------------------------------
/// Lambda function to move an object in a child array field
//--------------------------------------------------------------------------------------------------
auto moveObjectInChildArrayField = []( auto& childArrayField, auto* object, int destinationIndex )
{
    auto currentIndex = childArrayField.indexOf( object );
    if ( currentIndex < childArrayField.size() )
    {
        childArrayField.erase( currentIndex );
        childArrayField.insertAt( std::min( destinationIndex, static_cast<int>( childArrayField.size() ) ), object );
    }
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::moveCase( RimSummaryCase* summaryCase, int destinationIndex )
{
    moveObjectInChildArrayField( m_cases, summaryCase, destinationIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryEnsemble* RimSummaryCaseMainCollection::addEnsemble( const std::vector<RimSummaryCase*>&  summaryCases,
                                                               const QString&                       collectionName,
                                                               bool                                 isEnsemble,
                                                               std::function<RimSummaryEnsemble*()> allocator )
{
    RimSummaryEnsemble* ensemble = allocator();
    if ( !collectionName.isEmpty() ) ensemble->setNameTemplate( collectionName );

    for ( RimSummaryCase* summaryCase : summaryCases )
    {
        auto currentSummaryCaseCollection = summaryCase->firstAncestorOrThisOfType<RimSummaryEnsemble>();
        if ( currentSummaryCaseCollection )
        {
            currentSummaryCaseCollection->removeCase( summaryCase );
        }
        else
        {
            m_cases.removeChild( summaryCase );
        }

        ensemble->addCase( summaryCase );
        if ( isEnsemble )
        {
            summaryCase->setDisplayNameOption( RimCaseDisplayNameTools::DisplayName::SHORT_CASE_NAME );
        }
    }

    ensemble->setAsEnsemble( isEnsemble );

    addEnsemble( ensemble );

    return ensemble;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::moveEnsemble( RimSummaryEnsemble* ensemble, int destinationIndex )
{
    moveObjectInChildArrayField( m_ensembles, ensemble, destinationIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::removeEnsemble( RimSummaryEnsemble* ensemble )
{
    m_ensembles.removeChild( ensemble );

    RiaSummaryTools::updateSummaryEnsembleNames();

    dataSourceHasChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::addEnsemble( RimSummaryEnsemble* ensemble )
{
    CVF_ASSERT( ensemble );

    m_ensembles.push_back( ensemble );

    if ( ensemble->ensembleId() == -1 )
    {
        RimProject* project = RimProject::current();
        project->assignIdToEnsemble( ensemble );
    }

    ensemble->caseNameChanged.connect( this, &RimSummaryCaseMainCollection::onCaseNameChanged );

    dataSourceHasChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimSummaryCaseMainCollection::summaryCase( size_t idx )
{
    return m_cases[idx];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimSummaryCaseMainCollection::summaryCaseCount() const
{
    return m_cases.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RimSummaryCaseMainCollection::allSummaryCases() const
{
    std::vector<RimSummaryCase*> cases;

    if ( !m_cases.empty() ) cases.insert( cases.end(), m_cases.begin(), m_cases.end() );

    for ( auto& ensemble : m_ensembles )
    {
        auto collCases = ensemble->allSummaryCases();
        if ( collCases.empty() ) continue;
        cases.insert( cases.end(), collCases.begin(), collCases.end() );
    }

    return cases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RimSummaryCaseMainCollection::topLevelSummaryCases() const
{
    std::vector<RimSummaryCase*> cases;
    for ( const auto& sumCase : m_cases )
    {
        cases.push_back( sumCase );
    }
    return cases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryEnsemble*> RimSummaryCaseMainCollection::summaryEnsembles() const
{
    std::vector<RimSummaryEnsemble*> ensembles;
    for ( const auto& ensemble : m_ensembles )
    {
        ensembles.push_back( ensemble );
    }
    return ensembles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::loadAllSummaryCaseData()
{
    for ( auto ensemble : summaryEnsembles() )
    {
        auto sumCases = ensemble->allSummaryCases();

        const bool extractStateFromFirstCase = true;
        RimSummaryCaseMainCollection::loadSummaryCaseData( sumCases, extractStateFromFirstCase );
    }

    std::vector<RimSummaryCase*> sumCases = topLevelSummaryCases();

    const bool extractStateFromFirstCase = false;
    RimSummaryCaseMainCollection::loadSummaryCaseData( sumCases, extractStateFromFirstCase );

    // Create addresses for all single summary cases (not part of an ensemble)
    for ( auto sumCase : sumCases )
    {
        if ( sumCase->summaryReader() )
        {
            sumCase->summaryReader()->createAddressesIfRequired();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::initAfterRead()
{
    for ( auto sumCase : topLevelSummaryCases() )
    {
        sumCase->nameChanged.connect( this, &RimSummaryCaseMainCollection::onCaseNameChanged );
    }

    for ( auto ensemble : summaryEnsembles() )
    {
        ensemble->caseNameChanged.connect( this, &RimSummaryCaseMainCollection::onCaseNameChanged );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::loadSummaryCaseData( const std::vector<RimSummaryCase*>& summaryCases, bool extractStateFromFirstCase )
{
    std::vector<RimFileSummaryCase*> fileSummaryCases;
    std::vector<RimSummaryCase*>     otherSummaryCases;

    RiaPreferencesSummary* prefs = RiaPreferencesSummary::current();

    if ( prefs->useMultipleThreadsWhenLoadingSummaryData() )
    {
        for ( auto c : summaryCases )
        {
            auto fileCase = dynamic_cast<RimFileSummaryCase*>( c );
            if ( fileCase )
            {
                fileSummaryCases.push_back( fileCase );
            }
            else
            {
                otherSummaryCases.push_back( c );
            }
        }
    }
    else
    {
        otherSummaryCases = summaryCases;
    }

    if ( !fileSummaryCases.empty() )
    {
        loadFileSummaryCaseData( fileSummaryCases, extractStateFromFirstCase );
    }

    if ( !otherSummaryCases.empty() )
    {
        caf::ProgressInfo progInfo( otherSummaryCases.size(), "Loading Summary Cases" );

        for ( int cIdx = 0; cIdx < static_cast<int>( otherSummaryCases.size() ); ++cIdx )
        {
            RimSummaryCase* sumCase = otherSummaryCases[cIdx];
            if ( sumCase )
            {
                sumCase->createSummaryReaderInterface();
                sumCase->createRftReaderInterface();
                addCaseRealizationParametersIfFound( *sumCase, sumCase->summaryHeaderFilename(), {} );
            }

            {
                progInfo.incrementProgress();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::loadFileSummaryCaseData( const std::vector<RimFileSummaryCase*>& fileSummaryCases,
                                                            bool                                    extractStateFromFirstCase )
{
    RiaPreferencesSummary* prefs = RiaPreferencesSummary::current();

    // If h5 mode, check all summary files and create or recreate h5 files if required
#ifdef USE_HDF5
    {
        if ( prefs->summaryDataReader() == RiaPreferencesSummary::SummaryReaderMode::HDF5_OPM_COMMON )
        {
            int threadCount = 1;
#ifdef USE_OPENMP
            threadCount = prefs->createH5SummaryDataThreadCount();
#endif
            std::vector<std::string> headerFileNames;
            std::vector<std::string> h5FileNames;

            for ( const auto fileSummaryCase : fileSummaryCases )
            {
                auto headerFileName = fileSummaryCase->summaryHeaderFilename();

                QFileInfo fi( headerFileName );

                // NB! Use canonicalPath to make sure any symlinks are resolved to absolute file paths
                QString h5FilenameCandidate = fi.canonicalPath() + "/" + fi.baseName() + ".h5";

                headerFileNames.push_back( headerFileName.toStdString() );
                h5FileNames.push_back( h5FilenameCandidate.toStdString() );
            }

            RifHdf5SummaryExporter::ensureHdf5FileIsCreatedMultithreaded( headerFileNames,
                                                                          h5FileNames,
                                                                          prefs->createH5SummaryDataFiles(),
                                                                          threadCount );
        }
    }
#endif

    RifEnsembleImportConfig importState;
    if ( extractStateFromFirstCase && !RiaPreferencesSystem::current()->useImprovedSummaryImport() )
    {
        extractStateFromFirstCase = false;
    }

    if ( extractStateFromFirstCase )
    {
        // If we are extracting state from the first case, we need to make sure that the first case is loaded
        // before we start loading the rest of the cases.
        if ( fileSummaryCases.size() > 1 )
        {
            std::vector<QString> warnings;

            auto headerFileName0 = fileSummaryCases[0]->summaryHeaderFilename();
            auto headerFileName1 = fileSummaryCases[1]->summaryHeaderFilename();

            RifEnsembleImportConfig state;
            state.computePatternsFromSummaryFilePaths( headerFileName0, headerFileName1 );

            importState = state;
        }
    }

    // Use openMP when reading file summary case meta data. Avoid using the virtual interface of base class
    // RimSummaryCase, as it is difficult to make sure all variants of the leaf classes are thread safe.
    // Only open the summary file reader in parallel loop to reduce risk of multi threading issues

    {
        caf::ProgressInfo progInfo( fileSummaryCases.size(), "Loading Summary Cases" );
        RiaLogging::info( "Loading Summary Cases" );
        RifOpmCommonEclipseSummary::resetEnhancedSummaryFileCount();

        RiaThreadSafeLogger threadSafeLogger;
        QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents );

        // The HDF5 reader requires a special configuration to be thread safe. Disable threading for HDF reader.
        [[maybe_unused]] bool canUseMultipleThreads =
            ( prefs->summaryDataReader() != RiaPreferencesSummary::SummaryReaderMode::HDF5_OPM_COMMON );

        canUseMultipleThreads = canUseMultipleThreads && RiaPreferencesSystem::current()->useMultiThreadingForSummaryImport();

#pragma omp parallel for schedule( dynamic ) if ( canUseMultipleThreads )
        for ( int cIdx = 0; cIdx < static_cast<int>( fileSummaryCases.size() ); ++cIdx )
        {
            RimFileSummaryCase* fileSummaryCase = fileSummaryCases[cIdx];
            if ( fileSummaryCase )
            {
                fileSummaryCase->createSummaryReaderInterfaceThreadSafe( importState, &threadSafeLogger );

                QString parameterFilePath;
                if ( importState.useConfigValues() )
                {
                    auto realizationNumber = RifOpmSummaryTools::extractRealizationNumber( fileSummaryCase->summaryHeaderFilename() );
                    if ( realizationNumber.has_value() )
                    {
                        parameterFilePath = importState.pathToParameterFile( realizationNumber.value() );
                    }
                }

                auto startTime = RiaLogging::currentTime();
                addCaseRealizationParametersIfFound( *fileSummaryCase, fileSummaryCase->summaryHeaderFilename(), parameterFilePath );
                bool isLoggingEnabled = RiaPreferencesSystem::current()->isLoggingActivatedForKeyword( "OpmSummaryImport" );
                if ( isLoggingEnabled ) RiaLogging::logElapsedTime( "Setting of realization parameters", startTime );
            }

            RiaLogging::info( QString( "Completed %1" ).arg( fileSummaryCase->summaryHeaderFilename() ) );

            progInfo.setProgress( cIdx );
        }
        for ( const auto& txt : threadSafeLogger.messages() )
        {
            RiaLogging::info( txt );
        }
    }

    auto numberOfEsmryFilesCreated = RifOpmCommonEclipseSummary::numberOfEnhancedSummaryFileCreated();
    if ( numberOfEsmryFilesCreated > 0 )
    {
        RiaLogging::info( QString( "Summary Files : Converted and created %1 '*.ESMRY' files on disk." ).arg( numberOfEsmryFilesCreated ) );
    }

    // This loop is not thread safe, use serial loop
    for ( RimFileSummaryCase* fileSummaryCase : fileSummaryCases )
    {
        if ( fileSummaryCase )
        {
            fileSummaryCase->createRftReaderInterface();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryEnsemble* RimSummaryCaseMainCollection::defaultAllocator()
{
    return new RimSummaryEnsemble();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::onCaseNameChanged( const SignalEmitter* emitter )
{
    RiaSummaryTools::updateSummaryEnsembleNames();

    RimSummaryMultiPlotCollection* summaryPlotColl = RiaSummaryTools::summaryMultiPlotCollection();
    summaryPlotColl->updateSummaryNameHasChanged();

    // Update RFT plots, as they may depend on the summary case ensemble names
    auto rftPlotColl = RimMainPlotCollection::current()->rftPlotCollection();
    for ( auto plot : rftPlotColl->rftPlots() )
    {
        plot->loadDataAndUpdate();
    }

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*>
    RimSummaryCaseMainCollection::createSummaryCasesFromFileInfos( const std::vector<RifSummaryCaseFileResultInfo>& summaryHeaderFileInfos,
                                                                   bool                                             readStateFromFirstFile,
                                                                   bool                                             showProgress )
{
    RimProject* project = RimProject::current();

    std::vector<RimSummaryCase*> sumCases;

    // Split into two stages to be able to use multi threading
    // First stage : Create summary case objects
    // Second stage : Load data
    {
        std::unique_ptr<caf::ProgressInfo> progress;

        if ( showProgress )
        {
            progress = std::make_unique<caf::ProgressInfo>( summaryHeaderFileInfos.size(), "Creating summary cases" );
        }

        for ( const RifSummaryCaseFileResultInfo& fileInfo : summaryHeaderFileInfos )
        {
            QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents );

            auto existingSummaryCase = findTopLevelSummaryCaseFromFileName( fileInfo.summaryFileName() );
            if ( !existingSummaryCase )
            {
                const QString& smspecFileName = fileInfo.summaryFileName();

                if ( !smspecFileName.isEmpty() )
                {
                    RimSummaryCase* newSumCase = nullptr;

                    if ( fileInfo.fileType() == RiaDefines::FileType::SMSPEC )
                    {
                        auto sumCase = new RimFileSummaryCase();
                        sumCase->setIncludeRestartFiles( fileInfo.includeRestartFiles() );
                        newSumCase = sumCase;
                    }
                    else
                    {
                        auto sumCase = new RimCsvSummaryCase();
                        if ( fileInfo.fileType() == RiaDefines::FileType::STIMPLAN_SUMMARY )
                            sumCase->setFileType( RimCsvSummaryCase::FileType::STIMPLAN );
                        else
                            sumCase->setFileType( RimCsvSummaryCase::FileType::REVEAL );

                        newSumCase = sumCase;
                    }

                    newSumCase->setSummaryHeaderFileName( smspecFileName );
                    project->assignCaseIdToSummaryCase( newSumCase );

                    sumCases.push_back( newSumCase );
                }
                else
                {
                    QString txt = QString( "No UNSMRY file found for %1" ).arg( smspecFileName );
                    RiaLogging::warning( txt );
                }
            }

            if ( progress != nullptr ) progress->incrementProgress();
        }

        QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
    }

    RimSummaryCaseMainCollection::loadSummaryCaseData( sumCases, readStateFromFirstFile );

    return sumCases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCaseMainCollection::uniqueShortNameForCase( RimSummaryCase* summaryCase )
{
    std::set<QString> allAutoShortNames;

    for ( RimSummaryCase* sumCase : m_cases )
    {
        if ( sumCase && sumCase != summaryCase )
        {
            allAutoShortNames.insert( sumCase->displayCaseName() );
        }
    }

    bool foundUnique = false;

    QString caseName = summaryCase->nativeCaseName();
    QString shortName;

    if ( caseName.size() > 2 )
    {
        QString candidate;
        candidate += caseName[0];

        for ( int i = 1; i < caseName.size(); ++i )
        {
            if ( allAutoShortNames.count( candidate + caseName[i] ) == 0 )
            {
                shortName   = candidate + caseName[i];
                foundUnique = true;
                break;
            }
        }
    }
    else
    {
        shortName = caseName.left( 2 );
        if ( allAutoShortNames.count( shortName ) == 0 )
        {
            foundUnique = true;
        }
    }

    int autoNumber = 0;

    while ( !foundUnique )
    {
        QString candidate = shortName + QString::number( autoNumber++ );
        if ( allAutoShortNames.count( candidate ) == 0 )
        {
            shortName   = candidate;
            foundUnique = true;
        }
    }

    return shortName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::updateAutoShortName()
{
    // This update is required if the file path for the summary case is updated. To be able to produce plots
    // automatically, the short name must be generated on load
    //
    // https://github.com/OPM/ResInsight/issues/7438

    auto sumCases = allSummaryCases();

#pragma omp parallel for
    for ( int cIdx = 0; cIdx < static_cast<int>( sumCases.size() ); ++cIdx )
    {
        auto sumCase = sumCases[cIdx];
        sumCase->updateAutoShortName();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::onProjectBeingSaved()
{
    auto sumCases = allSummaryCases();
    for ( auto s : sumCases )
    {
        auto fileSumCase = dynamic_cast<RimFileSummaryCase*>( s );
        if ( fileSumCase )
        {
            fileSumCase->onProjectBeingSaved();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::updateEnsembleNames()
{
    std::set<std::string> key1;
    std::set<std::string> key2;

    auto ensembles = summaryEnsembles();

    for ( const auto& ensemble : ensembles )
    {
        const auto keys = ensemble->nameKeys();
        key1.insert( keys.first );
        key2.insert( keys.second );
    }

    bool useKey1 = key1.size() > 1;
    bool useKey2 = key2.size() > 1;

    if ( !useKey1 && !useKey2 )
    {
        useKey2 = true;
    }

    std::set<QString> existingNames;
    for ( auto ensemble : ensembles )
    {
        ensemble->setUsePathKey1( useKey1 );
        ensemble->setUsePathKey2( useKey2 );
        ensemble->updateName( existingNames );
        existingNames.insert( ensemble->name() );
    }
}
