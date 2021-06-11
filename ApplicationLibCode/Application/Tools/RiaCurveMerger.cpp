#include "RiaCurveMerger.h"

#include <algorithm>
#include <ctime>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <>
bool XValueComparator<double>::equals( const double& lhs, const double& rhs )
{
    double eps = 1.0e-12 * std::max( std::fabs( lhs ), std::fabs( rhs ) );
    return std::fabs( lhs - rhs ) < eps;
}

template <>
double XValueComparator<time_t>::diff( const time_t& lhs, const time_t& rhs )
{
    return difftime( lhs, rhs );
}