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
#include "cvfTextureImage.h"
#include "cvfVector3.h"

#include <map>
#include <memory>
#include <vector>

class RigGriddedPart3d;
class RigMainGrid;
class RimFaultReactivationDataAccess;

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

    void setPlane( cvf::Vec3d anchorPoint, cvf::Vec3d normal );
    void setFaultPlaneIntersect( cvf::Vec3d faultPlaneTop, cvf::Vec3d faultPlaneBottom );
    void setMaxExtentFromAnchor( double maxExtentHorz, double minZ, double maxZ );

    void setCellCounts( int horzPart1, int horzPart2, int vertUpper, int vertMiddle, int vertLower );
    void setThickness( double thickness );
    void setLocalCoordTransformation( cvf::Mat4d transform );
    void setUseLocalCoordinates( bool useLocalCoordinates );

    void updateGeometry();

    cvf::Vec3d normal() const;

    void                        setPartColors( cvf::Color3f part1Color, cvf::Color3f part2Color );
    std::vector<cvf::Vec3d>     rect( ModelParts part ) const;
    cvf::ref<cvf::TextureImage> texture( ModelParts part ) const;

    const std::vector<std::vector<cvf::Vec3d>>& meshLines( GridPart part ) const;

    std::shared_ptr<RigGriddedPart3d> grid( GridPart part ) const;

    void generateElementSets( const RimFaultReactivationDataAccess* dataAccess, const RigMainGrid* grid );

protected:
    void generateGrids( cvf::Vec3dArray points );

private:
    cvf::Vec3d m_planeNormal;
    cvf::Vec3d m_planeAnchor;

    cvf::Vec3d m_faultPlaneIntersectTop;
    cvf::Vec3d m_faultPlaneIntersectBottom;

    double m_maxHorzExtent;
    double m_minZ;
    double m_maxZ;

    double m_thickness;

    int m_cellCountHorzPart1;
    int m_cellCountHorzPart2;
    int m_cellCountVertUpper;
    int m_cellCountVertMiddle;
    int m_cellCountVertLower;

    std::map<ModelParts, std::vector<int>> m_cornerIndexes;

    std::map<ModelParts, RigFRModelPart> m_parts;
    bool                                 m_isValid;

    std::map<GridPart, std::shared_ptr<RigGriddedPart3d>> m_3dparts;

    cvf::Mat4d m_localCoordTransform;
};
