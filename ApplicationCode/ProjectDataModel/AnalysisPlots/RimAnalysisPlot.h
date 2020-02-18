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

#include "RimPlot.h"

#include <QDateTime>

class RiuSummaryQwtPlot;
class RimAnalysisPlotDataEntry;
class RiuGroupedBarChartBuilder;

//==================================================================================================
///
///
//==================================================================================================
class RimAnalysisPlot : public RimPlot
{
    CAF_PDM_HEADER_INIT;

public:
    RimAnalysisPlot();
    ~RimAnalysisPlot() override;

    bool showPlotTitle() const;
    void setShowPlotTitle( bool showTitle );

    void detachAllCurves() override;
    void reattachAllCurves() override;

    void              updateAxes() override;
    QWidget*          viewWidget() override;
    RiuQwtPlotWidget* viewer() override;

    QString asciiDataForPlotExport() const override;

    void updateLegend() override;

    bool hasCustomFontSizes( RiaDefines::FontSettingType fontSettingType, int defaultFontSize ) const override;
    bool applyFontSize( RiaDefines::FontSettingType fontSettingType,
                        int                         oldFontSize,
                        int                         fontSize,
                        bool                        forceChange = false ) override;

    void setAutoScaleXEnabled( bool enabled ) override;
    void setAutoScaleYEnabled( bool enabled ) override;

    void zoomAll() override;
    void updateZoomInQwt() override;
    void updateZoomFromQwt() override;

    caf::PdmObject* findPdmObjectFromQwtCurve( const QwtPlotCurve* curve ) const override;

    void onAxisSelected( int axis, bool toggle ) override;

    // RimViewWindow overrides
    void deleteViewWidget() override;

    enum SortGroupType
    {
        SUMMARY_ITEM,
        CASE,
        ENSEMBLE,
        VALUE,
        ABS_VALUE,
        OTHER_VALUE,
        ABS_OTHER_VALUE,
        TIME_STEP,
    };

    QString description() const override;

    void updateCaseNameHasChanged();

private:
    RiuQwtPlotWidget* doCreatePlotViewWidget( QWidget* mainWindowParent = nullptr ) override;
    void              cleanupBeforeClose();

    void doUpdateLayout() override;

    void doRemoveFromCollection() override;

    QImage snapshotWindowContent() override;

    // Overridden PDM methods
    caf::PdmFieldHandle* userDescriptionField() override;
    void                 fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void onLoadDataAndUpdate() override;

    void buildChart( RiuGroupedBarChartBuilder& chartBuilder );
    void buildTestPlot( RiuGroupedBarChartBuilder& chartBuilder );
    void updatePlotTitle();

private:
    QPointer<RiuQwtPlotWidget> m_plotWidget;

    caf::PdmField<bool>    m_showPlotTitle;
    caf::PdmField<bool>    m_useAutoPlotTitle;
    caf::PdmField<QString> m_description;

    caf::PdmField<QString> m_selectedVarsUiField;
    caf::PdmField<bool>    m_selectVariablesButtonField;

    caf::PdmChildArrayField<RimAnalysisPlotDataEntry*> m_data;

    caf::PdmField<std::vector<QDateTime>> m_selectedTimeSteps;

    caf::PdmField<std::vector<caf::AppEnum<SortGroupType>>> m_sortGroupSortingOrder;
    caf::PdmField<std::vector<caf::AppEnum<SortGroupType>>> m_sortGroupsToGroup;

    caf::PdmField<caf::AppEnum<SortGroupType>> m_sortGroupForLegend;
};
