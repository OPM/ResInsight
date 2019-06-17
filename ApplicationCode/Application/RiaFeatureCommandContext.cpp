/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "RiaFeatureCommandContext.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaFeatureCommandContext::RiaFeatureCommandContext() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaFeatureCommandContext::~RiaFeatureCommandContext() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaFeatureCommandContext::setObject(QObject* object)
{
    m_pointerToQObject = object;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QObject* RiaFeatureCommandContext::object() const
{
    return m_pointerToQObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaFeatureCommandContext* RiaFeatureCommandContext::instance()
{
    static RiaFeatureCommandContext* commandFileExecutorInstance = new RiaFeatureCommandContext();
    return commandFileExecutorInstance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaFeatureCommandContextHelper::RiaFeatureCommandContextHelper(QObject* object)
{
    RiaFeatureCommandContext::instance()->setObject(object);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaFeatureCommandContextHelper::~RiaFeatureCommandContextHelper()
{
    RiaFeatureCommandContext::instance()->setObject(nullptr);
}
