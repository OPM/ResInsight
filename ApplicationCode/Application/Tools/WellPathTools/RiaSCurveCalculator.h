/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "cvfVector3.h"

class RiaSCurveCalculator
{
public:
    RiaSCurveCalculator( cvf::Vec3d p1, double azi1, double inc1, double r1, cvf::Vec3d p2, double azi2, double inc2, double r2 );

    RiaSCurveCalculator( cvf::Vec3d p1, cvf::Vec3d q1, cvf::Vec3d p2, cvf::Vec3d q2 );

    enum CurveStatus
    {
        NOT_SET,
        OK,
        OK_INFINITE_RADIUS1,
        OK_INFINITE_RADIUS2,
        OK_INFINITE_RADIUS12,
        FAILED_INPUT_OVERLAP,
        FAILED_ARC_OVERLAP
    };
    enum SolveStatus
    {
        NOT_SOLVED,
        CONVERGED,
        FAILED_MAX_LENGTH_ALONG_TANGENT_REACHED,
        FAILED_MAX_TANGENT_STEP_REACHED,
        FAILED_MAX_ITERATIONS_REACHED
    };

    bool        isOk() const { return m_isCalculationOK; }
    CurveStatus curveStatus() const { return m_ctrlPpointCurveStatus; }
    SolveStatus solveStatus() const { return m_solveStatus; }

    cvf::Vec3d firstArcEndpoint() const { return m_firstArcEndpoint; }
    cvf::Vec3d secondArcStartpoint() const { return m_secondArcStartpoint; }
    cvf::Vec3d firstCenter() const { return m_c1; }
    cvf::Vec3d secondCenter() const { return m_c2; }
    cvf::Vec3d firstNormal() const { return m_n1; }
    cvf::Vec3d secondNormal() const { return m_n2; }
    double     firstRadius() const { return m_r1; }
    double     secondRadius() const { return m_r2; }

    void dump() const;

    static RiaSCurveCalculator fromTangentsAndLength( cvf::Vec3d p1,
                                                      double     azi1,
                                                      double     inc1,
                                                      double     lengthToQ1,
                                                      cvf::Vec3d p2,
                                                      double     azi2,
                                                      double     inc2,
                                                      double     lengthToQ2 );

private:
    void initializeByFinding_q1q2( cvf::Vec3d p1,
                                   double     azi1,
                                   double     inc1,
                                   double     r1,
                                   cvf::Vec3d p2,
                                   double     azi2,
                                   double     inc2,
                                   double     r2 );

    bool m_isCalculationOK;

    CurveStatus m_ctrlPpointCurveStatus;
    SolveStatus m_solveStatus;

    cvf::Vec3d m_p1;
    cvf::Vec3d m_p2;
    cvf::Vec3d m_firstArcEndpoint;
    cvf::Vec3d m_secondArcStartpoint;
    cvf::Vec3d m_c1;
    cvf::Vec3d m_c2;
    cvf::Vec3d m_n1;
    cvf::Vec3d m_n2;
    double     m_r1;
    double     m_r2;
};
