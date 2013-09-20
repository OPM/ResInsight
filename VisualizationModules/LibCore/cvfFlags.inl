//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


namespace cvf {


//==================================================================================================
///
/// \class cvf::Flags
/// \ingroup Core
///
/// The Flags class provides a type-safe way of storing OR-combinations of enum values
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Default constructor
//--------------------------------------------------------------------------------------------------
template<typename FlagEnum>
inline Flags<FlagEnum>::Flags() 
:   m_bitfield(0) 
{ 
}


//--------------------------------------------------------------------------------------------------
/// Copy constructor
//--------------------------------------------------------------------------------------------------
template<typename FlagEnum>
inline Flags<FlagEnum>::Flags(const Flags& other) 
:   m_bitfield(other.m_bitfield) 
{ 
}


//--------------------------------------------------------------------------------------------------
/// Constructor with initialization
//--------------------------------------------------------------------------------------------------
template<typename FlagEnum>
inline Flags<FlagEnum>::Flags(FlagEnum flag) 
:   m_bitfield(flag) 
{ 
}



//--------------------------------------------------------------------------------------------------
/// Assignment operator
//--------------------------------------------------------------------------------------------------
template<typename FlagEnum>
inline Flags<FlagEnum>& Flags<FlagEnum>::operator=(const Flags& rhs)
{
    m_bitfield = rhs.m_bitfield;
    return *this;
}


//--------------------------------------------------------------------------------------------------
/// Assignment operator
//--------------------------------------------------------------------------------------------------
template<typename FlagEnum>
inline Flags<FlagEnum>& Flags<FlagEnum>::operator=(FlagEnum flag)
{
    m_bitfield = flag;
    return *this;
}


//--------------------------------------------------------------------------------------------------
/// Performs a bitwise OR operation with rhs and stores the result in this Flags object.
//--------------------------------------------------------------------------------------------------
template<typename FlagEnum>
inline Flags<FlagEnum>& Flags<FlagEnum>::operator|=(const Flags& rhs)
{
    m_bitfield |= rhs.m_bitfield;
    return *this;
}


//--------------------------------------------------------------------------------------------------
/// Performs a bitwise OR operation with rhs and returns new object
//--------------------------------------------------------------------------------------------------
template<typename FlagEnum>
inline Flags<FlagEnum> Flags<FlagEnum>::operator|(const Flags& rhs) const
{
    Flags ret(*this);
    return (ret |= rhs);
}


//--------------------------------------------------------------------------------------------------
/// Performs a bitwise AND operation with rhs and stores the result in this Flags object.
//--------------------------------------------------------------------------------------------------
template<typename FlagEnum>
inline Flags<FlagEnum>& Flags<FlagEnum>::operator&=(const Flags& rhs)
{
    m_bitfield &= rhs.m_bitfield;
    return *this;
}


//--------------------------------------------------------------------------------------------------
/// Performs a bitwise AND operation with rhs and returns new object
//--------------------------------------------------------------------------------------------------
template<typename FlagEnum>
inline Flags<FlagEnum> Flags<FlagEnum>::operator&(const Flags& rhs) const
{
    Flags ret(*this);
    return (ret &= rhs);
}


//--------------------------------------------------------------------------------------------------
/// Comparison operator, equality
//--------------------------------------------------------------------------------------------------
template<typename FlagEnum>
inline bool Flags<FlagEnum>::operator==(const Flags& rhs) const
{
    return m_bitfield == rhs.m_bitfield;
}


//--------------------------------------------------------------------------------------------------
/// Comparison operator, equality
//--------------------------------------------------------------------------------------------------
template<typename FlagEnum>
inline bool Flags<FlagEnum>::operator==(int rhs) const
{
    return m_bitfield == rhs;
}


//--------------------------------------------------------------------------------------------------
/// Comparison operator, not equal
//--------------------------------------------------------------------------------------------------
template<typename FlagEnum>
inline bool Flags<FlagEnum>::operator!=(const Flags& rhs) const
{
    return m_bitfield != rhs.m_bitfield;
}


//--------------------------------------------------------------------------------------------------
/// Comparison operator, not equal
//--------------------------------------------------------------------------------------------------
template<typename FlagEnum>
inline bool Flags<FlagEnum>::operator!=(int rhs) const
{
    return m_bitfield != rhs;
}


//--------------------------------------------------------------------------------------------------
/// Return true only if ALL bits in flag are set 
//--------------------------------------------------------------------------------------------------
template<typename FlagEnum>
inline bool Flags<FlagEnum>::testFlag(FlagEnum flag) const
{ 
    return ((m_bitfield & flag) == flag); 
}


}  // namespace cvf
