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

#include "RimWellPathAttributeCurve.h"
#include "RimWellPathAttributeCollection.h"
#include "RimWellPath.h"

#include "cafPdmUiComboBoxEditor.h"

#include <cmath>

CAF_PDM_SOURCE_INIT(RimWellPathAttribute, "WellPathAttribute");

namespace caf
{
template<>
void caf::AppEnum<RimWellPathAttribute::AttributeType>::setUp()
{
    addItem(RimWellPathAttribute::AttributeCasing, "CASING", "Casing");
    addItem(RimWellPathAttribute::AttributeLiner, "LINER", "Liner");
    setDefault(RimWellPathAttribute::AttributeCasing);
}
}

double RimWellPathAttribute::MAX_DIAMETER_IN_INCHES = 30.0;
double RimWellPathAttribute::MIN_DIAMETER_IN_INCHES = 7.0;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathAttribute::RimWellPathAttribute()
{
    CAF_PDM_InitObject("RimWellPathAttribute", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_type, "AttributeType", "Type    ", "", "", "");
    CAF_PDM_InitField(&m_depthStart, "DepthStart", 0.0, "Start MD", "", "", "");
    CAF_PDM_InitField(&m_depthEnd,   "DepthEnd",   0.0, "End MD",   "", "", "");
    CAF_PDM_InitField(&m_diameterInInches, "DiameterInInches", MAX_DIAMETER_IN_INCHES, "Diameter", "", "", "");
    m_diameterInInches.uiCapability()->setUiEditorTypeName(caf::PdmUiComboBoxEditor::uiEditorTypeName());
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
double RimWellPathAttribute::diameterInInches() const
{
    return m_diameterInInches;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathAttribute::label() const
{
    if (m_type == AttributeCasing)
    {
        return QString("Casing %1").arg(diameterLabel());
    }
    else
    {
        return QString("Liner %1").arg(diameterLabel());
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathAttribute::diameterLabel() const
{
    return generateInchesLabel(m_diameterInInches());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellPathAttribute::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;
    if (fieldNeedingOptions == &m_type)
    {
        options.push_back(caf::PdmOptionItemInfo(AttributeTypeEnum::uiText(AttributeCasing), AttributeCasing));
        options.push_back(caf::PdmOptionItemInfo(AttributeTypeEnum::uiText(AttributeLiner), AttributeLiner));
    }
    else if (fieldNeedingOptions == &m_diameterInInches)
    {
        std::vector<double> values = { MAX_DIAMETER_IN_INCHES, 22.0, 20.0, 18.0 + 5.0 / 8.0, 16.0, 14.0, 13.0 + 3.0 / 8.0, 10.0 + 3.0 / 4.0, 9.0 + 7.0 / 8.0, 9.0 + 5.0 / 8.0, MIN_DIAMETER_IN_INCHES };

        for (double value : values)
        {
            options.push_back(caf::PdmOptionItemInfo(generateInchesLabel(value), value));
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathAttribute::generateInchesLabel(double diameter)
{
    double integerPart = 0.0;
    double fraction = modf(diameter, &integerPart);

    int numerator = static_cast<int>(std::round(8.0 * fraction));

    if (numerator > 0)
    {
        return QString("%1 %2/8\"").arg(static_cast<int>(integerPart)).arg(numerator);
    }
    return QString("%1\"").arg(static_cast<int>(integerPart));
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
            m_depthStart = 0;
        }
    }

    {
        RimWellPathAttributeCollection* collection = nullptr;
        this->firstAncestorOrThisOfTypeAsserted(collection);
        collection->updateAllReferringTracks();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathAttribute::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    m_depthStart.uiCapability()->setUiReadOnly(m_type() == AttributeCasing);
}
