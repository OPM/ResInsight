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
class QStringList;
class QString;

//==================================================================================================
//
// 
//
//==================================================================================================
class RifEclipseSummaryTools
{
public:
    static void                     findSummaryHeaderFile(const QString& inputFile, QString* headerFile, bool* isFormatted);
    static QStringList              findSummaryDataFiles(const QString& caseFile);
    static QString                  findGridCaseFileFromSummaryHeaderFile(const QString& summaryHeaderFile);

    static void                     findSummaryFiles(const QString& inputFile, QString* headerFile, QStringList* dataFiles);
    static bool                     hasSummaryFiles(const QString& gridFileName);
    static void                     dumpMetaData(RifSummaryReaderInterface* readerEclipseSummary);

private:
    static void                     findSummaryHeaderFileInfo(const QString& inputFile, QString* headerFile, QString* path, QString* base, bool* isFormatted);
};
