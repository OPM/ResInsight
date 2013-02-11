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
    virtual ~RifEclipseRestartFilesetAccess();

    bool                        open(const QStringList& fileSet);
    void                        close();

    size_t                      timeStepCount();
    QList<QDateTime>            timeSteps();

    void                        resultNames(QStringList* resultNames, std::vector<size_t>* resultDataItemCounts);
    bool                        results(const QString& resultName, size_t timeStep, size_t gridCount, std::vector<double>* values);

    virtual void                readWellData(well_info_type* well_info);

private:
    std::vector< ecl_file_type* > m_ecl_files;
};
