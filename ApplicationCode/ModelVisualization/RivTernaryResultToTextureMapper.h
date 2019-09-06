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

#include "RivTernaryScalarMapper.h"

#include "cvfObject.h"
#include "cvfStructGrid.h"
#include "cvfVector2.h"

#include <cmath>

class RivTernaryResultToTextureMapper : public cvf::Object
{
public:
    RivTernaryResultToTextureMapper( const RivTernaryScalarMapper* scalarMapper,
                                     const RigPipeInCellEvaluator* pipeInCellEvaluator )
        : m_scalarMapper( scalarMapper )
        , m_pipeInCellEvaluator( pipeInCellEvaluator )
    {
    }

    cvf::Vec2f getTexCoord( double soil, double sgas, size_t cellIndex ) const
    {
        if ( soil == HUGE_VAL || soil != soil || sgas == HUGE_VAL || sgas != sgas ) // a != a is true for NAN's
        {
            cvf::Vec2f texCoord( 1.0, 1.0 );
            return texCoord;
        }

        bool isTransparent = false;

        if ( m_pipeInCellEvaluator.notNull() )
        {
            isTransparent = m_pipeInCellEvaluator->isWellPipeInCell( cellIndex );
        }

        return m_scalarMapper->mapToTextureCoord( soil, sgas, isTransparent );
    }

private:
    cvf::cref<RivTernaryScalarMapper> m_scalarMapper;
    cvf::cref<RigPipeInCellEvaluator> m_pipeInCellEvaluator;
};
