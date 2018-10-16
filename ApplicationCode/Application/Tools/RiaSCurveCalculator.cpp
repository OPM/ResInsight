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

#include "cvfMatrix4.h"

#include <cmath>
#include <iostream>

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
                                         , m_ctrlPpointCurveStatus(NOT_SET)
                                         , m_solveStatus(NOT_SOLVED)
{
    initializeByFinding_q1q2(p1, azi1, inc1, rad1, p2, azi2, inc2, rad2);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaSCurveCalculator::RiaSCurveCalculator(cvf::Vec3d p1, cvf::Vec3d q1, 
                                         cvf::Vec3d p2, cvf::Vec3d q2)
    : m_isCalculationOK(true)
    , m_p1(p1)
    , m_p2(p2)
    , m_ctrlPpointCurveStatus(NOT_SET)
    , m_solveStatus(NOT_SOLVED)
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

    if (!m_isCalculationOK)
    {
        m_ctrlPpointCurveStatus = FAILED_INPUT_OVERLAP;
    }

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

            if (m_ctrlPpointCurveStatus == NOT_SET)
            {
                m_ctrlPpointCurveStatus = OK_INFINITE_RADIUS1;
            }
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

            if (m_ctrlPpointCurveStatus == NOT_SET)
            {
                m_ctrlPpointCurveStatus = OK_INFINITE_RADIUS2;
            }
            else if (m_ctrlPpointCurveStatus == OK_INFINITE_RADIUS1)
            {
                m_ctrlPpointCurveStatus = OK_INFINITE_RADIUS12;
            }
        }
    }

    m_firstArcEndpoint    = q1 + (q1 - p1).length() * tq1q2;
    m_secondArcStartpoint = q2 - (q2 - p2).length() * tq1q2;

    if (((q1 - p1).length() + (q2 - p2).length()) > (q2 - q1).length()) // first arc end and second arc start is overlapping
    {
        m_ctrlPpointCurveStatus = FAILED_ARC_OVERLAP;
        m_isCalculationOK = false;
    }

    if (m_ctrlPpointCurveStatus == NOT_SET)
    {
        m_ctrlPpointCurveStatus = OK;
    }

    // The Circle normals. Will be set to cvf::Vec3d::ZERO if undefined.

    m_n1 = (t1 ^ tq1q2).getNormalized();
    m_n2 = (tq1q2 ^ t2).getNormalized();
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

//--------------------------------------------------------------------------------------------------
/// 
/// Needs to calculate J^-1 * [R1_error, R2_error]
///       | dR1_dq1   dR1_dq2  |                           1                   |  dR2_dq2  -dR1_dq2 | 
///  J =  |                    |    J^-1 = ----------------------------------  |                    | 
///       | dR2_dq1   dR2_dq2  |           dR1_dq1*dR2_dq2 - dR1_dq2*dR2_dq1   | -dR2_dq1   dR1_dq1 | 
///
/// | q1_step |           | R1_Error |
/// |         |  = - J^-1 |          |
/// | q2_step |           | R2_Error |
///
//--------------------------------------------------------------------------------------------------
void calculateNewStepsFromJacobi(double dR1_dq1, double dR1_dq2,
                                 double dR2_dq1, double dR2_dq2, 
                                 double R1_error, 
                                 double R2_error,
                                 double * newStepq1, 
                                 double * newStepq2)
{
    double invJacobiScale = 1.0/ (dR1_dq1*dR2_dq2 - dR2_dq1*dR1_dq2);
    double invJacobi_R1q1 =  invJacobiScale * dR2_dq2;
    double invJacobi_R1q2 =  invJacobiScale * -dR1_dq2;
    double invJacobi_R2q1 =  invJacobiScale * -dR2_dq1;
    double invJacobi_R2q2 =  invJacobiScale * dR1_dq1;

    (*newStepq1) = - (invJacobi_R1q1 * R1_error + invJacobi_R1q2 * R2_error);
    (*newStepq2) = - (invJacobi_R2q1 * R1_error + invJacobi_R2q2 * R2_error) ;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool isZeroCrossing(double newError, double oldError, double maxError)
{
    if ( (newError < -maxError &&  maxError < oldError) ||  (newError > maxError &&  -maxError > oldError) )
    {
        return true;
    }    

    return false;
}

//--------------------------------------------------------------------------------------------------
///
/// Iterating with changing q1, q2 (lengths along tangent) to find solution with R1 = r1 and R2 = r2
/// R1(q1, q2), R2(q1, q2)
///
//--------------------------------------------------------------------------------------------------
void RiaSCurveCalculator::initializeByFinding_q1q2(cvf::Vec3d p1, double azi1, double inc1, double r1, 
                                                      cvf::Vec3d p2, double azi2, double inc2, double r2)
{
    // Algorithm options

    const int maxIterations = 40;
    const double maxError = 0.01;
    const double closeError = 40*maxError;
    const double maxStepSize = 1.0e9;
    const double maxLengthToQ = 1.0e10;
    bool enableBackstepping = false;
    //#define USE_JACOBI_UPDATE
    //#define DEBUG_OUTPUT_ON


    // Needs the initial partial derivatives to see the direction of change
    // dR1/dq1, dR1/dq2, dR2/dq1, dR2/dq2
    // Selects a sensible point in the domain for the evaluation

    double p1p2Length =  (p2 - p1).length();
    double delta =  0.01 * p1p2Length;
    double initialq1q2 = 0.2 * p1p2Length;
    double deltaPos = initialq1q2 + delta;

    RiaSCurveCalculator ev_0 = RiaSCurveCalculator::fromTangentsAndLength(p1, azi1, inc1, initialq1q2,
                                                                          p2, azi2, inc2, initialq1q2);

    if ( ev_0.curveStatus() == RiaSCurveCalculator::OK_INFINITE_RADIUS12 )
    {
        *this = ev_0;
        this->m_solveStatus = CONVERGED;
        return;
    } // Todo: Handle infinite radius in one place
    
    RiaSCurveCalculator ev_dq1 = RiaSCurveCalculator::fromTangentsAndLength(p1, azi1, inc1, deltaPos,
                                                                          p2, azi2, inc2, initialq1q2);
    RiaSCurveCalculator ev_dq2 = RiaSCurveCalculator::fromTangentsAndLength(p1, azi1, inc1, initialq1q2,
                                                                            p2, azi2, inc2, deltaPos);

    // Initial Jacobi
    double dR1_dq1 = ((r1 - ev_dq1.firstRadius()) - (r1 - ev_0.firstRadius()))/delta;
    double dR2_dq2 = ((r2 - ev_dq2.secondRadius()) - (r2 - ev_0.secondRadius()))/delta;

    // Initial function value (error)
    double R1_error = r1 - ev_0.firstRadius();
    double R2_error = r2 - ev_0.secondRadius();

    // First steps
    double q1Step = -R1_error/dR1_dq1;
    double q2Step = -R2_error/dR2_dq2;

    #ifdef USE_JACOBI_UPDATE
    double dR1_dq2 = ((r1 - ev_dq2.firstRadius()) - (r1 - ev_0.firstRadius()))/delta;
    double dR2_dq1 = ((r2 - ev_dq1.secondRadius()) - (r2 - ev_0.secondRadius()))/delta;

    calculateNewStepsFromJacobi(dR1_dq1, dR1_dq2, 
                                dR2_dq1, dR2_dq2, 
                                R1_error, R2_error, 
                                &q1Step, &q2Step);
    #endif

    double q1 = initialq1q2;
    double q2 = initialq1q2;
    

    #ifdef DEBUG_OUTPUT_ON
    std::cout << std::endl;
    std::cout << "Targets: R1, R2: " << r1 << " , " << r2 << std::endl;

    std::cout << 0 << ": " << q1Step << " , " << q2Step 
              << " : " << q1 << " , " << q2 << " | "
              << ev_0.isOk() << " : " << ev_0.firstRadius() << " , " << ev_0.secondRadius()
              << " : " << R1_error << " , " << R2_error  << std::endl;
    #endif
    
    SolveStatus solveResultStatus = NOT_SOLVED;

    int backstepLevel = 0;
    int iteration = 1;
    for ( iteration = 1; iteration < maxIterations; ++iteration)
    {
        if ( fabs(q1Step) > maxStepSize || fabs(q2Step) > maxStepSize )
        {
            solveResultStatus = FAILED_MAX_TANGENT_STEP_REACHED;

            break;
        }

        std::string q1R1StepCorrMarker;
        std::string q2R2StepCorrMarker;

        if (q1 + q1Step < 0) { q1Step =  -0.9*q1; q1R1StepCorrMarker = "*";}
        if (q2 + q2Step < 0) { q2Step =  -0.9*q2; q2R2StepCorrMarker = "*"; }

        q1 += q1Step;
        q2 += q2Step;

        if (fabs(q1) > maxLengthToQ || fabs(q2) > maxLengthToQ)
        {
            /// Max length along tangent reached
            solveResultStatus = FAILED_MAX_LENGTH_ALONG_TANGENT_REACHED;
            break;
        }

        RiaSCurveCalculator ev_1 = RiaSCurveCalculator::fromTangentsAndLength(p1, azi1, inc1, q1,
                                                                              p2, azi2, inc2, q2);

        double R1_error_new = r1 - ev_1.firstRadius();
        double R2_error_new = r2 - ev_1.secondRadius();

        #ifdef DEBUG_OUTPUT_ON
        std::cout << iteration << ": " << q1Step << q1R1StepCorrMarker << " , " << q2Step<< q2R2StepCorrMarker 
            << " : " << q1 << " , " << q2 << " | "
            << ev_1.isOk() << " : " << ev_1.firstRadius() << " , " << ev_1.secondRadius() 
            << " : " << R1_error_new << " , " << R2_error_new ;
        #endif

        if  ( ( fabs(R1_error_new) < maxError || ev_1.curveStatus() == OK_INFINITE_RADIUS1 ) 
           && ( fabs(R2_error_new) < maxError || ev_1.curveStatus() == OK_INFINITE_RADIUS2 ) )
        {
            ev_0 = ev_1;

            // Result ok !
            
            solveResultStatus = CONVERGED;

            #ifdef DEBUG_OUTPUT_ON
            std::cout << std::endl;
            #endif

            break;
        }

        if (enableBackstepping) // Experimental back-stepping 
        {
            bool isZeroCrossingR1 = isZeroCrossing(R1_error_new, R1_error, maxError);
            bool isZeroCrossingR2 = isZeroCrossing(R2_error_new, R2_error, maxError);

            if ( isZeroCrossingR2 || isZeroCrossingR1 )
            {
                q1 -= q1Step;
                q2 -= q2Step;

                //if (isZeroCrossingR1) 
                q1Step = 0.9* q1Step * fabs(R1_error) /(fabs(R1_error_new) + fabs(R1_error));
                //if (isZeroCrossingR2) 
                q2Step = 0.9* q2Step * fabs(R2_error) /(fabs(R2_error_new) + fabs(R2_error));

                ++backstepLevel;

                #ifdef DEBUG_OUTPUT_ON
                std::cout << " Backstep needed. "<< std::endl;
                #endif

                continue;
            }
            else
            {
                backstepLevel = 0;
            }
        }

        #ifdef DEBUG_OUTPUT_ON
        std::cout << std::endl;
        #endif
 
        #ifdef USE_JACOBI_UPDATE

        /// Update Jacobi using Broyden
        //               (R_error_n-Rerror_n-1) - Jn-1*dq
        //  J_n = Jn-1 + --------------------------------- (dq)T
        //                      | dqn |^2
        // 

        double dR1_error =  R1_error_new - R1_error;
        double dR2_error =  R2_error_new - R2_error;
        R1_error = R1_error_new;
        R2_error = R2_error_new;

        double stepNormScale = 1.0/(q1Step*q1Step + q2Step*q2Step);

        dR1_dq1 = dR1_dq1 + stepNormScale * (q1Step * (dR1_error - q1Step * dR1_dq1 + q2Step * dR1_dq2) );
        dR1_dq2 = dR1_dq2 + stepNormScale * (q2Step * (dR1_error - q1Step * dR1_dq1 + q2Step * dR1_dq2) );
        dR2_dq1 = dR2_dq1 + stepNormScale * (q1Step * (dR2_error - q1Step * dR2_dq1 + q2Step * dR2_dq2) );
        dR2_dq2 = dR2_dq2 + stepNormScale * (q2Step * (dR2_error - q1Step * dR2_dq1 + q2Step * dR2_dq2) );

        calculateNewStepsFromJacobi(dR1_dq1, dR1_dq2, 
                                    dR2_dq1, dR2_dq2, 
                                    R1_error, R2_error, 
                                    &q1Step, &q2Step);

        #else

        dR1_dq1 = ((r1 - ev_1.firstRadius()) - (r1 - ev_0.firstRadius()))/q1Step;
        dR2_dq2 = ((r2 - ev_1.secondRadius()) - (r2 - ev_0.secondRadius()))/q2Step;

        R1_error = R1_error_new;
        R2_error = R2_error_new;

        q1Step = -R1_error/dR1_dq1;
        q2Step = -R2_error/dR2_dq2;

        #endif

        ev_0 = ev_1;
    }


    *this = ev_0;
    if ( iteration >= maxIterations )
    {
        m_solveStatus = FAILED_MAX_ITERATIONS_REACHED;
        // Max iterations reached
    }
    else
    {
        m_solveStatus = solveResultStatus;
    }
  
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
    std::cout << "  CtrPointStatus: " << m_ctrlPpointCurveStatus << " SolveStatus: " <<  m_solveStatus << std::endl;

}

