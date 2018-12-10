/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "cafEffectGenerator.h"

class RivTernaryScalarMapper;



/*
    Thoughts on organizing the texture coords generation a bit.

    Conceptually several mappings takes place:

    1. ResultValues to ResultPointValues <-- Eg. Cell Center values to CellFace Values
    2. ResultPointValues to GeometryPointValues <-- Eg. CellCenter Values to Triangle Vertex
    3. GeometryPointValues to TextureCoordinates/Colors <-- Handled by ScalarMapper

    When evaluating, we normally use the geometry as starting point, as that often is
    a subset of the total results/geometry domain.

    To make this efficient, a minimum of internal storage should be used, so we want 
    to make the mappings as a set of functions called for each (or a few) texture 
    coordinate positions

    The mapping is then actually accessed in the opposite way of the above, while calculated in the 1-3 order

    Accessing correct values:
    GeometryPointIdx->ResultPointIdx->ResultValueIdx
    Calculating color:
    ResultValue->ResultPointValue->GeometryPointValue->Texture/ColorValue

    In ResInsight (for now)
    the ResultPointValue will be the same for all the corresponding GeometryPoints, 
    which means each quadvertex has the same texcoord for all corners.
  
    Proposal:
    ----------
    Let the FaceValue to Face vertex texture coordinate mapping be the same for all.
    Extract that from the code floating around.

    Create a PrimitiveFaceIdx to CellIdx with Face mapper class that handles the lookup, 
    created by the geometry generation

    Create separate calculators/mappers/Strategies to create FaceValues from results.

    Test Code
    -----------
    // Example code 
    // 1. CellCenterToCellFace
    // 2. CellFace to Quad Corners
    // 3. Quad Corner Values to tex coords

    texCoords.resize(m_quadsToGridCells.size()*4);
    for (i = 0; i < m_quadsToGridCells.size(); ++i)
    {
        cvf::Vec2f texCoord = scalarMapper->mapToTextureCoord(resultAccessor->cellScalar(m_quadsToGridCells[i]));
                                                               ResValue                     ResPoint To ResValue
        texCoords[i*4 + 0] = texCoord;
        texCoords[i*4 + 1] = texCoord;
        texCoords[i*4 + 2] = texCoord;
        texCoords[i*4 + 3] = texCoord;
    }
    
    Texturing needs in ResInsight:
    * ScalarMapper
    * Handle HugeVal/nan
    * PipeCellTransparency 
        - includes geometry point to cell mapping
    * Modify the Scalarmapper Texture
    * The domain values to convert pr geometry point



*/

//==================================================================================================
//
// Cell Face Effect
//
//==================================================================================================
class CellEdgeEffectGenerator : public caf::EffectGenerator
{
public:
    explicit CellEdgeEffectGenerator(const cvf::ScalarMapper* edgeScalarMapper);

    void                            setScalarMapper(const cvf::ScalarMapper* cellScalarMapper);
    void                            setTernaryScalarMapper(const RivTernaryScalarMapper* ternaryScalarMapper);

    void                            setOpacityLevel(float opacity)          { m_opacityLevel = cvf::Math::clamp(opacity, 0.0f , 1.0f ); }
    void                            setUndefinedColor(cvf::Color3f color)   { m_undefinedColor = color; }
    void                            setFaceCulling(caf::FaceCulling faceCulling) { m_cullBackfaces = faceCulling; }
    void                            setDefaultCellColor(cvf::Color3f color) { m_defaultCellColor = color; }
    void                            disableLighting(bool disable)           { m_disableLighting = disable; }

protected:
    bool                    isEqual( const EffectGenerator* other ) const override;
    EffectGenerator*        copy() const override;

    void                    updateForShaderBasedRendering(cvf::Effect* effect) const override;
    void                    updateForFixedFunctionRendering(cvf::Effect* effect) const override;

private:
    cvf::cref<cvf::ScalarMapper>        m_edgeScalarMapper;
    mutable cvf::ref<cvf::TextureImage> m_edgeTextureImage;
    cvf::cref<cvf::ScalarMapper>        m_cellScalarMapper;
    mutable cvf::ref<cvf::TextureImage> m_cellTextureImage;

    cvf::cref<RivTernaryScalarMapper>    m_ternaryCellScalarMapper;

    float                           m_opacityLevel;
    caf::FaceCulling                m_cullBackfaces;
    cvf::Color3f                    m_undefinedColor;
    cvf::Color3f                    m_defaultCellColor;
    bool                            m_disableLighting;
};

