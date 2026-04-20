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

#include "RimPlot.h"

#include "Appearance/RimFontSizeField.h"

#include "cafPdmField.h"
#include "cafPdmPtrField.h"
#include "cvfColor3.h"

#include <QDateTime>
#include <QPointer>

#include <functional>
#include <map>

class RimEclipseResultCase;
class RimSummaryEnsemble;
class RiuQwtPlotWidget;

//==================================================================================================
///
/// Tornado plot showing Pearson correlation between ensemble parameters and mean RFT pressure
/// within a depth range. Used as a child of RimRftCorrelationReportPlot.
///
//==================================================================================================
class RimRftTornadoPlot : public RimPlot
{
    Q_OBJECT;
    CAF_PDM_HEADER_INIT;

public:
    RimRftTornadoPlot();
    ~RimRftTornadoPlot() override;

    using ParameterSelectedCallback = std::function<void( const QString& )>;
    void setParameterSelectedCallback( ParameterSelectedCallback callback );

    // Inputs set by the parent report plot
    void setEnsemble( RimSummaryEnsemble* ensemble );
    void setWellName( const QString& wellName );
    void setTimeStep( const QDateTime& timeStep );
    void setEclipseCase( RimEclipseResultCase* eclipseCase );
    void setUseDepthRange( bool useDepthRange );
    void setDepthRange( double minMd, double maxMd );
    void setSelectedParameter( const QString& paramName );

    RiuQwtPlotWidget* viewer();

    // RimPlot pure virtual overrides
    RiuPlotWidget* plotWidget() override;
    void           setAutoScaleXEnabled( bool ) override {}
    void           setAutoScaleYEnabled( bool ) override {}
    void           updateAxes() override;
    void           updateLegend() override {}
    QString        asciiDataForPlotExport() const override;
    void           reattachAllCurves() override {}
    void           detachAllCurves() override;
    QString        description() const override;

private:
    QWidget*       viewWidget() override;
    void           zoomAll() override {}
    void           deleteViewWidget() override;
    void           doRenderWindowContent( QPaintDevice* paintDevice ) override;
    RiuPlotWidget* doCreatePlotViewWidget( QWidget* parent = nullptr ) override;

    void onLoadDataAndUpdate() override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void onPlotItemSelected( std::shared_ptr<RiuPlotItem> plotItem, bool toggle, int sampleIndex ) override;

    std::map<QString, double> addDataToChartBuilder( class RiuGroupedBarChartBuilder& chartBuilder ) const;
    void                      updatePlotTitle();
    void                      cleanupBeforeClose();

private:
    // Data source inputs
    caf::PdmPtrField<RimSummaryEnsemble*>   m_ensemble;
    caf::PdmField<QString>                  m_wellName;
    caf::PdmField<QDateTime>                m_selectedTimeStep;
    caf::PdmPtrField<RimEclipseResultCase*> m_eclipseCase;
    caf::PdmField<bool>                     m_useDepthRange;
    caf::PdmField<double>                   m_depthRangeMin;
    caf::PdmField<double>                   m_depthRangeMax;

    // Tornado settings
    caf::PdmField<bool>         m_showAbsoluteValues;
    caf::PdmField<bool>         m_sortByAbsoluteValues;
    caf::PdmField<bool>         m_showOnlyTopNCorrelations;
    caf::PdmField<int>          m_topNFilterCount;
    caf::PdmField<cvf::Color3f> m_barColor;
    caf::PdmField<cvf::Color3f> m_highlightBarColor;

    caf::PdmField<bool>    m_useAutoPlotTitle;
    caf::PdmField<QString> m_description;

    RimFontSizeField m_labelFontSize;
    RimFontSizeField m_axisTitleFontSize;
    RimFontSizeField m_axisValueFontSize;

    ParameterSelectedCallback m_parameterSelectedCallback;
    QString                   m_selectedParameter;
    std::map<QString, double> m_lastCorrelations; // param.name -> pearson value, updated in onLoadDataAndUpdate

    QPointer<RiuQwtPlotWidget> m_plotWidget;
};
