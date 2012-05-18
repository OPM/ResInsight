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

#include "RimDefines.h"
#include <QDateTime>

struct RigWellResultCell
{
    RigWellResultCell() : 
        m_gridIndex(cvf::UNDEFINED_SIZE_T), 
        m_gridCellIndex(cvf::UNDEFINED_SIZE_T), 
        m_isOpen(false), 
        m_isTemporarilyOut(false) 
    { }

    size_t m_gridIndex;
    size_t m_gridCellIndex;     //< Index to cell which is included in the well
    bool   m_isOpen;            //< Marks the well as open or closed as of Eclipse simulation
    bool   m_isTemporarilyOut;  //< Marks the cell as not member of the well at the current time step, (but it is a member at some other time step)
};

struct RigWellResultBranch
{
    RigWellResultBranch() :
        m_branchNumber(cvf::UNDEFINED_SIZE_T)
    {}

    size_t                         m_branchNumber;
    std::vector<RigWellResultCell> m_wellCells;
};

class RigWellResultFrame
{
public:
    enum WellProductionType { PRODUCER, OIL_INJECTOR, GAS_INJECTOR, WATER_INJECTOR, UNDEFINED_PRODUCTION_TYPE };

public:
    RigWellResultFrame() :
        m_isOpen(false),
        m_productionType(UNDEFINED_PRODUCTION_TYPE)
    { }

    WellProductionType  m_productionType;
    bool                m_isOpen;
    RigWellResultCell   m_wellHead;
    QDateTime           m_timestamp;
    
    std::vector<RigWellResultBranch> m_wellResultBranches;
};


//==================================================================================================
/// 
//==================================================================================================
class RigWellResults : public cvf::Object
{
public:
    bool hasWellResult(size_t resultTimeStepIndex) const;
    size_t firstResultTimeStep() const;

    const RigWellResultFrame& wellResultFrame(size_t resultTimeStepIndex) const;

    void computeStaticWellCellPath();

    void computeMappingFromResultTimeIndicesToWellTimeIndices(const QList<QDateTime>& resultTimes);

public:
    QString                             m_wellName;

    std::vector<size_t>                 m_resultTimeStepIndexToWellTimeStepIndex;   // Well result timesteps may differ from result timesteps
    std::vector< RigWellResultFrame >   m_wellCellsTimeSteps;
    RigWellResultFrame                  m_staticWellCells;


};

