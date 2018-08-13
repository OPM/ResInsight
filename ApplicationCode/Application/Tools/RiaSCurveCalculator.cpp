/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include "RiaSCurveCalculator.h"

#include "RiaOffshoreSphericalCoords.h"

#include "SolveSpaceSystem.h"
#include <cmath>
#include "cvfMatrix4.h"
#include <iostream>
#include <algorithm>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaSCurveCalculator::RiaSCurveCalculator(cvf::Vec3d p1, double azi1, double inc1, double rad1, 
                                         cvf::Vec3d p2, double azi2, double inc2, double rad2)
                                         : m_isCalculationOK(false)
                                         , m_p1(p1)
                                         , m_p2(p2)
                                         , m_firstArcEndpoint(p1 + 0.3*(p2-p1))
                                         , m_secondArcStartpoint(p1 + 0.6*(p2-p1))
                                         , m_r1(rad1)
                                         , m_r2(rad2)
{
    // Estimate 

    double est_p_c1x  =  10.0;
    double est_p_c1y  =   2.0;
    double est_p_p11x =   2.0;
    double est_p_p11y = -10.0;

    double est_p_c2x  =  10.0;
    double est_p_c2y  =   2.0;
    double est_p_p22x =   2.0;
    double est_p_p22y = -10.0;

    double est_p_Plane1Qw = 0.0;
    double est_p_Plane1Qx = 0.0;
    double est_p_Plane1Qy = 0.0;
    double est_p_Plane1Qz = 0.0;

    double est_p_Plane2Qw = 0.0;
    double est_p_Plane2Qx = 0.0;
    double est_p_Plane2Qy = 0.0;
    double est_p_Plane2Qz = 0.0;

    Slvs_MakeQuaternion(1, 0, 0,
                        0, 1, 0, 
                        &est_p_Plane1Qw, 
                        &est_p_Plane1Qx, 
                        &est_p_Plane1Qy, 
                        &est_p_Plane1Qz);
    Slvs_MakeQuaternion(1, 0, 0,
                        0, 1, 0, 
                        &est_p_Plane2Qw, 
                        &est_p_Plane2Qx, 
                        &est_p_Plane2Qy, 
                        &est_p_Plane2Qz);
    double est_rad1 = rad1;
    double est_rad2 = rad2;


    if (true)
    {
        cvf::Vec3d p1p2 = p2 - p1;

        double p1p2Length =  (p1p2).length();
        RiaSCurveCalculator estimatedCurveCalc = RiaSCurveCalculator::fromTangentsAndLength(p1, azi1, inc1, 0.2 * p1p2Length,
                                                                                            p2, azi2, inc2, 0.2 * p1p2Length);

        est_rad1 = estimatedCurveCalc.firstRadius() ;
        est_rad2 = estimatedCurveCalc.secondRadius();

        if (est_rad1 >= 1e10 || est_rad2 >= 1e10)
        {
            return;
        }

        #if 1
        std::cout << "Estimate:"  << std::endl;
        estimatedCurveCalc.dump();
        #endif

        cvf::Vec3d t1(RiaOffshoreSphericalCoords::unitVectorFromAziInc(azi1,inc1)); 
        cvf::Vec3d t2(RiaOffshoreSphericalCoords::unitVectorFromAziInc(azi2,inc2));

        cvf::Vec3d est_tp1c1 = (estimatedCurveCalc.firstCenter() - p1).getNormalized();
        cvf::Vec3d est_tp2c2 = (estimatedCurveCalc.secondCenter() - p2).getNormalized();

        cvf::Mat4d mx1 =  cvf::Mat4d::fromCoordSystemAxes(&t1, &est_tp1c1, nullptr );
        mx1.setTranslation(p1);
        cvf::Vec3d est_p11 = estimatedCurveCalc.firstArcEndpoint();
        est_p11.transformPoint(mx1.getInverted());
        CVF_ASSERT(fabs(est_p11.z()) < 1e-4 );
        cvf::Mat4d mx2 =  cvf::Mat4d::fromCoordSystemAxes(&t2, &est_tp2c2, nullptr );
        mx2.setTranslation(p2);
        cvf::Vec3d est_p22 = estimatedCurveCalc.secondArcStartpoint();
        est_p22.transformPoint(mx2.getInverted());
        CVF_ASSERT(fabs(est_p22.z()) < 1e-4 );

        est_p_c1x  =  0.0; 
        est_p_c1y  = estimatedCurveCalc.m_r1; 
        est_p_p11x = est_p11.x(); 
        est_p_p11y = est_p11.y(); 

        est_p_c2x  =  0.0;
        est_p_c2y  =  estimatedCurveCalc.m_r2;
        est_p_p22x = est_p22.x();
        est_p_p22y = est_p22.y();

        Slvs_MakeQuaternion(t1.x(), t1.y(), t1.z(),
                            est_tp1c1.x(), est_tp1c1.y(), est_tp1c1.z(),
                            &est_p_Plane1Qw, &est_p_Plane1Qx, &est_p_Plane1Qy, &est_p_Plane1Qz);
        Slvs_MakeQuaternion(t2.x(), t2.y(), t2.z(),
                            est_tp2c2.x(), est_tp2c2.y(), est_tp2c2.z(),
                            &est_p_Plane2Qw, &est_p_Plane2Qx, &est_p_Plane2Qy, &est_p_Plane2Qz);
    }

    // 
    SolveSpaceSystem sys;

    Slvs_hGroup group1 = 1;
    Slvs_hGroup group2 = 2;

    ///////////////////////////////////////////////////////////////////////////
    // Group 1, Fixed

    // P1
    Slvs_hParam p_p1x  = sys.addParam(Slvs_MakeParam(-1, group1, p1.x()));
    Slvs_hParam p_p1y  = sys.addParam(Slvs_MakeParam(-1, group1, p1.y()));
    Slvs_hParam p_p1z  = sys.addParam(Slvs_MakeParam(-1, group1, p1.z()));

    Slvs_hEntity e_P1 = sys.addEntity(Slvs_MakePoint3d(-1, group1, p_p1x, p_p1y, p_p1z));

    // PT1
    double pt1x = p1.x() + sin(azi1)*sin(inc1);
    double pt1y = p1.y() + cos(azi1)*sin(inc1);
    double pt1z = p1.z() - cos(inc1);

    Slvs_hParam p_pt1x  = sys.addParam(Slvs_MakeParam(-1, group1, pt1x));
    Slvs_hParam p_pt1y  = sys.addParam(Slvs_MakeParam(-1, group1, pt1y));
    Slvs_hParam p_pt1z  = sys.addParam(Slvs_MakeParam(-1, group1, pt1z));

    Slvs_hEntity e_PT1 = sys.addEntity(Slvs_MakePoint3d(-1, group1, p_pt1x, p_pt1y, p_pt1z));

    // Tangent Line 1

    Slvs_hEntity e_LT1 = sys.addEntity(Slvs_MakeLineSegment(-1, group1, SLVS_FREE_IN_3D, e_P1, e_PT1));

    // P2
    Slvs_hParam p_p2x  = sys.addParam(Slvs_MakeParam(-1, group1, p2.x()));
    Slvs_hParam p_p2y  = sys.addParam(Slvs_MakeParam(-1, group1, p2.y()));
    Slvs_hParam p_p2z  = sys.addParam(Slvs_MakeParam(-1, group1, p2.z()));

    Slvs_hEntity e_P2 = sys.addEntity(Slvs_MakePoint3d(-1, group1, p_p2x, p_p2y, p_p2z));

    // PT2
    double pt2x = p2.x() + sin(azi2)*sin(inc2);
    double pt2y = p2.y() + cos(azi2)*sin(inc2);
    double pt2z = p2.z() - cos(inc2);

    Slvs_hParam p_pt2x  = sys.addParam(Slvs_MakeParam(-1, group1, pt2x));
    Slvs_hParam p_pt2y  = sys.addParam(Slvs_MakeParam(-1, group1, pt2y));
    Slvs_hParam p_pt2z  = sys.addParam(Slvs_MakeParam(-1, group1, pt2z));

    Slvs_hEntity e_PT2 = sys.addEntity(Slvs_MakePoint3d(-1, group1, p_pt2x, p_pt2y, p_pt2z));

    // Tangent Line 2

    Slvs_hEntity e_LT2 = sys.addEntity(Slvs_MakeLineSegment(-1, group1, SLVS_FREE_IN_3D, e_P2, e_PT2));

    //
    /////////////////////////////////////////////////////////////////////////


    // Plane1


    // Plane 1

    Slvs_hParam p_Plane1Qw = sys.addParam(Slvs_MakeParam(-1, group2, est_p_Plane1Qw));
    Slvs_hParam p_Plane1Qx = sys.addParam(Slvs_MakeParam(-1, group2, est_p_Plane1Qx));
    Slvs_hParam p_Plane1Qy = sys.addParam(Slvs_MakeParam(-1, group2, est_p_Plane1Qy));
    Slvs_hParam p_Plane1Qz = sys.addParam(Slvs_MakeParam(-1, group2, est_p_Plane1Qz));
    Slvs_hEntity e_Plane1Q = sys.addEntity(Slvs_MakeNormal3d(-1, group2,
                                                             p_Plane1Qw,
                                                             p_Plane1Qx,
                                                             p_Plane1Qy,
                                                             p_Plane1Qz));
    Slvs_hEntity e_Plane1 = sys.addEntity(Slvs_MakeWorkplane(-1, group2, e_P1, e_Plane1Q));

    Slvs_hConstraint c_PT1Plane1 = sys.addConstr(Slvs_MakeConstraint(-1,
                                                                     group2,
                                                                     SLVS_C_PT_IN_PLANE,
                                                                     SLVS_FREE_IN_3D,
                                                                     0.0,
                                                                     e_PT1,
                                                                     -1,
                                                                     e_Plane1,
                                                                     -1));
    // Arc1 center
    Slvs_hParam p_c1x  = sys.addParam(Slvs_MakeParam(-1, group2, est_p_c1x)); // Needs a better guess 
    Slvs_hParam p_c1y  = sys.addParam(Slvs_MakeParam(-1, group2, est_p_c1y));

    Slvs_hEntity e_C1 = sys.addEntity(Slvs_MakePoint2d(-1, group2, e_Plane1, p_c1x, p_c1y));

    Slvs_hEntity e_LP1C1 = sys.addEntity(Slvs_MakeLineSegment(-1, group2, e_Plane1, e_P1, e_C1));

    Slvs_hConstraint c_perpT1_LP1C1 = sys.addConstr(Slvs_MakeConstraint(-1,
                                                                        group2,
                                                                        SLVS_C_PERPENDICULAR,
                                                                        e_Plane1,
                                                                        0.0,
                                                                        -1,
                                                                        -1,
                                                                        e_LT1,
                                                                        e_LP1C1));

    Slvs_hConstraint c_dist_P1C1 = sys.addConstr(Slvs_MakeConstraint(-1,
                                                                     group2,
                                                                     SLVS_C_PT_PT_DISTANCE,
                                                                     e_Plane1,
                                                                     est_rad1,
                                                                     e_P1,
                                                                     e_C1,
                                                                     -1,
                                                                     -1));

    // Arc1 end

    Slvs_hParam p_p11x  = sys.addParam(Slvs_MakeParam(-1, group2, est_p_p11x)); // Needs a better guess: Perp on p_c1x/p_c1y
    Slvs_hParam p_p11y  = sys.addParam(Slvs_MakeParam(-1, group2, est_p_p11y));

    Slvs_hEntity e_P11 = sys.addEntity(Slvs_MakePoint2d(-1, group2, e_Plane1, p_p11x, p_p11y));

    Slvs_hEntity e_LC1P11 = sys.addEntity(Slvs_MakeLineSegment(-1, group2, e_Plane1, e_C1, e_P11));


    Slvs_hConstraint c_dist_C1P11 = sys.addConstr(Slvs_MakeConstraint(-1,
                                                                      group2,
                                                                      SLVS_C_EQUAL_LENGTH_LINES,
                                                                      e_Plane1,
                                                                      0.0,
                                                                      -1,
                                                                      -1,
                                                                      e_LP1C1,
                                                                      e_LC1P11));





    // Plane 2

    Slvs_hParam p_Plane2Qw = sys.addParam(Slvs_MakeParam(-1, group2, est_p_Plane2Qw));
    Slvs_hParam p_Plane2Qx = sys.addParam(Slvs_MakeParam(-1, group2, est_p_Plane2Qx));
    Slvs_hParam p_Plane2Qy = sys.addParam(Slvs_MakeParam(-1, group2, est_p_Plane2Qy));
    Slvs_hParam p_Plane2Qz = sys.addParam(Slvs_MakeParam(-1, group2, est_p_Plane2Qz));
    Slvs_hEntity e_Plane2Q = sys.addEntity(Slvs_MakeNormal3d(-1, group2,
                                                             p_Plane2Qw,
                                                             p_Plane2Qx,
                                                             p_Plane2Qy,
                                                             p_Plane2Qz));

    Slvs_hEntity e_Plane2 = sys.addEntity(Slvs_MakeWorkplane(-1, group2, e_P2, e_Plane2Q));

    Slvs_hConstraint c_PT2Plane2 = sys.addConstr(Slvs_MakeConstraint(-1,
                                                                     group2,
                                                                     SLVS_C_PT_IN_PLANE,
                                                                     SLVS_FREE_IN_3D,
                                                                     0.0,
                                                                     e_PT2,
                                                                     -1,
                                                                     e_Plane2,
                                                                     -1));

    // Arc2 center

    Slvs_hParam p_c2x  = sys.addParam(Slvs_MakeParam(-1, group2, est_p_c2x)); 
    Slvs_hParam p_c2y  = sys.addParam(Slvs_MakeParam(-1, group2, est_p_c2y));

    Slvs_hEntity e_C2 = sys.addEntity(Slvs_MakePoint2d(-1, group2, e_Plane2, p_c2x, p_c2y));

    Slvs_hEntity e_LP2C2 = sys.addEntity(Slvs_MakeLineSegment(-1, group2, e_Plane2, e_P2, e_C2));

    Slvs_hConstraint c_perpT2_LP2C2 = sys.addConstr(Slvs_MakeConstraint(-1,
                                                                        group2,
                                                                        SLVS_C_PERPENDICULAR,
                                                                        e_Plane2,
                                                                        0.0,
                                                                        -1,
                                                                        -1,
                                                                        e_LT2,
                                                                        e_LP2C2));

    Slvs_hConstraint c_dist_P2C2 = sys.addConstr(Slvs_MakeConstraint(-1,
                                                                     group2,
                                                                     SLVS_C_PT_PT_DISTANCE,
                                                                     e_Plane2,
                                                                     est_rad2,
                                                                     e_P2,
                                                                     e_C2,
                                                                     -1,
                                                                     -1));

    // Arc2 end

    Slvs_hParam p_p22x  = sys.addParam(Slvs_MakeParam(-1, group2, est_p_p22x)); // Needs a better guess: Perp on p_c1x/p_c1y
    Slvs_hParam p_p22y  = sys.addParam(Slvs_MakeParam(-1, group2, est_p_p22y));

    Slvs_hEntity e_P22 = sys.addEntity(Slvs_MakePoint2d(-1, group2, e_Plane2, p_p22x, p_p22y));

    Slvs_hEntity e_LC2P22 = sys.addEntity(Slvs_MakeLineSegment(-1, group2, e_Plane2, e_C2, e_P22));


    Slvs_hConstraint c_dist_C2P22 = sys.addConstr(Slvs_MakeConstraint(-1,
                                                                      group2,
                                                                      SLVS_C_EQUAL_LENGTH_LINES,
                                                                      e_Plane2,
                                                                      0.0,
                                                                      -1,
                                                                      -1,
                                                                      e_LP2C2,
                                                                      e_LC2P22));

    SolveSpaceSystem::ResultStatus solveResult;
    #if 0

    solveResult = sys.solve(group2, true);
    
    if(solveResult != SolveSpaceSystem::RESULT_OKAY) 
    {
        return;
    }
    #endif

    // Connecting the two planes

    // Connecting line
    Slvs_hEntity e_LP11P22 = sys.addEntity(Slvs_MakeLineSegment(-1, group2, SLVS_FREE_IN_3D, e_P11, e_P22));

    // Perpendicular constraints

    Slvs_hConstraint c_perpC1P11_LP11P22 = sys.addConstr(Slvs_MakeConstraint(-1,
                                                                             group2,
                                                                             SLVS_C_PERPENDICULAR,
                                                                             SLVS_FREE_IN_3D,
                                                                             0.0,
                                                                             -1,
                                                                             -1,
                                                                             e_LC1P11,
                                                                             e_LP11P22));
    #if 0

    solveResult = sys.solve(group2, true);
    
    if(solveResult != SolveSpaceSystem::RESULT_OKAY) 
    {
        return;
    }
    #endif



    Slvs_hConstraint c_perpC2P22_LP11P22 = sys.addConstr(Slvs_MakeConstraint(-1,
                                                                             group2,
                                                                             SLVS_C_PERPENDICULAR,
                                                                             SLVS_FREE_IN_3D,
                                                                             0.0,
                                                                             -1,
                                                                             -1,
                                                                             e_LC2P22,
                                                                             e_LP11P22));
    #if 0

    solveResult = sys.solve(group2, true);
    
    if(solveResult != SolveSpaceSystem::RESULT_OKAY) 
    {
        return;
    }
    #endif


    // P11, P22 in plane constraints

    Slvs_hConstraint c_P11InPlane2 = sys.addConstr(Slvs_MakeConstraint(-1,
                                                                       group2,
                                                                       SLVS_C_PT_IN_PLANE,
                                                                       SLVS_FREE_IN_3D,
                                                                       0.0,
                                                                       e_P11,
                                                                       -1,
                                                                       e_Plane2,
                                                                       -1));
    #if 0
    solveResult = sys.solve(group2, true);
 
    if(solveResult != SolveSpaceSystem::RESULT_OKAY) 
    {
        return;
    }
    #endif

    Slvs_hConstraint c_P22InPlane1 = sys.addConstr(Slvs_MakeConstraint(-1,
                                                                       group2,
                                                                       SLVS_C_PT_IN_PLANE,
                                                                       SLVS_FREE_IN_3D,
                                                                       0.0,
                                                                       e_P22,
                                                                       -1,
                                                                       e_Plane1,
                                                                       -1));

    
    m_isCalculationOK = true;
    #if 0
    std::cout << std::endl;
    for ( int iter = 0; iter < 2; ++iter )
    {
        double newRad1 = est_rad1 - iter*1.0*(est_rad1 - rad1);
        double newRad2 = est_rad2 - iter*1.0*(est_rad2 - rad2);

        sys.constraint(c_dist_P1C1).valA = newRad1;
        sys.constraint(c_dist_P2C2).valA = newRad2;

        solveResult = sys.solve(group2, true);

        if ( solveResult != SolveSpaceSystem::RESULT_OKAY )
        {
            std::cout << std::endl;
            m_isCalculationOK = false;
            if (iter > 0)
                break;
            else
                return;
        }
        std::cout << iter ;
    }
    std::cout << std::endl;

    #else
    std::cout << std::endl;

    // Initial solve using the precalculated estimated curve

    solveResult = sys.solve(group2, true);
    if ( solveResult != SolveSpaceSystem::RESULT_OKAY )
    {
        std::cout << std::endl;
        m_isCalculationOK = false;
        return;
    }
    
    // Change radius from estimate towards the radii provided in steps.
    // Try all in one go first. 
    // if solution diverges, reduce step by half until the solution converges.
    // Keep stepsize one step before trying to double it. 
    
    double currentRadius1 = est_rad1;
    double currentRadius2 = est_rad2;
    double nextStepR1 = rad1 - currentRadius1;
    double nextStepR2 = rad2 - currentRadius2;
    int iter = 0; int maxIter = 12;
    bool isIncreaseStepOk = false;
    bool hasReachedRadiusTargets = false;

    while ( iter < maxIter 
            && !hasReachedRadiusTargets )
    {
        
        double newRad1 = currentRadius1 + nextStepR1;
        double newRad2 = currentRadius2 + nextStepR2;

        sys.constraint(c_dist_P1C1).valA = newRad1;
        sys.constraint(c_dist_P2C2).valA = newRad2;

        solveResult = sys.solve(group2, true);

        iter++;
        std::cout << iter ;
        if ( solveResult != SolveSpaceSystem::RESULT_OKAY )
        {
            nextStepR1 = 0.5* nextStepR1;
            nextStepR2 = 0.5* nextStepR2;
            isIncreaseStepOk = false;
            std::cout << "-";
        }
        else
        {
            currentRadius1 = newRad1;
            currentRadius2 = newRad2;
            if ( isIncreaseStepOk )
            {
                nextStepR1 = std::min(2*nextStepR1, rad1 - currentRadius1);
                nextStepR2 = std::min(2*nextStepR2, rad2 - currentRadius2);
                std::cout << "++";

            }
            else
            {
                nextStepR1 = std::min(nextStepR1, rad1 - currentRadius1);
                nextStepR2 = std::min(nextStepR2, rad2 - currentRadius2);
                isIncreaseStepOk = true;
                std::cout << "+";
            }
        }
        hasReachedRadiusTargets =  (    fabs(currentRadius1 - rad1) < 1e-5 
                                     && fabs(currentRadius2 - rad2) < 1e-5);
    }

    m_isCalculationOK = (hasReachedRadiusTargets && solveResult == SolveSpaceSystem::RESULT_OKAY);
    
    std::cout << std::endl;
    #endif

    // Circle Center, Plane normals, P11, P22

    std::valarray<double> v_C1 = sys.global3DPos(e_C1);
    m_c1[0] = v_C1[0];
    m_c1[1] = v_C1[1];
    m_c1[2] = v_C1[2];

    std::valarray<double> v_C2 = sys.global3DPos(e_C2);
    m_c2[0] = v_C2[0];
    m_c2[1] = v_C2[1];
    m_c2[2] = v_C2[2];

    std::valarray<double> v_N1 = std::get<2>(sys.orientationMx(e_Plane1Q));
    m_n1[0] = v_N1[0];
    m_n1[1] = v_N1[1];
    m_n1[2] = v_N1[2];

    std::valarray<double> v_N2 = std::get<2>(sys.orientationMx(e_Plane2Q));
    m_n2[0] = v_N2[0];
    m_n2[1] = v_N2[1];
    m_n2[2] = v_N2[2];

    std::valarray<double> v_P11 = sys.global3DPos(e_P11);
    m_firstArcEndpoint[0] = v_P11[0];
    m_firstArcEndpoint[1] = v_P11[1];
    m_firstArcEndpoint[2] = v_P11[2];

    std::valarray<double> v_P22 = sys.global3DPos(e_P22);
    m_secondArcStartpoint[0] = v_P22[0];
    m_secondArcStartpoint[1] = v_P22[1];
    m_secondArcStartpoint[2] = v_P22[2];

    m_r1 = (m_c1 - m_p1).length();
    m_r2 = (m_c2 - m_p2).length();

    // Validate solution
    // Normal1 x C1P11 == tP11P22
    // Normal2 x C2P22 == tP11P22

    cvf::Vec3d tP11P22 = (m_secondArcStartpoint - m_firstArcEndpoint).getNormalized();

   double error1 = ((m_n1 ^ (m_firstArcEndpoint - m_c1).getNormalized() ) -  tP11P22).lengthSquared();
   double error2 = ((m_n2 ^ (m_secondArcStartpoint - m_c2).getNormalized() ) -  tP11P22).lengthSquared();

    if (    error1 > 1e-9 && error2 > 1e-9 )
    {
        // Solution is invalid. The line is not continuing the arcs in the right direction
        m_isCalculationOK =  false;
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaSCurveCalculator::RiaSCurveCalculator(cvf::Vec3d p1, cvf::Vec3d q1, 
                                         cvf::Vec3d p2, cvf::Vec3d q2)
    : m_isCalculationOK(true)
    , m_p1(p1)
    , m_p2(p2)
{
    using Vec3d = cvf::Vec3d;
    bool isOk = true;
    m_isCalculationOK = true;

    Vec3d tq1q2 = (q2 - q1).getNormalized(&isOk); // !ok means the control points are in the same place. Could fallback to use only one circle segment + one line. 
    m_isCalculationOK = m_isCalculationOK && isOk;
    Vec3d t1    = (q1 - p1).getNormalized(&isOk); // !ok means no tangent specified. Could fallback to use only one circle segment + one line.
    m_isCalculationOK = m_isCalculationOK && isOk;
    Vec3d t2    = (p2 - q2).getNormalized(&isOk); // !ok means no tangent specified. Could fallback to use only one circle segment + one line or only one straight line if both tangents are missing
    m_isCalculationOK = m_isCalculationOK && isOk;

    {
        Vec3d td1 = (tq1q2 - t1);
        double td1Length = td1.length();

        if ( td1Length > 1e-10 )
        {
            td1 /= td1Length;
            m_c1 = q1 + ((q1 - p1).length() / (td1 * (-t1))) * td1;
            m_r1 = (m_c1 - p1).length();
        }
        else // both control points are along t1. First curve has infinite radius
        {
            m_c1 = cvf::Vec3d::UNDEFINED;
            m_r1 = std::numeric_limits<double>::infinity();
        }
    }

    {
        Vec3d td2 = (-tq1q2 + t2);
        double td2Length = td2.length();

        if ( td2Length > 1e-10 )
        {
            td2 /= td2Length;
            m_c2 = q2 + ((q2 - p2).length() / (td2 * (t2))) * td2;
            m_r2 = (m_c2 - p2).length();
        }
        else // both control points are along t2. Second curve has infinite radius
        {
            m_c2 = cvf::Vec3d::UNDEFINED;
            m_r2 = std::numeric_limits<double>::infinity();
        }
    }

    m_firstArcEndpoint    = q1 + (q1 - p1).length() * tq1q2;
    m_secondArcStartpoint = q2 - (q2 - p2).length() * tq1q2;

    if (((q1 - p1).length() + (q2 - p2).length()) > (q2 - q1).length()) // first arc end and second arc start is overlapping
    {
        m_isCalculationOK = false;
    }

    // The Circle normals. Will be set to cvf::Vec3d::ZERO if undefined.

    m_n1 = (t1 ^ tq1q2).getNormalized();
    m_n2 = (tq1q2 ^ t2).getNormalized();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaSCurveCalculator::dump() const
{
    cvf::Vec3d v_C1 = firstCenter();
    cvf::Vec3d v_C2 = secondCenter();
    cvf::Vec3d v_N1 = firstNormal();
    cvf::Vec3d v_N2 = secondNormal();
    cvf::Vec3d v_P11 = firstArcEndpoint();
    cvf::Vec3d v_P22 = secondArcStartpoint();

    std::cout << "  P1:  " << "[ " << m_p1[0]  << "  " << m_p1[1]  << "  " << m_p1[2]  << " " << std::endl;
    std::cout << "  P11: " << "[ " << v_P11[0] << "  " << v_P11[1] << "  " << v_P11[2] << " " << std::endl;
    std::cout << "  P22: " << "[ " << v_P22[0] << "  " << v_P22[1] << "  " << v_P22[2] << " " << std::endl;
    std::cout << "  P2:  " << "[ " << m_p2[0]  << "  " << m_p2[1]  << "  " << m_p2[2]  << " " << std::endl;
    std::cout << "  C1:  " << "[ " << v_C1[0]  << "  " << v_C1[1]  << "  " << v_C1[2]  << " " << std::endl;
    std::cout << "  C2:  " << "[ " << v_C2[0]  << "  " << v_C2[1]  << "  " << v_C2[2]  << " " << std::endl;
    std::cout << "  N1:  " << "[ " << v_N1[0]  << "  " << v_N1[1]  << "  " << v_N1[2]  << " " << std::endl;
    std::cout << "  N2:  " << "[ " << v_N2[0]  << "  " << v_N2[1]  << "  " << v_N2[2]  << " " << std::endl;
    std::cout << "  R1:  " << "[ " << firstRadius()  << " ]" << std::endl;
    std::cout << "  R2:  " << "[ " << secondRadius() << " ]" << std::endl;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaSCurveCalculator RiaSCurveCalculator::fromTangentsAndLength(cvf::Vec3d p1, double azi1, double inc1, double lengthToQ1, 
                                                               cvf::Vec3d p2, double azi2, double inc2, double lengthToQ2)
{
    cvf::Vec3d t1(RiaOffshoreSphericalCoords::unitVectorFromAziInc(azi1,inc1)); 
    cvf::Vec3d t2(RiaOffshoreSphericalCoords::unitVectorFromAziInc(azi2,inc2));

    cvf::Vec3d Q1 = p1 + lengthToQ1 * t1;
    cvf::Vec3d Q2 = p2 - lengthToQ2 * t2;

    RiaSCurveCalculator curveFromControlPoints(p1, Q1, 
                                               p2, Q2);

    return curveFromControlPoints;
}

