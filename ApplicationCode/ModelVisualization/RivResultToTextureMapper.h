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

#include "cvfVector2.h"
#include "cvfScalarMapper.h"
#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfStructGrid.h"

#include <cmath>

class RivResultToTextureMapper : public cvf::Object
{
public:
    explicit RivResultToTextureMapper(const cvf::ScalarMapper* scalarMapper, 
        const RigPipeInCellEvaluator* pipeInCellEvaluator) 
        : m_scalarMapper(scalarMapper), m_pipeInCellEvaluator(pipeInCellEvaluator)
    {}

    virtual cvf::Vec2f getTexCoord(double resultValue, size_t cellIndex) const = 0;
  
protected:
    cvf::cref<cvf::ScalarMapper>      m_scalarMapper;
    cvf::cref<RigPipeInCellEvaluator> m_pipeInCellEvaluator;
};


