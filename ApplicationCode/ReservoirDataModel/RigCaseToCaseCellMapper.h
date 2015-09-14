/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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
#include "cvfMath.h"
#include "cvfVector3.h"

#include <vector>

class RigMainGrid;
class RigFemPart;


//==================================================================================================
/// 
//==================================================================================================
class RigCaseToCaseCellMapper : public cvf::Object
{
public:
    RigCaseToCaseCellMapper(RigMainGrid* eclipseGrid, RigFemPart* femPart);
   
    const int * masterCaseCellIndices(int dependentCaseReservoirCellIndex, int* masterCaseCellIndexCount);

    
private:
#if 0
    cvf::Vec3i  m_ecToGmOffset;
    cvf::Matrix3<int> m_ecToGmAxisTransform;
     

    std::vector<int>    m_ecgmCellOrIntervalIndex;
    std::vector<std::vector<int> > m_ecgmCellIndexIntervals;
    std::vector<int>    m_gmecCellOrIntervalIndex;
    std::vector<std::vector<int> > m_gmecCellIndexIntervals;
#endif
};
