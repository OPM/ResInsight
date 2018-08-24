/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include "RicCreateMultipleFracturesOptionItemUi.h"

#include "RiaApplication.h"

#include "RicCreateMultipleFracturesUi.h"
#include "RimFractureTemplate.h"
#include "RimFractureTemplateCollection.h"
#include "RimOilField.h"
#include "RimProject.h"

CAF_PDM_SOURCE_INIT(RicCreateMultipleFracturesOptionItemUi, "RiuMultipleFractionsOptions");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicCreateMultipleFracturesOptionItemUi::RicCreateMultipleFracturesOptionItemUi()
{
    CAF_PDM_InitField(&m_topKOneBased, "TopKLayer", 1, "Top K Layer", "", "", "");
    CAF_PDM_InitField(&m_baseKOneBased, "BaseKLayer", 1, "Base K Layer", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_fractureTemplate, "Template", "Template", "", "", "");
    CAF_PDM_InitField(&m_minSpacing, "MinSpacing", 300.0, "Spacing", "", "", "");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateMultipleFracturesOptionItemUi::setValues(int                  topKOneBased,
                                                       int                  baseKOneBased,
                                                       RimFractureTemplate* fractureTemplate,
                                                       double               minimumSpacing)
{
    m_topKOneBased     = topKOneBased;
    m_baseKOneBased    = baseKOneBased;
    m_fractureTemplate = fractureTemplate;
    m_minSpacing       = minimumSpacing;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RicCreateMultipleFracturesOptionItemUi::topKLayer() const
{
    return m_topKOneBased;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RicCreateMultipleFracturesOptionItemUi::baseKLayer() const
{
    return m_baseKOneBased;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureTemplate* RicCreateMultipleFracturesOptionItemUi::fractureTemplate() const
{
    return m_fractureTemplate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RicCreateMultipleFracturesOptionItemUi::minimumSpacing() const
{
    return m_minSpacing;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCreateMultipleFracturesOptionItemUi::isKLayerContained(int k) const
{
    auto minMax = std::minmax(m_topKOneBased, m_baseKOneBased);

    if (k < minMax.first) return false;
    if (k < minMax.second) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCreateMultipleFracturesOptionItemUi::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                                              const QVariant&            oldValue,
                                                              const QVariant&            newValue)
{
    RiuCreateMultipleFractionsUi* parent = nullptr;
    this->firstAncestorOrThisOfType(parent);
    if (parent)
    {
        parent->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo>
    RicCreateMultipleFracturesOptionItemUi::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                  bool*                      useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    RimProject* proj = RiaApplication::instance()->project();
    CVF_ASSERT(proj);

    if (fieldNeedingOptions == &m_fractureTemplate)
    {
        RimOilField* oilField = proj->activeOilField();
        if (oilField && oilField->fractureDefinitionCollection)
        {
            RimFractureTemplateCollection* fracDefColl = oilField->fractureDefinitionCollection();

            for (RimFractureTemplate* fracDef : fracDefColl->fractureTemplates())
            {
                QString displayText = fracDef->nameAndUnit();

                options.push_back(caf::PdmOptionItemInfo(displayText, fracDef));
            }
        }
    }

    return options;
}
