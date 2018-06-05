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

#include "cafTensor3.h"
#include "cvfVector3.h"
#include "cvfVector4.h"

#include <vector>

class RigGeoMechBoreHoleStressCalculator
{
public:
    RigGeoMechBoreHoleStressCalculator(const caf::Ten3d& tensor, double porePressure, double poissonRatio, double uniaxialCompressiveStrength, int nThetaSubSamples);
    double solveFractureGradient(double* thetaOut = nullptr);
    double solveStassiDalia(double* thetaOut = nullptr);
    cvf::Vec3d principleStressesAtWall(double pw, double theta) const;
private:
    typedef double (RigGeoMechBoreHoleStressCalculator::*MemberFunc)(double pw, double* thetaOut) const;
    double solveBisection(double minPw, double maxPw, MemberFunc fn, double* thetaOut);
    double solveSecant(MemberFunc fn, double* thetaOut);
    double sigmaTMinOfMin(double wellPressure, double* thetaAtMin) const;
    double stassiDalia(double wellPressure, double* thetaAtMin) const;
    void calculateStressComponents();
    cvf::Vec4d calculateStressComponentsForSegmentAngle(double theta) const;

    caf::Ten3d m_tensor;
    double m_porePressure;
    double m_poissonRatio;
    double m_uniaxialCompressiveStrength;
    int m_nThetaSubSamples;
    std::vector<cvf::Vec4d> m_stressComponents;
};

