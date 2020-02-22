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

#include "RigLocalGrid.h"

RigLocalGrid::RigLocalGrid( RigMainGrid* mainGrid )
    : RigGridBase( mainGrid )
    , m_parentGrid( nullptr )
    , m_isTempGrid( false )
    , m_associatedWellPathName( "" )
{
}

RigLocalGrid::~RigLocalGrid()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGridBase* RigLocalGrid::parentGrid() const
{
    return m_parentGrid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigLocalGrid::setParentGrid( RigGridBase* parentGrid )
{
    m_parentGrid = parentGrid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigLocalGrid::setAsTempGrid( bool isTemp )
{
    m_isTempGrid = isTemp;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigLocalGrid::isTempGrid() const
{
    return m_isTempGrid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigLocalGrid::setAssociatedWellPathName( const std::string& wellPathName )
{
    m_associatedWellPathName = wellPathName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::string& RigLocalGrid::associatedWellPathName() const
{
    return m_associatedWellPathName;
}
