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

#include "RifReaderInterface.h"

#include "cvfCollection.h"

#include <memory>

class RifEclipseOutputFileTools;
class RifEclipseRestartDataAccess;
class RifHdf5ReaderInterface;
class RigActiveCellInfo;
class RigFault;
class RigEclipseTimeStepInfo;
class RigGridBase;
class RigMainGrid;
class QDateTime;

struct RigWellResultPoint;

typedef struct ecl_grid_struct ecl_grid_type;
typedef struct ecl_file_struct ecl_file_type;
typedef struct well_conn_struct well_conn_type;

//==================================================================================================
//
// File interface for Eclipse output files
//
//==================================================================================================
class RifReaderEclipseOutput : public RifReaderInterface
{
public:
    RifReaderEclipseOutput();
    ~RifReaderEclipseOutput() override;

    bool                    open(const QString& fileName, RigEclipseCaseData* eclipseCase) override;
    void                    setHdf5FileName(const QString& fileName);
    void                    setFileDataAccess(RifEclipseRestartDataAccess* restartDataAccess);

    virtual bool            openAndReadActiveCellData(const QString& fileName, const std::vector<QDateTime>& mainCaseTimeSteps, RigEclipseCaseData* eclipseCase);

    bool                    staticResult(const QString& result, RiaDefines::PorosityModelType matrixOrFracture, std::vector<double>* values) override;
    bool                    dynamicResult(const QString& result, RiaDefines::PorosityModelType matrixOrFracture, size_t stepIndex, std::vector<double>* values) override;
    void                    sourSimRlResult(const QString& result, size_t stepIndex, std::vector<double>* values);

    std::vector<QDateTime>  allTimeSteps() const;

    static bool             transferGeometry(const ecl_grid_type* mainEclGrid, RigEclipseCaseData* eclipseCase);
    static void             transferCoarseningInfo(const ecl_grid_type* eclGrid, RigGridBase* grid);

    std::set<RiaDefines::PhaseType> availablePhases() const override;

private:
    bool                    readActiveCellInfo();
    void                    buildMetaData(ecl_grid_type* grid);
    void                    readWellCells(const ecl_grid_type* mainEclGrid, bool importCompleteMswData);

    std::string             ertGridName( size_t gridNr );

    RigWellResultPoint      createWellResultPoint(const RigGridBase* grid, const well_conn_type* ert_connection, int ertBranchId, int ertSegmentId, const char* wellName);
    
    void                    importFaults(const QStringList& fileSet, cvf::Collection<RigFault>* faults);

    void                    openInitFile();

    void                    extractResultValuesBasedOnPorosityModel(RiaDefines::PorosityModelType matrixOrFracture, std::vector<double>* values, const std::vector<double>& fileValues);
    void                    transferStaticNNCData(const ecl_grid_type* mainEclGrid , ecl_file_type* init_file, RigMainGrid* mainGrid);
    void                    transferDynamicNNCData(const ecl_grid_type* mainEclGrid, RigMainGrid* mainGrid);
    
    void                    ensureDynamicResultAccessIsPresent();

    QStringList             validKeywordsForPorosityModel(const QStringList& keywords, const std::vector<size_t>& keywordDataItemCounts, const RigActiveCellInfo* activeCellInfo, const RigActiveCellInfo* fractureActiveCellInfo, RiaDefines::PorosityModelType matrixOrFracture, size_t timeStepCount) const;
    
    std::vector<RigEclipseTimeStepInfo> createFilteredTimeStepInfos();

    static bool             isEclipseAndSoursimTimeStepsEqual(const QDateTime& eclipseDateTime, const QDateTime& sourSimDateTime);

    ecl_grid_type*          createMainGrid() const;

private:
    QString                                 m_fileName;                 // Name of file used to start accessing Eclipse output files
    QStringList                             m_filesWithSameBaseName;    // Set of files in filename's path with same base name as filename

    RigEclipseCaseData*                     m_eclipseCase;

    ecl_file_type*                          m_ecl_init_file;            // File access to static results
    mutable cvf::ref<RifEclipseRestartDataAccess>   m_dynamicResultsAccess;     // File access to dynamic results

    std::unique_ptr<RifHdf5ReaderInterface> m_hdfReaderInterface;
};
