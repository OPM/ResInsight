/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 -     Statoil ASA
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

#include "RimCheckableObject.h"

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"

// Include to make Pdm work for cvf::Color
#include "cafPdmFieldCvfColor.h"

namespace caf
{
class PdmOptionItemInfo;
}

class RimRegularLegendConfig;
class RimFractureTemplateCollection;

//==================================================================================================
///
///
//==================================================================================================
class RimStimPlanColors : public RimCheckableObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum StimPlanResultColorType
    {
        COLOR_INTERPOLATION,
        SINGLE_ELEMENT_COLOR
    };

public:
    RimStimPlanColors();
    ~RimStimPlanColors() override;

    RimRegularLegendConfig* activeLegend() const;
    QString                 uiResultName() const;
    void                    setDefaultResultName();
    QString                 unit() const;
    cvf::Color3f            defaultColor() const;
    bool                    showStimPlanMesh() const { return m_showStimPlanMesh; }
    void                    setShowStimPlanMesh( bool showStimPlanMesh );

    void loadDataAndUpdate();
    void updateLegendData();

    void                    updateStimPlanTemplates() const;
    StimPlanResultColorType stimPlanResultColorType() const { return m_stimPlanCellVizMode(); };

    void updateConductivityResultName();

protected:
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    RimFractureTemplateCollection* fractureTemplateCollection() const;

    static QString toResultName( const QString& resultNameAndUnit );
    static QString toUnit( const QString& resultNameAndUnit );

private:
    caf::PdmField<cvf::Color3f>                          m_defaultColor;
    caf::PdmField<QString>                               m_resultNameAndUnit;
    caf::PdmChildArrayField<RimRegularLegendConfig*>     m_legendConfigurations;
    caf::PdmField<bool>                                  m_showStimPlanMesh;
    caf::PdmField<caf::AppEnum<StimPlanResultColorType>> m_stimPlanCellVizMode;
};
