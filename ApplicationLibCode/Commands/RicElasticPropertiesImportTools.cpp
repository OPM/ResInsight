/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RicElasticPropertiesImportTools.h"

#include "RiaLogging.h"

#include "RifCsvUserDataParser.h"
#include "RifElasticPropertiesReader.h"
#include "RifFileParseTools.h"

#include "RigElasticProperties.h"

#include "RigFormationNames.h"
#include "RimElasticProperties.h"
#include "RimFormationNames.h"
#include "RimFormationNamesCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimStimPlanModelTemplate.h"

#include <set>
#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicElasticPropertiesImportTools::importElasticPropertiesFromFile( const QString&            filePath,
                                                                       RimStimPlanModelTemplate* stimPlanModelTemplate,
                                                                       const QString&            formationWildCard )
{
    RifCsvUserDataFileParser csvParser( filePath );
    QString                  separator = csvParser.tryDetermineCellSeparator();

    typedef std::tuple<QString, QString, QString> FaciesKey;

    // Read the facies properties from file
    std::vector<RifElasticProperties> rifElasticProperties;
    try
    {
        QStringList filePaths;
        filePaths << filePath;
        RifElasticPropertiesReader::readElasticProperties( rifElasticProperties, filePaths, separator );
    }
    catch ( FileParseException& exception )
    {
        RiaLogging::warning( QString( "Facies properties import failed: '%1'." ).arg( exception.message ) );
        return;
    }

    std::vector<QString> formationNames = getFormationNames();

    // Find the unique facies keys (combination of field, formation and facies names)
    std::set<FaciesKey> faciesKeys;
    for ( RifElasticProperties item : rifElasticProperties )
    {
        if ( item.formationName == formationWildCard )
        {
            // Found wildcard: generate keys for all formations
            for ( QString formationName : formationNames )
            {
                FaciesKey faciesKey = std::make_tuple( item.fieldName, formationName, item.faciesName );
                faciesKeys.insert( faciesKey );
            }
        }
        else
        {
            FaciesKey faciesKey = std::make_tuple( item.fieldName, item.formationName, item.faciesName );
            faciesKeys.insert( faciesKey );
        }
    }

    RimElasticProperties* rimElasticProperties = stimPlanModelTemplate->elasticProperties();
    if ( !rimElasticProperties ) rimElasticProperties = new RimElasticProperties;

    // Clear the properties to avoid keeping data which has been deleted
    // from the file since the last import.
    rimElasticProperties->clearProperties();

    for ( FaciesKey key : faciesKeys )
    {
        std::vector<RifElasticProperties> matchingFacies;

        QString fieldName     = std::get<0>( key );
        QString formationName = std::get<1>( key );
        QString faciesName    = std::get<2>( key );

        // Group the items with a given facies key
        for ( RifElasticProperties item : rifElasticProperties )
        {
            if ( item.fieldName == fieldName &&
                 ( item.formationName == formationName || item.formationName == formationWildCard ) &&
                 item.faciesName == faciesName )
            {
                matchingFacies.push_back( item );
            }
        }

        // Sort the matching items by porosity
        std::sort( matchingFacies.begin(),
                   matchingFacies.end(),
                   []( const RifElasticProperties& a, const RifElasticProperties& b ) { return a.porosity < b.porosity; } );

        std::vector<QString> matchingFormations;
        if ( formationName == formationWildCard )
        {
            // Duplicate values for all formations when encountering a wild card
            matchingFormations = formationNames;
        }
        else
        {
            matchingFormations.push_back( formationName );
        }

        for ( QString matchingFormationName : matchingFormations )
        {
            // Finally add the values
            RigElasticProperties rigElasticProperties( fieldName, matchingFormationName, faciesName );
            for ( RifElasticProperties item : matchingFacies )
            {
                rigElasticProperties.appendValues( item.porosity,
                                                   item.youngsModulus,
                                                   item.poissonsRatio,
                                                   item.K_Ic,
                                                   item.proppantEmbedment,
                                                   item.biotCoefficient,
                                                   item.k0,
                                                   item.fluidLossCoefficient,
                                                   item.spurtLoss,
                                                   item.immobileFluidSaturation );
            }

            // Avoid using the field name in the match for now
            FaciesKey noFieldKey = std::make_tuple( "", matchingFormationName, faciesName );
            rimElasticProperties->setPropertiesForFacies( noFieldKey, rigElasticProperties );
        }
    }

    rimElasticProperties->setFilePath( filePath );
    stimPlanModelTemplate->setElasticProperties( rimElasticProperties );
    stimPlanModelTemplate->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// Finds first formation names.
//--------------------------------------------------------------------------------------------------
std::vector<QString> RicElasticPropertiesImportTools::getFormationNames()
{
    RimProject*  project  = RimProject::current();
    RimOilField* oilField = project->activeOilField();

    RimFormationNamesCollection* formationNamesCollection = oilField->formationNamesCollection();
    for ( RimFormationNames* formationNames : formationNamesCollection->formationNamesList().childObjects() )
    {
        if ( formationNames && formationNames->formationNamesData() )
        {
            RigFormationNames* rigFormationNames = formationNames->formationNamesData();
            if ( rigFormationNames ) return rigFormationNames->formationNames();
        }
    }

    return std::vector<QString>();
}
