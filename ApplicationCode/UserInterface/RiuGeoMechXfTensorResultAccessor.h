/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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
#include <array>
#include <vector>

#include "cafTensor3.h"
#include <functional>

class RigFemPartResultsCollection;
class RigFemResultAddress;
class RivIntersectionVertexWeights;

class RiuGeoMechXfTensorResultAccessor
{
public:
    RiuGeoMechXfTensorResultAccessor( RigFemPartResultsCollection* femResCollection,
                                      const RigFemResultAddress&   resVarAddress,
                                      int                          timeStepIdx );

    void calculateInterpolatedValue( const cvf::Vec3f                   triangle[3],
                                     const RivIntersectionVertexWeights vertexWeights[3],
                                     float                              returnValues[3] );

    float calculateElmNodeValue( const std::array<cvf::Vec3f, 3>& triangle, int globalElmNodeResIndex );

private:
    const std::vector<float>* tens11;
    const std::vector<float>* tens22;
    const std::vector<float>* tens33;
    const std::vector<float>* tens12;
    const std::vector<float>* tens23;
    const std::vector<float>* tens13;

    float m_tanFricAng;
    float m_cohPrTanFricAngle;

    std::function<float( const RiuGeoMechXfTensorResultAccessor&, const caf::Ten3f& )> m_tensorOperation;

    float SN( const caf::Ten3f& t ) const;
    float STH( const caf::Ten3f& t ) const;
    float STQV( const caf::Ten3f& t ) const;
    float TNH( const caf::Ten3f& t ) const;
    float TNQV( const caf::Ten3f& t ) const;
    float THQV( const caf::Ten3f& t ) const;
    float TP( const caf::Ten3f& t ) const;
    float TPinc( const caf::Ten3f& t ) const;
    float FAULTMOB( const caf::Ten3f& t ) const;
    float PCRIT( const caf::Ten3f& t ) const;
};
