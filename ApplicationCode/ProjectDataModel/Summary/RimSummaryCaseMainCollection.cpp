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

#include "RifCaseRealizationParametersReader.h"
#include "RifEclipseSummaryTools.h"
#include "RifSummaryCaseRestartSelector.h"

#include "RimDerivedEnsembleCaseCollection.h"
#include "RimEclipseResultCase.h"
#include "RimFileSummaryCase.h"
#include "RimGridSummaryCase.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCurve.h"

#include "cafProgressInfo.h"
#include <QDir>

CAF_PDM_SOURCE_INIT( RimSummaryCaseMainCollection, "SummaryCaseCollection" );

//--------------------------------------------------------------------------------------------------
/// Internal function
//--------------------------------------------------------------------------------------------------
void addCaseRealizationParametersIfFound( RimSummaryCase& sumCase, const QString modelFolderOrFile )
{
    std::shared_ptr<RigCaseRealizationParameters> parameters;
    QString parametersFile = RifCaseRealizationParametersFileLocator::locate( modelFolderOrFile );
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
    parameters->addParameter( "RI:REALIZATION_NUM", realizationNumber );

    sumCase.setCaseRealizationParameters( parameters );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCaseMainCollection::RimSummaryCaseMainCollection()
{
    CAF_PDM_InitObject( "Summary Cases", ":/SummaryCases16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_cases, "SummaryCases", "", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_caseCollections, "SummaryCaseCollections", "", "", "", "" );

    m_cases.uiCapability()->setUiHidden( true );
    m_caseCollections.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCaseMainCollection::~RimSummaryCaseMainCollection()
{
    m_cases.deleteAllChildObjectsAsync();
    m_caseCollections.deleteAllChildObjectsAsync();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase*
    RimSummaryCaseMainCollection::findSummaryCaseFromEclipseResultCase( const RimEclipseResultCase* eclipseResultCase ) const
{
    for ( RimSummaryCase* summaryCase : m_cases )
    {
        RimGridSummaryCase* gridSummaryCase = dynamic_cast<RimGridSummaryCase*>( summaryCase );
        if ( gridSummaryCase && gridSummaryCase->associatedEclipseCase() )
        {
            if ( gridSummaryCase->associatedEclipseCase()->gridFileName() == eclipseResultCase->gridFileName() )
            {
                return gridSummaryCase;
            }
        }
    }

    for ( auto collection : m_caseCollections )
    {
        for ( RimSummaryCase* sumCase : collection->allSummaryCases() )
        {
            RimGridSummaryCase* gridSummaryCase = dynamic_cast<RimGridSummaryCase*>( sumCase );
            if ( gridSummaryCase &&
                 gridSummaryCase->associatedEclipseCase()->gridFileName() == eclipseResultCase->gridFileName() )
            {
                return gridSummaryCase;
            }
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimSummaryCaseMainCollection::findSummaryCaseFromFileName( const QString& fileName ) const
{
    // Use QFileInfo object to compare two file names to avoid mix of / and \\

    QFileInfo incomingFileInfo( fileName );

    for ( RimSummaryCase* summaryCase : m_cases )
    {
        if ( summaryCase )
        {
            QFileInfo summaryFileInfo( summaryCase->summaryHeaderFilename() );
            if ( incomingFileInfo == summaryFileInfo )
            {
                return summaryCase;
            }
        }
    }

    for ( auto collection : m_caseCollections )
    {
        for ( RimSummaryCase* summaryCase : collection->allSummaryCases() )
        {
            if ( summaryCase )
            {
                QFileInfo summaryFileInfo( summaryCase->summaryHeaderFilename() );
                if ( incomingFileInfo == summaryFileInfo )
                {
                    return summaryCase;
                }
            }
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::convertGridSummaryCasesToFileSummaryCases( RimGridSummaryCase* gridSummaryCase )
{
    RimFileSummaryCase* fileSummaryCase = gridSummaryCase->createFileSummaryCaseCopy();
    addCaseRealizationParametersIfFound( *fileSummaryCase, fileSummaryCase->summaryHeaderFilename() );

    RimSummaryCaseCollection* collection;
    gridSummaryCase->firstAncestorOrThisOfType( collection );

    if ( collection )
    {
        collection->addCase( fileSummaryCase );
        collection->updateConnectedEditors();
    }
    else
    {
        this->addCase( fileSummaryCase );
        this->updateConnectedEditors();
    }

    loadSummaryCaseData( {fileSummaryCase} );
    reassignSummaryCurves( gridSummaryCase, fileSummaryCase );

    removeCase( gridSummaryCase );
    delete gridSummaryCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::addCases( const std::vector<RimSummaryCase*> cases )
{
    for ( RimSummaryCase* sumCase : cases )
    {
        addCase( sumCase );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::addCase( RimSummaryCase* summaryCase )
{
    m_cases.push_back( summaryCase );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::removeCase( RimSummaryCase* summaryCase )
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

    m_cases.removeChildObject( summaryCase );
    for ( RimSummaryCaseCollection* summaryCaseCollection : m_caseCollections )
    {
        summaryCaseCollection->removeCase( summaryCase );
    }

    // Update derived ensemble cases (if any)
    for ( auto derEnsemble : derivedEnsembles )
    {
        derEnsemble->updateDerivedEnsembleCases();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCaseCollection*
    RimSummaryCaseMainCollection::addCaseCollection( std::vector<RimSummaryCase*>               summaryCases,
                                                     const QString&                             collectionName,
                                                     bool                                       isEnsemble,
                                                     std::function<RimSummaryCaseCollection*()> allocator )
{
    RimSummaryCaseCollection* summaryCaseCollection = allocator();
    if ( !collectionName.isEmpty() ) summaryCaseCollection->setName( collectionName );

    for ( RimSummaryCase* summaryCase : summaryCases )
    {
        RimSummaryCaseCollection* currentSummaryCaseCollection = nullptr;
        summaryCase->firstAncestorOrThisOfType( currentSummaryCaseCollection );

        if ( currentSummaryCaseCollection )
        {
            currentSummaryCaseCollection->removeCase( summaryCase );
        }
        else
        {
            m_cases.removeChildObject( summaryCase );
        }

        summaryCaseCollection->addCase( summaryCase );
    }

    summaryCaseCollection->setAsEnsemble( isEnsemble );

    m_caseCollections.push_back( summaryCaseCollection );

    return summaryCaseCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::removeCaseCollection( RimSummaryCaseCollection* caseCollection )
{
    m_caseCollections.removeChildObject( caseCollection );
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
    this->descendantsIncludingThisOfType( cases );

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
void RimSummaryCaseMainCollection::loadSummaryCaseData( std::vector<RimSummaryCase*> summaryCases )
{
    std::vector<RimFileSummaryCase*> fileSummaryCases;
    std::vector<RimSummaryCase*>     otherSummaryCases;

    if ( RiaApplication::instance()->preferences()->useMultipleThreadsWhenReadingSummaryData() )
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::loadFileSummaryCaseData( std::vector<RimFileSummaryCase*> fileSummaryCases )
{
    // Use openMP when reading file summary case meta data. Avoid using the virtual interface of base class
    // RimSummaryCase, as it is difficult to make sure all variants of the leaf classes are thread safe.
    // Only open the summary file reader in parallel loop to reduce risk of multi threading issues

    caf::ProgressInfo progInfo( fileSummaryCases.size(), "Loading Summary Cases" );

#pragma omp parallel for schedule( dynamic )
    for ( int cIdx = 0; cIdx < static_cast<int>( fileSummaryCases.size() ); ++cIdx )
    {
        RimFileSummaryCase* fileSummaryCase = fileSummaryCases[cIdx];
        if ( fileSummaryCase )
        {
            fileSummaryCase->createSummaryReaderInterface();
        }

        progInfo.setProgress( cIdx );
    }

    for ( int cIdx = 0; cIdx < static_cast<int>( fileSummaryCases.size() ); ++cIdx )
    {
        RimFileSummaryCase* fileSummaryCase = fileSummaryCases[cIdx];
        if ( fileSummaryCase )
        {
            fileSummaryCase->createRftReaderInterface();
            addCaseRealizationParametersIfFound( *fileSummaryCase, fileSummaryCase->summaryHeaderFilename() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseMainCollection::reassignSummaryCurves( const RimGridSummaryCase* gridSummaryCase,
                                                          RimFileSummaryCase*       fileSummaryCase )
{
    std::vector<caf::PdmFieldHandle*> referringFields;
    gridSummaryCase->referringPtrFields( referringFields );
    for ( caf::PdmFieldHandle* field : referringFields )
    {
        RimSummaryCurve* summaryCurve = dynamic_cast<RimSummaryCurve*>( field->ownerObject() );
        if ( summaryCurve )
        {
            bool updated = false;
            if ( summaryCurve->summaryCaseX() == gridSummaryCase )
            {
                summaryCurve->setSummaryCaseX( fileSummaryCase );
                updated = true;
            }
            if ( summaryCurve->summaryCaseY() == gridSummaryCase )
            {
                summaryCurve->setSummaryCaseY( fileSummaryCase );
                updated = true;
            }
            if ( updated )
            {
                summaryCurve->loadDataAndUpdate( false );
                summaryCurve->updateConnectedEditors();
            }
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
std::vector<RimSummaryCase*> RimSummaryCaseMainCollection::createSummaryCasesFromFileInfos(
    const std::vector<RifSummaryCaseFileResultInfo>& summaryHeaderFileInfos,
    bool                                             showProgress )
{
    RimProject* project = RiaApplication::instance()->project();

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
            RimEclipseCase* eclCase = nullptr;
            QString         gridCaseFile =
                RifEclipseSummaryTools::findGridCaseFileFromSummaryHeaderFile( fileInfo.summaryFileName() );
            if ( !gridCaseFile.isEmpty() )
            {
                eclCase = project->eclipseCaseFromGridFileName( gridCaseFile );
            }

            RimGridSummaryCase* existingGridSummaryCase =
                dynamic_cast<RimGridSummaryCase*>( findSummaryCaseFromFileName( fileInfo.summaryFileName() ) );

            if ( eclCase && !existingGridSummaryCase )
            {
                RimGridSummaryCase* newSumCase = new RimGridSummaryCase();

                newSumCase->setIncludeRestartFiles( fileInfo.includeRestartFiles() );
                newSumCase->setAssociatedEclipseCase( eclCase );
                newSumCase->updateOptionSensitivity();
                sumCases.push_back( newSumCase );
            }
            else
            {
                RimFileSummaryCase* newSumCase = new RimFileSummaryCase();

                newSumCase->setIncludeRestartFiles( fileInfo.includeRestartFiles() );
                newSumCase->setSummaryHeaderFileName( fileInfo.summaryFileName() );
                newSumCase->updateOptionSensitivity();
                sumCases.push_back( newSumCase );
            }

            if ( progress != nullptr ) progress->incrementProgress();
        }
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
void RimSummaryCaseMainCollection::updateFilePathsFromProjectPath( const QString& newProjectPath,
                                                                   const QString& oldProjectPath )
{
    for ( auto summaryCase : m_cases )
    {
        summaryCase->updateFilePathsFromProjectPath( newProjectPath, oldProjectPath );
    }
}
