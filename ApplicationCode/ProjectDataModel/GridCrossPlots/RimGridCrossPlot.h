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
#include "RimRiuQwtPlotOwnerInterface.h"
#include "RimViewWindow.h"

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
    RimGridCrossPlotNameConfig( RimNameConfigHolderInterface* holder = nullptr );

public:
    caf::PdmField<bool> addDataSetNames;

protected:
    virtual void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
};

class RimGridCrossPlot : public RimViewWindow, public RimRiuQwtPlotOwnerInterface, public RimNameConfigHolderInterface
{
    CAF_PDM_HEADER_INIT;

public:
    RimGridCrossPlot();
    ~RimGridCrossPlot();

    RimGridCrossPlotDataSet* createDataSet();
    int                      indexOfDataSet( const RimGridCrossPlotDataSet* dataSet ) const;
    void                     addDataSet( RimGridCrossPlotDataSet* dataSet );

    std::vector<RimGridCrossPlotDataSet*> dataSets() const;

    QWidget* viewWidget() override;
    QImage   snapshotWindowContent() override;
    void     zoomAll() override;
    void     calculateZoomRangeAndUpdateQwt();
    void     reattachCurvesToQwtAndReplot();
    QString  createAutoName() const override;

    bool                 showInfoBox() const;
    caf::PdmFieldHandle* userDescriptionField() override;
    void                 detachAllCurves();
    void                 performAutoNameUpdate() override;
    void                 updateCurveNamesAndPlotTitle();
    void                 swapAxes();
    QString              asciiTitleForPlotExport( int dataSetIndex ) const;
    QString              asciiDataForPlotExport( int dataSetIndex ) const;
    RiuGridCrossQwtPlot* qwtPlot() const;
    bool                 isXAxisLogarithmic() const;
    bool                 isYAxisLogarithmic() const;
    void                 setYAxisInverted( bool inverted );
    int                  legendFontSize() const;

    bool hasCustomFontSizes( RiaDefines::FontSettingType fontSettingType, int defaultFontSize ) const override;
    bool applyFontSize( RiaDefines::FontSettingType fontSettingType,
                        int                         oldFontSize,
                        int                         fontSize,
                        bool                        forceChange = false ) override;

public:
    // Rim2dPlotInterface overrides
    void            updateAxisScaling() override;
    void            updateAxisDisplay() override;
    void            updateZoomWindowFromQwt() override;
    void            selectAxisInPropertyEditor( int axis ) override;
    void            setAutoZoomForAllAxes( bool enableAutoZoom ) override;
    caf::PdmObject* findRimPlotObjectFromQwtCurve( const QwtPlotCurve* curve ) const override;

protected:
    QWidget* createViewWidget( QWidget* mainWindowParent ) override;
    void     deleteViewWidget() override;
    void     onLoadDataAndUpdate() override;
    void     defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void     defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                    const QVariant&            oldValue,
                                                    const QVariant&            newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

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
    caf::PdmField<bool>                             m_showInfoBox;
    caf::PdmField<bool>                             m_showLegend;
    caf::PdmField<int>                              m_legendFontSize;
    caf::PdmChildField<RimGridCrossPlotNameConfig*> m_nameConfig;

    caf::PdmChildField<RimPlotAxisProperties*> m_yAxisProperties;
    caf::PdmChildField<RimPlotAxisProperties*> m_xAxisProperties;

    caf::PdmChildArrayField<RimGridCrossPlotDataSet*> m_crossPlotDataSets;

    QPointer<RiuGridCrossQwtPlot> m_qwtPlot;
};
