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

#include "cvfMatrix4.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include <map>
#include <vector>

//==================================================================================================
///
///
//==================================================================================================
class RigGriddedPart3d : public cvf::Object
{
    using ElementSets = RimFaultReactivation::ElementSets;
    using Boundary    = RimFaultReactivation::Boundary;

public:
    RigGriddedPart3d();
    ~RigGriddedPart3d() override;

    void reset();

    void generateGeometry( const std::array<cvf::Vec3d, 12> inputPoints,
                           const std::vector<cvf::Vec3d>    reservoirLayers,
                           const std::vector<int>           kLayers,
                           double                           maxCellHeight,
                           double                           cellSizeFactor,
                           int                              nHorzCells,
                           double                           modelThickness );

    void generateLocalNodes( const cvf::Mat4d transform );

    const std::vector<cvf::Vec3d>& nodes() const;
    const std::vector<cvf::Vec3d>& globalNodes() const;
    void                           setUseLocalCoordinates( bool useLocalCoordinates );
    bool                           useLocalCoordinates() const;

    const std::vector<std::vector<unsigned int>>&                                   elementIndices() const;
    const std::map<RimFaultReactivation::BorderSurface, std::vector<unsigned int>>& borderSurfaceElements() const;

    const std::vector<std::vector<cvf::Vec3d>>&             meshLines() const;
    const std::map<Boundary, std::vector<unsigned int>>&    boundaryElements() const;
    const std::map<Boundary, std::vector<unsigned int>>&    boundaryNodes() const;
    const std::map<ElementSets, std::vector<unsigned int>>& elementSets() const;
    const std::vector<int>                                  elementKLayer() const;
    const std::vector<cvf::Vec3d>                           elementCorners( size_t elementIndex ) const;

protected:
    static cvf::Vec3d          stepVector( cvf::Vec3d start, cvf::Vec3d stop, int nSteps );
    static std::vector<double> generateConstantLayers( double zFrom, double zTo, double maxSize );
    static std::vector<double> generateGrowingLayers( double zFrom, double zTo, double maxSize, double growfactor );
    static std::vector<double> extractZValues( std::vector<cvf::Vec3d> );

    void generateVerticalMeshlines( const std::vector<cvf::Vec3d>& cornerPoints, int numHorzCells );

private:
    enum class Regions
    {
        LowerUnderburden = 0, // deepest region goes first
        UpperUnderburden,
        Reservoir,
        LowerOverburden,
        UpperOverburden
    };

    static std::vector<Regions> allRegions();

private:
    bool m_useLocalCoordinates;

    std::vector<cvf::Vec3d>                                                  m_nodes;
    std::vector<cvf::Vec3d>                                                  m_localNodes;
    std::vector<std::vector<unsigned int>>                                   m_elementIndices;
    std::vector<int>                                                         m_elementKLayer;
    std::map<RimFaultReactivation::BorderSurface, std::vector<unsigned int>> m_borderSurfaceElements;
    std::vector<std::vector<cvf::Vec3d>>                                     m_meshLines;
    std::map<Boundary, std::vector<unsigned int>>                            m_boundaryElements;
    std::map<Boundary, std::vector<unsigned int>>                            m_boundaryNodes;
    std::map<ElementSets, std::vector<unsigned int>>                         m_elementSets;

    std::vector<cvf::Vec3d> m_reservoirRect;
};
