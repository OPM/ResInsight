//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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
//##################################################################################################

#pragma once

namespace cvf {


//=================================================================================================
// 
// Flags class
// 
//=================================================================================================
template<typename FlagEnum>
class Flags
{
public:
    inline Flags(); 
    inline Flags(const Flags& other);
    inline Flags(FlagEnum flag);

    inline Flags&  operator=(const Flags& rhs);
    inline Flags&  operator=(FlagEnum flag);
    
    inline Flags&  operator|=(const Flags& rhs);
    inline Flags   operator|(const Flags& rhs) const;
    inline Flags&  operator&=(const Flags& rhs);
    inline Flags   operator&(const Flags& rhs) const;

    inline bool operator==(const Flags& rhs) const;
    inline bool operator==(int rhs) const;
    inline bool operator!=(const Flags& rhs) const;
    inline bool operator!=(int rhs) const;
    inline bool testFlag(FlagEnum flag) const;

private:
    int m_bitfield;
};

}

#include "cvfFlags.inl"
