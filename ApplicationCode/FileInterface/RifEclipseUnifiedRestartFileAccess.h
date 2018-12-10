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

#include "RifEclipseRestartDataAccess.h"

class RifEclipseOutputFileTools;

//typedef struct ecl_file_struct ecl_file_type;

#include "ert/ecl_well/well_info.hpp"



//==================================================================================================
//
// Class for access to results from a unified restart file
//
//==================================================================================================
class RifEclipseUnifiedRestartFileAccess : public RifEclipseRestartDataAccess
{
public:
    RifEclipseUnifiedRestartFileAccess();
    ~RifEclipseUnifiedRestartFileAccess() override;

    void                        setRestartFiles(const QStringList& fileSet) override;
    bool                        open() override;
    void                        close() override;

    size_t                      timeStepCount() override;
    void                        timeSteps(std::vector<QDateTime>* timeSteps, std::vector<double>* daysSinceSimulationStart) override;
    std::vector<int>            reportNumbers() override;

    void                        resultNames(QStringList* resultNames, std::vector<size_t>* resultDataItemCounts) override;
    bool                        results(const QString& resultName, size_t timeStep, size_t gridCount, std::vector<double>* values) override;

    bool                        dynamicNNCResults(const ecl_grid_type* grid, size_t timeStep, std::vector<double>* waterFlux, std::vector<double>* oilFlux, std::vector<double>* gasFlux) override;

    void                readWellData(well_info_type * well_info, bool importCompleteMswData) override;
    int                 readUnitsType() override;

    std::set<RiaDefines::PhaseType> availablePhases() const override;

private:
    bool                        openFile();
    bool                        useResultIndexFile() const;
    void                        extractTimestepsFromEclipse();

private:
    QString                     m_filename;
    ecl_file_type*              m_ecl_file;

    std::vector<QDateTime>      m_timeSteps;
    std::vector<double>         m_daysSinceSimulationStart;
    std::vector<int>            m_reportNr;

    std::set<RiaDefines::PhaseType>  m_availablePhases;
};
