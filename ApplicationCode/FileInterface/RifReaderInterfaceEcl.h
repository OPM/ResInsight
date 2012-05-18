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

class RifReaderEclipseFileAccess;
class RifReaderEclipseResultsAccess;

//==================================================================================================
//
// File interface for Eclipse output files
//
//==================================================================================================
class RifReaderInterfaceECL : public RifReaderInterface
{
public:
    RifReaderInterfaceECL();
    virtual ~RifReaderInterfaceECL();

    bool                open(const QString& fileName, RigReservoir* reservoir);
    void                close();

    const QStringList&  staticResults() const;
    const QStringList&  dynamicResults() const;
    
    size_t                  numTimeSteps() const;
    const QStringList&      timeStepText() const;
    const QList<QDateTime>& timeSteps() const;

    bool                staticResult(const QString& result, std::vector<double>* values);
    bool                dynamicResult(const QString& result, size_t stepIndex, std::vector<double>* values);

private:
    void                ground();
    bool                buildMetaData(RigReservoir* reservoir);
    bool                readGeometry(const QString& fileName, RigReservoir* reservoir);
    void                readWellCells(RigReservoir* reservoir);

    static RifReaderEclipseResultsAccess*   staticResultsAccess(const QStringList& fileSet, size_t numGrids, size_t numActiveCells);
    static RifReaderEclipseResultsAccess*   dynamicResultsAccess(const QStringList& fileSet, size_t numGrids, size_t numActiveCells);

private:
    QString             m_fileName;         // Name of file used to start accessing Eclipse output files
    QStringList         m_fileSet;          // Set of files in filename's path with same base name as filename
    QStringList         m_staticResults;    // List of names of static results
    QStringList         m_dynamicResults;   // List of names of dynamic results
    QStringList         m_timeStepTexts;    // List of time step infos (dates)
    QList<QDateTime>    m_timeSteps;
    size_t              m_numGrids;         // Total number of grids (main grid and all sub grids)

    cvf::ref<RifReaderEclipseFileAccess>    m_staticResultsAccess;    // File access to static results
    cvf::ref<RifReaderEclipseResultsAccess> m_dynamicResultsAccess;   // File access to dynamic results
};
