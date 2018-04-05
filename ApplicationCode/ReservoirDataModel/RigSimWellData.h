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

#include "RigWellResultPoint.h"

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfMath.h"
#include "cvfVector3.h"

#include <QDateTime>

#include <vector>

//==================================================================================================
/// 
//==================================================================================================
class RigSimWellData : public cvf::Object
{
public:
    RigSimWellData();

    void                                   setMultiSegmentWell(bool isMultiSegmentWell);
    bool                                   isMultiSegmentWell() const;

    bool                                   hasWellResult(size_t resultTimeStepIndex) const;
    bool                                   hasAnyValidCells(size_t resultTimeStepIndex) const;

    const RigWellResultFrame&              wellResultFrame(size_t resultTimeStepIndex) const;
    bool                                   isOpen(size_t resultTimeStepIndex) const;
    RigWellResultFrame::WellProductionType wellProductionType(size_t resultTimeStepIndex) const;

    const RigWellResultFrame&              staticWellCells() const;
    
    void                                   computeMappingFromResultTimeIndicesToWellTimeIndices(const std::vector<QDateTime>& resultTimes);
                                      
public:  // Todo: Clean up this regarding public members and constness etc.                     
    QString                                m_wellName;
                                           
    std::vector<size_t>                    m_resultTimeStepIndexToWellTimeStepIndex;   // Well result timesteps may differ from result timesteps
    std::vector< RigWellResultFrame >      m_wellCellsTimeSteps;
private:
    mutable RigWellResultFrame             m_staticWellCells;

    void                                   computeStaticWellCellPath() const;

private:
    bool                                   m_isMultiSegmentWell;
};

