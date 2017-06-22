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

#include "RifReaderInterface.h"
#include "RifEclipseRestartDataAccess.h"

#include <QString>
#include <QStringList>
#include <QDateTime>

#include "ert/ecl/ecl_util.h"

typedef struct ecl_file_struct ecl_file_type;


//==================================================================================================
//
// 
//
//==================================================================================================
class RifEclipseOutputFileTools
{
public:
    RifEclipseOutputFileTools();
    virtual ~RifEclipseOutputFileTools();

    static void         findKeywordsAndItemCount(std::vector<ecl_file_type*> ecl_files, QStringList* resultNames, std::vector<size_t>* resultDataItemCounts);

    static bool         keywordData(ecl_file_type* ecl_file, const QString& keyword, size_t fileKeywordOccurrence, std::vector<double>* values);
    static bool         keywordData(ecl_file_type* ecl_file, const QString& keyword, size_t fileKeywordOccurrence, std::vector<int>* values);

    static void         timeSteps(ecl_file_type* ecl_file, std::vector<QDateTime>* timeSteps, std::vector<double>* daysSinceSimulationStart);

    static bool         findSiblingFilesWithSameBaseName(const QString& fileName, QStringList* fileSet);

    static QString      firstFileNameOfType(const QStringList& fileSet, ecl_file_enum fileType);
    static QStringList  filterFileNamesOfType(const QStringList& fileSet, ecl_file_enum fileType);

    static void         readGridDimensions(const QString& gridFileName, std::vector< std::vector<int> >& gridDimensions);

    static int          readUnitsType(ecl_file_type* ecl_file);

private:
    static void         createReportStepsMetaData(std::vector<ecl_file_type*> ecl_files, std::vector<RifRestartReportStep>* reportSteps);
};
