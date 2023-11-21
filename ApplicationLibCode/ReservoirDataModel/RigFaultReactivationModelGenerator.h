/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "cvfMatrix4.h"
#include "cvfObject.h"
#include "cvfPlane.h"
#include "cvfStructGrid.h"
#include "cvfVector3.h"

#include <array>
#include <vector>

#include <QString>

class RigFault;
class RigMainGrid;
class RigGriddedPart3d;
class RigActiveCellInfo;

class RigFaultReactivationModelGenerator : cvf::Object
{
public:
    RigFaultReactivationModelGenerator( cvf::Vec3d position, cvf::Vec3d normal );
    ~RigFaultReactivationModelGenerator();

    void setFault( const RigFault* fault );
    void setGrid( const RigMainGrid* grid );
    void setActiveCellInfo( const RigActiveCellInfo* activeCellInfo );
    void setFaultBufferDepth( double aboveFault, double belowFault );
    void setModelSize( double startDepth, double depthBelowFault, double horzExtentFromFault );
    void setModelThickness( double thickness );
    void setModelGriddingOptions( double maxCellHeight, double cellSizeFactor, int noOfCellsHorzFront, int noOfCellsHorzBack );

    void setUseLocalCoordinates( bool useLocalCoordinates );
    void setupLocalCoordinateTransform();

    std::pair<cvf::Vec3d, cvf::Vec3d> modelLocalNormalsXY();

    void generateGeometry( size_t                             startCellIndex,
                           cvf::StructGridInterface::FaceType startFace,
                           RigGriddedPart3d*                  frontPart,
                           RigGriddedPart3d*                  backPart );

    const std::array<cvf::Vec3d, 12>&       frontPoints() const;
    const std::array<cvf::Vec3d, 12>&       backPoints() const;
    const cvf::Vec3d                        normal() const;
    const std::pair<cvf::Vec3d, cvf::Vec3d> faultTopBottomPoints() const;

protected:
    static const std::array<int, 4>      faceIJCornerIndexes( cvf::StructGridInterface::FaceType face );
    static const std::vector<cvf::Vec3d> interpolateExtraPoints( cvf::Vec3d from, cvf::Vec3d to, double maxStep );

    static cvf::Vec3d lineIntersect( const cvf::Plane& plane, cvf::Vec3d lineA, cvf::Vec3d lineB );
    static cvf::Vec3d extrapolatePoint( cvf::Vec3d startPoint, cvf::Vec3d endPoint, double stopDepth, bool upwards );
    static void       splitLargeLayers( std::map<double, cvf::Vec3d>& layers, std::vector<int>& kLayers, double maxHeight );

    std::map<double, cvf::Vec3d> elementLayers( cvf::StructGridInterface::FaceType face, const std::vector<size_t>& cellIndexColumn );
    void                         addFilter( QString name, std::vector<size_t> cells );

    size_t oppositeStartCellIndex( const std::vector<size_t> cellIndexColumn, cvf::StructGridInterface::FaceType face );

    void generatePointsFrontBack();

private:
    cvf::Vec3d m_startPosition;
    cvf::Vec3d m_normal;

    std::array<cvf::Vec3d, 12> m_frontPoints;
    std::array<cvf::Vec3d, 12> m_backPoints;

    cvf::cref<RigFault>          m_fault;
    cvf::cref<RigMainGrid>       m_grid;
    cvf::cref<RigActiveCellInfo> m_activeCellInfo;

    double m_bufferAboveFault;
    double m_bufferBelowFault;

    double m_startDepth;
    double m_depthBelowFault;
    double m_horzExtentFromFault;
    double m_modelThickness;

    double m_maxCellHeight;
    double m_cellSizeFactor;

    int m_noOfCellsHorzFront;
    int m_noOfCellsHorzBack;

    cvf::Vec3d m_topReservoirFront;
    cvf::Vec3d m_topReservoirBack;

    cvf::Vec3d m_bottomReservoirFront;
    cvf::Vec3d m_bottomReservoirBack;

    cvf::Vec3d m_topFault;
    cvf::Vec3d m_bottomFault;

    cvf::Mat4d m_localCoordTransform;
    bool       m_useLocalCoordinates;
};
