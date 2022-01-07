/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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
#include "RimSummaryDataSourceStepping.h"

#include "cafPdmChildField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrArrayField.h"

class RimMultiPlot;
class RimSummaryPlot;
class RimSummaryPlotSourceStepping;
class RimSummaryPlotNameHelper;
class RimSummaryNameHelper;

//==================================================================================================
///
///
//==================================================================================================
class RimSummaryMultiPlot : public RimPlotWindow, public RimSummaryDataSourceStepping
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryMultiPlot();
    ~RimSummaryMultiPlot() override;

    QWidget* viewWidget() override;
    QImage   snapshotWindowContent() override;
    void     zoomAll() override;
    QString  description() const override;

    void addPlot( RimSummaryPlot* plot );

    void                        updatePlotTitles();
    const RimSummaryNameHelper* nameHelper() const;

    void setAutoTitlePlot( bool enable );
    void setAutoTitleGraphs( bool enable );

    static RimSummaryMultiPlot* createAndAppendMultiPlot( const std::vector<RimSummaryPlot*>& plots );

    std::vector<RimSummaryDataSourceStepping::Axis> availableAxes() const override;
    std::vector<RimSummaryCurve*>     curvesForStepping( RimSummaryDataSourceStepping::Axis axis ) const override;
    std::vector<RimEnsembleCurveSet*> curveSets() const override;
    std::vector<RimSummaryCurve*>     allCurves( RimSummaryDataSourceStepping::Axis axis ) const override;

private:
    QWidget* createViewWidget( QWidget* mainWindowParent = nullptr ) override;
    void     deleteViewWidget() override;
    void     onLoadDataAndUpdate() override;

    void doRenderWindowContent( QPaintDevice* paintDevice ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName ) override;

    void updatePlots();
    void populateNameHelper( RimSummaryPlotNameHelper* nameHelper );

    std::vector<RimSummaryPlot*> summaryPlots() const;

    static void insertGraphsIntoPlot( RimSummaryMultiPlot* plot, const std::vector<RimSummaryPlot*>& graphs );

private:
    caf::PdmField<QString> m_filterText;
    caf::PdmField<bool>    m_individualPlotPerVector;
    caf::PdmField<bool>    m_individualPlotPerDataSource;
    caf::PdmField<bool>    m_individualPlotPerObject;
    caf::PdmField<bool>    m_autoPlotTitles;
    caf::PdmField<bool>    m_autoPlotTitlesOnSubPlots;

    caf::PdmField<bool>               m_showMultiPlotInProjectTree;
    caf::PdmChildField<RimMultiPlot*> m_multiPlot;

    caf::PdmChildField<RimSummaryPlotSourceStepping*> m_sourceStepping;

    std::unique_ptr<RimSummaryPlotNameHelper> m_nameHelper;
};
