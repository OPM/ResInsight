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
#include "RiaLogging.h"
#include "RiaPreferencesSummary.h"
#include "RiaSummaryTools.h"

#include "RifCaseRealizationParametersReader.h"
#include "RifEclipseSummaryTools.h"
#include "RifOpmCommonSummary.h"
#include "RifSummaryCaseRestartSelector.h"

#ifdef USE_HDF5
#include "RifHdf5SummaryExporter.h"
#endif

#include "RimCaseDisplayNameTools.h"
#include "RimCsvSummaryCase.h"
#include "RimDerivedEnsembleCaseCollection.h"
#include "RimEclipseResultCase.h"
#include "RimFileSummaryCase.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryMultiPlotCollection.h"
#include "RimSummaryPlot.h"

#include "cafProgressInfo.h"

#include <QCoreApplication>
#include <QDir>

CAF_PDM_SOURCE_INIT( RimSummaryCaseMainCollection, "SummaryCaseCollection" );

//--------------------------------------------------------------------------------------------------
/// Internal function
//--------------------------------------------------------------------------------------------------
void addCaseRealizationParametersIfFound( RimSummaryCase& sumCase, const QString modelFolderOrFile )
{
    std::shared_ptr<RigCaseRealizationParameters> parameters;
    QString                                       parametersFile = RifCaseRealizationParametersFileLocator::locate( modelFolderOrFile );
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
        parameters = std::shared_ptr<RigCaseRealizationParameters>( new RigCaseRealizationParameters() );
    }

    int realizationNumber = RifCaseRealizationParametersFileLocator::realizationNumber( modelFolderOrFile );
    parameters->setRealizationNumber( realizationNumber );
    parameters->addParameter( RiaDefines::summaryRealizationNumber(), realizationNumber );

    sumCase.setCaseRealizationParameters( parameters );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCaseMainCollection::RimSummaryCaseMainCollection()
    : dataSourceHasChanged( this )
{
    CAF_PDM_InitObject( "Summary Cases", ":/SummaryCases16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_cases, "SummaryCases", "" );
    CAF_PDM_InitFieldNoDefault( &m_caseCollections, "SummaryCaseCollections", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCaseMainCollection::~RimSummaryCaseMainCollection()
{
    m_cases.deleteChildrenAsync();
    m_caseCollections.deleteChildrenAsync();
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
    std::vector<RimDerivedEnsembleCaseCollection*> derivedEnsembles;

    // Build a list of derived ensembles that must be updated after delete
    for ( auto group : summaryCaseCollections() )
    {
        auto derEnsemble = dynamic_cast<RimDerivedEnsembleCaseCollection*>( group );
        if ( derEnsemble )
        {
            if ( derEnsemble->hasCaseReference( summaryCase ) )
            {
                derivedEnsembles.push_back( derEnsemble );
            }
        }
    }

    m_cases.removeChild( summaryCase );

    for ( RimSummaryCaseCollection* summaryCaseCollection : m_caseCollections )
    {
        summaryCaseCollection->removeCase( summaryCase, notifyChange );
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

    for ( RimSummaryCaseCollection* summaryCaseCollection : m_caseCollections )
    {
        summaryCaseCollection->updateReferringCurveSets();
    }

    dataSourceHasChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCaseCollection* RimSummaryCaseMainCollection::addCaseCollection( std::vector<RimSummaryCase*>               summaryCases,
                                                                           const QString&                             collectionName,
                                                                           bool                                       isEnsemble,
                                                                           std::function<RimSummaryCaseCollection*()> allocator )
{
    RimSummaryCaseCollection* summaryCaseCollection = allocator();
    if ( !collectionName.isEmpty() ) summaryCaseCollection->setName( collectionName );

    if ( summaryCaseCollection->ensembleId() == -1 )
    {
        RimProject* project = RimProject::current();
        project->assignIdToEnsemble( summaryCaseCollection );
    }

    for ( RimSummaryCase* summaryCase : summaryCases )
    {
        auto currentSummaryCaseCollection = summaryCase->firstAncestorOrThisOfType<RimSummaryCaseCollection>();
        if ( currentSummaryCaseCollection )
        {
            currentSummaryCaseCollection->removeCase( summaryCase );
        }
        else
        {
            m_cases.removeChild( summaryCase );
        }

        summaryCaseCollection->addCase( summaryCase );
        if ( isEnsemble )
        {
            summaryCase->setDisplayNameOption( RimCaseDisplayNameTools::DisplayName::SHORT_CASE_NAME );
        }
    }

    summaryCaseCollection->setAsEnsemble( isEnsemble );

    summaryCaseCollection->caseNameChanged.connect( this, &RimSummaryCaseMainCollection::onCaseNameChanged );
    m_caseCollections.push_back( summaryCaseCollection );

    dataSourceHasChanged.send();

    return summaryCaseCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::removeCaseCollection( RimSummaryCaseCollection* caseCollection )
{
    m_caseCollections.removeChild( caseCollection );

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

    for ( auto& coll : m_caseCollections )
    {
        auto collCases = coll->allSummaryCases();
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
std::vector<RimSummaryCaseCollection*> RimSummaryCaseMainCollection::summaryCaseCollections() const
{
    std::vector<RimSummaryCaseCollection*> summaryCaseCollections;
    for ( const auto& caseColl : m_caseCollections )
    {
        summaryCaseCollections.push_back( caseColl );
    }
    return summaryCaseCollections;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::loadAllSummaryCaseData()
{
    std::vector<RimSummaryCase*> sumCases = allSummaryCases();

    RimSummaryCaseMainCollection::loadSummaryCaseData( sumCases );
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

    for ( auto caseCollection : summaryCaseCollections() )
    {
        caseCollection->caseNameChanged.connect( this, &RimSummaryCaseMainCollection::onCaseNameChanged );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::loadSummaryCaseData( std::vector<RimSummaryCase*> summaryCases )
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
        loadFileSummaryCaseData( fileSummaryCases );
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
                addCaseRealizationParametersIfFound( *sumCase, sumCase->summaryHeaderFilename() );
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
void RimSummaryCaseMainCollection::loadFileSummaryCaseData( std::vector<RimFileSummaryCase*> fileSummaryCases )
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

    // Use openMP when reading file summary case meta data. Avoid using the virtual interface of base class
    // RimSummaryCase, as it is difficult to make sure all variants of the leaf classes are thread safe.
    // Only open the summary file reader in parallel loop to reduce risk of multi threading issues

    {
        caf::ProgressInfo progInfo( fileSummaryCases.size(), "Loading Summary Cases" );

        RifOpmCommonEclipseSummary::resetEnhancedSummaryFileCount();

        RiaThreadSafeLogger threadSafeLogger;
        QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents );

        // The HDF5 reader requires a special configuration to be thread safe. Disable threading for HDF reader.
        bool canUseMultipleTreads = ( prefs->summaryDataReader() != RiaPreferencesSummary::SummaryReaderMode::HDF5_OPM_COMMON );

#pragma omp parallel for schedule( dynamic ) if ( canUseMultipleTreads )
        for ( int cIdx = 0; cIdx < static_cast<int>( fileSummaryCases.size() ); ++cIdx )
        {
            RimFileSummaryCase* fileSummaryCase = fileSummaryCases[cIdx];
            if ( fileSummaryCase )
            {
                fileSummaryCase->createSummaryReaderInterfaceThreadSafe( &threadSafeLogger );
                addCaseRealizationParametersIfFound( *fileSummaryCase, fileSummaryCase->summaryHeaderFilename() );
            }

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
RimSummaryCaseCollection* RimSummaryCaseMainCollection::defaultAllocator()
{
    return new RimSummaryCaseCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::onCaseNameChanged( const SignalEmitter* emitter )
{
    RimSummaryMultiPlotCollection* summaryPlotColl = RiaSummaryTools::summaryMultiPlotCollection();
    summaryPlotColl->updateSummaryNameHasChanged();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*>
    RimSummaryCaseMainCollection::createSummaryCasesFromFileInfos( const std::vector<RifSummaryCaseFileResultInfo>& summaryHeaderFileInfos,
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
            progress.reset( new caf::ProgressInfo( summaryHeaderFileInfos.size(), "Creating summary cases" ) );
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

    RimSummaryCaseMainCollection::loadSummaryCaseData( sumCases );

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
