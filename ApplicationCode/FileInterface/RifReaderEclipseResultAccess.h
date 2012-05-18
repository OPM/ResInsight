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

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfLibCore.h"

#include <QStringList>
#include <QDateTime>

#include <vector>

#ifdef USE_ECL_LIB
#include "well_info.h"
#endif // USE_ECL_LIB

//==================================================================================================
//
// Abstract class for results access
//
//==================================================================================================
class RifReaderEclipseResultsAccess : public cvf::Object
{
public:
    RifReaderEclipseResultsAccess(size_t numGrids, size_t numActiveCells);
    virtual ~RifReaderEclipseResultsAccess();

    virtual bool                open(const QStringList& fileSet) = 0;
    virtual void                close() = 0;

    virtual size_t              numTimeSteps() = 0;
    virtual QStringList         timeStepsText() = 0;
    virtual QList<QDateTime>    timeSteps() = 0;

    virtual QStringList         resultNames() = 0;
    virtual bool                results(const QString& resultName, size_t timeStep, std::vector<double>* values) = 0;


#ifdef USE_ECL_LIB
    virtual void                readWellData(well_info_type * well_info) = 0;
#endif

protected:
    size_t                      m_numGrids;
    size_t                      m_numActiveCells;
};
