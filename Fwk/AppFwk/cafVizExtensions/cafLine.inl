#include "cafLine.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename S>
caf::Line<S>::Line()
    : m_start( cvf::Vector3<S>::UNDEFINED )
    , m_end( cvf::Vector3<S>::UNDEFINED )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename S>
caf::Line<S>::Line( const cvf::Vector3<S>& startPoint, const cvf::Vector3<S>& endPoint )
    : m_start( startPoint )
    , m_end( endPoint )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename S>
caf::Line<S>::Line( const Line& copyFrom )
{
    m_start = copyFrom.start();
    m_end   = copyFrom.end();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename S>
caf::Line<S>& caf::Line<S>::operator=( const Line& copyFrom )
{
    m_start = copyFrom.start();
    m_end   = copyFrom.end();
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename S>
const cvf::Vector3<S>& caf::Line<S>::start() const
{
    return m_start;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename S>
const cvf::Vector3<S>& caf::Line<S>::end() const
{
    return m_end;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename S>
cvf::Vector3<S> caf::Line<S>::vector() const
{
    return m_end - m_start;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename S>
caf::Line<S> caf::Line<S>::findLineBetweenNearestPoints( const Line& otherLine, bool* withinLineSegments )
{
    // Taken from Real-Time Collision Detection, Christer Ericson, 2005, p146-147
    cvf::Vector3<S> d1 = vector();
    cvf::Vector3<S> d2 = otherLine.vector();

    S a = d1.dot( d1 );
    S b = d1.dot( d2 );
    S e = d2.dot( d2 );

    S d = a * e - b * b;

    if ( d < std::numeric_limits<S>::epsilon() )
    {
        // Parallel lines. Choice of closest points is arbitrary.
        // Just use start to start.
        if ( withinLineSegments ) *withinLineSegments = true;
        return Line( start(), otherLine.start() );
    }

    cvf::Vector3<S> r = start() - otherLine.start();
    S               c = d1.dot( r );
    S               f = d2.dot( r );

    S s = ( b * f - c * e ) / d;
    S t = ( a * f - b * c ) / d;

    if ( withinLineSegments )
    {
        *withinLineSegments = s >= 0 && s <= 1 && t >= 0 && t <= 1;
    }
    return Line( start() + s * d1, otherLine.start() + t * d2 );
}
