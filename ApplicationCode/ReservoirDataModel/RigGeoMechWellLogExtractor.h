/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RigWellLogExtractor.h"

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfMath.h"
#include "cvfVector3.h"

#include <vector>
#include "cvfStructGrid.h"

class RigWellPath;
class RigGeoMechCaseData;
class RigFemResultAddress;

namespace cvf {
    class BoundingBox;
}

//==================================================================================================
/// 
//==================================================================================================
class RigGeoMechWellLogExtractor : public RigWellLogExtractor
{
public:
    RigGeoMechWellLogExtractor(RigGeoMechCaseData* aCase, const RigWellPath* wellpath, const std::string& wellCaseErrorMsgName);

    void                         curveData(const RigFemResultAddress& resAddr, int frameIndex, std::vector<double>* values );
    const RigGeoMechCaseData*    caseData()     { return m_caseData.p();}

private:
    void                         calculateIntersection();
    std::vector<size_t>          findCloseCells(const cvf::BoundingBox& bb);
    virtual cvf::Vec3d           calculateLengthInCell(size_t cellIndex, 
                                                       const cvf::Vec3d& startPoint, 
                                                       const cvf::Vec3d& endPoint) const override;

    cvf::ref<RigGeoMechCaseData> m_caseData;
};


