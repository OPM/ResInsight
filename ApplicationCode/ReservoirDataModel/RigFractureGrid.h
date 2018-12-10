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

#include "cvfBase.h"
#include "cvfObject.h"
#include "RigFractureCell.h"

#include <vector>

class RigFractureCell;

//==================================================================================================
///  
///  
//==================================================================================================
class RigFractureGrid : public cvf::Object
{
public:
    RigFractureGrid();

    void setFractureCells(std::vector<RigFractureCell> fractureCells);
    void setWellCenterFractureCellIJ(std::pair<size_t, size_t>  wellCenterFractureCellIJ);
    void setICellCount(size_t iCellCount);
    void setJCellCount(size_t jCellCount);

    const std::vector<RigFractureCell>&     fractureCells() const;
    size_t                                  getGlobalIndexFromIJ(size_t i, size_t j) const;
    const RigFractureCell&                  cellFromIndex(size_t index) const;
    size_t                                  jCellCount() const;
    size_t                                  iCellCount() const;

    std::pair<size_t, size_t>               fractureCellAtWellCenter() const;

private:
    std::vector<RigFractureCell> m_fractureCells;
    std::pair<size_t, size_t>    m_wellCenterFractureCellIJ;
    size_t                       m_iCellCount;
    size_t                       m_jCellCount;
};
