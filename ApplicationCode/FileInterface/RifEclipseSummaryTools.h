/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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

#include "RifEclipseSummaryAddress.h"

#include <string>
#include <vector>

class RifSummaryReaderInterface;

//==================================================================================================
//
// 
//
//==================================================================================================
class RifEclipseSummaryTools
{
public:
    static void                     findSummaryHeaderFile(const std::string& inputFile, std::string* headerFile, bool* isFormatted);
    static std::vector<std::string> findSummaryDataFiles(const std::string& caseFile);

    static void                     findSummaryFiles(const std::string& inputFile, std::string* headerFile, std::vector<std::string>* dataFiles);
    static bool                     hasSummaryFiles(const std::string& gridFileName);
    static void                     dumpMetaData(RifSummaryReaderInterface* readerEclipseSummary);

private:
    static void                     findSummaryHeaderFileInfo(const std::string& inputFile, std::string* headerFile, std::string* path, std::string* base, bool* isFormatted);
};
