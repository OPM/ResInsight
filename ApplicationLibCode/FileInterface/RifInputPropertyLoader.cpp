/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022-     Equinor ASA
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

#include "RifInputPropertyLoader.h"

#include "RifEclipseInputFileTools.h"
#include "RifEclipseInputPropertyLoader.h"
#include "RifRoffFileTools.h"

#include "RigEclipseCaseData.h"

#include "RimEclipseInputProperty.h"
#include "RimEclipseInputPropertyCollection.h"

#include "cafProgressInfo.h"

#include <QFile>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifInputPropertyLoader::loadAndSynchronizeInputProperties( RimEclipseInputPropertyCollection* inputPropertyCollection,
                                                                RigEclipseCaseData*                eclipseCaseData,
                                                                const std::vector<QString>&        filenames,
                                                                bool                               allowImportOfFaults )
{
    if ( !inputPropertyCollection || !eclipseCaseData || filenames.empty() ) return;

    std::vector<RimEclipseInputProperty*> existingProperties = inputPropertyCollection->inputProperties.childrenByType();

    caf::ProgressInfo progInfo( static_cast<int>( filenames.size() ), "Reading Input properties" );

    for ( const auto& filename : filenames )
    {
        progInfo.setProgressDescription( filename );

        auto resultNamesEclipseKeywords = RifInputPropertyLoader::readProperties( filename, eclipseCaseData );

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
std::map<QString, QString> RifInputPropertyLoader::readProperties( const QString& fileName, RigEclipseCaseData* eclipseCaseData )
{
    RiaDefines::ImportFileType fileType = RiaDefines::obtainFileTypeFromFileName( fileName );
    if ( fileType == RiaDefines::ImportFileType::ROFF_FILE )
    {
        auto [isOk, keywordMapping] = RifRoffFileTools::createInputProperties( fileName, eclipseCaseData );
        return keywordMapping;
    }

    return RifEclipseInputPropertyLoader::readProperties( fileName, eclipseCaseData );
}
