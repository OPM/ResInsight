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
#include "cvfObject.h"
#include <vector>

//==================================================================================================
/// 
//==================================================================================================

class RigFemScalarResultFrames: public cvf::Object
{
public:
    explicit RigFemScalarResultFrames(int frameCount);
    virtual ~RigFemScalarResultFrames();

    void                      enableAsSingleFrameResult();

    std::vector<float>&       frameData(size_t frameIndex);
    const std::vector<float>& frameData(size_t frameIndex) const;
    int                       frameCount() const;

private:
    std::vector< std::vector<float> > m_dataForEachFrame;
    bool                              m_isSingleFrameResult;
};
