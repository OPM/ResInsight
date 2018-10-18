/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "RigWellLogExtractor.h"

#include "RigFemResultPosEnum.h"

#include "cafTensor3.h"

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfMath.h"
#include "cvfStructGrid.h"
#include "cvfVector3.h"

#include <vector>

class RigFemPart;
class RigFemResultAddress;
class RigGeoMechCaseData;
class RigWellPath;

namespace cvf {
    class BoundingBox;
}

//==================================================================================================
/// 
//==================================================================================================
class RigGeoMechWellLogExtractor : public RigWellLogExtractor
{
public:
    RigGeoMechWellLogExtractor(RigGeoMechCaseData* aCase, const RigWellPath* wellpath, const std::string& wellCaseErrorMsgName);

    void                         curveData(const RigFemResultAddress& resAddr, int frameIndex, std::vector<double>* values );
    const RigGeoMechCaseData*    caseData();
    void                         setRkbDiff(double rkbDiff);

    void                         setWellLogMdAndMudWeightKgPerM3(const std::vector<std::pair<double, double>>& mudWeightKgPerM3);
    void                         setWellLogMdAndUcsBar(const std::vector<std::pair<double, double>>& ucsValues);
    void                         setWellLogMdAndPoissonRatio(const std::vector<std::pair<double, double>>& poissonRatio);


private:
    enum WellPathTangentCalculation
    {
        TangentFollowWellPathSegments,
        TangentConstantWithinCell
    };

    float                        calculatePorePressureInSegment(int64_t intersectionIdx, float averageSegmentPorePressureBar, double hydroStaticPorePressureBar, double effectiveDepthMeters, const std::vector<float>& poreElementPressuresPascal) const;
    float                        calculatePoissonRatio(int64_t intersectionIdx, const std::vector<float>& poissonRatios) const;
    float                        calculateUcs(int64_t intersectionIdx, const std::vector<float>& ucsValuesPascal) const;

    void                         wellPathAngles(const RigFemResultAddress& resAddr, std::vector<double>* values);
    void                         wellPathScaledCurveData(const RigFemResultAddress& resAddr, int frameIndex, std::vector<double>* values);


    void                         wellBoreWallCurveData(const RigFemResultAddress& resAddr, int frameIndex, std::vector<double>* values);

    template<typename T>
    T                            interpolateGridResultValue(RigFemResultPosEnum resultPosType, const std::vector<T>& gridResultValues, int64_t intersectionIdx, bool averageNodeElementResults) const;
    size_t                       gridResultIndexFace(size_t elementIdx, cvf::StructGridInterface::FaceType cellFace, int faceLocalNodeIdx) const;
    void                         calculateIntersection();
    std::vector<size_t>          findCloseCells(const cvf::BoundingBox& bb);
    cvf::Vec3d           calculateLengthInCell(size_t cellIndex, 
                                                       const cvf::Vec3d& startPoint, 
                                                       const cvf::Vec3d& endPoint) const override;
    cvf::Vec3d                   calculateWellPathTangent(int64_t intersectionIdx, WellPathTangentCalculation calculationType) const;
    static caf::Ten3d            transformTensorToWellPathOrientation(const cvf::Vec3d& wellPathTangent,
                                                                      const caf::Ten3d& wellPathTensor);

    cvf::Vec3f                   cellCentroid(size_t intersectionIdx) const;
    double                       getWellLogSegmentValue(size_t intersectionIdx, const std::vector<std::pair<double, double>>& wellLogValues) const;

    template<typename T>
    bool                         averageIntersectionValuesToSegmentValue(size_t intersectionIdx, const std::vector<T>& intersectionValues, const T& invalidValue, T* averagedSegmentValue) const;
    static double                pascalToBar(double pascalValue);
private:
    cvf::ref<RigGeoMechCaseData> m_caseData;
    double                       m_rkbDiff;
    std::vector<std::pair<double, double>> m_wellLogMdAndMudWeightKgPerM3;
    std::vector<std::pair<double, double>> m_wellLogMdAndUcsBar;
    std::vector<std::pair<double, double>> m_wellLogMdAndPoissonRatios;

    static const double          UNIT_WEIGHT_OF_WATER;
};

