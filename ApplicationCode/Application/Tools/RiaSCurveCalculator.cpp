#include "RiaSCurveCalculator.h"

#include "SolveSpaceSystem.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaSCurveCalculator::RiaSCurveCalculator(cvf::Vec3d p1, double azi1, double inc1, double rad1, 
                                         cvf::Vec3d p2, double azi2, double inc2, double rad2)
                                         : m_isCalculationOK(false)
                                         , m_firstArcEndpoint(p1 + 0.3*(p2-p1))
                                         , m_secondArcStartpoint(p1 + 0.6*(p2-p1))
                                         , m_r1(rad1)
                                         , m_r2(rad2)
{
    // Estimate 

    cvf::Vec3d t1(sin(azi1)*sin(inc1),
                  cos(azi1)*sin(inc1),
                  -cos(inc1));
    cvf::Vec3d t2(sin(azi2)*sin(inc2),
                  cos(azi2)*sin(inc2),
                  -cos(inc2));

    cvf::Vec3d p1p2 = p2-p1;
    double p1p2Length = p1p2.length();

    cvf::Vec3d Q1 = p1 + 0.2 * p1p2Length * t1;
    cvf::Vec3d Q2 = p2 - 0.2 * p1p2Length * t2;
    cvf::Vec3d tQ1Q2 = (Q2 - Q1).getNormalized();
    
    RiaSCurveCalculator estimatedCurveCalc(p1, Q1, p2, Q2);
    m_firstArcEndpoint = estimatedCurveCalc.firstArcEndpoint();
    m_secondArcStartpoint = estimatedCurveCalc.secondArcStartpoint();
    m_c1 = estimatedCurveCalc.firstCenter();
    m_c2 = estimatedCurveCalc.secondCenter();
    m_n1 = estimatedCurveCalc.firstNormal();
    m_n2 = estimatedCurveCalc.secondNormal();

    #if 1 // Bypass SolveSpace
    m_r1 = estimatedCurveCalc.firstRadius();
    m_r2 = estimatedCurveCalc.secondRadius();
    return;
    #endif

    cvf::Vec3d tp1c1 = (estimatedCurveCalc.firstCenter() - p1).getNormalized();
    cvf::Vec3d tp2c2 = (estimatedCurveCalc.secondCenter() - p2).getNormalized();

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

    double unitQw, unitQx, unitQy, unitQz;
    Slvs_MakeQuaternion(t1.x(), t1.y(), t1.z(),
                        tp1c1.x(), tp1c1.y() , tp1c1.z(),
                        &unitQw, &unitQx, &unitQy, &unitQz);

    // Plane 1

    Slvs_hParam p_Plane1Qw = sys.addParam(Slvs_MakeParam(-1, group2, unitQw));
    Slvs_hParam p_Plane1Qx = sys.addParam(Slvs_MakeParam(-1, group2, unitQx));
    Slvs_hParam p_Plane1Qy = sys.addParam(Slvs_MakeParam(-1, group2, unitQy));
    Slvs_hParam p_Plane1Qz = sys.addParam(Slvs_MakeParam(-1, group2, unitQz));
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
    Slvs_hParam p_c1x  = sys.addParam(Slvs_MakeParam(-1, group2, 0)); // Needs a better guess 
    Slvs_hParam p_c1y  = sys.addParam(Slvs_MakeParam(-1, group2, rad1));

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
                                                                     rad1,
                                                                     e_P1,
                                                                     e_C1,
                                                                     -1,
                                                                     -1));

    // Arc1 end

    Slvs_hParam p_p11x  = sys.addParam(Slvs_MakeParam(-1, group2, rad1)); // Needs a better guess: Perp on p_c1x/p_c1y
    Slvs_hParam p_p11y  = sys.addParam(Slvs_MakeParam(-1, group2, rad1));

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




    double unitQw2, unitQx2, unitQy2, unitQz2;
    Slvs_MakeQuaternion(t2.x(), t2.y(), t2.z(),
                        tp1c1.x(), tp1c1.y(), tp1c1.z(),
                        &unitQw2, &unitQx2, &unitQy2, &unitQz2);

    // Plane 2

    Slvs_hParam p_Plane2Qw = sys.addParam(Slvs_MakeParam(-1, group2, unitQw2));
    Slvs_hParam p_Plane2Qx = sys.addParam(Slvs_MakeParam(-1, group2, unitQx2));
    Slvs_hParam p_Plane2Qy = sys.addParam(Slvs_MakeParam(-1, group2, unitQy2));
    Slvs_hParam p_Plane2Qz = sys.addParam(Slvs_MakeParam(-1, group2, unitQz2));
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

    Slvs_hParam p_c2x  = sys.addParam(Slvs_MakeParam(-1, group2, 0)); // Needs a better guess 
    Slvs_hParam p_c2y  = sys.addParam(Slvs_MakeParam(-1, group2, rad2));

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
                                                                     rad2,
                                                                     e_P2,
                                                                     e_C2,
                                                                     -1,
                                                                     -1));

    // Arc2 end

    Slvs_hParam p_p22x  = sys.addParam(Slvs_MakeParam(-1, group2, -rad2)); // Needs a better guess: Perp on p_c1x/p_c1y
    Slvs_hParam p_p22y  = sys.addParam(Slvs_MakeParam(-1, group2,  rad2));

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



    auto solveResult = sys.solve(group2, true);

    if(solveResult != SolveSpaceSystem::RESULT_OKAY) 
    {
        return;
    }

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

    solveResult = sys.solve(group2, true);

    if(solveResult != SolveSpaceSystem::RESULT_OKAY) return;



    Slvs_hConstraint c_perpC2P22_LP11P22 = sys.addConstr(Slvs_MakeConstraint(-1,
                                                                             group2,
                                                                             SLVS_C_PERPENDICULAR,
                                                                             SLVS_FREE_IN_3D,
                                                                             0.0,
                                                                             -1,
                                                                             -1,
                                                                             e_LC2P22,
                                                                             e_LP11P22));

    solveResult = sys.solve(group2, true);

    if(solveResult != SolveSpaceSystem::RESULT_OKAY) return;


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

    solveResult = sys.solve(group2, true);

    if(solveResult != SolveSpaceSystem::RESULT_OKAY) return;


    Slvs_hConstraint c_P22InPlane1 = sys.addConstr(Slvs_MakeConstraint(-1,
                                                                       group2,
                                                                       SLVS_C_PT_IN_PLANE,
                                                                       SLVS_FREE_IN_3D,
                                                                       0.0,
                                                                       e_P22,
                                                                       -1,
                                                                       e_Plane1,
                                                                       -1));


    solveResult = sys.solve(group2, true);

    if(solveResult != SolveSpaceSystem::RESULT_OKAY) return;

    m_isCalculationOK = true;

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

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaSCurveCalculator::RiaSCurveCalculator(cvf::Vec3d p1, cvf::Vec3d q1, 
                                         cvf::Vec3d p2, cvf::Vec3d q2)
{
    using Vec3d = cvf::Vec3d;

    Vec3d tq1q2 = (q2-q1).getNormalized();
    Vec3d t1 = (q1 - p1).getNormalized();
    Vec3d t2 = (p2 - q2).getNormalized();

    Vec3d td1 = (tq1q2 - t1).getNormalized();
    Vec3d td2 = (-tq1q2 + t2).getNormalized();

    m_c1 = q1 + (q1-p1).length() * (td1 * (-t1))*td1;
    m_c2 = q2 + (q2-p2).length() * (td2 * (t2))*td2;

    m_firstArcEndpoint = q1 + (q1 - p1).length()*tq1q2;
    m_secondArcStartpoint = q2 - (q2 - p2).length()*tq1q2;

    m_n1 = t1 ^ tq1q2;
    m_n2 = tq1q2 ^t2;

    m_r1 = (m_c1 - p1).length();
    m_r2 = (m_c2 - p2).length();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiaSCurveCalculator::calculateEstimatedSolution()
{
    // Plane1 basisvectors
    // C1 position in Plane1
    // P11 position in Plane 1

    // Plane2 basisvectors

}

