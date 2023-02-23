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

#include "cafFontTools.h"
#include "cafSignal.h"

#include "cvfScalarMapper.h"

#include "qwt_plot.h"

#include <QPointer>
#include <QWidget>

#include <span>

// TODO: Rename to RiuMatrixPlotWidgetWrapper?
class RiuMatrixPlotWidget : public QWidget, public RiuInterfaceToViewWindow, public caf::SignalEmitter
{
    Q_OBJECT

public:
    caf::Signal<std::pair<int, int>> matrixCellSelected;

public:
    RiuMatrixPlotWidget( RimViewWindow* ownerViewWindow, QWidget* parent = nullptr );
    ~RiuMatrixPlotWidget();

    QwtPlot* qwtPlot() const;

    void createPlot();
    void clearPlotData();
    void setColumnHeaders( const std::vector<QString>& columnHeaders );
    void setRowValues( const QString& rowLabel, const std::vector<double>& values );

    void setPlotTitle( const QString& title );
    void setPlotTitleFontSize( int fontSize );
    void setPlotTitleEnabled( bool enabled );
    void setLegendFontSize( int fontSize );
    void setAxisTitleText( RiuPlotAxis axis, const QString& title );
    void setAxisTitleFontSize( int fontSize );
    void setAxisLabelFontSize( int fontSize );
    void setValueFontSize( int fontSize );

    void setScalarMapper( const cvf::ScalarMapper* scalarMapper );

    void scheduleReplot();

    virtual RimViewWindow* ownerViewWindow() const override;

private slots:
    void onPlotItemSelected( std::shared_ptr<RiuPlotItem> plotItem, bool toggle, int sampleIndex );

private:
    void                      updateAxes();
    void                      createMatrixCells();
    std::map<size_t, QString> createIndexLabelMap( const std::vector<QString>& labels );

private:
    QPointer<RiuQwtPlotWidget>     m_plotWidget;
    caf::PdmPointer<RimViewWindow> m_ownerViewWindow; // Only intended to be used by ownerViewWindow()

    cvf::cref<cvf::ScalarMapper> m_scalarMapper;

    std::vector<QString>             m_columnHeaders;
    std::vector<QString>             m_rowHeaders;
    std::vector<std::vector<double>> m_rowValues;

    QString m_yAxisTitle;
    QString m_xAxisTitle;
    int     m_axisTitleFontSize = 8;
    int     m_axisLabelFontSize = 8;
    int     m_valueFontSize     = 8;
};
