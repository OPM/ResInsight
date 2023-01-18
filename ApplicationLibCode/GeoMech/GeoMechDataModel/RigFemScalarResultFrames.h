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

class RigFemScalarResultFrames : public cvf::Object
{
public:
    explicit RigFemScalarResultFrames( int timeStepCount );
    ~RigFemScalarResultFrames() override;

    void enableAsSingleStepResult();

    std::vector<float>&       frameData( int timeStepIndex, int frameIndex );
    const std::vector<float>& frameData( int timeStepIndex, int frameIndex ) const;
    int                       timeStepCount() const;
    int                       frameCount( int timeStepIndex ) const;

private:
    std::vector<std::vector<std::vector<float>>> m_dataForEachFrame;
    std::vector<float>                           m_noData;
    bool                                         m_isSingleStepResult;
};
