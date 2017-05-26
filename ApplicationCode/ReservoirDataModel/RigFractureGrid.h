/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 -     Statoil ASA
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

#include "RigStimPlanFracTemplateCell.h"
#include <vector>
class RigStimPlanFracTemplateCell;

//==================================================================================================
///  
///  
//==================================================================================================
class RigFractureGrid
{
public:
    RigFractureGrid();

    void setFractureCells(std::vector<RigStimPlanFracTemplateCell> stimPlanCells) { m_fractureCells = stimPlanCells; }
    void setWellCenterStimPlanCellIJ(std::pair<size_t, size_t>  wellCenterStimPlanCellIJ) { m_wellCenterStimPlanCellIJ = wellCenterStimPlanCellIJ; }
    void setIcount(size_t icount) { m_iCount = icount; }
    void setJcount(size_t jcount) { m_jCount = jcount; }

    const std::vector<RigStimPlanFracTemplateCell>&     getStimPlanCells() const { return m_fractureCells; }
    size_t                                  getGlobalIndexFromIJ(size_t i, size_t j) const;
    const RigStimPlanFracTemplateCell&      stimPlanCellFromIndex(size_t index) const;
    size_t                                  stimPlanGridNumberOfRows() const { return m_jCount; }
    size_t                                  stimPlanGridNumberOfColums() const { return m_iCount; }

    std::pair<size_t, size_t>               getStimPlanCellAtWellCenter() const { return m_wellCenterStimPlanCellIJ; }


private:
    std::vector<RigStimPlanFracTemplateCell>    m_fractureCells;
    std::pair<size_t, size_t>                   m_wellCenterStimPlanCellIJ;
    size_t                                      m_iCount;
    size_t                                      m_jCount;

    

};

