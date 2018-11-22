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

#include "RiaColorTables.h"
#include "RigWellPath.h"

#include "RimProject.h"
#include "RimWellPathAttributeCollection.h"
#include "RimWellPath.h"

#include "cafPdmUiComboBoxEditor.h"

#include <cmath>

CAF_PDM_SOURCE_INIT(RimWellPathAttribute, "WellPathAttribute");

double RimWellPathAttribute::MAX_DIAMETER_IN_INCHES = 30.0;
double RimWellPathAttribute::MIN_DIAMETER_IN_INCHES = 7.0;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathAttribute::RimWellPathAttribute()
{
    CAF_PDM_InitObject("RimWellPathAttribute", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_type, "CompletionType", "Type    ", "", "", "");
    CAF_PDM_InitField(&m_startMD, "DepthStart", -1.0, "Start MD", "", "", "");
    CAF_PDM_InitField(&m_endMD,   "DepthEnd",   -1.0, "End MD",   "", "", "");
    CAF_PDM_InitField(&m_diameterInInches, "DiameterInInches", MAX_DIAMETER_IN_INCHES, "Diameter", "", "", "");
    m_type = RiaDefines::CASING;
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
double RimWellPathAttribute::diameterInInches() const
{
    return m_diameterInInches;
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
bool RimWellPathAttribute::operator<(const RimWellPathAttribute& rhs) const
{
    if (componentType() != rhs.componentType())
    {
        return componentType() < rhs.componentType();
    }
    return endMD() > rhs.endMD();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathAttribute::setDepthsFromWellPath(const RimWellPath* wellPath)
{
    m_startMD = wellPath->wellPathGeometry()->measureDepths().front();
    m_endMD = wellPath->wellPathGeometry()->measureDepths().back();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::WellPathComponentType RimWellPathAttribute::componentType() const
{
    return m_type();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathAttribute::componentLabel() const
{
    QString fullLabel = componentTypeLabel();
    if (m_type() == RiaDefines::CASING || m_type() == RiaDefines::LINER)
    {
        fullLabel += QString(" %1").arg(diameterLabel());
    }
    return fullLabel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathAttribute::componentTypeLabel() const
{
    return m_type().uiText();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimWellPathAttribute::defaultComponentColor() const
{
    return RiaColorTables::wellPathComponentColors()[componentType()];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPathAttribute::startMD() const
{
    return m_startMD();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPathAttribute::endMD() const
{
    return m_endMD();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPathAttribute::isDiameterSupported() const
{
    return m_type() == RiaDefines::CASING || m_type() == RiaDefines::LINER;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellPathAttribute::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;
    if (fieldNeedingOptions == &m_type)
    {
        std::set<RiaDefines::WellPathComponentType> supportedTypes = { RiaDefines::CASING, RiaDefines::LINER, RiaDefines::PACKER };
        for (RiaDefines::WellPathComponentType type : supportedTypes)
        {
            options.push_back(caf::PdmOptionItemInfo(CompletionTypeEnum::uiText(type), type));
        }
    }
    else if (fieldNeedingOptions == &m_diameterInInches)
    {
        if (isDiameterSupported())
        {
            std::vector<double> values = { MAX_DIAMETER_IN_INCHES, 22.0, 20.0, 18.0 + 5.0 / 8.0, 16.0, 14.0, 13.0 + 3.0 / 8.0, 10.0 + 3.0 / 4.0, 9.0 + 7.0 / 8.0, 9.0 + 5.0 / 8.0, MIN_DIAMETER_IN_INCHES };

            for (double value : values)
            {
                options.push_back(caf::PdmOptionItemInfo(generateInchesLabel(value), value));
            }
        }
        else
        {
            options.push_back(caf::PdmOptionItemInfo("N/A", MAX_DIAMETER_IN_INCHES));
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
        if (m_type() == RiaDefines::CASING)
        {
            RimWellPath* wellPath = nullptr;
            this->firstAncestorOrThisOfTypeAsserted(wellPath);
            m_startMD = wellPath->wellPathGeometry()->measureDepths().front();
        }
        else if (m_type() == RiaDefines::PACKER)
        {
            m_endMD = m_startMD + 50;
        }
    }
    if (changedField == &m_startMD)
    {
        if (m_type() == RiaDefines::PACKER)
        {
            m_endMD = m_startMD + 50;
        }
    }

    {
        RimWellPathAttributeCollection* collection = nullptr;
        this->firstAncestorOrThisOfTypeAsserted(collection);
        collection->updateAllReferringTracks();
    }
    {
        RimProject* proj;
        this->firstAncestorOrThisOfTypeAsserted(proj);
        proj->reloadCompletionTypeResultsInAllViews();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathAttribute::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{ 
    bool startDepthAvailable = m_type() != RiaDefines::CASING;

    m_startMD.uiCapability()->setUiReadOnly(!startDepthAvailable);    
    m_diameterInInches.uiCapability()->setUiReadOnly(!isDiameterSupported());    
}
