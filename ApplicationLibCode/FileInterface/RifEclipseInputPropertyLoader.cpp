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
#include "RifEclipseKeywordContent.h"
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
            bool isProperyPresent      = false;
            bool isFaultKeywordPresent = false;
            for ( const auto* propertyObj : existingProperties )
            {
                if ( propertyObj->resultName() == resultName )
                {
                    isProperyPresent = true;
                }
                else if ( propertyObj->resultName() == "FAULTS" )
                {
                    isFaultKeywordPresent = true;
                }
            }

            if ( !isProperyPresent )
            {
                RimEclipseInputProperty* inputProperty = new RimEclipseInputProperty;
                inputProperty->resultName              = resultName;
                inputProperty->eclipseKeyword          = eclipseKeyword;
                inputProperty->fileName                = filename;
                inputProperty->resolvedState           = RimEclipseInputProperty::RESOLVED;
                inputPropertyCollection->inputProperties.push_back( inputProperty );
            }

            if ( allowImportOfFaults && isFaultKeywordPresent )
            {
                RifEclipseInputFileTools::importFaultsFromFile( eclipseCaseData, filename );
            }
        }

        progInfo.incrementProgress();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifEclipseInputPropertyLoader::evaluateAndCreateInputPropertyResult( RigEclipseCaseData* eclipseCase,
                                                                             const RifEclipseKeywordContent& keywordContent,
                                                                             QString* errorMessage )
{
    auto eclipseKeyword = keywordContent.keyword;
    if ( isInputPropertyCandidate( eclipseCase, eclipseKeyword, keywordContent.values.size() ) )
    {
        QString newResultName = eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
                                    ->makeResultNameUnique( QString::fromStdString( eclipseKeyword ) );

        if ( appendNewInputPropertyResult( eclipseCase, newResultName, eclipseKeyword, keywordContent.values, errorMessage ) )
        {
            return newResultName;
        }
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseInputPropertyLoader::createInputPropertiesFromKeywords( RigEclipseCaseData* eclipseCase,
                                                                       const std::vector<RifEclipseKeywordContent>& keywordContent )
{
    for ( const auto& keywordAndValues : keywordContent )
    {
        RifEclipseInputPropertyLoader::evaluateAndCreateInputPropertyResult( eclipseCase, keywordAndValues, nullptr );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifEclipseInputPropertyLoader::isInputPropertyCandidate( const RigEclipseCaseData* caseData,
                                                              const std::string&        eclipseKeyword,
                                                              size_t                    numberOfValues )
{
    CVF_ASSERT( caseData );

    if ( !isValidDataKeyword( QString::fromStdString( eclipseKeyword ) ) ) return false;

    return ( numberOfValues == caseData->mainGrid()->cellCount() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString, QString> RifEclipseInputPropertyLoader::readProperties( const QString&      fileName,
                                                                          RigEclipseCaseData* eclipseCase )
{
    std::map<QString, QString> resultNameAndEclipseNameMap;

    auto keywordContent = RifEclipseTextFileReader::readKeywordAndValues( fileName.toStdString() );
    for ( const auto& keywordAndValues : keywordContent )
    {
        QString errorText;

        QString newResultName = evaluateAndCreateInputPropertyResult( eclipseCase, keywordAndValues, &errorText );
        if ( !newResultName.isEmpty() )
        {
            resultNameAndEclipseNameMap[newResultName] = QString::fromStdString( keywordAndValues.keyword );
        }
        else
        {
            RiaLogging::error( errorText );
        }
    }

    return resultNameAndEclipseNameMap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<QString>& RifEclipseInputPropertyLoader::invalidPropertyDataKeywords()
{
    // List of keywords that will be skipped when looking for property data

    static std::vector<QString> keywords =
        { "COORD", "ZCORN", "SPECGRID", "MAPAXES", "NOECHO", "ECHO", "MAPUNITS", "GRIDUNIT", "GDORIENT", "INC", "DEC", "FAULTS" };

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
bool RifEclipseInputPropertyLoader::appendNewInputPropertyResult( RigEclipseCaseData*       caseData,
                                                                  const QString&            resultName,
                                                                  const std::string&        eclipseKeyword,
                                                                  const std::vector<float>& values,
                                                                  QString*                  errMsg )
{
    if ( !isValidDataKeyword( QString::fromStdString( eclipseKeyword ) ) ) return false;

    CVF_ASSERT( caseData );

    size_t keywordItemCount = values.size();
    if ( keywordItemCount != caseData->mainGrid()->cellCount() )
    {
        if ( errMsg )
        {
            QString errFormat( "Size mismatch: Main Grid has %1 cells, keyword %2 has %3 cells" );
            *errMsg = errFormat.arg( caseData->mainGrid()->cellCount() ).arg( resultName ).arg( keywordItemCount );
        }

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
