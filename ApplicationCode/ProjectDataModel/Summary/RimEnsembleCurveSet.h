/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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

#include "RifEclipseSummaryAddress.h"

#include "RiaDefines.h"

#include "RimRegularLegendConfig.h"

#include "cafPdmFieldCvfColor.h"    
#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrArrayField.h"
#include "cafPdmPtrField.h"
#include "cafAppEnum.h"

#include "RifEclipseSummaryAddressQMetaType.h"
#include "cafPdmProxyValueField.h"

class QwtPlot;
class QwtPlotCurve;
class RimSummaryCase;
class RimSummaryCaseCollection;
class RimSummaryCurve;
class RimSummaryAddress;
class RimSummaryFilter;
class RimSummaryPlotSourceStepping;
class RimSummaryCurveAutoName;
class QKeyEvent;

//==================================================================================================
///  
//==================================================================================================
class RimEnsembleCurveSet : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum ColorMode {SINGLE_COLOR, BY_ENSEMBLE_PARAM};

    RimEnsembleCurveSet();
    virtual ~RimEnsembleCurveSet();

    bool                                    isCurvesVisible();
    void                                    setColor(cvf::Color3f color);

    void                                    loadDataAndUpdate(bool updateParentPlot);
    void                                    setParentQwtPlotNoReplot(QwtPlot* plot);
    void                                    detachQwtCurves();

    void                                    addCurve(RimSummaryCurve* curve);
    void                                    deleteCurve(RimSummaryCurve* curve);

    RifEclipseSummaryAddress                summaryAddress() const;
    std::vector<RimSummaryCurve*>           curves() const;
    std::vector<RimSummaryCurve*>           visibleCurves() const;

    void                                    deleteAllCurves();

    RimRegularLegendConfig*                 legendConfig();
    void                                    onLegendDefinitionChanged();

    void                                    setSummaryCaseCollection(RimSummaryCaseCollection* sumCaseCollection);
    RimSummaryCaseCollection*               summaryCaseCollection() const;

private:
    caf::PdmFieldHandle*                    userDescriptionField() override;
    caf::PdmFieldHandle*                    objectToggleField();
    virtual void                            defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override;

    virtual QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly);
    virtual void                            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void                            defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "") override;

    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                                             const QVariant& oldValue, const QVariant& newValue) override;

    static void                             getOptionsForSummaryAddresses(std::map<QString, RifEclipseSummaryAddress>* options,
                                                                              RimSummaryCase* summaryCase,
                                                                              RimSummaryFilter* summaryFilter);

    void                                    appendOptionItemsForSummaryAddresses(QList<caf::PdmOptionItemInfo>* options,
                                                                                 RimSummaryCaseCollection* summaryCaseGroup,
                                                                                 RimSummaryFilter* summaryFilter);

    void                                    updateCurveColors();
    void                                    updateQwtPlotAxis();
    void                                    updateAllCurves();
    std::vector<QString>                    ensembleParameters() const;

    QString                                 createAutoName() const;


private:
    caf::PdmField<bool>                             m_showCurves;
    caf::PdmChildArrayField<RimSummaryCurve*>       m_curves;

    caf::PdmPointer<RimSummaryCurve>                m_currentSummaryCurve;

    caf::PdmPtrField<RimSummaryCaseCollection*>     m_yValuesSummaryGroup;
    caf::PdmChildField<RimSummaryAddress*>          m_yValuesCurveVariable;
    caf::PdmField<QString>                          m_yValuesSelectedVariableDisplayField;
    caf::PdmChildField<RimSummaryFilter*>           m_yValuesSummaryFilter;
    caf::PdmField<RifEclipseSummaryAddress>         m_yValuesUiFilterResultSelection;
    caf::PdmField<bool>                             m_yPushButtonSelectSummaryAddress;

    caf::PdmField<caf::AppEnum<ColorMode>>          m_colorMode;
    caf::PdmField<cvf::Color3f>                     m_color;
    caf::PdmField<QString>                          m_ensembleParameter;

    caf::PdmField<caf::AppEnum< RiaDefines::PlotAxis>>  m_plotAxis;

    caf::PdmChildField<RimRegularLegendConfig*>     m_legendConfig;

    caf::PdmField<bool>                             m_isUsingAutoName;
    caf::PdmField<QString>                          m_userDefinedName;
    caf::PdmProxyValueField<QString>                m_autoGeneratedName;
    caf::PdmChildField<RimSummaryCurveAutoName*>    m_summaryAddressNameTools;

    std::set<RifEclipseSummaryAddress>              m_allAddressesCache;
};

