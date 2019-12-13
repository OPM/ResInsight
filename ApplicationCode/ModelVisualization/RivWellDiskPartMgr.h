/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019 Equinor ASA
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

#include "cafPdmPointer.h"
#include "cvfObject.h"

namespace cvf
{
class Part;
class ModelBasicList;
class Transform;
class Font;
class Effect;
class ShaderProgram;
} // namespace cvf

namespace caf
{
class DisplayCoordTransform;
}

class Rim3dView;
class RimSimWellInView;
class RimSimWellInViewCollection;

class RivWellDiskPartMgr : public cvf::Object
{
public:
    RivWellDiskPartMgr( RimSimWellInView* well );
    ~RivWellDiskPartMgr() override;

    void appendDynamicGeometryPartsToModel( cvf::ModelBasicList*              model,
                                            size_t                            frameIndex,
                                            const caf::DisplayCoordTransform* displayXf );
    void appendFlattenedDynamicGeometryPartsToModel( cvf::ModelBasicList*              model,
                                                     size_t                            frameIndex,
                                                     const caf::DisplayCoordTransform* displayXf,
                                                     double                            xOffset );

private:
    void                        buildWellDiskParts( size_t                            frameIndex,
                                                    const caf::DisplayCoordTransform* displayXf,
                                                    bool                              doFlatten,
                                                    double                            xOffset );
    void                        clearAllGeometry();
    Rim3dView*                  viewWithSettings();
    RimSimWellInViewCollection* simWellInViewCollection();

private:
    caf::PdmPointer<RimSimWellInView> m_rimWell;

    cvf::ref<cvf::Part> m_wellDiskPart;
    cvf::ref<cvf::Part> m_wellDiskLabelPart;

    bool                         m_useShaders;
    cvf::ref<cvf::ShaderProgram> m_shaderProg;
    cvf::ref<cvf::Effect>        m_fixedFuncEffect;
    cvf::ref<cvf::Effect>        m_shaderEffect;
};
