/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "cvfStructGrid.h"

class RigFemPart;

class RigFemPartGrid : public cvf::StructGridInterface
{
public:
    RigFemPartGrid();
    ~RigFemPartGrid() override;

    void setFemPart( const RigFemPart* femPart );

    bool   ijkFromCellIndex( size_t cellIndex, size_t* i, size_t* j, size_t* k ) const override;
    size_t cellIndexFromIJK( size_t i, size_t j, size_t k ) const override;

    size_t gridPointCountI() const override;
    size_t gridPointCountJ() const override;
    size_t gridPointCountK() const override;

    cvf::Vec3i findMainIJKFaces( int elementIndex ) const;

    std::pair<cvf::Vec3st, cvf::Vec3st> reservoirIJKBoundingBox() const;
    void                                cellCornerVertices( size_t cellIndex, cvf::Vec3d vertices[8] ) const override;
    cvf::Vec3d                          cellCentroid( size_t cellIndex ) const override;

private:
    void generateStructGridData();
    int  findElmIdxForIJK000();
    int  perpendicularFaceInDirection( cvf::Vec3f direction, int perpFaceIdx, int elmIdx );

    const RigFemPart* m_femPart;

    std::vector<cvf::Vec3i> m_ijkPrElement;
    cvf::Vec3st             m_elementIJKCounts;

private: // Unused, Not implemented
    bool       isCellValid( size_t i, size_t j, size_t k ) const override;
    cvf::Vec3d minCoordinate() const override;
    cvf::Vec3d maxCoordinate() const override;
    bool       cellIJKNeighbor( size_t i, size_t j, size_t k, FaceType face, size_t* neighborCellIndex ) const override;

    bool cellIJKFromCoordinate( const cvf::Vec3d& coord, size_t* i, size_t* j, size_t* k ) const override;
    void cellMinMaxCordinates( size_t cellIndex, cvf::Vec3d* minCoordinate, cvf::Vec3d* maxCoordinate ) const override;
    size_t     gridPointIndexFromIJK( size_t i, size_t j, size_t k ) const override;
    cvf::Vec3d gridPointCoordinate( size_t i, size_t j, size_t k ) const override;

    class IJKArray
    {
    public:
        IJKArray()
            : m_iCount( 0 )
            , m_jCount( 0 )
        {
        }

        void resize( size_t iCount, size_t jCount, size_t kCount )
        {
            data.resize( iCount * jCount * kCount, cvf::UNDEFINED_SIZE_T );
            m_iCount = iCount;
            m_jCount = jCount;
        }

        size_t& at( size_t i, size_t j, size_t k ) { return data[i + j * m_iCount + k * m_iCount * m_jCount]; }

        size_t at( size_t i, size_t j, size_t k ) const { return data[i + j * m_iCount + k * m_iCount * m_jCount]; }

    private:
        size_t m_iCount;
        size_t m_jCount;

        std::vector<size_t> data;
    };

    IJKArray                            m_elmIdxPrIJK;
    std::pair<cvf::Vec3st, cvf::Vec3st> m_reservoirIJKBoundingBox;
};
