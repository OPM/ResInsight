/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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
#include "RigGridBase.h"

class RigLocalGrid : public RigGridBase
{
public:
    explicit RigLocalGrid(RigMainGrid* mainGrid);
    ~RigLocalGrid() override;

    RigGridBase *   parentGrid() const;
    void            setParentGrid(RigGridBase * parentGrid);

    size_t          positionInParentGrid() const;
    void            setPositionInParentGrid(size_t positionInParentGrid);

private:
    RigGridBase *   m_parentGrid;
    size_t          m_positionInParentGrid;
};

