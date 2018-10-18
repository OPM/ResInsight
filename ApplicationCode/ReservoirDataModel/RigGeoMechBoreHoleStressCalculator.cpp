#include "RigGeoMechBoreHoleStressCalculator.h"

//==================================================================================================
/// Internal root finding class to find a Well Pressure that gives:
/// a) a zero SigmaT for estimating the fracture gradient.
/// b) a solution to the Stassi-d'Alia failure criterion for estimating the shear failure gradient.
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGeoMechBoreHoleStressCalculator::RigGeoMechBoreHoleStressCalculator(const caf::Ten3d& tensor,
                                                                       double            porePressure,
                                                                       double            poissonRatio,
                                                                       double            uniaxialCompressiveStrength,
                                                                       int               nThetaSubSamples)
    : m_tensor(tensor)
    , m_porePressure(porePressure)
    , m_poissonRatio(poissonRatio)
    , m_uniaxialCompressiveStrength(uniaxialCompressiveStrength)
    , m_nThetaSubSamples(nThetaSubSamples)
{
    calculateStressComponents();
}


//--------------------------------------------------------------------------------------------------
/// Simple bisection method for now
//--------------------------------------------------------------------------------------------------
double RigGeoMechBoreHoleStressCalculator::solveFractureGradient(double* thetaOut)
{
    MemberFunc fn = &RigGeoMechBoreHoleStressCalculator::sigmaTMinOfMin;
    return solveSecant(fn, thetaOut);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigGeoMechBoreHoleStressCalculator::solveStassiDalia(double* thetaOut)
{
    MemberFunc fn = &RigGeoMechBoreHoleStressCalculator::stassiDalia;
    return solveSecant(fn, thetaOut);
}

//--------------------------------------------------------------------------------------------------
/// Bi-section root finding method: https://en.wikipedia.org/wiki/Bisection_method
/// Used as fall-back in case the secant method doesn't converge.
//--------------------------------------------------------------------------------------------------
double RigGeoMechBoreHoleStressCalculator::solveBisection(double minPw, double maxPw, MemberFunc fn, double* thetaOut)
{
    const int N = 50;
    const double epsilon = 1.0e-10;

    double theta = 0.0;

    std::pair<double, double> largestNegativeValue(0.0, -std::numeric_limits<double>::infinity());
    std::pair<double, double> smallestPositiveValue (0.0, std::numeric_limits<double>::infinity());
        
    for (int i = 0; i <= N; ++i)
    {
        double pw = minPw + (maxPw - minPw) * i / static_cast<double>(N);
        double f_pw = (this->*fn)(pw, &theta);
        if (f_pw >= 0.0 && f_pw < smallestPositiveValue.second)
        {
            smallestPositiveValue = std::make_pair(pw, f_pw);
        }
        if (f_pw < 0.0 && f_pw > largestNegativeValue.second)
        {
            largestNegativeValue = std::make_pair(pw, f_pw);
        }        
    }

    // TODO: Provide a warning if there was no solution to the equation
    if (largestNegativeValue.second == -std::numeric_limits<double>::infinity())
    {
        // No solution. Function is always positive. Pick smallest value.
        return smallestPositiveValue.first;
    }
    if (smallestPositiveValue.second == std::numeric_limits<double>::infinity())
    {
        // No solution. Function is always negative. Pick largest value.
        return largestNegativeValue.first;        
    }
    minPw = largestNegativeValue.first;
    double minPwFuncVal = largestNegativeValue.second;
    maxPw = smallestPositiveValue.first;
    double maxPwFuncVal = smallestPositiveValue.second;

    double range = std::abs(maxPw - minPw);
    
    int i = 0;
    for (; i <= N && range > m_porePressure * epsilon; ++i)
    {
        double midPw = (minPw + maxPw) * 0.5;
        double midPwFuncVal = (this->*fn)(midPw, &theta);
        if (midPwFuncVal * minPwFuncVal < 0.0)
        {
            maxPw = midPw;
            maxPwFuncVal = midPwFuncVal;
        }
        else
        {
            minPw = midPw;
            minPwFuncVal = midPwFuncVal;
        }
        range = std::abs(maxPw - minPw);
    }
    CVF_ASSERT(i < N); // Otherwise it hasn't converged                       

    if (thetaOut)
    {
        *thetaOut = theta;
    }
    
    // Return average of minPw and maxPw.
    return 0.5 * (maxPw + minPw);
}

//--------------------------------------------------------------------------------------------------
/// Secant root finding method: https://en.wikipedia.org/wiki/Secant_method
/// Basically a Newton's method using finite differences for the derivative.
//--------------------------------------------------------------------------------------------------
double RigGeoMechBoreHoleStressCalculator::solveSecant(MemberFunc fn, double* thetaOut)
{
    const double epsilon = 1.0e-10;
    const int N = 50;
    double theta = 0.0;

    double x_0 = 0.0;    
    double f_x0 = (this->*fn)(x_0, &theta);
    double x_1 = m_porePressure;
    double f_x1 = (this->*fn)(x_1, &theta);
    double x = 0.0;
    double f_x = 0.0;
    int i = 0;
    for (; i <= N && std::abs(f_x1 - f_x0) > epsilon; ++i)
    {
        x = x_1 - f_x1 * (x_1 - x_0) / (f_x1 - f_x0);
        f_x = (this->*fn)(x, &theta);
        if (std::abs(f_x) < epsilon * m_porePressure) break;

        // Update iteration variables
        x_0 = x_1;
        f_x0 = f_x1;
        x_1 = x;
        f_x1 = f_x;        
    }

    if (i == N || std::abs(f_x) > epsilon * m_porePressure)
    {
        // Fallback to bisection if secant doesn't converge or converged to a wrong solution.
        return solveBisection(0.0, m_porePressure * 2.0, fn, thetaOut);
    }

    if (thetaOut)
    {
        *thetaOut = theta;
    }
    return x;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigGeoMechBoreHoleStressCalculator::sigmaTMinOfMin(double wellPressure, double* thetaAtMin) const
{
    CVF_ASSERT(thetaAtMin);
    double sigma_t_min_min = std::numeric_limits<double>::max();
    for (const cvf::Vec4d& stressComponentsForAngle : m_stressComponents)
    {
        // Perform all these internal calculations in double to reduce significance errors
        double sigma_theta = stressComponentsForAngle[1] - wellPressure;
        const double& sigma_z = stressComponentsForAngle[2];
        double tauSqrx4 = std::pow(stressComponentsForAngle[3], 2) * 4.0;
        double sigma_t_min = 0.5 * ((sigma_z + sigma_theta) - std::sqrt(std::pow(sigma_z - sigma_theta, 2) + tauSqrx4)) - m_porePressure;
        if (sigma_t_min < sigma_t_min_min)
        {
            sigma_t_min_min = sigma_t_min;
            *thetaAtMin = stressComponentsForAngle[0];
        }
    }
    return sigma_t_min_min;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigGeoMechBoreHoleStressCalculator::stassiDalia(double wellPressure, double* thetaAtMin) const
{
    CVF_ASSERT(thetaAtMin);
    double minStassiDalia = std::numeric_limits<double>::max();
    for (const cvf::Vec4d& stressComponentsForAngle : m_stressComponents)
    {
        double sigma_theta = stressComponentsForAngle[1] - wellPressure;
        const double& sigma_z = stressComponentsForAngle[2];
        double tauSqrx4 = std::pow(stressComponentsForAngle[3], 2) * 4.0;

        double sigma_1 = wellPressure - m_porePressure;
        double sigma_2 = 0.5 * ((sigma_z + sigma_theta) + std::sqrt(std::pow(sigma_z - sigma_theta, 2) + tauSqrx4)) - m_porePressure;
        double sigma_3 = 0.5 * ((sigma_z + sigma_theta) - std::sqrt(std::pow(sigma_z - sigma_theta, 2) + tauSqrx4)) - m_porePressure;

        double stassiDalia = std::pow(sigma_1 - sigma_2, 2) + std::pow(sigma_2 - sigma_3, 2) + std::pow(sigma_1 - sigma_3, 2)
                          - 2 * m_uniaxialCompressiveStrength * (sigma_1 + sigma_2 + sigma_3);

        if (stassiDalia < minStassiDalia)
        {
            minStassiDalia = stassiDalia;
            *thetaAtMin = stressComponentsForAngle[0];
        }
    }
    return minStassiDalia;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigGeoMechBoreHoleStressCalculator::calculateStressComponents()
{
    m_stressComponents.reserve(m_nThetaSubSamples);

    for (int i = 0; i < m_nThetaSubSamples; ++i)
    {
        double theta = (i *cvf::PI_F) / (m_nThetaSubSamples - 1.0);
        cvf::Vec4d stressComponentsForAngle = calculateStressComponentsForSegmentAngle(theta);
        m_stressComponents.push_back(stressComponentsForAngle);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Vec4d RigGeoMechBoreHoleStressCalculator::calculateStressComponentsForSegmentAngle(double theta) const
{
    cvf::Vec4d stressComponents;

    const double& sx = m_tensor[caf::Ten3d::SXX];
    const double& sy = m_tensor[caf::Ten3d::SYY];
    const double& sz = m_tensor[caf::Ten3d::SZZ];
    const double& txy = m_tensor[caf::Ten3d::SXY];
    const double& txz = m_tensor[caf::Ten3d::SZX];
    const double& tyz = m_tensor[caf::Ten3d::SYZ];

    stressComponents[0] = theta;
    stressComponents[1] = sx + sy - 2 * (sx - sy) * cos(2 * theta) - 4 * txy * sin(2 * theta);
    stressComponents[2] = sz - m_poissonRatio * (2 * (sx - sy) * cos(2 * theta) + 4 * txy * sin(2 * theta));
    stressComponents[3] = 2 * (tyz * cos(theta) - txz * sin(theta));

    return stressComponents;
}



