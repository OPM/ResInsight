/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafPdmPtrField.h"
#include "cafPdmChildField.h"

#include "RiaDefines.h"
#include "RifEclipseSummaryAddressQMetaType.h"
#include "RimPlotCurve.h"

#include "cafAppEnum.h"

class RifSummaryReaderInterface;
class RimSummaryCase;
class RimSummaryFilter;
class RiuLineSegmentQwtPlotCurve;
class RimSummaryCurveAutoName;
class RimSummaryAddress;


//==================================================================================================
///  
///  
//==================================================================================================
class RimSummaryCurve : public RimPlotCurve
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryCurve();
    virtual ~RimSummaryCurve();

    // Y Axis functions
    void                                    setSummaryCaseY(RimSummaryCase* sumCase);
    RimSummaryCase*                         summaryCaseY() const; 
    void                                    setSummaryAddressY(const RifEclipseSummaryAddress& address);
    RifEclipseSummaryAddress                summaryAddressY() const;
    std::string                             unitNameY() const;
    std::vector<double>                     valuesY() const;
    void                                    setLeftOrRightAxisY(RiaDefines::PlotAxis plotAxis);
    RiaDefines::PlotAxis                    axisY() const;
    const std::vector<time_t>&              timeStepsY() const;

    // X Axis functions
    void                                    setSummaryCaseX(RimSummaryCase* sumCase);
    RimSummaryCase*                         summaryCaseX() const; 
    RifEclipseSummaryAddress                summaryAddressX() const;
    void                                    setSummaryAddressX(const RifEclipseSummaryAddress& address);
    std::string                             unitNameX() const;
    std::vector<double>                     valuesX() const;

    // Other
    void                                    updateQwtPlotAxis();
    void                                    applyCurveAutoNameSettings(const RimSummaryCurveAutoName& autoNameSettings);

    virtual QString                         curveExportDescription() const override;
    void                                    forceUpdateCurveAppearanceFromCaseType();

protected:
    // RimPlotCurve overrides
    virtual QString                         createCurveAutoName() override;
    virtual void                            updateZoomInParentPlot() override;
    virtual void                            onLoadDataAndUpdate(bool updateParentPlot) override;

private:
    RifSummaryReaderInterface*              valuesSummaryReaderX() const;
    RifSummaryReaderInterface*              valuesSummaryReaderY() const;
    const std::vector<time_t>&              timeStepsX() const;

    void                                    calculateCurveInterpolationFromAddress();

    // Overridden PDM methods
    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly);
    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void                            defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override;

    static void                             appendOptionItemsForSummaryAddresses(QList<caf::PdmOptionItemInfo>* options,
                                                                                 RimSummaryCase* summaryCase,
                                                                                 RimSummaryFilter* summaryFilter);

private:
    // Y values
    caf::PdmPtrField<RimSummaryCase*>       m_yValuesSummaryCase;
    caf::PdmChildField<RimSummaryAddress*>  m_yValuesCurveVariable;
    caf::PdmField<QString>                  m_yValuesSelectedVariableDisplayField;
    caf::PdmChildField<RimSummaryFilter*>   m_yValuesSummaryFilter;
    caf::PdmField<RifEclipseSummaryAddress> m_yValuesUiFilterResultSelection;
    caf::PdmField<bool>                     m_yPushButtonSelectSummaryAddress;

    // X values
    caf::PdmPtrField<RimSummaryCase*>       m_xValuesSummaryCase;
    caf::PdmChildField<RimSummaryAddress*>  m_xValuesCurveVariable;
    caf::PdmField<QString>                  m_xValuesSelectedVariableDisplayField;
    caf::PdmChildField<RimSummaryFilter*>   m_xValuesSummaryFilter;
    caf::PdmField<RifEclipseSummaryAddress> m_xValuesUiFilterResultSelection;
    caf::PdmField<bool>                     m_xPushButtonSelectSummaryAddress;

    caf::PdmChildField<RimSummaryCurveAutoName*>        m_curveNameConfig;
    caf::PdmField<caf::AppEnum< RiaDefines::PlotAxis>>  m_plotAxis;
};
