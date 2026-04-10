/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include <QObject>

class RimParameterRftCrossPlot;
class RimRftTornadoPlot;
class RimWellRftPlot;

namespace ads
{
class CDockManager;
class CDockWidget;
} // namespace ads

//==================================================================================================
///
/// Composite report plot combining a RimWellRftPlot (pressure profile) and a
/// RimParameterRftCrossPlot (ensemble parameter vs mean pressure in a depth range).
///
//==================================================================================================
class RimRftCorrelationReportPlot : public QObject, public RimPlotWindow
{
    CAF_PDM_HEADER_INIT;

public:
    RimRftCorrelationReportPlot();
    ~RimRftCorrelationReportPlot() override;

    QWidget* viewWidget() override;
    QString  description() const override;
    void     zoomAll() override {}

    caf::PdmFieldHandle* userDescriptionField() override;

    RimWellRftPlot*           wellRftPlot() const;
    RimParameterRftCrossPlot* crossPlot() const;
    RimRftTornadoPlot*        correlationPlot() const;

    void initializeFromSourcePlot( RimWellRftPlot* source );

private:
    QString createDescription() const;
    void    recreatePlotWidgets();
    void    cleanupBeforeClose();

    void     setupBeforeSave() override;
    void     doRenderWindowContent( QPaintDevice* paintDevice ) override;
    QWidget* createViewWidget( QWidget* mainWindowParent = nullptr ) override;
    void     deleteViewWidget() override;
    void     onLoadDataAndUpdate() override;
    void     defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void     defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void     fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void     childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField ) override;
    void     doUpdateLayout() override {}

    void updateDockTitleBarsVisibility();
    void syncCrossPlotSelectionToRftPlot();
    void onTornadoParameterSelected( const QString& paramName );
    void syncTornadoInputsFromCrossPlot();

private:
    caf::PdmProxyValueField<QString> m_name;

    caf::PdmChildField<RimWellRftPlot*>           m_wellRftPlot;
    caf::PdmChildField<RimParameterRftCrossPlot*> m_parameterRftCrossPlot;
    caf::PdmChildField<RimRftTornadoPlot*>        m_correlationPlot;

    caf::PdmField<bool>    m_showDockTitleBars;
    caf::PdmField<QString> m_dockState;

    QWidget*           m_viewWidget            = nullptr;
    QObject*           m_contextMenuFilter     = nullptr;
    ads::CDockManager* m_dockManager           = nullptr;
    ads::CDockWidget*  m_rftDockWidget         = nullptr;
    ads::CDockWidget*  m_correlationDockWidget = nullptr;
    ads::CDockWidget*  m_crossPlotDockWidget   = nullptr;
};
