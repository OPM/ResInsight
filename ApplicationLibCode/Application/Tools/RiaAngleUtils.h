/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#pragma once

#include "cvfMath.h"
#include "cvfVector3.h"

#include <cmath>

namespace Ria
{

//==================================================================================================
//
// Type-safe angle representation classes
//
//==================================================================================================

template <typename T>
class Radians;

template <typename T>
class Degrees
{
public:
    constexpr Degrees()
        : m_value( T{} )
    {
    }
    constexpr explicit Degrees( T value )
        : m_value( value )
    {
    }
    constexpr Degrees( const Degrees& other )
        : m_value( other.m_value )
    {
    }
    constexpr Degrees( const Radians<T>& radians );

    constexpr T value() const { return m_value; }
    constexpr   operator T() const { return m_value; }

    constexpr Degrees& operator=( const Degrees& other )
    {
        m_value = other.m_value;
        return *this;
    }
    constexpr Degrees& operator=( const Radians<T>& radians );

    constexpr Degrees operator+( const Degrees& other ) const { return Degrees( m_value + other.m_value ); }
    constexpr Degrees operator-( const Degrees& other ) const { return Degrees( m_value - other.m_value ); }
    constexpr Degrees operator*( T scalar ) const { return Degrees( m_value * scalar ); }
    constexpr Degrees operator/( T scalar ) const { return Degrees( m_value / scalar ); }
    constexpr Degrees operator-() const { return Degrees( -m_value ); }

    constexpr Degrees& operator+=( const Degrees& other )
    {
        m_value += other.m_value;
        return *this;
    }
    constexpr Degrees& operator-=( const Degrees& other )
    {
        m_value -= other.m_value;
        return *this;
    }
    constexpr Degrees& operator*=( T scalar )
    {
        m_value *= scalar;
        return *this;
    }
    constexpr Degrees& operator/=( T scalar )
    {
        m_value /= scalar;
        return *this;
    }

    constexpr bool operator==( const Degrees& other ) const { return m_value == other.m_value; }
    constexpr bool operator!=( const Degrees& other ) const { return m_value != other.m_value; }
    constexpr bool operator<( const Degrees& other ) const { return m_value < other.m_value; }
    constexpr bool operator<=( const Degrees& other ) const { return m_value <= other.m_value; }
    constexpr bool operator>( const Degrees& other ) const { return m_value > other.m_value; }
    constexpr bool operator>=( const Degrees& other ) const { return m_value >= other.m_value; }

    Degrees normalized() const;

private:
    T m_value;
};

template <typename T>
class Radians
{
public:
    constexpr Radians()
        : m_value( T{} )
    {
    }
    constexpr explicit Radians( T value )
        : m_value( value )
    {
    }
    constexpr Radians( const Radians& other )
        : m_value( other.m_value )
    {
    }
    constexpr Radians( const Degrees<T>& degrees );

    constexpr T value() const { return m_value; }
    constexpr   operator T() const { return m_value; }

    constexpr Radians& operator=( const Radians& other )
    {
        m_value = other.m_value;
        return *this;
    }
    constexpr Radians& operator=( const Degrees<T>& degrees );

    constexpr Radians operator+( const Radians& other ) const { return Radians( m_value + other.m_value ); }
    constexpr Radians operator-( const Radians& other ) const { return Radians( m_value - other.m_value ); }
    constexpr Radians operator*( T scalar ) const { return Radians( m_value * scalar ); }
    constexpr Radians operator/( T scalar ) const { return Radians( m_value / scalar ); }
    constexpr Radians operator-() const { return Radians( -m_value ); }

    constexpr Radians& operator+=( const Radians& other )
    {
        m_value += other.m_value;
        return *this;
    }
    constexpr Radians& operator-=( const Radians& other )
    {
        m_value -= other.m_value;
        return *this;
    }
    constexpr Radians& operator*=( T scalar )
    {
        m_value *= scalar;
        return *this;
    }
    constexpr Radians& operator/=( T scalar )
    {
        m_value /= scalar;
        return *this;
    }

    constexpr bool operator==( const Radians& other ) const { return m_value == other.m_value; }
    constexpr bool operator!=( const Radians& other ) const { return m_value != other.m_value; }
    constexpr bool operator<( const Radians& other ) const { return m_value < other.m_value; }
    constexpr bool operator<=( const Radians& other ) const { return m_value <= other.m_value; }
    constexpr bool operator>( const Radians& other ) const { return m_value > other.m_value; }
    constexpr bool operator>=( const Radians& other ) const { return m_value >= other.m_value; }

    Radians normalized() const;

private:
    T m_value;
};

// Cross-conversions between Degrees and Radians
template <typename T>
constexpr Degrees<T>::Degrees( const Radians<T>& radians )
    : m_value( cvf::Math::toDegrees( radians.value() ) )
{
}

template <typename T>
constexpr Degrees<T>& Degrees<T>::operator=( const Radians<T>& radians )
{
    m_value = cvf::Math::toDegrees( radians.value() );
    return *this;
}

template <typename T>
constexpr Radians<T>::Radians( const Degrees<T>& degrees )
    : m_value( cvf::Math::toRadians( degrees.value() ) )
{
}

template <typename T>
constexpr Radians<T>& Radians<T>::operator=( const Degrees<T>& degrees )
{
    m_value = cvf::Math::toRadians( degrees.value() );
    return *this;
}

// Scalar multiplication (scalar * angle)
template <typename T>
constexpr Degrees<T> operator*( T scalar, const Degrees<T>& degrees )
{
    return degrees * scalar;
}

template <typename T>
constexpr Radians<T> operator*( T scalar, const Radians<T>& radians )
{
    return radians * scalar;
}

// Type aliases
using Degreesf = Degrees<float>;
using Degreesd = Degrees<double>;
using Radiansf = Radians<float>;
using Radiansd = Radians<double>;

//==================================================================================================
//
// Trigonometric functions for typed angles
//
//==================================================================================================

template <typename T>
T sin( const Degrees<T>& angle )
{
    return cvf::Math::sin( cvf::Math::toRadians( angle.value() ) );
}

template <typename T>
T sin( const Radians<T>& angle )
{
    return cvf::Math::sin( angle.value() );
}

template <typename T>
T cos( const Degrees<T>& angle )
{
    return cvf::Math::cos( cvf::Math::toRadians( angle.value() ) );
}

template <typename T>
T cos( const Radians<T>& angle )
{
    return cvf::Math::cos( angle.value() );
}

template <typename T>
T tan( const Degrees<T>& angle )
{
    return cvf::Math::tan( cvf::Math::toRadians( angle.value() ) );
}

template <typename T>
T tan( const Radians<T>& angle )
{
    return cvf::Math::tan( angle.value() );
}

template <typename T>
Radians<T> asin( T value )
{
    return Radians<T>( cvf::Math::asin( value ) );
}

template <typename T>
Radians<T> acos( T value )
{
    return Radians<T>( cvf::Math::acos( value ) );
}

template <typename T>
Radians<T> atan( T value )
{
    return Radians<T>( cvf::Math::atan( value ) );
}

template <typename T>
Radians<T> atan2( T y, T x )
{
    return Radians<T>( std::atan2( y, x ) );
}

//==================================================================================================
//
// Coordinate system conversion utilities
//
//==================================================================================================

struct CylindricalCoordinate
{
    template <typename T>
    CylindricalCoordinate( T radius, const Radians<T>& angle, T height )
        : radius( radius )
        , angle( angle.value() )
        , height( height )
    {
    }

    template <typename T>
    CylindricalCoordinate( T radius, const Degrees<T>& angle, T height )
        : radius( radius )
        , angle( cvf::Math::toRadians( angle.value() ) )
        , height( height )
    {
    }

    double radius;
    double angle; // In radians
    double height;
};

class CoordinateConverter
{
public:
    template <typename T>
    static cvf::Vector3<T> cylindricalToCartesian( T radius, const Radians<T>& angle, T height );

    template <typename T>
    static cvf::Vector3<T> cylindricalToCartesian( T radius, const Degrees<T>& angle, T height );

    template <typename T>
    static cvf::Vector3<T> cylindricalToCartesian( const CylindricalCoordinate& coord );

    template <typename T>
    static CylindricalCoordinate cartesianToCylindrical( const cvf::Vector3<T>& cartesian );

    template <typename T>
    static CylindricalCoordinate cartesianToCylindrical( T x, T y, T z );
};

//==================================================================================================
//
// Convenience functions for creating angle literals
//
//==================================================================================================

constexpr Degreesd operator"" _deg( long double value )
{
    return Degreesd( static_cast<double>( value ) );
}
constexpr Degreesf operator"" _degf( long double value )
{
    return Degreesf( static_cast<float>( value ) );
}
constexpr Radiansd operator"" _rad( long double value )
{
    return Radiansd( static_cast<double>( value ) );
}
constexpr Radiansf operator"" _radf( long double value )
{
    return Radiansf( static_cast<float>( value ) );
}

} // namespace Ria

#include "RiaAngleUtils.inl"
