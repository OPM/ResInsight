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

//--------------------------------------------------------------------------------------------------
/// Loads input property data from the gridFile and additional files
/// Creates new InputProperties if necessary, and flags the unused ones as obsolete
//--------------------------------------------------------------------------------------------------
void RifEclipseInputPropertyLoader::loadAndSyncronizeInputProperties( RimEclipseInputPropertyCollection* inputPropertyCollection,
                                                                      RigEclipseCaseData*         eclipseCaseData,
                                                                      const std::vector<QString>& filenames )
{
    CVF_ASSERT( inputPropertyCollection );
    CVF_ASSERT( eclipseCaseData );
    CVF_ASSERT( eclipseCaseData->mainGrid()->gridPointDimensions() != cvf::Vec3st( 0, 0, 0 ) );

    size_t            inputPropCount = inputPropertyCollection->inputProperties.size();
    caf::ProgressInfo progInfo( static_cast<int>( filenames.size() * inputPropCount ), "Reading Input properties" );

    int i = 0;
    for ( const QString& filename : filenames )
    {
        int progress = static_cast<int>( i * inputPropCount );
        progInfo.setProgressDescription( filename );

        QFileInfo fileNameInfo( filename );
        bool      isExistingFile = fileNameInfo.exists();

        // Find all the keywords present on the file
        std::set<QString> fileKeywordSet = extractKeywordsOnFile( filenames[i], isExistingFile );

        readDataForEachInputProperty( inputPropertyCollection,
                                      eclipseCaseData,
                                      filename,
                                      isExistingFile,
                                      &fileKeywordSet,
                                      &progInfo,
                                      progress );

        progInfo.setProgress( static_cast<int>( progress + inputPropCount ) );

        // Check if there are more known property keywords left on file.
        // If it is, read them and create inputProperty objects
        readInputPropertiesForRemainingKeywords( inputPropertyCollection, eclipseCaseData, filename, &fileKeywordSet );
        i++;
    }

    // All input properties still unknown at this stage is missing a file
    setResolvedState( inputPropertyCollection, RimEclipseInputProperty::UNKNOWN, RimEclipseInputProperty::FILE_MISSING );
}

//--------------------------------------------------------------------------------------------------
/// Loads input property data from additional files.
//--------------------------------------------------------------------------------------------------
bool RifEclipseInputPropertyLoader::readInputPropertiesFromFiles( RimEclipseInputPropertyCollection* inputPropertyCollection,
                                                                  RigEclipseCaseData*                eclipseCaseData,
                                                                  bool                               importFaults,
                                                                  const std::vector<QString>&        filenames )
{
    for ( const QString& propertyFileName : filenames )
    {
        std::map<QString, QString> readProperties =
            RifEclipseInputFileTools::readProperties( propertyFileName, eclipseCaseData );

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

        // Avoid importing faults from the input property files when faults already exists in
        // the eclipse case. Faults can theoretically appear in any of the files, but reading
        // and appending them to the existing fault collection is not currently supported.
        if ( importFaults && eclipseCaseData->mainGrid()->faults().empty() )
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

//--------------------------------------------------------------------------------------------------
/// Extract keywords from a input property file.
//--------------------------------------------------------------------------------------------------
std::set<QString> RifEclipseInputPropertyLoader::extractKeywordsOnFile( const QString& filename, bool isExistingFile )
{
    std::set<QString> fileKeywordSet;
    if ( isExistingFile )
    {
        std::vector<RifKeywordAndFilePos> fileKeywords;
        RifEclipseInputFileTools::findKeywordsOnFile( filename, &fileKeywords );

        for ( const RifKeywordAndFilePos& fileKeyword : fileKeywords )
        {
            fileKeywordSet.insert( fileKeyword.keyword );
        }
    }
    return fileKeywordSet;
}

//--------------------------------------------------------------------------------------------------
/// Change the resolved state of all matching input properties in a collection.
//--------------------------------------------------------------------------------------------------
void RifEclipseInputPropertyLoader::setResolvedState( RimEclipseInputPropertyCollection*    inputPropertyCollection,
                                                      RimEclipseInputProperty::ResolveState currentState,
                                                      RimEclipseInputProperty::ResolveState newState )
{
    for ( RimEclipseInputProperty* inputProperty : inputPropertyCollection->inputProperties )
    {
        if ( inputProperty->resolvedState() == currentState )
        {
            inputProperty->resolvedState = newState;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseInputPropertyLoader::readDataForEachInputProperty( RimEclipseInputPropertyCollection* inputPropertyCollection,
                                                                  RigEclipseCaseData*                eclipseCaseData,
                                                                  const QString&                     filename,
                                                                  bool                               isExistingFile,
                                                                  std::set<QString>*                 fileKeywordSet,
                                                                  caf::ProgressInfo*                 progressInfo,
                                                                  int                                progressOffset )
{
    // Find the input property objects referring to the file
    std::vector<RimEclipseInputProperty*> ipsUsingThisFile = inputPropertyCollection->findInputProperties( filename );

    // Read property data for each inputProperty
    int progress = 0;
    for ( RimEclipseInputProperty* inputProperty : ipsUsingThisFile )
    {
        if ( !isExistingFile )
        {
            inputProperty->resolvedState = RimEclipseInputProperty::FILE_MISSING;
        }
        else
        {
            inputProperty->resolvedState = RimEclipseInputProperty::KEYWORD_NOT_IN_FILE;

            QString kw = inputProperty->eclipseKeyword();
            if ( fileKeywordSet->count( kw ) )
            {
                if ( RifEclipseInputFileTools::readProperty( filename, eclipseCaseData, kw, inputProperty->resultName ) )
                {
                    inputProperty->resolvedState = RimEclipseInputProperty::RESOLVED;
                }
            }
            fileKeywordSet->erase( kw );
        }

        progressInfo->setProgress( static_cast<int>( progressOffset + progress ) );
        progress++;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifEclipseInputPropertyLoader::readInputPropertiesForRemainingKeywords( RimEclipseInputPropertyCollection* inputPropertyCollection,
                                                                             RigEclipseCaseData* eclipseCaseData,
                                                                             const QString&      filename,
                                                                             std::set<QString>*  fileKeywordSet )
{
    for ( const QString& fileKeyword : *fileKeywordSet )
    {
        QString resultName = eclipseCaseData->results( RiaDefines::MATRIX_MODEL )->makeResultNameUnique( fileKeyword );
        if ( RifEclipseInputFileTools::readProperty( filename, eclipseCaseData, fileKeyword, resultName ) )
        {
            RimEclipseInputProperty* inputProperty = new RimEclipseInputProperty;
            inputProperty->resultName              = resultName;
            inputProperty->eclipseKeyword          = fileKeyword;
            inputProperty->fileName                = filename;
            inputProperty->resolvedState           = RimEclipseInputProperty::RESOLVED;
            inputPropertyCollection->inputProperties.push_back( inputProperty );
        }
    }
}
