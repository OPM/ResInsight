#include "gtest/gtest.h"

#include "SolveSpaceSystem.h"
#include <slvs.h>

#include <assert.h>
#include <iostream>

#include "SolveSpaceSystem.h"
#include "RiaSCurveCalculator.h"

/*-----------------------------------------------------------------------------
* Test calculation of an S-shape curve:
* Arc-Line-Arc with start and endpoint along with their tangents as input.
*---------------------------------------------------------------------------*/
void example_S_Curve(double p1x,
                     double p1y,
                     double p1z,
                     double azi1,
                     double inc1,
                     double rad1,
                     double p2x,
                     double p2y,
                     double p2z,
                     double azi2,
                     double inc2,
                     double rad2)
{
    SolveSpaceSystem sys;

    Slvs_hGroup group1 = 1;
    Slvs_hGroup group2 = 2;

    // Group 1, Fixed
    // P1
    Slvs_hParam p_p1x  = sys.addParam( Slvs_MakeParam(-1, group1, p1x) ); 
    Slvs_hParam p_p1y  = sys.addParam( Slvs_MakeParam(-1, group1, p1y) ); 
    Slvs_hParam p_p1z  = sys.addParam( Slvs_MakeParam(-1, group1, p1z) );  

    Slvs_hEntity e_P1 = sys.addEntity( Slvs_MakePoint3d(-1, group1, p_p1x, p_p1y, p_p1z) );

    // PT1
    double pt1x = p1x + sin(azi1)*sin(inc1);
    double pt1y = p1y + cos(azi1)*sin(inc1); 
    double pt1z = p1z - cos(inc1);

    Slvs_hParam p_pt1x  = sys.addParam( Slvs_MakeParam(-1, group1, pt1x) ); 
    Slvs_hParam p_pt1y  = sys.addParam( Slvs_MakeParam(-1, group1, pt1y) ); 
    Slvs_hParam p_pt1z  = sys.addParam( Slvs_MakeParam(-1, group1, pt1z) );  

    Slvs_hEntity e_PT1 = sys.addEntity( Slvs_MakePoint3d(-1, group1, p_pt1x, p_pt1y, p_pt1z) );

    // Tangent Line 1

    Slvs_hEntity e_LT1 = sys.addEntity(Slvs_MakeLineSegment(-1, group1, SLVS_FREE_IN_3D, e_P1, e_PT1));

    // P2
    Slvs_hParam p_p2x  = sys.addParam( Slvs_MakeParam(-1, group1, p2x) ); 
    Slvs_hParam p_p2y  = sys.addParam( Slvs_MakeParam(-1, group1, p2y) ); 
    Slvs_hParam p_p2z  = sys.addParam( Slvs_MakeParam(-1, group1, p2z) );  

    Slvs_hEntity e_P2 = sys.addEntity( Slvs_MakePoint3d(-1, group1, p_p2x, p_p2y, p_p2z) );

    // PT2
    double pt2x = p2x + sin(azi2)*sin(inc2);
    double pt2y = p2y + cos(azi2)*sin(inc2); 
    double pt2z = p2z - cos(inc2);

    Slvs_hParam p_pt2x  = sys.addParam( Slvs_MakeParam(-1, group1, pt2x) ); 
    Slvs_hParam p_pt2y  = sys.addParam( Slvs_MakeParam(-1, group1, pt2y) ); 
    Slvs_hParam p_pt2z  = sys.addParam( Slvs_MakeParam(-1, group1, pt2z) );  

    Slvs_hEntity e_PT2 = sys.addEntity( Slvs_MakePoint3d(-1, group1, p_pt2x, p_pt2y, p_pt2z) );

    // Tangent Line 2

    Slvs_hEntity e_LT2 = sys.addEntity(Slvs_MakeLineSegment(-1, group1, SLVS_FREE_IN_3D, e_P2, e_PT2));

    // Plane1

    double unitQw, unitQx, unitQy, unitQz;
    Slvs_MakeQuaternion(1, 0, 0,
                        0, 1, 0, 
                        &unitQw, &unitQx, &unitQy, &unitQz);

    // Plane 1

    Slvs_hParam p_Plane1Qw = sys.addParam( Slvs_MakeParam(-1, group2, unitQw) );  
    Slvs_hParam p_Plane1Qx = sys.addParam( Slvs_MakeParam(-1, group2, unitQx) ); 
    Slvs_hParam p_Plane1Qy = sys.addParam( Slvs_MakeParam(-1, group2, unitQy) ); 
    Slvs_hParam p_Plane1Qz = sys.addParam( Slvs_MakeParam(-1, group2, unitQz));
    Slvs_hEntity e_Plane1Q = sys.addEntity( Slvs_MakeNormal3d(-1, group2,
                                                              p_Plane1Qw,
                                                              p_Plane1Qx,
                                                              p_Plane1Qy,
                                                              p_Plane1Qz));
    Slvs_hEntity e_Plane1 = sys.addEntity( Slvs_MakeWorkplane(-1, group2, e_P1, e_Plane1Q) );

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
    Slvs_hParam p_c1x  = sys.addParam( Slvs_MakeParam(-1, group2, 10.0) ); // Needs a better guess 
    Slvs_hParam p_c1y  = sys.addParam( Slvs_MakeParam(-1, group2, 2.0) ); 

    Slvs_hEntity e_C1 = sys.addEntity( Slvs_MakePoint2d(-1, group2, e_Plane1, p_c1x, p_c1y) );

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

    Slvs_hParam p_p11x  = sys.addParam( Slvs_MakeParam(-1, group2, 2.0) ); // Needs a better guess: Perp on p_c1x/p_c1y
    Slvs_hParam p_p11y  = sys.addParam( Slvs_MakeParam(-1, group2, -10.0) ); 

    Slvs_hEntity e_P11 = sys.addEntity( Slvs_MakePoint2d(-1, group2, e_Plane1, p_p11x, p_p11y) );

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

    Slvs_hParam p_Plane2Qw = sys.addParam( Slvs_MakeParam(-1, group2, unitQw) );  
    Slvs_hParam p_Plane2Qx = sys.addParam( Slvs_MakeParam(-1, group2, unitQx) ); 
    Slvs_hParam p_Plane2Qy = sys.addParam( Slvs_MakeParam(-1, group2, unitQy) ); 
    Slvs_hParam p_Plane2Qz = sys.addParam( Slvs_MakeParam(-1, group2, unitQz));
    Slvs_hEntity e_Plane2Q = sys.addEntity( Slvs_MakeNormal3d(-1, group2,
                                                              p_Plane2Qw,
                                                              p_Plane2Qx,
                                                              p_Plane2Qy,
                                                              p_Plane2Qz));

    Slvs_hEntity e_Plane2 = sys.addEntity( Slvs_MakeWorkplane(-1, group2, e_P2, e_Plane2Q) );

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

    Slvs_hParam p_c2x  = sys.addParam( Slvs_MakeParam(-1, group2, 10.0) ); // Needs a better guess 
    Slvs_hParam p_c2y  = sys.addParam( Slvs_MakeParam(-1, group2, 2.0) ); 

    Slvs_hEntity e_C2 = sys.addEntity( Slvs_MakePoint2d(-1, group2, e_Plane2, p_c2x, p_c2y) );

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

    Slvs_hParam p_p22x  = sys.addParam( Slvs_MakeParam(-1, group2, 2.0) ); // Needs a better guess: Perp on p_c1x/p_c1y
    Slvs_hParam p_p22y  = sys.addParam( Slvs_MakeParam(-1, group2, -10.0) ); 

    Slvs_hEntity e_P22 = sys.addEntity( Slvs_MakePoint2d(-1, group2, e_Plane2, p_p22x, p_p22y) );

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
    assert(solveResult == SolveSpaceSystem::RESULT_OKAY);

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
    assert(solveResult == SolveSpaceSystem::RESULT_OKAY);


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
    assert(solveResult == SolveSpaceSystem::RESULT_OKAY);

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
    assert(solveResult == SolveSpaceSystem::RESULT_OKAY);

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
    assert(solveResult == SolveSpaceSystem::RESULT_OKAY);


    // Circle Center, Plane normals, P11, P22

    std::valarray<double> v_C1 = sys.global3DPos(e_C1);
    std::valarray<double> v_C2 = sys.global3DPos(e_C2);

    std::valarray<double> v_N1 = std::get<2>( sys.orientationMx(e_Plane1Q));
    std::valarray<double> v_N2 = std::get<2>( sys.orientationMx(e_Plane2Q));

    std::valarray<double> v_P11 = sys.global3DPos(e_P11);
    std::valarray<double> v_P22 = sys.global3DPos(e_P22);

    std::cout << "P1:  " << "[ " <<  p1x     << "  " << p1y      << "  " << p1z      << " " << std::endl;
    std::cout << "P11: " << "[ " << v_P11[0] << "  " << v_P11[1] << "  " << v_P11[2] << " " << std::endl;
    std::cout << "P22: " << "[ " << v_P22[0] << "  " << v_P22[1] << "  " << v_P22[2] << " " << std::endl;
    std::cout << "P2:  " << "[ " <<  p2x     << "  " << p2y      << "  " << p2z      << " " << std::endl;
    std::cout << "C1:  " << "[ " << v_C1[0]  << "  " << v_C1[1]  << "  " << v_C1[2]  << " " << std::endl;
    std::cout << "C2:  " << "[ " << v_C2[0]  << "  " << v_C2[1]  << "  " << v_C2[2]  << " " << std::endl;
    std::cout << "N1:  " << "[ " << v_N1[0]  << "  " << v_N1[1]  << "  " << v_N1[2]  << " " << std::endl;
    std::cout << "N2:  " << "[ " << v_N2[0]  << "  " << v_N2[1]  << "  " << v_N2[2]  << " " << std::endl;

}

#define M_PI       3.14159265358979323846   // pi

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(SolveSpaceSolverTest, SCurve)
{
    example_S_Curve(100, 100,     0,    0, M_PI/4, 12,
                    100, 150, -1000, M_PI, M_PI/4, 12);


}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaSCurveCalculator, Test1)

{
    RiaSCurveCalculator sCurveCalc({ 100, 100, 0 },
                                   0,
                                   M_PI/4,
                                   12,
                                   { 100, 150, -1000 },
                                   M_PI,
                                   M_PI/4,
                                   12);
    EXPECT_TRUE(sCurveCalc.isOk() );
    
    sCurveCalc.dump();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaSCurveCalculator, Test1AtEstimate)

{
    RiaSCurveCalculator sCurveCalc({ 100, 100, 0 },
                                   0,
                                   M_PI/4,
                                   535.452,
                                   { 100, 150, -1000 },
                                   M_PI,
                                   M_PI/4,
                                   439.508);
    EXPECT_TRUE(sCurveCalc.isOk() );

    sCurveCalc.dump();

}

TEST(RiaSCurveCalculator, Test2)
{
    RiaSCurveCalculator sCurveCalc({ 100, 100, 0 },
                                   0,
                                   M_PI/4,
                                   50,
                                   { 100, 150, -1000 },
                                   M_PI,
                                   M_PI/4,
                                   50);

    EXPECT_TRUE(sCurveCalc.isOk());

    sCurveCalc.dump();

}


TEST(RiaSCurveCalculator, Test3)
{
    RiaSCurveCalculator sCurveCalc({ 100, 100, 0 },
                                   0,
                                   0.3,
                                   50,
                                   { 100, 150, -1000 },
                                   0,
                                   0.4,
                                   50);

    EXPECT_TRUE(sCurveCalc.isOk() );

    sCurveCalc.dump();

}

void printQ1Q2R1R2Matrix(cvf::Vec3d p1, double azi1, double inc1,
                         cvf::Vec3d p2, double azi2, double inc2)
{

    double q1Start = 0.0; 
    double q1End = 3000;
    double step1 = 100;
    double q2Start = 0.0; 
    double q2End = 3000;
    double step2 = 100;

    std::cout << "R1" <<  std::endl;
    std::cout << "q1\\q2" << "  ";

    for (double q2 = q2Start; q2 < q2End; q2 += step2)
    {
        std::cout << q2 << "  "; 
    }
    std::cout << std::endl;

    for (double q1 = q1Start; q1 < q1End; q1 += step1)
    {
        std::cout << q1 << "  "; 
        for (double q2 = q2Start; q2 < q2End; q2 += step2)
        {
            RiaSCurveCalculator sCurveCalc = RiaSCurveCalculator::fromTangentsAndLength(p1, azi1, inc1, q1,
                                                                                        p2, azi2, inc2, q2);
            if ( sCurveCalc.isOk() )
            {
                std::cout << sCurveCalc.firstRadius() << "  " ;
            }
            else
            {
                std::cout << "NS" << "  " ;
            }
        }

        std::cout <<  std::endl;
    }

    std::cout <<  std::endl;
    std::cout << "R2" <<  std::endl;
    std::cout << "q1\\q2" << "  ";

    for (double q2 = q2Start; q2 < q2End; q2 += step2)
    {
        std::cout << q2 << "  "; 
    }
    std::cout << std::endl;

    for (double q1 = q1Start; q1 < q1End; q1 += step1)
    {
        std::cout << q1 << "  "; 
        for (double q2 = q2Start; q2 < q2End; q2 += step2)
        {
            RiaSCurveCalculator sCurveCalc = RiaSCurveCalculator::fromTangentsAndLength(p1, azi1, inc1, q1,
                                                                                        p2, azi2, inc2, q2);
            if ( sCurveCalc.isOk() )
            {
                std::cout << sCurveCalc.secondRadius() << "  " ;
            }
            else
            {
                std::cout << "NS" << "  " ;
            }
        }

        std::cout <<  std::endl;
    }

}

TEST(DISABLED_RiaSCurveCalculator, q_r_relation)
{
    std::cout << "Config 1" << std::endl;

    printQ1Q2R1R2Matrix(
    { 0,0,0 }, 0, 0,
    { 0,0,-1000 }, 0, 0);

    std::cout << "Config 1a" << std::endl;

    printQ1Q2R1R2Matrix(
    { 0,0,0 }, 0, 0,
    { 0,0,-1000 }, 0, 0.01);

    std::cout << "Config 2" << std::endl;

    printQ1Q2R1R2Matrix(
    { 0,0,0 }, 0, 0,
    { 0,0,-1000 }, 0, M_PI/2.0);

    std::cout << "Config 3" << std::endl;

    printQ1Q2R1R2Matrix(
    { 0,0,0 }, 0, 0,
    { 0,0,-1000 }, 0, M_PI);

    std::cout << "Config 3a" << std::endl;

    printQ1Q2R1R2Matrix(
    { 0,0,0 }, 0, 0,
    { 0,0,-1000 }, 0, M_PI-0.01);

    std::cout << "Config 4" << std::endl;

    printQ1Q2R1R2Matrix(
    { 0,0,0 }, 0,  M_PI/2.0,
    { 0,0,-1000 }, 0,  M_PI/2.0);

    std::cout << "Config 5" << std::endl;

    printQ1Q2R1R2Matrix(
    { 0,0,0 }, 0,  M_PI/2.0,
    { 0,0,-1000 }, M_PI,  M_PI/2.0);

    std::cout << "Config 6" << std::endl;

    printQ1Q2R1R2Matrix(
    { 0,0,0 }, 0,  M_PI,
    { 0,0,-1000 }, 0,  0);

    std::cout << "Config 6a" << std::endl;

    printQ1Q2R1R2Matrix(
    { 0,0,0 }, 0,  M_PI,
    { 0,0,-1000 }, 0,  0.01);

    std::cout << "Config 6b" << std::endl;

    printQ1Q2R1R2Matrix(
    { 0,0,0 }, 0,  M_PI-0.01,
    { 0,0,-1000 }, 0,  0.00);

    std::cout << "Config 7" << std::endl;

    printQ1Q2R1R2Matrix(
    { 0,0,0 }, 0,  M_PI,
    { 0,0,-1000 }, 0,   M_PI/2.0);

    std::cout << "Config 8" << std::endl;

    printQ1Q2R1R2Matrix(
    { 0,0,0 }, 0,  M_PI,
    { 0,0,-1000 }, 0,   M_PI);

    std::cout << "Config 8a" << std::endl;

    printQ1Q2R1R2Matrix(
    { 0,0,0 }, 0,  M_PI,
    { 0,0,-1000 }, 0,   M_PI-0.01);

    std::cout << "Config 8b" << std::endl;

    printQ1Q2R1R2Matrix(
    { 0,0,0 }, 0,  M_PI-0.01,
    { 0,0,-1000 }, 0,   M_PI);

    std::cout << "Config 9" << std::endl;

    printQ1Q2R1R2Matrix(
    { 0,0,0 }, 0,  M_PI/2,
    { 0,0,-1000 }, M_PI/2,   M_PI/2);

}
