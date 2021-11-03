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

#include <QString>

#include <map>
#include <set>
#include <vector>

class RimEclipseInputPropertyCollection;
class RigEclipseCaseData;
class RifEclipseKeywordContent;

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

    static void createInputPropertiesFromKeywords( RigEclipseCaseData*                          eclipseCase,
                                                   const std::vector<RifEclipseKeywordContent>& keywordContent );

private:
    // Hide constructor to prevent instantiation
    RifEclipseInputPropertyLoader();

    // Returns map of assigned resultName and Eclipse Keyword.
    static std::map<QString, QString> readProperties( const QString& fileName, RigEclipseCaseData* eclipseCase );

    static const std::vector<QString>& invalidPropertyDataKeywords();
    static bool                        isValidDataKeyword( const QString& keyword );

    static bool isInputPropertyCandidate( const RigEclipseCaseData* caseData,
                                          const std::string&        eclipseKeyword,
                                          size_t                    numberOfValues );

    static bool appendNewInputPropertyResult( RigEclipseCaseData*       caseData,
                                              const QString&            resultName,
                                              const std::string&        eclipseKeyword,
                                              const std::vector<float>& values,
                                              QString*                  errMsg );

    static QString evaluateAndCreateInputPropertyResult( RigEclipseCaseData*             eclipseCase,
                                                         const RifEclipseKeywordContent& keywordContent,
                                                         QString*                        errorMessage );
};
