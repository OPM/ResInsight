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

#include <QString>

#include <array>
#include <map>
#include <utility>
#include <vector>

class RigFault;
class RigMainGrid;
class RigGriddedPart3d;
class RigActiveCellInfo;
class RigCell;

class RigFaultReactivationModelGenerator : cvf::Object
{
    using FaceType = cvf::StructGridInterface::FaceType;

public:
    RigFaultReactivationModelGenerator( cvf::Vec3d position, cvf::Vec3d modelNormal, cvf::Vec3d direction );
    ~RigFaultReactivationModelGenerator() override;

    void setFault( const RigFault* fault );
    void setGrid( const RigMainGrid* grid );
    void setActiveCellInfo( const RigActiveCellInfo* activeCellInfo );
    void setFaultBufferDepth( double aboveFault, double belowFault, int faultZoneCells );
    void setModelSize( double startDepth, double depthBelowFault, double horzExtentFromFault );
    void setModelThickness( double thickness );
    void setModelGriddingOptions( double minCellHeight,
                                  double maxCellHeight,
                                  double cellSizeFactorHeight,
                                  double minCellWidth,
                                  double cellSizeFactorWidth );

    void setUseLocalCoordinates( bool useLocalCoordinates );
    void setupLocalCoordinateTransform();

    cvf::Vec3d transformPointIfNeeded( const cvf::Vec3d point ) const;

    std::pair<cvf::Vec3d, cvf::Vec3d> modelLocalNormalsXY();

    void generateGeometry( size_t startCellIndex, FaceType startFace, RigGriddedPart3d* frontPart, RigGriddedPart3d* backPart );

    const std::array<cvf::Vec3d, 12>&       frontPoints() const;
    const std::array<cvf::Vec3d, 12>&       backPoints() const;
    const cvf::Vec3d                        modelNormal() const;
    const std::pair<cvf::Vec3d, cvf::Vec3d> faultTopBottomPoints() const;
    std::pair<double, double>               depthTopBottom() const;

protected:
    static const std::array<int, 4>      faceIJCornerIndexes( FaceType face );
    static const std::vector<cvf::Vec3d> interpolateExtraPoints( cvf::Vec3d from, cvf::Vec3d to, double maxStep );
    static const std::vector<double>     partition( double distance, double startSize, double sizeFactor );
    static std::pair<FaceType, FaceType> sideFacesIJ( FaceType face );

    static cvf::Vec3d                 extrapolatePoint( cvf::Vec3d startPoint, cvf::Vec3d endPoint, double stopDepth );
    static void                       splitLargeLayers( std::map<double, cvf::Vec3d>& layers, double maxHeight );
    static void                       mergeTinyLayers( std::map<double, cvf::Vec3d>& layers, double minHeight );
    static std::vector<double>        extractZValues( const std::vector<cvf::Vec3d>& points );
    static std::array<cvf::Vec3d, 12> shiftOrigin( const std::array<cvf::Vec3d, 12>& points, const cvf::Vec3d& newOrigin );

    std::vector<size_t> buildCellColumn( size_t startCell, FaceType startFace, std::map<double, cvf::Vec3d>& layers );

    void updateFilters( std::vector<size_t> frontCells, std::vector<size_t> backCells );

    size_t oppositeStartCellIndex( const std::vector<size_t> cellIndexColumn, FaceType face );

    void generatePointsFrontBack();

    std::pair<size_t, size_t> findCellWithIntersection( const std::vector<RigCell>& cellRow,
                                                        FaceType                    face,
                                                        size_t&                     cellIndex,
                                                        cvf::Vec3d&                 intersect1,
                                                        cvf::Vec3d&                 intersect2,
                                                        bool                        goingUp );

private:
    cvf::Vec3d m_startPosition;
    cvf::Vec3d m_modelNormal;
    cvf::Vec3d m_modelDirection;

    cvf::Plane m_modelPlane;

    std::array<cvf::Vec3d, 12> m_frontPoints;
    std::array<cvf::Vec3d, 12> m_backPoints;

    std::vector<double> m_horizontalPartition;

    cvf::cref<RigFault>          m_fault;
    cvf::cref<RigMainGrid>       m_grid;
    cvf::cref<RigActiveCellInfo> m_activeCellInfo;

    double m_bufferAboveFault;
    double m_bufferBelowFault;
    int    m_faultZoneCells;

    double m_startDepth;
    double m_bottomDepth;
    double m_depthBelowFault;
    double m_horzExtentFromFault;
    double m_modelThickness;

    double m_minCellHeight;
    double m_maxCellHeight;
    double m_cellSizeHeightFactor;
    double m_minCellWidth;
    double m_cellSizeWidthFactor;

    cvf::Vec3d m_topReservoirFront;
    cvf::Vec3d m_topReservoirBack;

    cvf::Vec3d m_bottomReservoirFront;
    cvf::Vec3d m_bottomReservoirBack;

    cvf::Vec3d m_topFault;
    cvf::Vec3d m_bottomFault;

    cvf::Mat4d m_localCoordTransform;
    bool       m_useLocalCoordinates;
};
