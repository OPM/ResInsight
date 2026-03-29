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

#include "RimPlotWindow.h"

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

#include <QDateTime>
#include <QObject>

class RimAnalysisPlotDataEntry;
class RimCorrelationMatrixPlot;
class RimParameterResultCrossPlot;
class RimSummaryEnsemble;
class RimCorrelationPlot;
class RiaSummaryCurveDefinition;
class RimSummaryPlot;
class RimSummaryAddressSelector;

namespace ads
{
class CDockManager;
class CDockWidget;
} // namespace ads

class RimCorrelationReportPlot : public QObject, public RimPlotWindow
{
    CAF_PDM_HEADER_INIT;

public:
    RimCorrelationReportPlot();
    ~RimCorrelationReportPlot() override;

    QWidget* viewWidget() override;
    QString  description() const override;
    QImage   snapshotWindowContent() override;
    void     zoomAll() override;

    caf::PdmFieldHandle* userDescriptionField() override;

    RimCorrelationMatrixPlot*    matrixPlot() const;
    RimCorrelationPlot*          correlationPlot() const;
    RimParameterResultCrossPlot* crossPlot() const;

    int subTitleFontSize() const;
    int axisTitleFontSize() const;
    int axisValueFontSize() const;

private:
    QString createPlotWindowTitle() const;
    QString createDescription() const;
    void    recreatePlotWidgets();
    void    cleanupBeforeClose();

    void     setupBeforeSave() override;
    void     doRenderWindowContent( QPaintDevice* paintDevice ) override;
    QWidget* createViewWidget( QWidget* mainWindowParent = nullptr ) override;
    void     deleteViewWidget() override;
    void     onLoadDataAndUpdate() override;
    void     defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void     fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void     childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField ) override;
    void     doUpdateLayout() override;

    void onDataSelection( const caf::SignalEmitter* emitter, std::pair<QString, RiaSummaryCurveDefinition> parameterAndCurveDef );
    void onSummaryPlotMousePressed( double xPlotCoordinate );
    void onAddressSelectorChanged( const caf::SignalEmitter* emitter );
    void onSaveDefaultDockLayout();
    void onRestoreDefaultDockLayout();

private:
    caf::PdmProxyValueField<QString> m_name;

    caf::PdmChildField<RimCorrelationMatrixPlot*>    m_correlationMatrixPlot;
    caf::PdmChildField<RimCorrelationPlot*>          m_correlationPlot;
    caf::PdmChildField<RimParameterResultCrossPlot*> m_parameterResultCrossPlot;
    RimFontSizeField                                 m_subTitleFontSize;
    RimFontSizeField                                 m_labelFontSize;
    RimFontSizeField                                 m_axisTitleFontSize;
    RimFontSizeField                                 m_axisValueFontSize;

    caf::PdmField<bool>                            m_showSummaryPlot;
    caf::PdmChildField<RimSummaryPlot*>            m_summaryPlot;
    caf::PdmChildField<RimSummaryAddressSelector*> m_summaryAddressSelector;

    caf::PdmField<QString> m_dockState;

    QWidget*           m_viewWidget            = nullptr;
    QObject*           m_contextMenuFilter     = nullptr;
    ads::CDockManager* m_dockManager           = nullptr;
    ads::CDockWidget*  m_matrixDockWidget      = nullptr;
    ads::CDockWidget*  m_correlationDockWidget = nullptr;
    ads::CDockWidget*  m_crossPlotDockWidget   = nullptr;
    ads::CDockWidget*  m_summaryDockWidget     = nullptr;
};
