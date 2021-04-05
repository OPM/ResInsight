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
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

#include <QDateTime>
#include <QObject>
#include <QPointer>

class RimAnalysisPlotDataEntry;
class RimCorrelationMatrixPlot;
class RimParameterResultCrossPlot;
class RimSummaryCaseCollection;
class RimCorrelationPlot;
class RiaSummaryCurveDefinition;	

class RiuMultiPlotPage;

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

    int columnCount() const override;
    int subTitleFontSize() const;
    int axisTitleFontSize() const;
    int axisValueFontSize() const;

private:
    QString createPlotWindowTitle() const;
    void    recreatePlotWidgets();
    void    cleanupBeforeClose();

    void     doRenderWindowContent( QPaintDevice* paintDevice ) override;
    QWidget* createViewWidget( QWidget* mainWindowParent = nullptr ) override;
    void     deleteViewWidget() override;
    void     onLoadDataAndUpdate() override;
    void     defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void     fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void     childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField ) override;
    void     doUpdateLayout() override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          onDataSelection( const caf::SignalEmitter*                     emitter,
                                                   std::pair<QString, RiaSummaryCurveDefinition> parameterAndCurveDef );

private:
    caf::PdmProxyValueField<QString> m_plotWindowTitle;

    caf::PdmChildField<RimCorrelationMatrixPlot*>    m_correlationMatrixPlot;
    caf::PdmChildField<RimCorrelationPlot*>          m_correlationPlot;
    caf::PdmChildField<RimParameterResultCrossPlot*> m_parameterResultCrossPlot;
    caf::PdmField<caf::FontTools::RelativeSizeEnum>  m_subTitleFontSize;
    caf::PdmField<caf::FontTools::RelativeSizeEnum>  m_labelFontSize;
    caf::PdmField<caf::FontTools::RelativeSizeEnum>  m_axisTitleFontSize;
    caf::PdmField<caf::FontTools::RelativeSizeEnum>  m_axisValueFontSize;

    QPointer<RiuMultiPlotPage> m_viewer;
};
