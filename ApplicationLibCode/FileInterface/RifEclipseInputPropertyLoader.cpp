/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RifEclipseInputPropertyLoader.h"

#include "RiaLogging.h"

#include "RifEclipseInputFileTools.h"
#include "RifEclipseTextFileReader.h"
#include "RifReaderEclipseInput.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"

#include "RimEclipseInputProperty.h"
#include "RimEclipseInputPropertyCollection.h"
#include "RimTools.h"

#include "cafProgressInfo.h"

#include <QFile>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseInputPropertyLoader::loadAndSyncronizeInputProperties( RimEclipseInputPropertyCollection* inputPropertyCollection,
                                                                      RigEclipseCaseData*         eclipseCaseData,
                                                                      const std::vector<QString>& filenames,
                                                                      bool                        allowImportOfFaults )
{
    std::vector<RimEclipseInputProperty*> existingProperties = inputPropertyCollection->inputProperties.childObjects();

    caf::ProgressInfo progInfo( static_cast<int>( filenames.size() ), "Reading Input properties" );

    for ( const auto& filename : filenames )
    {
        progInfo.setProgressDescription( filename );

        auto resultNamesEclipseKeywords = RifEclipseInputPropertyLoader::readProperties( filename, eclipseCaseData );

        for ( const auto& [resultName, eclipseKeyword] : resultNamesEclipseKeywords )
        {
            bool isPresent = false;
            for ( const auto* propertyObj : existingProperties )
            {
                if ( propertyObj->resultName() == resultName )
                {
                    isPresent = true;
                    break;
                }
            }

            if ( !isPresent )
            {
                RimEclipseInputProperty* inputProperty = new RimEclipseInputProperty;
                inputProperty->resultName              = resultName;
                inputProperty->eclipseKeyword          = eclipseKeyword;
                inputProperty->fileName                = filename;
                inputProperty->resolvedState           = RimEclipseInputProperty::RESOLVED;
                inputPropertyCollection->inputProperties.push_back( inputProperty );
            }

            if ( allowImportOfFaults )
            {
                importFaultsFromFile( eclipseCaseData, filename );
            }
        }

        progInfo.incrementProgress();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifEclipseInputPropertyLoader::importFaultsFromFile( RigEclipseCaseData* eclipseCaseData, const QString& fileName )
{
    cvf::Collection<RigFault> faultCollectionFromFile;
    RifEclipseInputFileTools::parseAndReadFaults( fileName, &faultCollectionFromFile );
    if ( faultCollectionFromFile.empty() )
    {
        return false;
    }

    cvf::Collection<RigFault> faults;
    {
        cvf::Collection<RigFault> faultCollection = eclipseCaseData->mainGrid()->faults();
        for ( size_t i = 0; i < faultCollection.size(); i++ )
        {
            RigFault* f = faultCollection.at( i );
            if ( f->name() == RiaResultNames::undefinedGridFaultName() ||
                 f->name() == RiaResultNames::undefinedGridFaultName() )
            {
                // Do not include undefined grid faults, as these are recomputed based on the imported faults from filesa
                continue;
            }

            faults.push_back( f );
        }
    }

    for ( size_t i = 0; i < faultCollectionFromFile.size(); i++ )
    {
        RigFault* faultFromFile = faultCollectionFromFile.at( i );

        bool existFaultWithSameName = false;
        for ( size_t j = 0; j < faults.size(); j++ )
        {
            RigFault* existingFault = faults.at( j );

            if ( existingFault->name() == faultFromFile->name() )
            {
                existFaultWithSameName = true;
            }
        }

        if ( !existFaultWithSameName )
        {
            faults.push_back( faultFromFile );
        }
    }

    eclipseCaseData->mainGrid()->setFaults( faults );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString, QString> RifEclipseInputPropertyLoader::readProperties( const QString&      fileName,
                                                                          RigEclipseCaseData* eclipseCase )
{
    std::string fileContent;
    {
        QFile data( fileName );
        if ( !data.open( QFile::ReadOnly ) )
        {
            RiaLogging::error( "Failed to open " + fileName );
            return {};
        }

        fileContent = data.readAll();
    }

    size_t offset    = 0;
    size_t bytesRead = 0;

    std::map<QString, QString> resultNameAndEclipseNameMap;
    while ( offset < fileContent.size() )
    {
        RifEclipseTextFileReader reader;
        auto [eclipseKeyword, values] = reader.readKeywordAndValues( fileContent, offset, bytesRead );
        offset += bytesRead;

        if ( !values.empty() )
        {
            QString newResultName = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
                                        ->makeResultNameUnique( QString::fromStdString( eclipseKeyword ) );

            QString errorText;
            if ( appendInputPropertyResult( eclipseCase, newResultName, eclipseKeyword, values, &errorText ) )
            {
                resultNameAndEclipseNameMap[newResultName] = QString::fromStdString( eclipseKeyword );
            }
            else
            {
                RiaLogging::error( errorText );
            }
        }
    }

    return resultNameAndEclipseNameMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<QString>& RifEclipseInputPropertyLoader::invalidPropertyDataKeywords()
{
    static std::vector<QString> keywords;
    static bool                 isInitialized = false;
    if ( !isInitialized )
    {
        // Related to geometry
        keywords.push_back( "COORD" );
        keywords.push_back( "ZCORN" );
        keywords.push_back( "SPECGRID" );
        keywords.push_back( "MAPAXES" );
        keywords.push_back( "NOECHO" );
        keywords.push_back( "ECHO" );
        keywords.push_back( "MAPUNITS" );
        keywords.push_back( "GRIDUNIT" );

        keywords.push_back( "FAULTS" );

        isInitialized = true;
    }

    return keywords;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifEclipseInputPropertyLoader::isValidDataKeyword( const QString& keyword )
{
    const std::vector<QString>& keywordsToSkip = RifEclipseInputPropertyLoader::invalidPropertyDataKeywords();
    for ( const QString& keywordToSkip : keywordsToSkip )
    {
        if ( keywordToSkip == keyword.toUpper() )
        {
            return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifEclipseInputPropertyLoader::appendInputPropertyResult( RigEclipseCaseData*       caseData,
                                                               const QString&            resultName,
                                                               const std::string&        eclipseKeyword,
                                                               const std::vector<float>& values,
                                                               QString*                  errMsg )
{
    if ( !isValidDataKeyword( QString::fromStdString( eclipseKeyword ) ) ) return false;

    CVF_ASSERT( caseData );
    CVF_ASSERT( errMsg );

    size_t keywordItemCount = values.size();
    if ( keywordItemCount != caseData->mainGrid()->cellCount() )
    {
        QString errFormat( "Size mismatch: Main Grid has %1 cells, keyword %2 has %3 cells" );
        *errMsg = errFormat.arg( caseData->mainGrid()->cellCount() ).arg( resultName ).arg( keywordItemCount );
        return false;
    }

    RigEclipseResultAddress resAddr( RiaDefines::ResultCatType::INPUT_PROPERTY, resultName );
    caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->createResultEntry( resAddr, false );

    auto newPropertyData =
        caseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->modifiableCellScalarResultTimesteps( resAddr );

    std::vector<double> doubleVals;
    doubleVals.insert( doubleVals.begin(), values.begin(), values.end() );

    newPropertyData->push_back( doubleVals );

    return true;
}
