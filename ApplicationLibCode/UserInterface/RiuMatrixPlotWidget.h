/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RiuInterfaceToViewWindow.h"
#include "RiuPlotItem.h"
#include "RiuQwtPlotWidget.h"

#include "cafPdmPointer.h"
#include "cafSignal.h"

#include "cvfScalarMapper.h"

#include "qwt_plot.h"

#include <QPointer>
#include <QWidget>

class RimViewWindow;
class RimRegularLegendConfig;
class RiuAbstractLegendFrame;

class RiuMatrixPlotWidget : public QWidget, public RiuInterfaceToViewWindow, public caf::SignalEmitter
{
    Q_OBJECT

public:
    caf::Signal<std::pair<int, int>> matrixCellSelected;

public:
    RiuMatrixPlotWidget( RimViewWindow* ownerViewWindow, RimRegularLegendConfig* legendConfig, QWidget* parent = nullptr );
    ~RiuMatrixPlotWidget() override;

    QwtPlot* qwtPlot() const;

    void createPlot();
    void clearPlotData();
    void setColumnHeaders( const std::vector<QString>& columnHeaders );
    void setRowValues( const QString& rowLabel, const std::vector<double>& values );

    void setPlotTitle( const QString& title );
    void setColumnTitle( const QString& title );
    void setRowTitle( const QString& title );

    void setInvalidValueColor( const cvf::Color3ub& color );
    void setUseInvalidValueColor( bool useInvalidValueColor );
    void setInvalidValueRange( double min, double max );

    void setShowValueLabel( bool showValueLabel );

    void setPlotTitleFontSize( int fontSize );
    void setPlotTitleEnabled( bool enabled );
    void setLegendFontSize( int fontSize );

    void setAxisTitleFontSize( int fontSize );
    void setAxisLabelFontSize( int fontSize );
    void setValueFontSize( int fontSize );
    void setMaxColumnLabelCount( int maxLabelCount );

    void scheduleReplot();

    RimViewWindow* ownerViewWindow() const override;

protected:
    void contextMenuEvent( QContextMenuEvent* ) override;

private slots:
    void onPlotItemSelected( std::shared_ptr<RiuPlotItem> plotItem, bool toggle, int sampleIndex );

private:
    void                      updateAxes();
    void                      createMatrixCells();
    std::map<size_t, QString> createIndexLabelMap( const std::vector<QString>& labels, int maxLabelCount );

private:
    QPointer<RiuQwtPlotWidget>       m_plotWidget;
    QPointer<RiuAbstractLegendFrame> m_legendFrame;

    caf::PdmPointer<RimViewWindow>          m_ownerViewWindow; // Only intended to be used by ownerViewWindow()
    caf::PdmPointer<RimRegularLegendConfig> m_legendConfig;

    std::vector<QString>             m_columnHeaders;
    std::vector<QString>             m_rowHeaders;
    std::vector<std::vector<double>> m_rowValues;

    cvf::Color3ub             m_invalidValueColor    = cvf::Color3ub::WHITE;
    bool                      m_useInvalidValueColor = false;
    std::pair<double, double> m_invalidValueRange    = { 0.0, 0.0 };

    bool m_showValueLabel = true;

    QString m_rowTitle;
    QString m_columnTitle;
    int     m_axisTitleFontSize   = 8;
    int     m_axisLabelFontSize   = 8;
    int     m_valueFontSize       = 8;
    int     m_maxColumnLabelCount = 100;
};
