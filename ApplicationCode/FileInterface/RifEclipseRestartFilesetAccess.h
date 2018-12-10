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

#include <vector>

class RifEclipseOutputFileTools;

//==================================================================================================
//
// Class for access to results from a set of restart files
//
//==================================================================================================
class RifEclipseRestartFilesetAccess : public RifEclipseRestartDataAccess
{
public:
    RifEclipseRestartFilesetAccess();
    ~RifEclipseRestartFilesetAccess() override;

    bool                        open() override;
    void                        setRestartFiles(const QStringList& fileSet) override;
    void                        close() override;

    void                        setTimeSteps(const std::vector<QDateTime>& timeSteps) override;
    size_t                      timeStepCount() override;
    void                        timeSteps(std::vector<QDateTime>* timeSteps, std::vector<double>* daysSinceSimulationStart) override;
    std::vector<int>            reportNumbers() override;

    void                        resultNames(QStringList* resultNames, std::vector<size_t>* resultDataItemCounts) override;
    bool                        results(const QString& resultName, size_t timeStep, size_t gridCount, std::vector<double>* values) override;

    bool                        dynamicNNCResults(const ecl_grid_type* grid, size_t timeStep, std::vector<double>* waterFlux, std::vector<double>* oilFlux, std::vector<double>* gasFlux) override;

    void                readWellData(well_info_type* well_info, bool importCompleteMswData) override;
    int                 readUnitsType() override;

    std::set<RiaDefines::PhaseType> availablePhases() const override;

private:
    void                        openTimeStep(size_t timeStep);
    static int                  reportNumber(const ecl_file_type* ecl_file);

private:
    QStringList                     m_fileNames;
    std::vector<QDateTime>          m_timeSteps;
    std::vector<double>             m_daysSinceSimulationStart;

    std::vector< ecl_file_type* >   m_ecl_files;
    std::set<RiaDefines::PhaseType> m_availablePhases;
};
