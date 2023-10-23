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

class RigMainGrid;
class RimFaultReactivationDataAccess;

//==================================================================================================
///
///
//==================================================================================================
class RigGriddedPart3d : public cvf::Object
{
    using ElementSets = RimFaultReactivation::ElementSets;
    using Boundary    = RimFaultReactivation::Boundary;

public:
    RigGriddedPart3d( bool flipFrontBack );
    ~RigGriddedPart3d() override;

    void reset();
    void clearModelData();

    void generateGeometry( std::vector<cvf::Vec3d> inputPoints,
                           int                     nHorzCells,
                           int                     nVertCellsLower,
                           int                     nVertCellsMiddle,
                           int                     nVertCellsUpper,
                           double                  thickness );

    void generateElementSets( const RimFaultReactivationDataAccess* dataAccess, const RigMainGrid* grid );
    void generateLocalNodes( const cvf::Mat4d transform );
    void extractModelData( RimFaultReactivationDataAccess* dataAccess, size_t outputTimeStep );

    const std::vector<cvf::Vec3d>& nodes() const;
    const std::vector<cvf::Vec3d>& globalNodes() const;
    void                           setUseLocalCoordinates( bool useLocalCoordinates );
    bool                           useLocalCoordinates() const;

    const std::vector<std::vector<unsigned int>>&                                   elementIndices() const;
    const std::map<RimFaultReactivation::BorderSurface, std::vector<unsigned int>>& borderSurfaceElements() const;

    const std::vector<std::vector<cvf::Vec3d>>& meshLines() const;
    std::vector<cvf::Vec3d>                     elementCorners( size_t elementIndex ) const;

    const std::map<Boundary, std::vector<unsigned int>>& boundaryElements() const;
    const std::map<Boundary, std::vector<unsigned int>>& boundaryNodes() const;

    const std::map<ElementSets, std::vector<unsigned int>>& elementSets() const;

    const std::vector<double>& nodePorePressure( size_t outputTimeStep ) const;

protected:
    cvf::Vec3d stepVector( cvf::Vec3d start, cvf::Vec3d stop, int nSteps );
    void       generateMeshlines( const std::vector<cvf::Vec3d>& cornerPoints, int numHorzCells, int numVertCells );

    bool elementIsAboveReservoir( const std::vector<cvf::Vec3d>& cornerPoints, double threshold ) const;
    bool elementIsBelowReservoir( const std::vector<cvf::Vec3d>& cornerPoints, double threshold ) const;

    std::pair<int, int> reservoirZTopBottom( const RigMainGrid* grid ) const;

private:
    bool m_useLocalCoordinates;

    std::vector<cvf::Vec3d>                                                  m_nodes;
    std::vector<cvf::Vec3d>                                                  m_localNodes;
    std::vector<std::vector<unsigned int>>                                   m_elementIndices;
    std::map<RimFaultReactivation::BorderSurface, std::vector<unsigned int>> m_borderSurfaceElements;
    std::vector<std::vector<cvf::Vec3d>>                                     m_meshLines;
    std::map<Boundary, std::vector<unsigned int>>                            m_boundaryElements;
    std::map<Boundary, std::vector<unsigned int>>                            m_boundaryNodes;
    std::map<ElementSets, std::vector<unsigned int>>                         m_elementSets;

    std::vector<std::vector<double>> m_nodePorePressure;
    const std::vector<double>        m_emptyData;

    std::vector<cvf::Vec3d> m_reservoirRect;
};
