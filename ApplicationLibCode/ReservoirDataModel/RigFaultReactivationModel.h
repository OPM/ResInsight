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
#include "cvfPlane.h"
#include "cvfStructGrid.h"
#include "cvfTextureImage.h"
#include "cvfVector3.h"

#include <map>
#include <memory>
#include <vector>

class RigGriddedPart3d;
class RigMainGrid;
class RimFaultReactivationDataAccess;
class RigFaultReactivationModelGenerator;

class RigFRModelPart
{
public:
    RigFRModelPart(){};
    ~RigFRModelPart(){};

    std::vector<cvf::Vec3d>     rect;
    cvf::ref<cvf::TextureImage> texture;
};

//==================================================================================================
///
///
//==================================================================================================
class RigFaultReactivationModel : public cvf::Object
{
    using ModelParts = RimFaultReactivation::ModelParts;
    using GridPart   = RimFaultReactivation::GridPart;

public:
    RigFaultReactivationModel();
    ~RigFaultReactivationModel() override;

    std::vector<ModelParts> allModelParts() const;
    std::vector<GridPart>   allGridParts() const;

    bool isValid() const;
    void reset();

    void setGenerator( std::shared_ptr<RigFaultReactivationModelGenerator> generator );

    std::pair<cvf::Vec3d, cvf::Vec3d> modelLocalNormalsXY() const;

    void updateGeometry( size_t startCell, cvf::StructGridInterface::FaceType startFace );

    void                        setPartColors( cvf::Color3f part1Color, cvf::Color3f part2Color );
    std::vector<cvf::Vec3d>     rect( ModelParts part ) const;
    cvf::ref<cvf::TextureImage> texture( ModelParts part ) const;

    const std::vector<std::vector<cvf::Vec3d>>& meshLines( GridPart part ) const;

    std::shared_ptr<RigGriddedPart3d> grid( GridPart part ) const;

    void generateElementSets( const RimFaultReactivationDataAccess* dataAccess, const RigMainGrid* grid );

protected:
    void generateGrids( cvf::Vec3dArray points );

private:
    std::shared_ptr<RigFaultReactivationModelGenerator> m_generator;

    std::map<ModelParts, std::vector<int>> m_cornerIndexes;

    std::map<ModelParts, RigFRModelPart> m_parts;
    bool                                 m_isValid;

    std::map<GridPart, std::shared_ptr<RigGriddedPart3d>> m_3dparts;
};
