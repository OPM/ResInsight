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
#include "RimSummarySourceSteppingInterface.h"

#include "cafPdmChildField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrArrayField.h"

class RimMultiPlot;
class RimSummaryPlot;
class RimSummaryPlotSourceStepping;

//==================================================================================================
///
///
//==================================================================================================
class RimSummaryMultiPlot : public RimPlotWindow, public RimSummarySourceSteppingInterface
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryMultiPlot();
    ~RimSummaryMultiPlot() override;

    QWidget* viewWidget() override;
    QImage   snapshotWindowContent() override;
    void     zoomAll() override;
    QString  description() const override;

    void addPlot( RimPlot* plot );

    static RimSummaryMultiPlot* createAndAppendMultiPlot( const std::vector<RimPlot*>& plots );

    std::vector<RimSummarySourceSteppingInterface::Axis> availableAxes() const override;
    std::vector<RimSummaryCurve*>     curvesForStepping( RimSummarySourceSteppingInterface::Axis axis ) const override;
    std::vector<RimEnsembleCurveSet*> curveSets() const override;
    std::vector<RimSummaryCurve*>     allCurves( RimSummarySourceSteppingInterface::Axis axis ) const override;

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

    std::vector<RimSummaryPlot*> summaryPlots() const;

private:
    caf::PdmField<QString> m_filterText;
    caf::PdmField<bool>    m_individualPlotPerVector;
    caf::PdmField<bool>    m_individualPlotPerDataSource;

    caf::PdmField<bool>               m_showMultiPlotInProjectTree;
    caf::PdmChildField<RimMultiPlot*> m_multiPlot;

    caf::PdmChildField<RimSummaryPlotSourceStepping*> m_sourceStepping;
};
