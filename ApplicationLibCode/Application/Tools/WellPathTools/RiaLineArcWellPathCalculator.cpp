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

#include "RiaLineArcWellPathCalculator.h"

#include "RiaJCurveCalculator.h"
#include "RiaOffshoreSphericalCoords.h"
#include "RiaSCurveCalculator.h"
#include "cvfAssert.h"

#define M_PI 3.14159265358979323846 // pi

cvf::Vec3d smootheningTargetTangent( const cvf::Vec3d& p1, const cvf::Vec3d& p2, const cvf::Vec3d& p3 );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaLineArcWellPathCalculator::RiaLineArcWellPathCalculator( const cvf::Vec3d&              referencePointXyz,
                                                            const std::vector<WellTarget>& activeWellPathTargets )
{
    // Handle incomplete input

    if ( activeWellPathTargets.size() < 2 )
    {
        m_startTangent = cvf::Vec3d::ZERO;

        if ( activeWellPathTargets.size() == 1 )
        {
            m_lineArcEndpoints.push_back( activeWellPathTargets[0].targetPointXYZ + referencePointXyz );
            m_targetStatuses
                .resize( activeWellPathTargets.size(),
                         { 0.0, 0.0, false, true, std::numeric_limits<double>::infinity(), false, true, std::numeric_limits<double>::infinity() } );
        }

        return;
    }

    m_targetStatuses
        .resize( activeWellPathTargets.size(),
                 { 0.0, 0.0, false, false, std::numeric_limits<double>::infinity(), false, false, std::numeric_limits<double>::infinity() } );

    std::vector<WellTarget> adjustedWellPathTargets = activeWellPathTargets;

    // Calculate sensible tangents for targets without a fixed one

    if ( activeWellPathTargets.size() > 2 )
    {
        for ( size_t tIdx = 0; tIdx < activeWellPathTargets.size() - 2; ++tIdx )
        {
            cvf::Vec3d tangent = smootheningTargetTangent( activeWellPathTargets[tIdx].targetPointXYZ,
                                                           activeWellPathTargets[tIdx + 1].targetPointXYZ,
                                                           activeWellPathTargets[tIdx + 2].targetPointXYZ );

            RiaOffshoreSphericalCoords tangentSphCS( tangent );
            if ( !adjustedWellPathTargets[tIdx + 1].isAzimuthConstrained )
                adjustedWellPathTargets[tIdx + 1].azimuthRadians = tangentSphCS.azi();

            if ( !adjustedWellPathTargets[tIdx + 1].isInclinationConstrained )
                adjustedWellPathTargets[tIdx + 1].inclinationRadians = tangentSphCS.inc();

            adjustedWellPathTargets[tIdx + 1].isAzimuthConstrained     = true;
            adjustedWellPathTargets[tIdx + 1].isInclinationConstrained = true;

            m_targetStatuses[tIdx + 1].resultAzimuthRadians     = adjustedWellPathTargets[tIdx + 1].azimuthRadians;
            m_targetStatuses[tIdx + 1].resultInclinationRadians = adjustedWellPathTargets[tIdx + 1].inclinationRadians;
        }
    }

    m_lineArcEndpoints.push_back( activeWellPathTargets[0].targetPointXYZ + referencePointXyz );

    // Handle first segment if it is not an S-Curve

    size_t startSSegmentIdx = 0;
    size_t endSSegementIdx  = activeWellPathTargets.size() - 1;

    if ( !adjustedWellPathTargets[0].isAnyDirectionFixed() )
    {
        startSSegmentIdx = 1;

        const WellTarget& target1       = adjustedWellPathTargets[0];
        const WellTarget& target2       = adjustedWellPathTargets[1];
        WellTargetStatus& target1Status = m_targetStatuses[0];
        WellTargetStatus& target2Status = m_targetStatuses[1];

        if ( adjustedWellPathTargets[1].isAnyDirectionFixed() )
        {
            // Create an upside down J curve from target 2 back to 1

            RiaJCurveCalculator jCurve( target2.targetPointXYZ,
                                        target2.azimuthRadians + M_PI,
                                        M_PI - target2.inclinationRadians,
                                        target2.radius1,
                                        target1.targetPointXYZ );

            if ( jCurve.curveStatus() == RiaJCurveCalculator::OK )
            {
                m_lineArcEndpoints.push_back( jCurve.firstArcEndpoint() + referencePointXyz );
            }
            else if ( jCurve.curveStatus() == RiaJCurveCalculator::FAILED_RADIUS_TOO_LARGE )
            {
                target2Status.hasOverriddenRadius1 = true;
            }
            target2Status.resultRadius1 = jCurve.radius();

            m_lineArcEndpoints.push_back( target2.targetPointXYZ + referencePointXyz );

            target1Status.resultAzimuthRadians     = jCurve.endAzimuth() + M_PI;
            target1Status.resultInclinationRadians = M_PI - jCurve.endInclination();

            target2Status.isRadius1Editable = true;
        }
        else // The complete wellpath is a straight line from target 1 to 2
        {
            m_lineArcEndpoints.push_back( target2.targetPointXYZ + referencePointXyz );
            cvf::Vec3d                 t12 = target2.targetPointXYZ - target1.targetPointXYZ;
            RiaOffshoreSphericalCoords t12Sph( t12 );

            target1Status.resultAzimuthRadians     = t12Sph.azi();
            target1Status.resultInclinationRadians = t12Sph.inc();

            target2Status.resultAzimuthRadians     = t12Sph.azi();
            target2Status.resultInclinationRadians = t12Sph.inc();
        }

        m_startTangent =
            RiaOffshoreSphericalCoords::unitVectorFromAziInc( target1Status.resultAzimuthRadians, target1Status.resultInclinationRadians );
    }
    else
    {
        m_startTangent = RiaOffshoreSphericalCoords::unitVectorFromAziInc( activeWellPathTargets[0].azimuthRadians,
                                                                           activeWellPathTargets[0].inclinationRadians );
    }

    if ( !adjustedWellPathTargets.back().isAnyDirectionFixed() )
    {
        endSSegementIdx -= 1;
    }

    // Calculate S-curves

    if ( activeWellPathTargets.size() > 1 )
    {
        for ( size_t tIdx = startSSegmentIdx; tIdx < endSSegementIdx; ++tIdx )
        {
            const WellTarget& target1       = adjustedWellPathTargets[tIdx];
            const WellTarget& target2       = adjustedWellPathTargets[tIdx + 1];
            WellTargetStatus& target1Status = m_targetStatuses[tIdx];
            WellTargetStatus& target2Status = m_targetStatuses[tIdx + 1];

            // Ignore targets in the same place
            if ( ( target1.targetPointXYZ - target2.targetPointXYZ ).length() < 1e-6 ) continue;

            if ( target1.isAnyDirectionFixed() && target2.isAnyDirectionFixed() )
            {
                RiaSCurveCalculator sCurveCalc( target1.targetPointXYZ,
                                                target1.azimuthRadians,
                                                target1.inclinationRadians,
                                                target1.radius2,
                                                target2.targetPointXYZ,
                                                target2.azimuthRadians,
                                                target2.inclinationRadians,
                                                target2.radius1 );

                if ( sCurveCalc.solveStatus() != RiaSCurveCalculator::CONVERGED )
                {
                    double p1p2Length = ( target2.targetPointXYZ - target1.targetPointXYZ ).length();
                    sCurveCalc        = RiaSCurveCalculator::fromTangentsAndLength( target1.targetPointXYZ,
                                                                             target1.azimuthRadians,
                                                                             target1.inclinationRadians,
                                                                             0.2 * p1p2Length,
                                                                             target2.targetPointXYZ,
                                                                             target2.azimuthRadians,
                                                                             target2.inclinationRadians,
                                                                             0.2 * p1p2Length );

                    // RiaLogging::warning("Using fall-back calculation of well path geometry between active target
                    // number: " + QString::number(tIdx+1) + " and " + QString::number(tIdx+2));

                    target1Status.hasOverriddenRadius2 = true;
                    target2Status.hasOverriddenRadius1 = true;
                }

                target2Status.resultRadius1     = sCurveCalc.secondRadius();
                target1Status.resultRadius2     = sCurveCalc.firstRadius();
                target2Status.isRadius1Editable = true;
                target1Status.isRadius2Editable = true;

                m_lineArcEndpoints.push_back( sCurveCalc.firstArcEndpoint() + referencePointXyz );
                m_lineArcEndpoints.push_back( sCurveCalc.secondArcStartpoint() + referencePointXyz );
                m_lineArcEndpoints.push_back( target2.targetPointXYZ + referencePointXyz );
            }
        }
    }

    // Handle last segment if (its not the same as the first) and it has not been handled as an S-Curve

    if ( adjustedWellPathTargets.size() > 2 && endSSegementIdx < ( adjustedWellPathTargets.size() - 1 ) )
    {
        size_t            targetCount   = adjustedWellPathTargets.size();
        const WellTarget& target1       = adjustedWellPathTargets[targetCount - 2];
        const WellTarget& target2       = adjustedWellPathTargets[targetCount - 1];
        WellTargetStatus& target1Status = m_targetStatuses[targetCount - 2];
        WellTargetStatus& target2Status = m_targetStatuses[targetCount - 1];

        // Create an ordinary J curve

        RiaJCurveCalculator jCurve( target1.targetPointXYZ,
                                    target1.azimuthRadians,
                                    target1.inclinationRadians,
                                    target1.radius2,
                                    target2.targetPointXYZ );

        if ( jCurve.curveStatus() == RiaJCurveCalculator::OK )
        {
            m_lineArcEndpoints.push_back( jCurve.firstArcEndpoint() + referencePointXyz );
        }
        else if ( jCurve.curveStatus() == RiaJCurveCalculator::FAILED_RADIUS_TOO_LARGE )
        {
            target1Status.hasOverriddenRadius2 = true;
        }

        target1Status.resultRadius2     = jCurve.radius();
        target1Status.isRadius2Editable = true;

        m_lineArcEndpoints.push_back( target2.targetPointXYZ + referencePointXyz );

        target2Status.resultAzimuthRadians     = jCurve.endAzimuth();
        target2Status.resultInclinationRadians = jCurve.endInclination();
    }
}

cvf::Vec3d smootheningTargetTangent( const cvf::Vec3d& p1, const cvf::Vec3d& p2, const cvf::Vec3d& p3 )
{
    cvf::Vec3d t12 = p2 - p1;
    cvf::Vec3d t23 = p3 - p2;

    double length12 = t12.length();
    double length23 = t23.length();

    t12 /= length12; // Normalize
    t23 /= length23; // Normalize

    cvf::Vec3d t1t2Hor( t12 );
    t1t2Hor.z()         = 0.0;
    double t12HorLength = t1t2Hor.length();

    cvf::Vec3d t23Hor( t23 );
    t23Hor.z()          = 0.0;
    double t23HorLength = t23Hor.length();

    // Calculate weights as combo of inverse distance and horizontal component

    double w12 = t12HorLength * 1.0 / length12;
    double w23 = t23HorLength * 1.0 / length23;

    // Weight the tangents

    t12 *= w12; // Weight
    t23 *= w23; // Weight

    // Sum and normalization of weights
    cvf::Vec3d averageTangent = 1.0 / ( w12 + w23 ) * ( t12 + t23 );

    return averageTangent;
}
