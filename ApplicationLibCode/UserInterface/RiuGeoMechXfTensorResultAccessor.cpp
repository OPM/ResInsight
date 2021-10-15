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

#include "RiuGeoMechXfTensorResultAccessor.h"
#include "RigFemPartResultsCollection.h"
#include "RivIntersectionVertexWeights.h"
#include "cvfAssert.h"
#include "cvfGeometryTools.h"

float RiuGeoMechXfTensorResultAccessor::SN( const caf::Ten3f& t ) const
{
    return t[caf::Ten3f::SZZ];
}
float RiuGeoMechXfTensorResultAccessor::STH( const caf::Ten3f& t ) const
{
    return t[caf::Ten3f::SXX];
}
float RiuGeoMechXfTensorResultAccessor::STQV( const caf::Ten3f& t ) const
{
    return t[caf::Ten3f::SYY];
}
float RiuGeoMechXfTensorResultAccessor::TNH( const caf::Ten3f& t ) const
{
    return t[caf::Ten3f::SZX];
}
float RiuGeoMechXfTensorResultAccessor::TNQV( const caf::Ten3f& t ) const
{
    return t[caf::Ten3f::SYZ];
}
float RiuGeoMechXfTensorResultAccessor::THQV( const caf::Ten3f& t ) const
{
    return t[caf::Ten3f::SXY];
}

float RiuGeoMechXfTensorResultAccessor::TP( const caf::Ten3f& t ) const
{
    float szx = t[caf::Ten3f::SZX];
    float szy = t[caf::Ten3f::SYZ];
    float tp  = sqrt( szx * szx + szy * szy );

    return tp;
}

float RiuGeoMechXfTensorResultAccessor::TPinc( const caf::Ten3f& t ) const
{
    float szy = t[caf::Ten3f::SYZ];

    float tp = TP( t );

    if ( tp > 1e-5 )
    {
        return cvf::Math::toDegrees( acos( szy / tp ) );
    }
    else
    {
        return std::numeric_limits<float>::infinity();
    }
}

float RiuGeoMechXfTensorResultAccessor::FAULTMOB( const caf::Ten3f& t ) const
{
    float szz = t[caf::Ten3f::SZZ];
    float tp  = TP( t );

    return tp / ( m_tanFricAng * ( szz + m_cohPrTanFricAngle ) );
}

float RiuGeoMechXfTensorResultAccessor::PCRIT( const caf::Ten3f& t ) const
{
    float szz = t[caf::Ten3f::SZZ];
    float tp  = TP( t );

    return szz - tp / m_tanFricAng;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuGeoMechXfTensorResultAccessor::RiuGeoMechXfTensorResultAccessor( RigFemPartResultsCollection* femResCollection,
                                                                    const RigFemResultAddress&   resVarAddress,
                                                                    int                          timeStepIdx )
{
    RigFemResultAddress tensComp = resVarAddress;
    tensComp.resultPosType       = RIG_ELEMENT_NODAL;

    tensComp.componentName = "S11";
    tens11                 = &femResCollection->resultValues( tensComp, 0, timeStepIdx );
    tensComp.componentName = "S22";
    tens22                 = &femResCollection->resultValues( tensComp, 0, timeStepIdx );
    tensComp.componentName = "S33";
    tens33                 = &femResCollection->resultValues( tensComp, 0, timeStepIdx );
    tensComp.componentName = "S12";
    tens12                 = &femResCollection->resultValues( tensComp, 0, timeStepIdx );
    tensComp.componentName = "S23";
    tens23                 = &femResCollection->resultValues( tensComp, 0, timeStepIdx );
    tensComp.componentName = "S13";
    tens13                 = &femResCollection->resultValues( tensComp, 0, timeStepIdx );

    if ( resVarAddress.componentName == "SN" )
    {
        m_tensorOperation = &RiuGeoMechXfTensorResultAccessor::SN;
    }
    if ( resVarAddress.componentName == "STH" )
    {
        m_tensorOperation = &RiuGeoMechXfTensorResultAccessor::STH;
    }
    if ( resVarAddress.componentName == "STQV" )
    {
        m_tensorOperation = &RiuGeoMechXfTensorResultAccessor::STQV;
    }
    if ( resVarAddress.componentName == "TPH" )
    {
        m_tensorOperation = &RiuGeoMechXfTensorResultAccessor::TNH;
    }
    if ( resVarAddress.componentName == "TPQV" )
    {
        m_tensorOperation = &RiuGeoMechXfTensorResultAccessor::TNQV;
    }
    if ( resVarAddress.componentName == "THQV" )
    {
        m_tensorOperation = &RiuGeoMechXfTensorResultAccessor::THQV;
    }
    if ( resVarAddress.componentName == "TP" )
    {
        m_tensorOperation = &RiuGeoMechXfTensorResultAccessor::TP;
    }
    if ( resVarAddress.componentName == "TPinc" )
    {
        m_tensorOperation = &RiuGeoMechXfTensorResultAccessor::TPinc;
    }
    if ( resVarAddress.componentName == "FAULTMOB" )
    {
        m_tensorOperation = &RiuGeoMechXfTensorResultAccessor::FAULTMOB;
    }
    if ( resVarAddress.componentName == "PCRIT" )
    {
        m_tensorOperation = &RiuGeoMechXfTensorResultAccessor::PCRIT;
    }

    m_tanFricAng        = tan( (float)femResCollection->parameterFrictionAngleRad() );
    m_cohPrTanFricAngle = (float)( (float)femResCollection->parameterCohesion() / m_tanFricAng );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGeoMechXfTensorResultAccessor::calculateInterpolatedValue( const cvf::Vec3f                   triangle[3],
                                                                   const RivIntersectionVertexWeights vertexWeights[3],
                                                                   float                              returnValues[3] )
{
    if ( tens11->size() == 0 )
    {
        returnValues[0] = returnValues[1] = returnValues[2] = std::numeric_limits<float>::infinity();
        return;
    }

    cvf::Mat3f triangleXf =
        cvf::GeometryTools::computePlaneHorizontalRotationMx( triangle[1] - triangle[0], triangle[2] - triangle[0] );

    for ( int triangleVxIdx = 0; triangleVxIdx < 3; ++triangleVxIdx )
    {
        float ipT11 = 0;
        float ipT22 = 0;
        float ipT33 = 0;
        float ipT12 = 0;
        float ipT23 = 0;
        float ipT13 = 0;

        int weightCount = vertexWeights[triangleVxIdx].size();
        for ( int wIdx = 0; wIdx < weightCount; ++wIdx )
        {
            size_t resIdx              = vertexWeights[triangleVxIdx].vxId( wIdx );
            float  interpolationWeight = vertexWeights[triangleVxIdx].weight( wIdx );
            ipT11 += ( *tens11 )[resIdx] * interpolationWeight;
            ipT22 += ( *tens22 )[resIdx] * interpolationWeight;
            ipT33 += ( *tens33 )[resIdx] * interpolationWeight;
            ipT12 += ( *tens12 )[resIdx] * interpolationWeight;
            ipT23 += ( *tens23 )[resIdx] * interpolationWeight;
            ipT13 += ( *tens13 )[resIdx] * interpolationWeight;
        }

        if ( ipT11 == HUGE_VAL || ipT11 != ipT11 || ipT22 == HUGE_VAL || ipT22 != ipT22 || ipT33 == HUGE_VAL ||
             ipT33 != ipT33 || ipT12 == HUGE_VAL || ipT12 != ipT12 || ipT23 == HUGE_VAL || ipT23 != ipT23 ||
             ipT13 == HUGE_VAL || ipT13 != ipT13 ) // a != a is true for NAN's
        {
            returnValues[triangleVxIdx] = std::numeric_limits<float>::infinity();
        }
        else
        {
            caf::Ten3f tensor( ipT11, ipT22, ipT33, ipT12, ipT23, ipT13 );
            caf::Ten3f xfTen = tensor.rotated( triangleXf );

            returnValues[triangleVxIdx] = m_tensorOperation( *this, xfTen );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
float RiuGeoMechXfTensorResultAccessor::calculateElmNodeValue( const std::array<cvf::Vec3f, 3>& triangle,
                                                               int                              globalElmNodeResIndex )
{
    if ( tens11->size() == 0 ) return std::numeric_limits<float>::infinity();

    cvf::Mat3f triangleXf =
        cvf::GeometryTools::computePlaneHorizontalRotationMx( triangle[1] - triangle[0], triangle[2] - triangle[0] );

    float ipT11 = ( *tens11 )[globalElmNodeResIndex];
    float ipT22 = ( *tens22 )[globalElmNodeResIndex];
    float ipT33 = ( *tens33 )[globalElmNodeResIndex];
    float ipT12 = ( *tens12 )[globalElmNodeResIndex];
    float ipT23 = ( *tens23 )[globalElmNodeResIndex];
    float ipT13 = ( *tens13 )[globalElmNodeResIndex];

    if ( ipT11 == HUGE_VAL || ipT11 != ipT11 || ipT22 == HUGE_VAL || ipT22 != ipT22 || ipT33 == HUGE_VAL ||
         ipT33 != ipT33 || ipT12 == HUGE_VAL || ipT12 != ipT12 || ipT23 == HUGE_VAL || ipT23 != ipT23 ||
         ipT13 == HUGE_VAL || ipT13 != ipT13 ) // a != a is true for NAN's
    {
        return std::numeric_limits<float>::infinity();
    }
    else
    {
        caf::Ten3f tensor( ipT11, ipT22, ipT33, ipT12, ipT23, ipT13 );
        caf::Ten3f xfTen = tensor.rotated( triangleXf );

        float scalarValue = m_tensorOperation( *this, xfTen );
        return scalarValue;
    }
}
