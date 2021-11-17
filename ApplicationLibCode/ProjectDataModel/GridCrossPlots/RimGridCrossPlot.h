/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmObject.h"

#include "RiaPlotDefines.h"
#include "RimNameConfig.h"
#include "RimQwtPlot.h"

#include <QPointer>

#include <set>

class RimPlotAxisPropertiesInterface;
class RimPlotAxisProperties;
class RimGridCrossPlotDataSet;
class RiuDraggableOverlayFrame;
class RiuGridCrossQwtPlot;

class RimGridCrossPlotNameConfig : public RimNameConfig
{
    CAF_PDM_HEADER_INIT;

public:
    RimGridCrossPlotNameConfig();

public:
    caf::PdmField<bool> addDataSetNames;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    void doEnableAllAutoNameTags( bool enable ) override;
};

class RimGridCrossPlot : public RimPlot, public RimNameConfigHolderInterface
{
    Q_OBJECT;
    CAF_PDM_HEADER_INIT;

public:
    RimGridCrossPlot();
    ~RimGridCrossPlot() override;

    QString description() const override;

    RimGridCrossPlotDataSet* createDataSet();
    int                      indexOfDataSet( const RimGridCrossPlotDataSet* dataSet ) const;
    void                     addDataSet( RimGridCrossPlotDataSet* dataSet );

    std::vector<RimGridCrossPlotDataSet*> dataSets() const;

    QWidget*          viewWidget() override;
    RiuQwtPlotWidget* viewer();
    RiuPlotWidget*    plotWidget() override;

    QImage  snapshotWindowContent() override;
    void    zoomAll() override;
    void    calculateZoomRangeAndUpdateQwt();
    QString createAutoName() const override;

    bool                 showInfoBox() const;
    void                 updateInfoBox();
    caf::PdmFieldHandle* userDescriptionField() override;

    void detachAllCurves() override;
    void reattachAllCurves() override;

    void performAutoNameUpdate() override;
    void updateCurveNamesAndPlotTitle();
    void swapAxes();

    QString asciiDataForPlotExport() const override;
    QString asciiTitleForPlotExport( int dataSetIndex ) const;
    QString asciiDataForGridCrossPlotExport( int dataSetIndex ) const;

    bool isXAxisLogarithmic() const;
    bool isYAxisLogarithmic() const;
    void setYAxisInverted( bool inverted );

    void updateLegend() override;

    void updateZoomInParentPlot() override;
    void updateZoomFromParentPlot() override;

    void            setAutoScaleXEnabled( bool enabled ) override;
    void            setAutoScaleYEnabled( bool enabled ) override;
    caf::PdmObject* findPdmObjectFromPlotCurve( const RiuPlotCurve* curve ) const;
    void            onAxisSelected( int axis, bool toggle );

    bool isDeletable() const override;

protected:
    void deleteViewWidget() override;
    void onLoadDataAndUpdate() override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    void updateAxes() override;
    void updatePlot();

    virtual QString xAxisParameterString() const;
    QString         yAxisParameterString() const;

    void                             updateAxisInQwt( RiaDefines::PlotAxis axisType );
    void                             updateAxisFromQwt( RiaDefines::PlotAxis axisType );
    std::vector<const RimPlotCurve*> visibleCurves() const;

    RimPlotAxisProperties* xAxisProperties();
    RimPlotAxisProperties* yAxisProperties();

    RimGridCrossPlotNameConfig* nameConfig();
    void                        setShowInfoBox( bool enable );

    std::set<RimPlotAxisPropertiesInterface*> allPlotAxes() const;

private:
    RiuQwtPlotWidget* doCreatePlotViewWidget( QWidget* mainWindowParent = nullptr ) override;

    void doUpdateLayout() override;
    void cleanupBeforeClose();

    QString generateInfoBoxText() const;

    void connectAxisSignals( RimPlotAxisProperties* axis );
    void axisSettingsChanged( const caf::SignalEmitter* emitter );
    void axisLogarithmicChanged( const caf::SignalEmitter* emitter, bool isLogarithmic );

private slots:
    void onPlotZoomed();

private:
    caf::PdmField<bool>                             m_showInfoBox;
    caf::PdmChildField<RimGridCrossPlotNameConfig*> m_nameConfig;

    caf::PdmChildField<RimPlotAxisProperties*> m_yAxisProperties;
    caf::PdmChildField<RimPlotAxisProperties*> m_xAxisProperties;

    caf::PdmChildArrayField<RimGridCrossPlotDataSet*> m_crossPlotDataSets;

    QPointer<RiuGridCrossQwtPlot>      m_plotWidget;
    QPointer<RiuDraggableOverlayFrame> m_infoBox;
};
