/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RimCheckableNamedObject.h"
#include "RimNameConfig.h"

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

class RimCase;
class RimGridCrossPlotCurve;
class RimEclipseResultDefinition;
class QwtPlot;
class QwtPlotCurve;

class RimGridCrossPlotCurveSetNameConfig : public RimNameConfig
{
    CAF_PDM_HEADER_INIT;

public:
    RimGridCrossPlotCurveSetNameConfig(RimNameConfigHolderInterface* parent = nullptr);

    caf::PdmField<bool> addCaseName;
    caf::PdmField<bool> addAxisVariables;
    caf::PdmField<bool> addTimestep;

protected:
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
};

//==================================================================================================
///
///
//==================================================================================================
class RimGridCrossPlotCurveSet : public RimCheckableNamedObject, public RimNameConfigHolderInterface
{
    CAF_PDM_HEADER_INIT;

public:
    enum CurveCategorization
    {
        NO_CATEGORIZATION,
        TIME_CATEGORIZATION,
        FORMATION_CATEGORIZATION
    };
    typedef caf::AppEnum<CurveCategorization> CurveCategorizationEnum;

public:
    RimGridCrossPlotCurveSet();
    ~RimGridCrossPlotCurveSet() = default;

    void    loadDataAndUpdate(bool updateParentPlot);
    void    setParentQwtPlotNoReplot(QwtPlot* parent);
    QString xAxisName() const;
    QString yAxisName() const;

    int     indexInPlot() const;
    QString createAutoName() const override;
    void    detachAllCurves();
    
    std::vector< RimGridCrossPlotCurve*> curves() const;

protected:
    void initAfterRead() override;
    void onLoadDataAndUpdate(bool updateParentPlot);
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                        bool*                      useOptionsOnly) override;
    void triggerReplotAndTreeRebuild();
    void performAutoNameUpdate() override;
    void setDefaults();
private:

    caf::PdmPtrField<RimCase*>                      m_case;
    caf::PdmField<int>                              m_timeStep;
    caf::PdmField<CurveCategorizationEnum>          m_categorization;
    caf::PdmChildField<RimEclipseResultDefinition*> m_xAxisProperty;
    caf::PdmChildField<RimEclipseResultDefinition*> m_yAxisProperty;

    caf::PdmChildField<RimGridCrossPlotCurveSetNameConfig*> m_nameConfig;

    caf::PdmChildArrayField<RimGridCrossPlotCurve*> m_crossPlotCurves;
    
};
