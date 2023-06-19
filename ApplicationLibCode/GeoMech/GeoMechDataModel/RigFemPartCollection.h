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

#include "RigFemPart.h"
#include "cvfCollection.h"

class RigFemPartCollection : public cvf::Object
{
public:
    RigFemPartCollection();
    ~RigFemPartCollection() override;

    void              addFemPart( RigFemPart* part );
    RigFemPart*       part( size_t index );
    const RigFemPart* part( size_t index ) const;

    int partCount() const;

    size_t           totalElementCount() const;
    float            characteristicElementSize() const;
    cvf::BoundingBox boundingBox() const;

    std::pair<int, size_t>               partIdAndElementIndex( size_t globalIndex ) const;
    std::pair<const RigFemPart*, size_t> partAndElementIndex( size_t globalIndex ) const;
    size_t                               globalElementIndex( int partId, size_t localIndex ) const;

    void findIntersectingGlobalElementIndices( const cvf::BoundingBox& intersectingBB,
                                               std::vector<size_t>*    intersectedGlobalElementIndices ) const;

    int nodeIdxFromElementNodeResultIdx( size_t globalResultIdx ) const;

    size_t globalElementNodeResultIdx( int part, int elementIdx, int elmLocalNodeIdx ) const;

private:
    cvf::Collection<RigFemPart> m_femParts;
    std::vector<size_t>         m_partElementOffset;
    std::vector<size_t>         m_partNodeOffset;
    std::vector<size_t>         m_partConnectivityOffset;
};
