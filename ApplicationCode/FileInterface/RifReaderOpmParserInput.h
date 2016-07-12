/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "RigFault.h"

#include "cvfCollection.h"

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <set>

#include <QString>

class RigCaseData;

namespace Opm {
    class Deck;
    class ParseContext;
}


//==================================================================================================
//
// 
//
//==================================================================================================
class RifReaderOpmParserInput
{
public:
    static void importGridPropertiesFaults(const QString& fileName, RigCaseData* eclipseCase);

    static void readFaults(const QString& fileName, cvf::Collection<RigFault>& faults);

/*
    static bool                 openGridFile(const QString& fileName, bool importProperties, RigCaseData* eclipseCase, RimEclipseInputCaseOpm* RimInputCase);
    static std::map<QString, QString> copyPropertiesToCaseData(const QString& fileName, RigCaseData* eclipseCase);
*/

private:
    static void importFaults(const Opm::Deck& deck, cvf::Collection<RigFault>& faults);
    static std::vector<std::string> knownPropertyKeywords();
    static std::vector<std::string> allParserConfigKeys();
    static size_t findFaultByName(const cvf::Collection<RigFault>& faults, const QString& name);
    static cvf::StructGridInterface::FaceEnum faceEnumFromText(const QString& faceString);
    static void initUsingWarnings(Opm::ParseContext* parseContext);
};



// This class is intended to be used for reading additional properties from standalone files
// Not yet in use
class RifReaderOpmParserPropertyReader
{
public:
    RifReaderOpmParserPropertyReader();
    virtual ~RifReaderOpmParserPropertyReader();

    bool open(const QString& fileName);
    std::set<QString> keywords() const;

    bool copyPropertyToCaseData(const QString& keywordName, RigCaseData* eclipseCase, const QString& resultName);

    static size_t               findOrCreateResult(const QString& newResultName, RigCaseData* reservoir);
    static void readAllProperties(std::shared_ptr< Opm::Deck > deck, RigCaseData* caseData, std::map<QString, QString>* newResults);
    static void getAllValuesForKeyword(std::shared_ptr< Opm::Deck > deck, const std::string& keyword, std::vector<double>& allValues);
    static bool isDataItemCountIdenticalToMainGridCellCount(std::shared_ptr< Opm::Deck > deck, const std::string& keyword, RigCaseData* caseData);
private:
    std::shared_ptr< Opm::Deck > m_deck;
};
