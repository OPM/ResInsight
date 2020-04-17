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

#include "cafPdmPtrField.h"

class RimSummaryAddress;

class RimAbstractCorrelationPlot : public RimPlot
{
    CAF_PDM_HEADER_INIT;

public:
    RimAbstractCorrelationPlot();
    ~RimAbstractCorrelationPlot() override;

protected:
    // Overridden PDM methods

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
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
    void updateZoomInQwt() override {}
    void updateZoomFromQwt() override {}
    void setAutoScaleXEnabled( bool enabled ) override {}
    void setAutoScaleYEnabled( bool enabled ) override {}
    void updateLegend() override;

    QString         asciiDataForPlotExport() const override { return ""; }
    caf::PdmObject* findPdmObjectFromQwtCurve( const QwtPlotCurve* curve ) const override { return nullptr; }

    void         cleanupBeforeClose();
    virtual void updatePlotTitle() = 0;

    static time_t timeDiff( time_t lhs, time_t rhs );

protected:
    QPointer<RiuQwtPlotWidget> m_plotWidget;

    // Fields
    caf::PdmPtrField<RimSummaryCaseCollection*> m_ensemble;
    caf::PdmChildField<RimSummaryAddress*>      m_summaryAddress;
    caf::PdmField<RifEclipseSummaryAddress>     m_summaryAddressUiField;
    caf::PdmField<bool>                         m_pushButtonSelectSummaryAddress;
    caf::PdmField<QDateTime>                    m_timeStep;

    caf::PdmField<bool>    m_showPlotTitle;
    caf::PdmField<bool>    m_useAutoPlotTitle;
    caf::PdmField<QString> m_description;
};
