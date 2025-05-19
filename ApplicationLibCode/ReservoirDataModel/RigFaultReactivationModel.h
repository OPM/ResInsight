/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023 Equinor ASA
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

#include "RimFaultReactivationEnums.h"

#include "cvfArray.h"
#include "cvfColor3.h"
#include "cvfMatrix4.h"
#include "cvfObject.h"
#include "cvfStructGrid.h"
#include "cvfTextureImage.h"
#include "cvfVector3.h"

#include <array>
#include <map>
#include <memory>
#include <utility>
#include <vector>

class RigGriddedPart3d;
class RigFaultReactivationModelGenerator;
class RimEclipseCase;

class RigFRModelPart
{
public:
    RigFRModelPart() {};
    ~RigFRModelPart() {};

    std::vector<cvf::Vec3d>     rect;
    cvf::ref<cvf::TextureImage> texture;
};

//==================================================================================================
///
///
//==================================================================================================
class RigFaultReactivationModel : public cvf::Object
{
    using GridPart = RimFaultReactivation::GridPart;

public:
    RigFaultReactivationModel();
    ~RigFaultReactivationModel() override;

    static int            numModelParts() { return 10; };
    std::vector<GridPart> allGridParts() const;

    bool isValid() const;
    void reset();

    void setGenerator( std::shared_ptr<RigFaultReactivationModelGenerator> generator );

    std::pair<cvf::Vec3d, cvf::Vec3d> modelLocalNormalsXY() const;
    cvf::Vec3d                        transformPointIfNeeded( const cvf::Vec3d point ) const;

    void updateGeometry( size_t startCell, cvf::StructGridInterface::FaceType startFace );

    void                        setPartColors( cvf::Color3f part1Color, cvf::Color3f part2Color );
    std::vector<cvf::Vec3d>     rect( int nPart ) const;
    cvf::ref<cvf::TextureImage> texture( int nPart ) const;

    const std::vector<std::vector<cvf::Vec3d>>& meshLines( GridPart part ) const;

    const RigGriddedPart3d* grid( GridPart part ) const;

    const cvf::Vec3d                        modelNormal() const;
    const std::pair<cvf::Vec3d, cvf::Vec3d> faultTopBottom() const;
    std::pair<double, double>               depthTopBottom() const;

    RimFaultReactivation::GridPart normalPointsAt() const;

    void postProcessElementSets( const RimEclipseCase* eCase );

private:
    std::shared_ptr<RigFaultReactivationModelGenerator> m_generator;

    std::array<std::vector<int>, 5> m_cornerIndexes;
    std::array<RigFRModelPart, 10>  m_parts;

    bool m_isValid;

    std::map<GridPart, RigGriddedPart3d*> m_3dparts;
    RimFaultReactivation::GridPart        m_normalPointsAt;
};
