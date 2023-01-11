//////////////////////////////////////////////////
// Taken from
// https://gist.github.com/StrikerX3/46b9058d6c61387b3f361ef9d7e00cd4
//////////////////////////////////////////////////

/*
Type-safe enum class bitmasks.
Based on work by Andre Haupt from his blog at
http://blog.bitwigglers.org/using-enum-classes-as-type-safe-bitmasks/
To enable enum classes to be used as bitmasks, use the ENABLE_BITMASK_OPERATORS
macro:
  enum class MyBitmask {
     None = 0b0000,
     One = 0b0001,
     Two = 0b0010,
     Three = 0b0100,
  };
  ENABLE_BITMASK_OPERATORS(MyBitmask)
From now on, MyBitmask's values can be used with bitwise operators.
You may find it cumbersome to check for the presence or absence of specific
values in enum class bitmasks. For example:
  MyBitmask bm = ...;
  MyBitmask oneAndThree = (MyBitmask::One | MyBitmask::Three);
  // Check if either bit one or three is set
  if (bm & oneAndThree != MyBitmask::None) {
      ...
  }
  // Check if both bits one and three are set
  if (bm & oneAndThree == oneAndThree) {
      ...
  }
To help with that, you can wrap the bitmask into the BitmaskEnum type, which
provides a set of bitmask checks and useful conversions:
  MyBitmask bm = ...;
  MyBitmask oneAndThree = (MyBitmask::One | MyBitmask::Three);
  auto wbm = BitmaskEnum(bm);
  // Check if either bit one or three is set
  if (bm.AnyOf(oneAndThree)) {
      ...
  }
  // Check if both bits one and three are set
  if (bm.AllOf(oneAndThree)) {
      ...
  }
  // Check if any bit is set
  if (bm) {
      ...
  }
  // Convert back to the enum class
  MyBitmask backToEnum = wbm;
-------------------------------------------------------------------------------
MIT License
Copyright (c) 2019 Ivan Roberto de Oliveira
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once

#include <type_traits>

#define ENABLE_BITMASK_OPERATORS( x )    \
    template <>                          \
    struct is_bitmask_enum<x>            \
    {                                    \
        static const bool enable = true; \
    };

template <typename Enum>
struct is_bitmask_enum
{
    static const bool enable = false;
};

template <class Enum>
inline constexpr bool is_bitmask_enum_v = is_bitmask_enum<Enum>::enable;

// ----- Bitwise operators ----------------------------------------------------

template <typename Enum>
typename std::enable_if_t<is_bitmask_enum_v<Enum>, Enum> operator|( Enum lhs, Enum rhs )
{
    using underlying = typename std::underlying_type_t<Enum>;
    return static_cast<Enum>( static_cast<underlying>( lhs ) | static_cast<underlying>( rhs ) );
}

template <typename Enum>
typename std::enable_if_t<is_bitmask_enum_v<Enum>, Enum> operator&( Enum lhs, Enum rhs )
{
    using underlying = typename std::underlying_type_t<Enum>;
    return static_cast<Enum>( static_cast<underlying>( lhs ) & static_cast<underlying>( rhs ) );
}

template <typename Enum>
typename std::enable_if_t<is_bitmask_enum_v<Enum>, Enum> operator^( Enum lhs, Enum rhs )
{
    using underlying = typename std::underlying_type_t<Enum>;
    return static_cast<Enum>( static_cast<underlying>( lhs ) ^ static_cast<underlying>( rhs ) );
}

template <typename Enum>
typename std::enable_if_t<is_bitmask_enum_v<Enum>, Enum> operator~( Enum rhs )
{
    using underlying = typename std::underlying_type_t<Enum>;
    return static_cast<Enum>( ~static_cast<underlying>( rhs ) );
}

// ----- Bitwise assignment operators -----------------------------------------

template <typename Enum>
typename std::enable_if_t<is_bitmask_enum_v<Enum>, Enum> operator|=( Enum& lhs, Enum rhs )
{
    using underlying = typename std::underlying_type_t<Enum>;
    lhs              = static_cast<Enum>( static_cast<underlying>( lhs ) | static_cast<underlying>( rhs ) );
    return lhs;
}

template <typename Enum>
typename std::enable_if_t<is_bitmask_enum_v<Enum>, Enum> operator&=( Enum& lhs, Enum rhs )
{
    using underlying = typename std::underlying_type_t<Enum>;
    lhs              = static_cast<Enum>( static_cast<underlying>( lhs ) & static_cast<underlying>( rhs ) );
    return lhs;
}

template <typename Enum>
typename std::enable_if_t<is_bitmask_enum_v<Enum>, Enum> operator^=( Enum& lhs, Enum rhs )
{
    using underlying = typename std::underlying_type_t<Enum>;
    lhs              = static_cast<Enum>( static_cast<underlying>( lhs ) ^ static_cast<underlying>( rhs ) );
    return lhs;
}

// ----- Bitwise mask checks --------------------------------------------------

template <typename Enum>
struct BitmaskEnum
{
    const Enum        value;
    static const Enum none = static_cast<Enum>( 0 );

    using underlying = typename std::underlying_type_t<Enum>;

    BitmaskEnum( Enum value )
        : value( value )
    {
        static_assert( is_bitmask_enum_v<Enum> );
    }

    // Convert back to enum if required
    inline operator Enum() const { return value; }

    // Convert to true if there is any bit set in the bitmask
    inline operator bool() const { return Any(); }

    // Returns true if any bit is set
    inline bool Any() const { return value != none; }

    // Returns true if all bits are clear
    inline bool None() const { return value == none; }

    // Returns true if any bit in the given mask is set
    inline bool AnyOf( const Enum& mask ) const { return ( value & mask ) != none; }

    // Returns true if all bits in the given mask are set
    inline bool AllOf( const Enum& mask ) const { return ( value & mask ) == mask; }

    // Returns true if none of the bits in the given mask are set
    inline bool NoneOf( const Enum& mask ) const { return ( value & mask ) == none; }

    // Returns true if any bits excluding the mask are set
    inline bool AnyExcept( const Enum& mask ) const { return ( value & ~mask ) != none; }

    // Returns true if no bits excluding the mask are set
    inline bool NoneExcept( const Enum& mask ) const { return ( value & ~mask ) == none; }
};
