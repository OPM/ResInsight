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

        auto resultNamesEclipseKeywords = RifEclipseInputFileTools::readProperties( filename, eclipseCaseData );

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
