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
                                                                      const std::vector<QString>& filenames,
                                                                      bool                        allowImportOfFaults )
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
                                      allowImportOfFaults,
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

        if ( importFaults )
        {
            bool anyFaultsImported = importFaultsFromFile( eclipseCaseData, propertyFileName );
            if ( anyFaultsImported )
            {
                RimEclipseInputProperty* inputProperty = new RimEclipseInputProperty;
                inputProperty->resultName              = "FAULTS";
                inputProperty->eclipseKeyword          = "FAULTS";
                inputProperty->fileName                = propertyFileName;
                inputProperty->resolvedState           = RimEclipseInputProperty::RESOLVED;
                inputPropertyCollection->inputProperties.push_back( inputProperty );
            }
        }
    }

    return true;
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
            if ( f->name() == RiaDefines::undefinedGridFaultName() || f->name() == RiaDefines::undefinedGridFaultName() )
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
                                                                  bool               allowImportOfFaults,
                                                                  std::set<QString>* fileKeywordSet,
                                                                  caf::ProgressInfo* progressInfo,
                                                                  int                progressOffset )
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
            if ( kw == "FAULTS" )
            {
                if ( allowImportOfFaults )
                {
                    importFaultsFromFile( eclipseCaseData, filename );
                }
            }
            else
            {
                if ( fileKeywordSet->count( kw ) )
                {
                    if ( RifEclipseInputFileTools::readProperty( filename, eclipseCaseData, kw, inputProperty->resultName ) )
                    {
                        inputProperty->resolvedState = RimEclipseInputProperty::RESOLVED;
                    }
                }
                fileKeywordSet->erase( kw );
            }
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
        QString resultName =
            eclipseCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->makeResultNameUnique( fileKeyword );
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
