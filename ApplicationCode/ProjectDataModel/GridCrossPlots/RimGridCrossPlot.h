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

#include "RiaDefines.h"
#include "RimNameConfig.h"
#include "RimPlot.h"

#include <QPointer>

#include <set>

class RimPlotAxisPropertiesInterface;
class RimPlotAxisProperties;
class RimGridCrossPlotDataSet;
class RiuGridCrossQwtPlot;

class RimGridCrossPlotNameConfig : public RimNameConfig
{
    CAF_PDM_HEADER_INIT;

public:
    RimGridCrossPlotNameConfig();

public:
    caf::PdmField<bool> addDataSetNames;

protected:
    virtual void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
};

class RimGridCrossPlot : public RimPlot, public RimNameConfigHolderInterface
{
    CAF_PDM_HEADER_INIT;

public:
    RimGridCrossPlot();
    ~RimGridCrossPlot();

    QString description() const override;

    RimGridCrossPlotDataSet* createDataSet();
    int                      indexOfDataSet( const RimGridCrossPlotDataSet* dataSet ) const;
    void                     addDataSet( RimGridCrossPlotDataSet* dataSet );

    std::vector<RimGridCrossPlotDataSet*> dataSets() const;

    QWidget*          viewWidget() override;
    RiuQwtPlotWidget* viewer() override;

    QImage  snapshotWindowContent() override;
    void    zoomAll() override;
    void    calculateZoomRangeAndUpdateQwt();
    QString createAutoName() const override;

    bool                 showInfoBox() const;
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

    bool hasCustomFontSizes( RiaDefines::FontSettingType fontSettingType, int defaultFontSize ) const override;
    bool applyFontSize( RiaDefines::FontSettingType fontSettingType,
                        int                         oldFontSize,
                        int                         fontSize,
                        bool                        forceChange = false ) override;

    void updateLegend() override;

    void updateZoomInQwt() override;
    void updateZoomFromQwt() override;

    void            setAutoScaleXEnabled( bool enabled ) override;
    void            setAutoScaleYEnabled( bool enabled ) override;
    caf::PdmObject* findPdmObjectFromQwtCurve( const QwtPlotCurve* curve ) const override;
    void            onAxisSelected( int axis, bool toggle ) override;

    void addOrUpdateDataSetLegend( RimGridCrossPlotDataSet* dataSet );
    void removeDataSetLegend( RimGridCrossPlotDataSet* dataSet );

protected:
    QWidget* createViewWidget( QWidget* mainWindowParent = nullptr ) override;
    void     deleteViewWidget() override;
    void     onLoadDataAndUpdate() override;
    void     initAfterRead() override;
    void     defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void     defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                           const QVariant&            oldValue,
                           const QVariant&            newValue ) override;

    void updateAxes() override;
    void updatePlot();

    virtual QString xAxisParameterString() const;
    QString         yAxisParameterString() const;

    void                             updateAxisInQwt( RiaDefines::PlotAxis axisType );
    void                             updateAxisFromQwt( RiaDefines::PlotAxis axisType );
    std::vector<const QwtPlotCurve*> visibleQwtCurves() const;

    RimPlotAxisProperties* xAxisProperties();
    RimPlotAxisProperties* yAxisProperties();

    RimGridCrossPlotNameConfig* nameConfig();
    void                        setShowInfoBox( bool enable );

    std::set<RimPlotAxisPropertiesInterface*> allPlotAxes() const;

private:
    void doUpdateLayout() override;
    void cleanupBeforeClose();

    void doRemoveFromCollection() override;

private:
    caf::PdmField<bool>                             m_showInfoBox;
    caf::PdmField<bool>                             m_showLegend_OBSOLETE;
    caf::PdmChildField<RimGridCrossPlotNameConfig*> m_nameConfig;

    caf::PdmChildField<RimPlotAxisProperties*> m_yAxisProperties;
    caf::PdmChildField<RimPlotAxisProperties*> m_xAxisProperties;

    caf::PdmChildArrayField<RimGridCrossPlotDataSet*> m_crossPlotDataSets;

    QPointer<RiuGridCrossQwtPlot> m_plotWidget;
};
