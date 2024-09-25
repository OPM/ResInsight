/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RigCell.h"
#include "RigGridBase.h"
#include "RigLocalGrid.h"

#include "cvfBoundingBox.h"
#include "cvfCollection.h"

#include <vector>

class RigActiveCellInfo;
class RigNNCData;

namespace cvf
{
class BoundingBoxTree;
}

class RigMainGrid : public RigGridBase
{
public:
    RigMainGrid();
    ~RigMainGrid() override;

    std::vector<cvf::Vec3d>&       nodes();
    const std::vector<cvf::Vec3d>& nodes() const;

    std::vector<RigCell>&       globalCellArray();
    const std::vector<RigCell>& globalCellArray() const;

    virtual RigGridBase*       gridAndGridLocalIdxFromGlobalCellIdx( size_t globalCellIdx, size_t* gridLocalCellIdx );
    virtual const RigGridBase* gridAndGridLocalIdxFromGlobalCellIdx( size_t globalCellIdx, size_t* gridLocalCellIdx ) const;

    virtual const RigCell& cellByGridAndGridLocalCellIdx( size_t gridIdx, size_t gridLocalCellIdx ) const;
    virtual size_t         reservoirCellIndexByGridAndGridLocalCellIndex( size_t gridIdx, size_t gridLocalCellIdx ) const;
    virtual size_t         findReservoirCellIndexFromPoint( const cvf::Vec3d& point ) const;

    void addLocalGrid( RigLocalGrid* localGrid );

    size_t             gridCountOnFile() const;
    size_t             gridCount() const;
    RigGridBase*       gridByIndex( size_t localGridIndex );
    const RigGridBase* gridByIndex( size_t localGridIndex ) const;
    RigGridBase*       gridById( int localGridId );
    RigGridBase*       gridByName( const std::string& name );

    size_t totalTemporaryGridCellCount() const;

    RigNNCData* nncData();

    void                             setFaults( const cvf::Collection<RigFault>& faults );
    const cvf::Collection<RigFault>& faults() const;
    cvf::Collection<RigFault>&       faults();
    void                             calculateFaults( const RigActiveCellInfo* activeCellInfo );

    void addUnNamedFaultFaces( int                               gcIdx,
                               const RigActiveCellInfo*          activeCellInfo,
                               const std::vector<cvf::Vec3d>&    vxs,
                               int                               unNamedFaultIdx,
                               int                               unNamedFaultWithInactiveIdx,
                               std::vector<RigFault::FaultFace>& unNamedFaultFaces,
                               std::vector<RigFault::FaultFace>& unNamedFaultFacesInactive,
                               RigFaultsPrCellAccumulator*       faultsPrCellAcc ) const;

    void distributeNNCsToFaults();

    const RigFault* findFaultFromCellIndexAndCellFace( size_t reservoirCellIndex, cvf::StructGridInterface::FaceType face ) const;
    bool            isFaceNormalsOutwards() const;
    void            computeFaceNormalsDirection( const std::vector<size_t>& reservoirCellIndices ) const;

    void computeCachedData( std::string* aabbTreeInfo = nullptr );
    void initAllSubGridsParentGridPointer();

    cvf::Vec3d displayModelOffset() const override;
    void       setDisplayModelOffset( cvf::Vec3d offset );

    void                setFlipAxis( bool flipXAxis, bool flipYAxis );
    std::vector<size_t> findIntersectingCells( const cvf::BoundingBox& inputBB ) const;

    cvf::BoundingBox boundingBox() const;

    bool               isTempGrid() const override;
    const std::string& associatedWellPathName() const override;

    void                         setUseMapAxes( bool useMapAxes );
    bool                         useMapAxes() const;
    void                         setMapAxes( const std::array<double, 6>& mapAxes );
    const std::array<double, 6>& mapAxes() const;
    std::array<float, 6>         mapAxesF() const;

    cvf::Mat4d mapAxisTransform() const;

    bool isDualPorosity() const;
    void setDualPorosity( bool enable );

protected:
    void initAllSubCellsMainGridCellIndex();
    void buildCellSearchTree();
    void buildCellSearchTreeOptimized( size_t cellsPerBoundingBox );
    bool hasFaultWithName( const QString& name ) const;
    void computeBoundingBox();

    static std::array<double, 6> defaultMapAxes();

protected:
    std::vector<cvf::Vec3d>       m_nodes; ///< Global vertex table
    std::vector<RigCell>          m_cells; ///< Global array of all cells in the reservoir (including the ones in LGR's)
    cvf::Collection<RigLocalGrid> m_localGrids; ///< List of all the LGR's in this reservoir
    std::vector<size_t>           m_gridIdToIndexMapping; ///< Mapping from LGR Id to index.

    cvf::Collection<RigFault>            m_faults;
    cvf::ref<RigNNCData>                 m_nncData;
    cvf::ref<RigFaultsPrCellAccumulator> m_faultsPrCellAcc;

    cvf::Vec3d                     m_displayModelOffset;
    cvf::ref<cvf::BoundingBoxTree> m_cellSearchTree;
    cvf::BoundingBox               m_boundingBox;

    bool m_flipXAxis;
    bool m_flipYAxis;

    bool                  m_useMapAxes;
    std::array<double, 6> m_mapAxes;

    bool m_dualPorosity;

    mutable bool m_isFaceNormalsOutwards;
    mutable bool m_isFaceNormalsOutwardsComputed;
};
