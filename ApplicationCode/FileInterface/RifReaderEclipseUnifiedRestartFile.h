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

#include "RifReaderEclipseResultAccess.h"

class RifReaderEclipseFileAccess;

#ifdef USE_ECL_LIB
#include "well_info.h"
#endif // USE_ECL_LIB

//==================================================================================================
//
// Class for access to results from a unified restart file
//
//==================================================================================================
class RifReaderEclipseUnifiedRestartFile : public RifReaderEclipseResultsAccess
{
public:
    RifReaderEclipseUnifiedRestartFile(size_t numGrids, size_t numActiveCells);
    virtual ~RifReaderEclipseUnifiedRestartFile();

    bool                        open(const QStringList& fileSet);
    void                        close();

    size_t                      numTimeSteps();
    QStringList                 timeStepsText();
    QList<QDateTime>            timeSteps();

    QStringList                 resultNames();
    bool                        results(const QString& resultName, size_t timeStep, std::vector<double>* values);

#ifdef USE_ECL_LIB
    virtual void                readWellData(well_info_type * well_info);
#endif

private:
    cvf::ref<RifReaderEclipseFileAccess> m_file;
};
