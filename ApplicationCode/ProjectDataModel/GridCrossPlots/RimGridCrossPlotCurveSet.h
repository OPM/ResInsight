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

#include "RigGridCrossPlotCurveCategorization.h"

#include "RimCheckableNamedObject.h"
#include "RimNameConfig.h"

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include <cvfArray.h>

#include <QList>
#include <map>

struct RigEclipseCrossPlotResult;
class RimCase;
class RimGridCrossPlotCurve;
class RimGridView;
class RimEclipseCase;
class RimEclipseCellColors;
class RimEclipseResultDefinition;
class RimRegularLegendConfig;
class QwtPlot;
class QwtPlotCurve;
class QString;

class RimGridCrossPlotCurveSetNameConfig : public RimNameConfig
{
    CAF_PDM_HEADER_INIT;

public:
    RimGridCrossPlotCurveSetNameConfig(RimNameConfigHolderInterface* parent = nullptr);

    caf::PdmField<bool> addCaseName;
    caf::PdmField<bool> addAxisVariables;
    caf::PdmField<bool> addTimestep;
    caf::PdmField<bool> addCategorization;

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
    
    typedef caf::AppEnum<RigGridCrossPlotCurveCategorization> CurveCategorizationEnum;

public:
    RimGridCrossPlotCurveSet();
    ~RimGridCrossPlotCurveSet() = default;

    void    setCellFilterView(RimGridView* cellFilterView);
    void    loadDataAndUpdate(bool updateParentPlot);
    void    setParentQwtPlotNoReplot(QwtPlot* parent);
    QString xAxisName() const;
    QString yAxisName() const;

    int     indexInPlot() const;
    QString createAutoName() const override;
    QString categoryTitle() const;
    void    detachAllCurves();
    void    cellFilterViewUpdated();
    
    RimRegularLegendConfig* legendConfig() const;

    std::vector< RimGridCrossPlotCurve*> curves() const;

    QString              caseNameString() const;
    QString              axisVariableString() const;
    QString              timeStepString() const;
    std::vector<QString> categoryStrings() const;

    void updateCurveNames(bool applyCaseName, bool applyAxisVariables, bool applyTimeStep, bool applyCategory);
    void updateLegend();
    bool hasCategoryResult() const;
    
protected:
    void initAfterRead() override;
    void onLoadDataAndUpdate(bool updateParentPlot);

    void createCurves(const RigEclipseCrossPlotResult& result);

    std::map<int, cvf::UByteArray> calculateCellVisibility(RimEclipseCase* eclipseCase) const;

    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                        bool*                      useOptionsOnly) override;
    void triggerPlotNameUpdateAndReplot();
    void updateDataSetName();
    void performAutoNameUpdate() override;
    void setDefaults();
    void defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override;

    void defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "") override;

private:
    caf::PdmPtrField<RimCase*>                      m_case;
    caf::PdmField<int>                              m_timeStep;
    caf::PdmPtrField<RimGridView*>                  m_cellFilterView;
    caf::PdmField<CurveCategorizationEnum>          m_categorization;
    caf::PdmChildField<RimEclipseResultDefinition*> m_xAxisProperty;
    caf::PdmChildField<RimEclipseResultDefinition*> m_yAxisProperty;
    caf::PdmChildField<RimEclipseCellColors*>       m_categoryProperty;

    caf::PdmChildField<RimGridCrossPlotCurveSetNameConfig*> m_nameConfig;

    caf::PdmChildArrayField<RimGridCrossPlotCurve*> m_crossPlotCurves;
};
