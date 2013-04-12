/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RifReaderInterface.h"
#include <QList>
#include <QDateTime>

class RifEclipseOutputFileTools;
class RifEclipseRestartDataAccess;
class RigGridBase;
class RigMainGrid;
class RigActiveCellInfo;

typedef struct ecl_grid_struct ecl_grid_type;
typedef struct ecl_file_struct ecl_file_type;


//==================================================================================================
//
// File interface for Eclipse output files
//
//==================================================================================================
class RifReaderEclipseOutput : public RifReaderInterface
{
public:
    RifReaderEclipseOutput();
    virtual ~RifReaderEclipseOutput();

    bool                    open(const QString& fileName, RigCaseData* eclipseCase);
    virtual bool            openAndReadActiveCellData(const QString& fileName, const std::vector<QDateTime>& mainCaseTimeSteps, RigCaseData* eclipseCase);
    void                    close();

    bool                    staticResult(const QString& result, PorosityModelResultType matrixOrFracture, std::vector<double>* values);
    bool                    dynamicResult(const QString& result, PorosityModelResultType matrixOrFracture, size_t stepIndex, std::vector<double>* values);

    static bool             transferGeometry(const ecl_grid_type* mainEclGrid, RigCaseData* eclipseCase);

private:
    bool                    readActiveCellInfo();
    bool                    buildMetaData();
    void                    readWellCells();
    
    bool                    openInitFile();
    bool                    openDynamicAccess();

    void                    extractResultValuesBasedOnPorosityModel(PorosityModelResultType matrixOrFracture, std::vector<double>* values, const std::vector<double>& fileValues);
    
    RifEclipseRestartDataAccess*   createDynamicResultsAccess();

    QStringList             validKeywordsForPorosityModel(const QStringList& keywords, const std::vector<size_t>& keywordDataItemCounts, const RigActiveCellInfo* activeCellInfo, const RigActiveCellInfo* fractureActiveCellInfo, PorosityModelResultType matrixOrFracture, size_t timeStepCount) const;

    virtual std::vector<QDateTime> timeSteps();
private:
    QString                                 m_fileName;         // Name of file used to start accessing Eclipse output files
    QStringList                             m_filesWithSameBaseName;          // Set of files in filename's path with same base name as filename

    RigCaseData*                            m_eclipseCase;

    std::vector<QDateTime>                  m_timeSteps;

    ecl_file_type*                          m_ecl_init_file;    // File access to static results
    cvf::ref<RifEclipseRestartDataAccess>   m_dynamicResultsAccess;   // File access to dynamic results
};
