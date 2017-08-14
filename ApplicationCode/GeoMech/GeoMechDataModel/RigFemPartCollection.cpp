/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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


#include "RigFemPartCollection.h"
#include "cvfBoundingBox.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemPartCollection::RigFemPartCollection()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemPartCollection::~RigFemPartCollection()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemPartCollection::addFemPart(RigFemPart* part)
{
    m_femParts.push_back(part);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemPart* RigFemPartCollection::part(size_t index)
{
    return m_femParts[index].p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigFemPart* RigFemPartCollection::part(size_t index) const
{
    return m_femParts[index].p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RigFemPartCollection::partCount() const
{
    return static_cast<int>(m_femParts.size());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigFemPartCollection::totalElementCount() const
{
    size_t elementCount = 0;

    for (int i = 0; i < partCount(); i++)
    {
        elementCount += part(i)->elementCount();
    }

    return elementCount;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
float RigFemPartCollection::characteristicElementSize() const
{
    if (partCount())
    {
        return part(0)->characteristicElementSize();
    }
    else
    {
        return 0;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RigFemPartCollection::boundingBox() const
{
    cvf::BoundingBox bBox;
    for (int i = 0; i < partCount(); i++)
    {
        bBox.add(part(i)->boundingBox());
    }
    return bBox;
}
