/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

namespace cvf
{
    class StructGridGeometryGenerator;
    class DrawableGeo;
}

class RimCellEdgeResultSlot;
class RimResultSlot;
class RigGridBase;


class RivCellEdgeGeometryGenerator 
{
public:
    static void addCellEdgeResultsToDrawableGeo(size_t timeStepIndex, 
        RimResultSlot* cellResultSlot,
        RimCellEdgeResultSlot* cellEdgeResultSlot,
        cvf::StructGridGeometryGenerator* generator,
        cvf::DrawableGeo* geo);
};



//==================================================================================================
//
// Cell Face Effect
//
//==================================================================================================
class CellEdgeEffectGenerator : public caf::EffectGenerator
{
public:
    CellEdgeEffectGenerator(const cvf::ScalarMapper* edgeScalarMapper, const cvf::ScalarMapper* cellScalarMapper);

    void                            setOpacityLevel(float opacity)          { m_opacityLevel = cvf::Math::clamp(opacity, 0.0f , 1.0f ); }
    void                            setUndefinedColor(cvf::Color3f color)   { m_undefinedColor = color; }
    void                            setCullBackfaces(bool cullBackFaces)    { m_cullBackfaces = cullBackFaces; }
    void                            setDefaultCellColor(cvf::Color3f color) { m_defaultCellColor = color; }

    virtual bool                    isEqual( const EffectGenerator* other ) const;
    virtual EffectGenerator*        copy() const;


protected:
    virtual void                    updateForShaderBasedRendering(cvf::Effect* effect) const;
    virtual void                    updateForFixedFunctionRendering(cvf::Effect* effect) const;

private:
    cvf::cref<cvf::ScalarMapper>    m_edgeScalarMapper;
    cvf::cref<cvf::ScalarMapper>    m_cellScalarMapper;

    float                           m_opacityLevel;
    bool                            m_cullBackfaces;
    cvf::Color3f                    m_undefinedColor;
    cvf::Color3f                    m_defaultCellColor;

};

