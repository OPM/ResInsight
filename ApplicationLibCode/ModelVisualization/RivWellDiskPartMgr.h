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
#include "cvfString.h"
#include "cvfVector3.h"

namespace cvf
{
class Part;
class ModelBasicList;
class Effect;
class ShaderProgram;
class Color4f;
} // namespace cvf

namespace caf
{
class DisplayCoordTransform;
}

namespace RiaDefines
{
enum class WellProductionType : short;
}

class Rim3dView;
class RimSimWellInView;
class RimSimWellInViewCollection;
class RigWellResultFrame;

class RivWellDiskPartMgr : public cvf::Object
{
public:
    RivWellDiskPartMgr( RimSimWellInView* well );
    ~RivWellDiskPartMgr() override;

    void appendDynamicGeometryPartsToModel( cvf::ModelBasicList*              model,
                                            size_t                            frameIndex,
                                            const caf::DisplayCoordTransform* displayXf );

private:
    void buildWellDiskParts( size_t frameIndex, const caf::DisplayCoordTransform* displayXf );

    std::pair<cvf::String, cvf::Vec3f> createTextAndLocation( const double aggregatedFraction,
                                                              cvf::Vec3d   diskPosition,
                                                              double       ijScaleFactor,
                                                              const double fraction );

    void                        clearAllGeometry();
    Rim3dView*                  viewWithSettings();
    RimSimWellInViewCollection* simWellInViewCollection();

    static cvf::Color4f getWellInjectionColor( RiaDefines::WellProductionType productionType );
    static QString      formatNumber( double num );
    static double       baseScaleFactor();

private:
    caf::PdmPointer<RimSimWellInView> m_rimWell;

    cvf::ref<cvf::Part> m_wellDiskPart;
    cvf::ref<cvf::Part> m_wellDiskLabelPart;
    cvf::ref<cvf::Part> m_wellDiskInjectorPart;

    cvf::ref<cvf::ShaderProgram> m_shaderProg;
    cvf::ref<cvf::Effect>        m_fixedFuncEffect;
    cvf::ref<cvf::Effect>        m_shaderEffect;
};
