/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#pragma once

#include "RimEclipseInputProperty.h"

#include <set>
#include <vector>

#include <QString>

class RimEclipseInputPropertyCollection;
class RigEclipseCaseData;
namespace caf
{
class ProgressInfo;
}

//==================================================================================================
//
//
//
//==================================================================================================
class RifEclipseInputPropertyLoader
{
public:
    static void loadAndSyncronizeInputProperties( RimEclipseInputPropertyCollection* inputPropertyCollection,
                                                  RigEclipseCaseData*                eclipseCaseData,
                                                  const std::vector<QString>&        filenames,
                                                  bool                               allowImportOfFaults );

    static bool readInputPropertiesFromFiles( RimEclipseInputPropertyCollection* inputPropertyCollection,
                                              RigEclipseCaseData*                eclipseCaseData,
                                              bool                               importFaults,
                                              const std::vector<QString>&        filenames );

    static bool importFaultsFromFile( RigEclipseCaseData* eclipseCase, const QString& fileName );

private:
    // Hide constructor to prevent instantiation
    RifEclipseInputPropertyLoader();

    static std::set<QString> extractKeywordsOnFile( const QString& filename, bool isExistingFile );

    static void setResolvedState( RimEclipseInputPropertyCollection*    inputPropertyCollection,
                                  RimEclipseInputProperty::ResolveState currentState,
                                  RimEclipseInputProperty::ResolveState newState );

    static void readDataForEachInputProperty( RimEclipseInputPropertyCollection* inputPropertyCollection,
                                              RigEclipseCaseData*                eclipseCaseData,
                                              const QString&                     filename,
                                              bool                               isExistingFile,
                                              bool                               allowImportOfFaults,
                                              std::set<QString>*                 fileKeywordSet,
                                              caf::ProgressInfo*                 progressInfo,
                                              int                                progressOffset );

    static void readInputPropertiesForRemainingKeywords( RimEclipseInputPropertyCollection* inputPropertyCollection,
                                                         RigEclipseCaseData*                eclipseCaseData,
                                                         const QString&                     filename,
                                                         std::set<QString>*                 fileKeywordSet );
};
