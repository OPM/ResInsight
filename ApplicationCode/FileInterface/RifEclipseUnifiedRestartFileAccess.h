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

class RifEclipseOutputFileTools;

#include "well_info.h"

//==================================================================================================
//
// Class for access to results from a unified restart file
//
//==================================================================================================
class RifEclipseUnifiedRestartFileAccess : public RifEclipseRestartDataAccess
{
public:
    RifEclipseUnifiedRestartFileAccess();
    virtual ~RifEclipseUnifiedRestartFileAccess();

    bool                        open(const QStringList& fileSet, const std::vector<size_t>& matrixModelActiveCellCounts, const std::vector<size_t>& fractureModelActiveCellCounts);
    void                        close();

    size_t                      numTimeSteps();
    QStringList                 timeStepsText();
    QList<QDateTime>            timeSteps();

    QStringList                 resultNames();
    bool                        results(const QString& resultName, RifReaderInterface::PorosityModelResultType matrixOrFracture, size_t timeStep, std::vector<double>* values);

    virtual void                readWellData(well_info_type * well_info);

private:
    cvf::ref<RifEclipseOutputFileTools> m_file;

    size_t m_gridCount;
};
