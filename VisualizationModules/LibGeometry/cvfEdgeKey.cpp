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

#include "cvfBase.h"
#include "cvfEdgeKey.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::EdgeKey
/// \ingroup Geometry
///
/// 
///
//==================================================================================================
EdgeKey::EdgeKey(uint vertexIdx1, uint vertexIdx2)
{
    if (vertexIdx1 <= vertexIdx2)
    {
        m_vertexIdx1 = vertexIdx1;
        m_vertexIdx2 = vertexIdx2;
    }
    else
    {
        m_vertexIdx1 = vertexIdx2;
        m_vertexIdx2 = vertexIdx1;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool EdgeKey::operator==(const EdgeKey& rhs) const
{
    if (m_vertexIdx1 == rhs.m_vertexIdx1 && m_vertexIdx2 == rhs.m_vertexIdx2)
    {
        return true;
    }
    else
    {
        return false;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool EdgeKey::operator<(const EdgeKey& rhs) const
{
    if (m_vertexIdx1 == rhs.m_vertexIdx1)
    {
        return (m_vertexIdx2 < rhs.m_vertexIdx2);
    }
    else
    {
        return (m_vertexIdx1 < rhs.m_vertexIdx1);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int64 EdgeKey::toKeyVal() const
{
    int64 edgeKey = m_vertexIdx1;
    edgeKey <<= 32;
    edgeKey += m_vertexIdx2;

    return edgeKey;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
EdgeKey EdgeKey::fromkeyVal(int64 edgeKeyVal)
{
    uint vertexIdx2 = static_cast<uint>(edgeKeyVal);

    edgeKeyVal >>= 32;
    uint vertexIdx1 = static_cast<uint>(edgeKeyVal);

    return EdgeKey(vertexIdx1, vertexIdx2);
}




} // namespace cvf

