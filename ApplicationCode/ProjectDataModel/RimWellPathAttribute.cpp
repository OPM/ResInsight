/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 -    Equinor ASA
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
#include "RimWellPathAttribute.h"

CAF_PDM_SOURCE_INIT(RimWellPathAttribute, "WellPathAttribute");

namespace caf
{
template<>
void caf::AppEnum<RimWellPathAttribute::AttributeType>::setUp()
{
    addItem(RimWellPathAttribute::AttributeCasing, "CASING", "Casing");
    addItem(RimWellPathAttribute::AttributeLining, "LINING", "Lining");
    setDefault(RimWellPathAttribute::AttributeCasing);
}
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathAttribute::RimWellPathAttribute()
{
    CAF_PDM_InitObject("RimWellPathAttribute", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_type, "AttributeType", "Type    ", "", "", "");
    CAF_PDM_InitField(&m_depthStart, "DepthStart", 0.0, "Depth Start", "", "", "");
    CAF_PDM_InitField(&m_depthEnd,   "DepthEnd",   0.0, "Depth End",   "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_label, "Label", "Label", "", "", "");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathAttribute::~RimWellPathAttribute()
{

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathAttribute::AttributeType RimWellPathAttribute::type() const
{
    return m_type();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPathAttribute::depthStart() const
{
    return m_depthStart();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPathAttribute::depthEnd() const
{
    return m_depthEnd();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathAttribute::label() const
{
    return m_label();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathAttribute::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                            const QVariant&            oldValue,
                                            const QVariant&            newValue)
{
    if (changedField == &m_type)
    {
        if (m_type() == AttributeCasing)
        {
            m_depthEnd = 0;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathAttribute::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    m_depthEnd.uiCapability()->setUiReadOnly(m_type() == AttributeCasing);
}
