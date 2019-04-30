#include "gtest/gtest.h"

#include <assert.h>
#include <iostream>

#include "RiaSCurveCalculator.h"

#define M_PI       3.14159265358979323846   // pi

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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
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


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaSCurveCalculator, Test4)
{
    RiaSCurveCalculator sCurveCalc({ 0, 0, 0 },
                                   0,
                                   45,
                                   115,
                                   { 0, 50, -1000 },
                                   0,
                                   0,
                                   115);

    EXPECT_TRUE(sCurveCalc.isOk() );

    sCurveCalc.dump();

}

double curveRadius = 115;
double angleEpsilon = 0.01;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaSCurveCalculator, Config1 ) 
{ 
    RiaSCurveCalculator sCurveCalc(
    { 0,0,0 }, 0, 0, curveRadius,
    { 0,0,-1000 }, 0, 0, curveRadius);
    sCurveCalc.dump();
    EXPECT_EQ(RiaSCurveCalculator::OK_INFINITE_RADIUS12, sCurveCalc.curveStatus());
    EXPECT_EQ(RiaSCurveCalculator::CONVERGED, sCurveCalc.solveStatus());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaSCurveCalculator, Config1a ) 
{ 
    RiaSCurveCalculator sCurveCalc(
    { 0,0,0 }, 0, 0, curveRadius,
    { 0,0,-1000 }, 0, angleEpsilon, curveRadius);
    sCurveCalc.dump();
    EXPECT_EQ(RiaSCurveCalculator::OK, sCurveCalc.curveStatus());
    EXPECT_EQ(RiaSCurveCalculator::CONVERGED, sCurveCalc.solveStatus());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaSCurveCalculator, Config2 ) 
{ 
    RiaSCurveCalculator sCurveCalc(
    { 0,0,0 }, 0, 0, curveRadius,
    { 0,0,-1000 }, 0, M_PI/2.0, curveRadius);
    sCurveCalc.dump();
    EXPECT_EQ(RiaSCurveCalculator::OK, sCurveCalc.curveStatus());
    EXPECT_EQ(RiaSCurveCalculator::CONVERGED, sCurveCalc.solveStatus());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(DISABLED_RiaSCurveCalculator, Config3 ) 
{ 
    RiaSCurveCalculator sCurveCalc(
    { 0,0,0 }, 0, 0, curveRadius,
    { 0,0,-1000 }, 0, M_PI, curveRadius);
    sCurveCalc.dump();
    EXPECT_EQ(RiaSCurveCalculator::OK, sCurveCalc.curveStatus());
    EXPECT_EQ(RiaSCurveCalculator::CONVERGED, sCurveCalc.solveStatus());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(DISABLED_RiaSCurveCalculator, Config3a ) 
{ 
    RiaSCurveCalculator sCurveCalc(
    { 0,0,0 }, 0, 0, curveRadius,
    { 0,0,-1000 }, 0, M_PI-angleEpsilon, curveRadius);
    sCurveCalc.dump();
    EXPECT_EQ(RiaSCurveCalculator::OK, sCurveCalc.curveStatus());
    EXPECT_EQ(RiaSCurveCalculator::CONVERGED, sCurveCalc.solveStatus());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaSCurveCalculator, Config4 ) 
{ 
    RiaSCurveCalculator sCurveCalc(
    { 0,0,0 }, 0,  M_PI/2.0, curveRadius,
    { 0,0,-1000 }, 0,  M_PI/2.0, curveRadius);
    sCurveCalc.dump();
    EXPECT_EQ(RiaSCurveCalculator::OK, sCurveCalc.curveStatus());
    EXPECT_EQ(RiaSCurveCalculator::CONVERGED, sCurveCalc.solveStatus());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaSCurveCalculator, Config5 ) 
{ 
    RiaSCurveCalculator sCurveCalc(
    { 0,0,0 }, 0,  M_PI/2.0, curveRadius,
    { 0,0,-1000 }, M_PI,  M_PI/2.0, curveRadius);
    sCurveCalc.dump();
    EXPECT_EQ(RiaSCurveCalculator::OK, sCurveCalc.curveStatus());
    EXPECT_EQ(RiaSCurveCalculator::CONVERGED, sCurveCalc.solveStatus());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(DISABLED_RiaSCurveCalculator, Config6 ) 
{ 
    RiaSCurveCalculator sCurveCalc(
    { 0,0,0 }, 0,  M_PI, curveRadius,
    { 0,0,-1000 }, 0,  0, curveRadius);
    sCurveCalc.dump();
    EXPECT_EQ(RiaSCurveCalculator::OK, sCurveCalc.curveStatus());
    EXPECT_EQ(RiaSCurveCalculator::CONVERGED, sCurveCalc.solveStatus());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(DISABLED_RiaSCurveCalculator, Config6a ) 
{ 
    RiaSCurveCalculator sCurveCalc(
    { 0,0,0 }, 0,  M_PI, curveRadius,
    { 0,0,-1000 }, 0,  angleEpsilon, curveRadius);
    sCurveCalc.dump();
    EXPECT_EQ(RiaSCurveCalculator::OK, sCurveCalc.curveStatus());
    EXPECT_EQ(RiaSCurveCalculator::CONVERGED, sCurveCalc.solveStatus());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(DISABLED_RiaSCurveCalculator, Config6b ) 
{ 
    RiaSCurveCalculator sCurveCalc(
    { 0,0,0 }, 0,  M_PI-angleEpsilon, curveRadius,
    { 0,0,-1000 }, 0,  0.00, curveRadius);
    sCurveCalc.dump();
    EXPECT_EQ(RiaSCurveCalculator::OK, sCurveCalc.curveStatus());
    EXPECT_EQ(RiaSCurveCalculator::CONVERGED, sCurveCalc.solveStatus());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(DISABLED_RiaSCurveCalculator, Config7 ) 
{ 
    RiaSCurveCalculator sCurveCalc(
    { 0,0,0 }, 0,  M_PI, curveRadius,
    { 0,0,-1000 }, 0,   M_PI/2.0, curveRadius+20);
    sCurveCalc.dump();
    EXPECT_EQ(RiaSCurveCalculator::OK, sCurveCalc.curveStatus());
    EXPECT_EQ(RiaSCurveCalculator::CONVERGED, sCurveCalc.solveStatus());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(DISABLED_RiaSCurveCalculator, Config8 ) 
{ 
    RiaSCurveCalculator sCurveCalc(
    { 0,0,0 }, 0,  M_PI, curveRadius,
    { 0,0,-1000 }, 0,   M_PI, curveRadius);
    sCurveCalc.dump();
    EXPECT_EQ(RiaSCurveCalculator::OK, sCurveCalc.curveStatus());
    EXPECT_EQ(RiaSCurveCalculator::CONVERGED, sCurveCalc.solveStatus());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(DISABLED_RiaSCurveCalculator, Config8a ) 
{ 
    RiaSCurveCalculator sCurveCalc(
    { 0,0,0 }, 0,  M_PI, curveRadius,
    { 0,0,-1000 }, 0,   M_PI-angleEpsilon, curveRadius);
    sCurveCalc.dump();
    EXPECT_EQ(RiaSCurveCalculator::OK, sCurveCalc.curveStatus());
    EXPECT_EQ(RiaSCurveCalculator::CONVERGED, sCurveCalc.solveStatus());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(DISABLED_RiaSCurveCalculator, Config8b ) 
{ 
    RiaSCurveCalculator sCurveCalc(
    { 0,0,0 }, 0,  M_PI-angleEpsilon, curveRadius,
    { 0,0,-1000 }, 0,   M_PI, curveRadius);
    sCurveCalc.dump();
    EXPECT_EQ(RiaSCurveCalculator::OK, sCurveCalc.curveStatus());
    EXPECT_EQ(RiaSCurveCalculator::CONVERGED, sCurveCalc.solveStatus());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaSCurveCalculator, Config9 ) 
{ 
    RiaSCurveCalculator sCurveCalc(
    { 0,0,0 }, 0,  M_PI/2, curveRadius,
    { 0,0,-1000 }, M_PI/2,   M_PI/2, curveRadius);
    sCurveCalc.dump();
    EXPECT_EQ(RiaSCurveCalculator::OK, sCurveCalc.curveStatus());
    EXPECT_EQ(RiaSCurveCalculator::CONVERGED, sCurveCalc.solveStatus());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaSCurveCalculator, InfiniteStartRadius ) 
{
    {
        RiaSCurveCalculator sCurveCalc(
        { 0,0,0 }, 0, 0, curveRadius,
        { 0,curveRadius,-1000 }, 0, M_PI/2, curveRadius);
        sCurveCalc.dump();
        EXPECT_EQ(RiaSCurveCalculator::OK_INFINITE_RADIUS1, sCurveCalc.curveStatus());
        EXPECT_EQ(RiaSCurveCalculator::CONVERGED, sCurveCalc.solveStatus());
    }

    {
        RiaSCurveCalculator sCurveCalc(
        { 0,0,0 }, 0, 0, curveRadius,
        { 0,curveRadius+0.01,-1000 }, 0, M_PI/2, curveRadius);
        sCurveCalc.dump();
        EXPECT_EQ(RiaSCurveCalculator::OK, sCurveCalc.curveStatus());
        EXPECT_EQ(RiaSCurveCalculator::CONVERGED, sCurveCalc.solveStatus());
    }
}

//--------------------------------------------------------------------------------------------------
/// Helper method to print the r1(q1, q2) r2(q1, q2) for plotting as 3D surface in Excel 
//--------------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------------
/// Test used to print and plot the relations between q1, q2, r1 and r2 in excel as 3d surface 
//--------------------------------------------------------------------------------------------------
TEST(DISABLED_RiaSCurveCalculator, q_r_relation)
{
    std::cout << "Config 1" << std::endl;

    printQ1Q2R1R2Matrix(
    { 0,0,0 }, 0, 0,
    { 0,0,-1000 }, 0, 0);

    std::cout << "Config 1a" << std::endl;

    printQ1Q2R1R2Matrix(
    { 0,0,0 }, 0, 0,
    { 0,0,-1000 }, 0, angleEpsilon);

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
    { 0,0,-1000 }, 0, M_PI-angleEpsilon);

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
    { 0,0,-1000 }, 0,  angleEpsilon);

    std::cout << "Config 6b" << std::endl;

    printQ1Q2R1R2Matrix(
    { 0,0,0 }, 0,  M_PI-angleEpsilon,
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
    { 0,0,-1000 }, 0,   M_PI-angleEpsilon);

    std::cout << "Config 8b" << std::endl;

    printQ1Q2R1R2Matrix(
    { 0,0,0 }, 0,  M_PI-angleEpsilon,
    { 0,0,-1000 }, 0,   M_PI);

    std::cout << "Config 9" << std::endl;

    printQ1Q2R1R2Matrix(
    { 0,0,0 }, 0,  M_PI/2,
    { 0,0,-1000 }, M_PI/2,   M_PI/2);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaSCurveCalculator, ControlPointCurve)
{
    {
        RiaSCurveCalculator sCurveCalc(
        { 0,0,0 }, { 0, 0, -100 },
        { 0,500,-1000 }, { 0, 0, -500 });

        EXPECT_EQ(RiaSCurveCalculator::FAILED_ARC_OVERLAP, sCurveCalc.curveStatus());
        EXPECT_EQ(RiaSCurveCalculator::NOT_SOLVED, sCurveCalc.solveStatus());
        //sCurveCalc.dump();
    }

    {
        RiaSCurveCalculator sCurveCalc(
        { 0,0,0 }, { 0, 0, -100 },
        { 0,100,-1000 }, { 0, 0, -900 });
        EXPECT_EQ(RiaSCurveCalculator::OK_INFINITE_RADIUS1, sCurveCalc.curveStatus());
    }
    {
        RiaSCurveCalculator sCurveCalc(
        { 0,100,0 }, { 0, 0, -100 },
        { 0,0,-1000 }, { 0, 0, -900 });
        EXPECT_EQ(RiaSCurveCalculator::OK_INFINITE_RADIUS2, sCurveCalc.curveStatus());
    }
    {
        RiaSCurveCalculator sCurveCalc(
        { 0,0,0 }, { 0, 0, -100},
        { 0,0,-1000 }, { 0, 0, -900 });
        EXPECT_EQ(RiaSCurveCalculator::OK_INFINITE_RADIUS12, sCurveCalc.curveStatus());
    }

    {
        RiaSCurveCalculator sCurveCalc(
        { 0,0,0 }, { 0, 0, -100},
        { 0,0, 0 }, { 0, 0, -900 });
        EXPECT_EQ(RiaSCurveCalculator::FAILED_ARC_OVERLAP, sCurveCalc.curveStatus());
        //sCurveCalc.dump();
    }

    {
        RiaSCurveCalculator sCurveCalc(
        { 0,0,0 }, { 0, 0, 0},
        { 0,0, -1000 }, { 0, 0, -900 });
        EXPECT_EQ(RiaSCurveCalculator::FAILED_INPUT_OVERLAP, sCurveCalc.curveStatus());
        //sCurveCalc.dump();
    }
}

#include "RiaJCurveCalculator.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaJCurveCalculator, Basic)
{
    {
        RiaJCurveCalculator calc({ 0,0,0 }, 0, M_PI/2, 100, { 0,100,-1000 });

        EXPECT_TRUE(calc.curveStatus() == RiaJCurveCalculator::OK);

        cvf::Vec3d p11 = calc.firstArcEndpoint();
        EXPECT_NEAR(0, p11.x(), 1e-5);
        EXPECT_NEAR(100, p11.y(), 1e-5);
        EXPECT_NEAR(-100, p11.z(), 1e-5);

        cvf::Vec3d n = calc.firstNormal();
        EXPECT_NEAR(-1, n.x(), 1e-5);
        EXPECT_NEAR(0, n.y(), 1e-5);
        EXPECT_NEAR(0, n.z(), 1e-5);

        cvf::Vec3d c = calc.firstCenter();
        EXPECT_NEAR(0, c.x(), 1e-5);
        EXPECT_NEAR(0, c.y(), 1e-5);
        EXPECT_NEAR(-100, c.z(), 1e-5);
    }

    {
        RiaJCurveCalculator calc({ 0,0,0 }, 0, 0, 100, { 0, 0,-1000 });

        EXPECT_TRUE(calc.curveStatus() == RiaJCurveCalculator::OK_STRAIGHT_LINE);
    }

}

#include "RiaArcCurveCalculator.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaArcCurveCalculator, Basic)
{
    {
        RiaArcCurveCalculator calc({ 0,0,0 }, 0, M_PI/2, { 0,1000,-1000 });

        EXPECT_TRUE(calc.curveStatus() == RiaArcCurveCalculator::OK);
        EXPECT_NEAR(1000.0, calc.radius(), 1e-5);
        EXPECT_NEAR(M_PI/2, calc.arcAngle(), 1e-5);
        EXPECT_NEAR(M_PI/2*1000, calc.arcLength(), 1e-5);

        cvf::Vec3d center = calc.center();
        EXPECT_NEAR(    0, center.x(), 1e-5);
        EXPECT_NEAR(    0, center.y(), 1e-5);
        EXPECT_NEAR(-1000, center.z(), 1e-5);

        cvf::Vec3d n = calc.normal();
        EXPECT_NEAR(-1, n.x(), 1e-5);
        EXPECT_NEAR( 0, n.y(), 1e-5);
        EXPECT_NEAR( 0, n.z(), 1e-5);

        cvf::Vec3d te = calc.endTangent();
        EXPECT_NEAR( 0,  te.x(), 1e-5);
        EXPECT_NEAR( 0,  te.y(), 1e-5);
        EXPECT_NEAR(-1, te.z(), 1e-5);
    }

    {
        RiaArcCurveCalculator calc({ 0,0,0 }, 0, 0, { 0, 0,-1000 });

        EXPECT_TRUE(calc.curveStatus() == RiaArcCurveCalculator::OK_STRAIGHT_LINE);

        cvf::Vec3d te = calc.endTangent();
        EXPECT_NEAR(0, te.x(), 1e-5);
        EXPECT_NEAR(0, te.y(), 1e-5);
        EXPECT_NEAR(-1, te.z(), 1e-5);
    }

}