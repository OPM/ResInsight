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

#include "RifEclipseInputFileTools.h"
#include "RifReaderEclipseInput.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"

#include "RimEclipseInputProperty.h"
#include "RimEclipseInputPropertyCollection.h"
#include "RimTools.h"

#include "cafProgressInfo.h"

#include <QFileInfo>

#define for_all( stdVector, indexName ) for ( size_t indexName = 0; indexName < stdVector.size(); ++indexName )
//--------------------------------------------------------------------------------------------------
/// Loads input property data from the gridFile and additional files
/// Creates new InputProperties if necessary, and flags the unused ones as obsolete
//--------------------------------------------------------------------------------------------------
void RifEclipseInputPropertyLoader::loadAndSyncronizeInputProperties(
    RimEclipseInputPropertyCollection* inputPropertyCollection,
    RigEclipseCaseData*                eclipseCaseData,
    const std::vector<QString>&        filenames )
{
    CVF_ASSERT( inputPropertyCollection );
    CVF_ASSERT( eclipseCaseData );
    CVF_ASSERT( eclipseCaseData->mainGrid()->gridPointDimensions() != cvf::Vec3st( 0, 0, 0 ) );

    size_t inputPropCount = inputPropertyCollection->inputProperties.size();

    caf::ProgressInfo progInfo( static_cast<int>( filenames.size() * inputPropCount ), "Reading Input properties" );

    for_all( filenames, i )
    {
        int progress = static_cast<int>( i * inputPropCount );
        // Find all the keywords present on the file

        progInfo.setProgressDescription( filenames[i] );

        QFileInfo fileNameInfo( filenames[i] );
        bool      isExistingFile = fileNameInfo.exists();

        std::set<QString> fileKeywordSet;

        if ( isExistingFile )
        {
            std::vector<RifKeywordAndFilePos> fileKeywords;
            RifEclipseInputFileTools::findKeywordsOnFile( filenames[i], &fileKeywords );

            for_all( fileKeywords, fkIt ) fileKeywordSet.insert( fileKeywords[fkIt].keyword );
        }

        // Find the input property objects referring to the file

        std::vector<RimEclipseInputProperty*> ipsUsingThisFile = inputPropertyCollection->findInputProperties(
            filenames[i] );

        // Read property data for each inputProperty

        for_all( ipsUsingThisFile, ipIdx )
        {
            if ( !isExistingFile )
            {
                ipsUsingThisFile[ipIdx]->resolvedState = RimEclipseInputProperty::FILE_MISSING;
            }
            else
            {
                QString kw                             = ipsUsingThisFile[ipIdx]->eclipseKeyword();
                ipsUsingThisFile[ipIdx]->resolvedState = RimEclipseInputProperty::KEYWORD_NOT_IN_FILE;
                if ( fileKeywordSet.count( kw ) )
                {
                    if ( RifEclipseInputFileTools::readProperty( filenames[i],
                                                                 eclipseCaseData,
                                                                 kw,
                                                                 ipsUsingThisFile[ipIdx]->resultName ) )
                    {
                        ipsUsingThisFile[ipIdx]->resolvedState = RimEclipseInputProperty::RESOLVED;
                    }
                }
                fileKeywordSet.erase( kw );
            }

            progInfo.setProgress( static_cast<int>( progress + ipIdx ) );
        }

        progInfo.setProgress( static_cast<int>( progress + inputPropCount ) );
        // Check if there are more known property keywords left on file. If it is, read them and create inputProperty
        // objects

        for ( const QString& fileKeyword : fileKeywordSet )
        {
            {
                QString resultName = eclipseCaseData->results( RiaDefines::MATRIX_MODEL )->makeResultNameUnique( fileKeyword );
                if ( RifEclipseInputFileTools::readProperty( filenames[i], eclipseCaseData, fileKeyword, resultName ) )
                {
                    RimEclipseInputProperty* inputProperty = new RimEclipseInputProperty;
                    inputProperty->resultName              = resultName;
                    inputProperty->eclipseKeyword          = fileKeyword;
                    inputProperty->fileName                = filenames[i];
                    inputProperty->resolvedState           = RimEclipseInputProperty::RESOLVED;
                    inputPropertyCollection->inputProperties.push_back( inputProperty );
                }
            }

            progInfo.setProgress( static_cast<int>( progress + inputPropCount ) );
        }
    }

    for_all( inputPropertyCollection->inputProperties, i )
    {
        if ( inputPropertyCollection->inputProperties[i]->resolvedState() == RimEclipseInputProperty::UNKNOWN )
        {
            inputPropertyCollection->inputProperties[i]->resolvedState = RimEclipseInputProperty::FILE_MISSING;
        }
    }
}

bool RifEclipseInputPropertyLoader::readInputPropertiesFromFiles( RimEclipseInputPropertyCollection* inputPropertyCollection,
                                                                  RigEclipseCaseData*                eclipseCaseData,
                                                                  bool                               importFaults,
                                                                  const std::vector<QString>&        filenames )
{
    for ( const QString& propertyFileName : filenames )
    {
        std::map<QString, QString> readProperties = RifEclipseInputFileTools::readProperties( propertyFileName,
                                                                                              eclipseCaseData );

        std::map<QString, QString>::iterator it;
        for ( it = readProperties.begin(); it != readProperties.end(); ++it )
        {
            RimEclipseInputProperty* inputProperty = new RimEclipseInputProperty;
            inputProperty->resultName              = it->first;
            inputProperty->eclipseKeyword          = it->second;
            inputProperty->fileName                = propertyFileName;
            inputProperty->resolvedState           = RimEclipseInputProperty::RESOLVED;
            inputPropertyCollection->inputProperties.push_back( inputProperty );
        }

        if ( importFaults )
        {
            cvf::Collection<RigFault> faultCollection;
            RifEclipseInputFileTools::parseAndReadFaults( propertyFileName, &faultCollection );

            if ( !faultCollection.empty() )
            {
                eclipseCaseData->mainGrid()->setFaults( faultCollection );
            }
        }
    }

    // TODO: seems a bit optimistic?
    return true;
}
