/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "Rim3dOverlayInfoConfig.h"

#include "cafPdmPointer.h"

#include <QDialog>

class QLabel;
class QTextEdit;
class QDialogButtonBox;
class QwtPlot;
class QwtPlotMarker;
class QMainWindow;
class QToolBar;
class RimEclipseView;

//==================================================================================================
///  
///  
//==================================================================================================
class RicGridStatisticsDialog : public QDialog
{
    Q_OBJECT

public:
    RicGridStatisticsDialog(QWidget* parent);
    ~RicGridStatisticsDialog();

    void                    setLabel(const QString& labelText);
    void                    updateFromRimView(RimView* rimView);
    QImage                  screenShotImage();

private:
    void                    setInfoText(RimView* eclipseView);
    void                    setHistogramData(RimView* eclipseView);

private:
    void                    createAndConnectToolbarActions();
    void                    deletePlotItems(QwtPlot* plot);
    static void             setMarkers(const Rim3dOverlayInfoConfig::HistogramData& histData, QwtPlot* plot);
    static QwtPlotMarker*   createVerticalPlotMarker(const QColor& color, double xValue);
    void                    adjustTextEditHeightToContent();

private slots:
    void slotDialogFinished();
    void slotScreenShotToClipboard();
    void slotScreenShotToFile();

private:
    QToolBar*                           m_toolBar;
    QWidget*                            m_mainViewWidget;
    QLabel*                             m_label;
    QTextEdit*                          m_textEdit;
    QwtPlot*                            m_historgramPlot;
    QwtPlot*                            m_aggregatedPlot;
    QDialogButtonBox*                   m_buttons;

    caf::PdmPointer<RimView>            m_currentRimView;
};
