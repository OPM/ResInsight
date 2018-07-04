#pragma once
#include "cvfBase.h"
#include "cvfVector3.h"

class RiaSCurveCalculator
{
public:
    RiaSCurveCalculator( cvf::Vec3d p1, double azi1, double inc1, double r1,
                         cvf::Vec3d p2, double azi2, double inc2, double r2 );

    RiaSCurveCalculator( cvf::Vec3d p1, cvf::Vec3d q1,
                         cvf::Vec3d p2, cvf::Vec3d q2 );

    bool isOk()                      { return m_isCalculationOK;}
    cvf::Vec3d firstArcEndpoint()    { return m_firstArcEndpoint; }
    cvf::Vec3d secondArcStartpoint() { return m_secondArcStartpoint; }
    cvf::Vec3d firstCenter()         { return m_c1; }
    cvf::Vec3d secondCenter()        { return m_c2; }
    cvf::Vec3d firstNormal()         { return m_n1; }
    cvf::Vec3d secondNormal()        { return m_n2; }
    double firstRadius()             { return m_r1; }
    double secondRadius()            { return m_r2; }

private:
    void calculateEstimatedSolution();

    bool m_isCalculationOK;

    cvf::Vec3d m_firstArcEndpoint;
    cvf::Vec3d m_secondArcStartpoint;

    cvf::Vec3d m_c1;
    cvf::Vec3d m_c2;

    cvf::Vec3d m_n1;
    cvf::Vec3d m_n2;

    double m_r1; 
    double m_r2;
};
















