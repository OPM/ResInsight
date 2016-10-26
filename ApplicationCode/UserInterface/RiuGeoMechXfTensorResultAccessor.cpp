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
#include "cvfGeometryTools.h"
#include "RivHexGridIntersectionTools.h"



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuGeoMechXfTensorResultAccessor::RiuGeoMechXfTensorResultAccessor(RigFemPartResultsCollection * femResCollection, const RigFemResultAddress& resVarAddress, int timeStepIdx)
{
    RigFemResultAddress tensComp = resVarAddress;
    tensComp.resultPosType = RIG_ELEMENT_NODAL;

    tensComp.componentName = "S11";
    tens11 = &femResCollection->resultValues(tensComp, 0, timeStepIdx);
    tensComp.componentName = "S22";
    tens22 = &femResCollection->resultValues(tensComp, 0, timeStepIdx);
    tensComp.componentName = "S33";
    tens33 = &femResCollection->resultValues(tensComp, 0, timeStepIdx);
    tensComp.componentName = "S12";
    tens12 = &femResCollection->resultValues(tensComp, 0, timeStepIdx);
    tensComp.componentName = "S23";
    tens23 = &femResCollection->resultValues(tensComp, 0, timeStepIdx);
    tensComp.componentName = "S13";
    tens13 = &femResCollection->resultValues(tensComp, 0, timeStepIdx);

    resultComponent = caf::Ten3f::SZZ;

    if ( resVarAddress.componentName == "SN" ) resultComponent = caf::Ten3f::SZZ;
    if ( resVarAddress.componentName == "STH" ) resultComponent = caf::Ten3f::SXX;
    if ( resVarAddress.componentName == "STQV" ) resultComponent = caf::Ten3f::SYY;
    if ( resVarAddress.componentName == "TNH" )  resultComponent = caf::Ten3f::SZX;
    if ( resVarAddress.componentName == "TNQV" ) resultComponent = caf::Ten3f::SYZ;
    if ( resVarAddress.componentName == "THQV" ) resultComponent = caf::Ten3f::SXY;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuGeoMechXfTensorResultAccessor::calculateInterpolatedValue(const cvf::Vec3f triangle[3], const RivIntersectionVertexWeights vertexWeights[3], float returnValues[3])
{
    if ( tens11->size() == 0 )
    {
        returnValues[0] = returnValues[1] = returnValues[2] = std::numeric_limits<float>::infinity();
        return;
    }

    cvf::Mat3f triangleXf = cvf::GeometryTools::computePlaneHorizontalRotationMx(triangle[1] - triangle[0], triangle[2] - triangle[0]);

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
            size_t resIdx = vertexWeights[triangleVxIdx].vxId(wIdx) ;
            float interpolationWeight = vertexWeights[triangleVxIdx].weight(wIdx);
            ipT11 += (*tens11)[resIdx] * interpolationWeight;
            ipT22 += (*tens22)[resIdx] * interpolationWeight;
            ipT33 += (*tens33)[resIdx] * interpolationWeight;
            ipT12 += (*tens12)[resIdx] * interpolationWeight;
            ipT23 += (*tens23)[resIdx] * interpolationWeight;
            ipT13 += (*tens13)[resIdx] * interpolationWeight;
        }

        if ( ipT11 == HUGE_VAL || ipT11 != ipT11
            || ipT22 == HUGE_VAL || ipT22 != ipT22
            || ipT33 == HUGE_VAL || ipT33 != ipT33
            || ipT12 == HUGE_VAL || ipT12 != ipT12
            || ipT23 == HUGE_VAL || ipT23 != ipT23
            || ipT13 == HUGE_VAL || ipT13 != ipT13 ) // a != a is true for NAN's
        {
            returnValues[triangleVxIdx] = std::numeric_limits<float>::infinity();
        }
        else
        {
            caf::Ten3f tensor(ipT11, ipT22, ipT33,
                              ipT12, ipT23, ipT13);
            caf::Ten3f xfTen = tensor.rotated(triangleXf);

            returnValues[triangleVxIdx] = xfTen[resultComponent];
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
float RiuGeoMechXfTensorResultAccessor::calculateElmNodeValue(const std::array<cvf::Vec3f, 3> & triangle, int globalElmNodeResIndex)
{
    if ( tens11->size() == 0 ) return std::numeric_limits<float>::infinity();

    cvf::Mat3f triangleXf = cvf::GeometryTools::computePlaneHorizontalRotationMx(triangle[1] - triangle[0], triangle[2] - triangle[0]);

    float ipT11 = (*tens11)[globalElmNodeResIndex];
    float ipT22 = (*tens22)[globalElmNodeResIndex];
    float ipT33 = (*tens33)[globalElmNodeResIndex];
    float ipT12 = (*tens12)[globalElmNodeResIndex];
    float ipT23 = (*tens23)[globalElmNodeResIndex];
    float ipT13 = (*tens13)[globalElmNodeResIndex];

    if ( ipT11 == HUGE_VAL || ipT11 != ipT11
        || ipT22 == HUGE_VAL || ipT22 != ipT22
        || ipT33 == HUGE_VAL || ipT33 != ipT33
        || ipT12 == HUGE_VAL || ipT12 != ipT12
        || ipT23 == HUGE_VAL || ipT23 != ipT23
        || ipT13 == HUGE_VAL || ipT13 != ipT13 ) // a != a is true for NAN's
    {
        return std::numeric_limits<float>::infinity();
    }
    else
    {
        caf::Ten3f tensor(ipT11, ipT22, ipT33,
                          ipT12, ipT23, ipT13);
        caf::Ten3f xfTen = tensor.rotated(triangleXf);

        float scalarValue = xfTen[resultComponent];

        return scalarValue;
    }
}
