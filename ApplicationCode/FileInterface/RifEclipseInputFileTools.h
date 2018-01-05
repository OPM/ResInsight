/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfCollection.h"

#include "RifReaderInterface.h"
#include "RigFault.h"

#include "ert/ecl/ecl_kw.h"

#include <map>

#include <QString>


class RigEclipseCaseData;
class QFile;
class RimEclipseResultDefinition;


//--------------------------------------------------------------------------------------------------
/// Structure used to cache file position of keywords
//--------------------------------------------------------------------------------------------------
struct RifKeywordAndFilePos
{
    QString keyword;
    qint64  filePos;
};


//==================================================================================================
//
// Class for access to Eclipse "keyword" files using libecl
//
//==================================================================================================
class RifEclipseInputFileTools : public cvf::Object
{
public:
    RifEclipseInputFileTools();
    virtual ~RifEclipseInputFileTools();

    static bool openGridFile(const QString& fileName, RigEclipseCaseData* eclipseCase, bool readFaultData);
    
    // Returns map of assigned resultName and Eclipse Keyword.
    static std::map<QString, QString> readProperties(const QString& fileName, RigEclipseCaseData* eclipseCase);
    static bool                       readProperty  (const QString& fileName, RigEclipseCaseData* eclipseCase, const QString& eclipseKeyWord, const QString& resultName );

    static void                       readFaultsInGridSection(const QString& fileName, cvf::Collection<RigFault>* faults, std::vector<QString>* filenamesWithFaults, const QString& faultIncludeFileAbsolutePathPrefix);
    static void                       readFaults(const QString& fileName, const std::vector< RifKeywordAndFilePos >& fileKeywords, cvf::Collection<RigFault>* faults);
    static void                       parseAndReadFaults(const QString& fileName, cvf::Collection<RigFault>* faults);

    static void                       readFaults(QFile &data, qint64 filePos, cvf::Collection<RigFault>* faults, bool* isEditKeywordDetected);
    static void                       findKeywordsOnFile(const QString &fileName, std::vector< RifKeywordAndFilePos >* keywords);

    static void                       parseAndReadPathAliasKeyword(const QString &fileName, std::vector< std::pair<QString, QString> >* pathAliasDefinitions);


    static bool     writePropertyToTextFile(const QString& fileName, RigEclipseCaseData* eclipseCase, size_t timeStep, const QString& resultName, const QString& eclipseKeyWord);
    static bool     writeBinaryResultToTextFile(const QString& fileName, RigEclipseCaseData* eclipseCase, size_t timeStep, RimEclipseResultDefinition* resultdefinition, const QString& eclipseKeyWord, const double undefinedValue);

    static bool     readFaultsAndParseIncludeStatementsRecursively( QFile& file, 
                                                                    qint64 startPos, 
                                                                    const std::vector< std::pair<QString, QString> >& pathAliasDefinitions,
                                                                    cvf::Collection<RigFault>* faults, 
                                                                    std::vector<QString>* filenamesWithFaults, 
                                                                    bool* isEditKeywordDetected,
                                                                    const QString& faultIncludeFileAbsolutePathPrefix);

    static cvf::StructGridInterface::FaceEnum faceEnumFromText(const QString& faceString);
    static void     writeDataToTextFile(QFile* file, const QString& eclipseKeyWord, const std::vector<double>& resultData);

private:
    static bool     readDataFromKeyword(ecl_kw_type* eclipseKeywordData, RigEclipseCaseData* caseData, const QString& resultName);
    static void     findGridKeywordPositions(const std::vector< RifKeywordAndFilePos >& keywords, qint64* coordPos, qint64* zcornPos, qint64* specgridPos, qint64* actnumPos, qint64* mapaxesPos);

    static size_t   findFaultByName(const cvf::Collection<RigFault>& faults, const QString& name);

    static qint64   findKeyword(const QString& keyword, QFile& file, qint64 startPos);
    static size_t   findOrCreateResult(const QString& newResultName, RigEclipseCaseData* reservoir);
    static bool     isValidDataKeyword(const QString& keyword);
    
private:
    static const std::vector<QString>& invalidPropertyDataKeywords(); 
};
