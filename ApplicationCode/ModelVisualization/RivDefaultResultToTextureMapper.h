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

#include "RigPipeInCellEvaluator.h"
#include "RivResultToTextureMapper.h"

#include "cvfVector2.h"
#include "cvfScalarMapper.h"
#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfStructGrid.h"

#include <cmath>

class RivDefaultResultToTextureMapper : public RivResultToTextureMapper
{
public:
    using RivResultToTextureMapper::RivResultToTextureMapper;

    cvf::Vec2f getTexCoord(double resultValue, size_t cellIndex) const
    {
        cvf::Vec2f texCoord(0,0);

       if (resultValue == HUGE_VAL || resultValue != resultValue) // a != a is true for NAN's
        {
            texCoord[1] = 1.0f;
            return texCoord;
        }

        texCoord = m_scalarMapper->mapToTextureCoord(resultValue);
  
        if (!m_pipeInCellEvaluator->isWellPipeInCell(cellIndex))
        {
            texCoord[1] = 0; // Set the Y texture coordinate to the opaque line in the texture
        }

        return texCoord;
    }
};


