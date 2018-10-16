/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018- Statoil ASA
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
#include "cvfBase.h"
#include "cvfAssert.h"
#include "RiaOffshoreSphericalCoords.h"
#include "RiaJCurveCalculator.h"
#include "RiaSCurveCalculator.h"

#define M_PI       3.14159265358979323846   // pi

cvf::Vec3d smootheningTargetTangent(const cvf::Vec3d& p1, const cvf::Vec3d& p2, const cvf::Vec3d& p3);

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaLineArcWellPathCalculator::RiaLineArcWellPathCalculator(const cvf::Vec3d& referencePointXyz,
                                                           const std::vector<WellTarget>& activeWellPathTargets)
{
    // Handle incomplete input

    if (activeWellPathTargets.size() < 2)
    {
        m_startTangent = cvf::Vec3d::ZERO;
 
        if (activeWellPathTargets.size() == 1)
        {
            m_lineArcEndpoints.push_back(  activeWellPathTargets[0].targetPointXYZ + referencePointXyz );
            m_targetStatuses.resize(activeWellPathTargets.size(), 
                                    { !activeWellPathTargets[0].isTangentConstrained, 0.0, 0.0,
                                       true, std::numeric_limits<double>::infinity(),
                                       true, std::numeric_limits<double>::infinity() });
        }

        return;
    }

    m_targetStatuses.resize(activeWellPathTargets.size(), { false, 0.0, 0.0, 
                                                            false, std::numeric_limits<double>::infinity(), 
                                                            false, std::numeric_limits<double>::infinity() });

    std::vector<WellTarget> adjustedWellPathTargets = activeWellPathTargets;

    // Calculate sensible tangents for targets without a fixed one

    if ( activeWellPathTargets.size() > 2 )
    {
        for ( size_t tIdx = 0; tIdx < activeWellPathTargets.size() - 2; ++tIdx )
        {
            if ( !activeWellPathTargets[tIdx+1].isTangentConstrained )
            {
                cvf::Vec3d tangent = smootheningTargetTangent(activeWellPathTargets[tIdx  ].targetPointXYZ,
                                                              activeWellPathTargets[tIdx+1].targetPointXYZ,
                                                              activeWellPathTargets[tIdx+2].targetPointXYZ);
                RiaOffshoreSphericalCoords tangentSphCS(tangent);
                adjustedWellPathTargets[tIdx+1].azimuth = tangentSphCS.azi();
                adjustedWellPathTargets[tIdx+1].inclination = tangentSphCS.inc();
                adjustedWellPathTargets[tIdx+1].isTangentConstrained = true;

                m_targetStatuses[tIdx+1].hasDerivedTangent = true;
                m_targetStatuses[tIdx+1].resultAzimuth = tangentSphCS.azi();
                m_targetStatuses[tIdx+1].resultInclination = tangentSphCS.inc();

            }
        }
    }


    m_lineArcEndpoints.push_back(  activeWellPathTargets[0].targetPointXYZ + referencePointXyz );

    // Handle first segment if it is not an S-Curve

    size_t startSSegmentIdx = 0;
    size_t endSSegementIdx = activeWellPathTargets.size() - 1;

    if (!adjustedWellPathTargets[0].isTangentConstrained)
    {
        startSSegmentIdx = 1; 

        const WellTarget& target1 = adjustedWellPathTargets[0];
        const WellTarget& target2 = adjustedWellPathTargets[1];
        WellTargetStatus& target1Status = m_targetStatuses[0];
        WellTargetStatus& target2Status = m_targetStatuses[1];

        if (adjustedWellPathTargets[1].isTangentConstrained)
        {
            // Create an upside down J curve from target 2 back to 1

            RiaJCurveCalculator jCurve(target2.targetPointXYZ,
                                       target2.azimuth + M_PI,
                                       M_PI - target2.inclination,
                                       target2.radius1,
                                       target1.targetPointXYZ);

            if ( jCurve.curveStatus() == RiaJCurveCalculator::OK )
            {
                m_lineArcEndpoints.push_back(jCurve.firstArcEndpoint() + referencePointXyz);
            }
            else if ( jCurve.curveStatus() == RiaJCurveCalculator::FAILED_RADIUS_TOO_LARGE )
            {
                target2Status.hasOverriddenRadius1 = true;
                target2Status.resultRadius1 = jCurve.radius();
            }

            m_lineArcEndpoints.push_back(target2.targetPointXYZ + referencePointXyz);

            target1Status.hasDerivedTangent = true;
            target1Status.resultAzimuth = jCurve.endAzimuth() + M_PI;
            target1Status.resultInclination = M_PI - jCurve.endInclination();
        }
        else // The complete wellpath is a straight line from target 1 to 2
        {
            m_lineArcEndpoints.push_back(target2.targetPointXYZ + referencePointXyz );
            cvf::Vec3d t12 = target2.targetPointXYZ - target1.targetPointXYZ;
            RiaOffshoreSphericalCoords t12Sph(t12);

            target1Status.hasDerivedTangent = true;
            target1Status.resultAzimuth = t12Sph.azi();
            target1Status.resultInclination = t12Sph.inc();

            target2Status.hasDerivedTangent = true;
            target2Status.resultAzimuth = t12Sph.azi();
            target2Status.resultInclination = t12Sph.inc();
        }

        m_startTangent = RiaOffshoreSphericalCoords::unitVectorFromAziInc( target1Status.resultAzimuth, target1Status.resultInclination);
    }
    else
    {
        m_startTangent = RiaOffshoreSphericalCoords::unitVectorFromAziInc( activeWellPathTargets[0].azimuth, activeWellPathTargets[0].inclination);
    }

    if (!adjustedWellPathTargets.back().isTangentConstrained)
    {
        endSSegementIdx -= 1;
    }

    // Calculate S-curves

    if ( activeWellPathTargets.size() > 1 )
    {
        for ( size_t tIdx = startSSegmentIdx; tIdx < endSSegementIdx; ++tIdx )
        {
            const WellTarget& target1 = adjustedWellPathTargets[tIdx];
            const WellTarget& target2 = adjustedWellPathTargets[tIdx+1];
            WellTargetStatus& target1Status = m_targetStatuses[tIdx];
            WellTargetStatus& target2Status = m_targetStatuses[tIdx+1];

            // Ignore targets in the same place
            if ( (target1.targetPointXYZ - target2.targetPointXYZ).length() < 1e-6 ) continue;

            if ( target1.isTangentConstrained
                && target2.isTangentConstrained )
            {
                RiaSCurveCalculator sCurveCalc(target1.targetPointXYZ,
                                               target1.azimuth,
                                               target1.inclination,
                                               target1.radius2,
                                               target2.targetPointXYZ,
                                               target2.azimuth,
                                               target2.inclination,
                                               target2.radius1);

                if ( sCurveCalc.solveStatus() != RiaSCurveCalculator::CONVERGED )
                {
                    double p1p2Length = (target2.targetPointXYZ - target1.targetPointXYZ).length();
                    sCurveCalc = RiaSCurveCalculator::fromTangentsAndLength(target1.targetPointXYZ,
                                                                            target1.azimuth,
                                                                            target1.inclination,
                                                                            0.2*p1p2Length,
                                                                            target2.targetPointXYZ,
                                                                            target2.azimuth,
                                                                            target2.inclination,
                                                                            0.2*p1p2Length);

                    //RiaLogging::warning("Using fall-back calculation of well path geometry between active target number: " + QString::number(tIdx+1) + " and " + QString::number(tIdx+2));

                    target1Status.hasOverriddenRadius2 = true;
                    target1Status.resultRadius2 = sCurveCalc.firstRadius();

                    target2Status.hasOverriddenRadius1 = true;
                    target2Status.resultRadius1 = sCurveCalc.secondRadius();
                }

                m_lineArcEndpoints.push_back(sCurveCalc.firstArcEndpoint() + referencePointXyz);
                m_lineArcEndpoints.push_back(sCurveCalc.secondArcStartpoint() + referencePointXyz);
                m_lineArcEndpoints.push_back(target2.targetPointXYZ + referencePointXyz);
            }

        }
    }

    // Handle last segment if (its not the same as the first) and it has not been handled as an S-Curve

    if ( adjustedWellPathTargets.size() > 2 && endSSegementIdx < (adjustedWellPathTargets.size() - 1) )
    {
        size_t targetCount = adjustedWellPathTargets.size();
        const WellTarget& target1 = adjustedWellPathTargets[targetCount-2];
        const WellTarget& target2 = adjustedWellPathTargets[targetCount-1];
        WellTargetStatus& target1Status = m_targetStatuses[targetCount-2];
        WellTargetStatus& target2Status = m_targetStatuses[targetCount-1];

        // Create an ordinary J curve

        RiaJCurveCalculator jCurve(target1.targetPointXYZ,
                                   target1.azimuth,
                                   target1.inclination,
                                   target1.radius2,
                                   target2.targetPointXYZ);

        if ( jCurve.curveStatus() == RiaJCurveCalculator::OK )
        {
            m_lineArcEndpoints.push_back(jCurve.firstArcEndpoint() + referencePointXyz);
        }
        else if ( jCurve.curveStatus() == RiaJCurveCalculator::FAILED_RADIUS_TOO_LARGE )
        {
            target1Status.hasOverriddenRadius2 = true;
            target1Status.resultRadius2 = jCurve.radius();
        }

        m_lineArcEndpoints.push_back(target2.targetPointXYZ + referencePointXyz);

        target2Status.hasDerivedTangent = true;
        target2Status.resultAzimuth = jCurve.endAzimuth();
        target2Status.resultInclination = jCurve.endInclination();
    }

}


cvf::Vec3d smootheningTargetTangent(const cvf::Vec3d& p1, const cvf::Vec3d& p2, const cvf::Vec3d& p3)
{
    cvf::Vec3d t12 = p2 - p1;
    cvf::Vec3d t23 = p3 - p2;

    double length12 = t12.length();
    double length23 = t23.length();

    t12 /= length12; // Normalize
    t23 /= length23; // Normalize

    cvf::Vec3d t1t2Hor(t12);
    t1t2Hor.z() = 0.0;
    double t12HorLength = t1t2Hor.length();

    cvf::Vec3d t23Hor(t23);
    t23Hor.z() = 0.0;
    double t23HorLength = t23Hor.length();

    // Calculate weights as combo of inverse distance and horizontal component

    double w12 = t12HorLength * 1.0/length12;
    double w23 = t23HorLength * 1.0/length23;

    // Weight the tangents 

    t12 *= w12; // Weight
    t23 *= w23; // Weight

                // Sum and normalization of weights
    cvf::Vec3d averageTangent = 1.0/(w12 + w23) * (t12 + t23);

    return averageTangent;
}

