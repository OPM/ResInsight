/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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
#include "RifEclipseSummaryAddressQMetaType.h"

#include "RimPlot.h"
#include "RimSummaryCaseCollection.h"

#include "cafAppEnum.h"
#include "cafPdmPtrField.h"

#include <QDateTime>

class RimSummaryAddress;
class RiuGroupedBarChartBuilder;

//==================================================================================================
///
///
//==================================================================================================
class RimCorrelationPlot : public RimPlot
{
    CAF_PDM_HEADER_INIT;

public:
    enum class CorrelationFactor
    {
        PEARSON,
        SPEARMAN
    };
    using CorrelationFactorEnum = caf::AppEnum<CorrelationFactor>;

public:
    RimCorrelationPlot();
    ~RimCorrelationPlot() override;

private:
    // Overridden PDM methods

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

    caf::PdmFieldHandle*          userDescriptionField() override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    std::set<time_t> allAvailableTimeSteps();

    // RimViewWindow overrides
    QWidget* viewWidget() override;
    void     deleteViewWidget() override;
    void     onLoadDataAndUpdate() override;
    void     zoomAll() override {}
    QImage   snapshotWindowContent() override;
    bool     applyFontSize( RiaDefines::FontSettingType fontSettingType,
                            int                         oldFontSize,
                            int                         fontSize,
                            bool                        forceChange = false ) override;

    // RimPlotWindow overrides
    QString description() const override;
    void    doUpdateLayout() override {}

    // RimPlot Overrides
    RiuQwtPlotWidget* doCreatePlotViewWidget( QWidget* mainWindowParent = nullptr ) override;
    RiuQwtPlotWidget* viewer() override;

    void detachAllCurves() override;
    void reattachAllCurves() override {}
    void doRemoveFromCollection() override {}
    void updateAxes() override;
    void onAxisSelected( int axis, bool toggle ) override;
    void updateZoomInQwt() override {}
    void updateZoomFromQwt() override {}
    void setAutoScaleXEnabled( bool enabled ) override {}
    void setAutoScaleYEnabled( bool enabled ) override {}
    void updateLegend() override{};

    QString         asciiDataForPlotExport() const override { return ""; }
    caf::PdmObject* findPdmObjectFromQwtCurve( const QwtPlotCurve* curve ) const override { return nullptr; }

    // Private methods
    void cleanupBeforeClose();
    void addDataToChartBuilder( RiuGroupedBarChartBuilder& chartBuilder );
    void updatePlotTitle();

private:
    QPointer<RiuQwtPlotWidget> m_plotWidget;

    // Fields
    caf::PdmPtrField<RimSummaryCaseCollection*> m_ensemble;
    caf::PdmChildField<RimSummaryAddress*>      m_summaryAddress;
    caf::PdmField<RifEclipseSummaryAddress>     m_summaryAddressUiField;
    caf::PdmField<bool>                         m_pushButtonSelectSummaryAddress;
    caf::PdmField<QDateTime>                    m_timeStep;
    caf::PdmField<CorrelationFactorEnum>        m_correlationFactor;
    caf::PdmField<bool>                         m_showAbsoluteValues;
    caf::PdmField<bool>                         m_sortByAbsoluteValues;

    caf::PdmField<bool>    m_showPlotTitle;
    caf::PdmField<bool>    m_useAutoPlotTitle;
    caf::PdmField<QString> m_description;
};
