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

#include "RiaSummaryCurveDefinition.h"

#include "RimPlot.h"

#include "cafPdmPtrField.h"

#include <QDateTime>

class RiuSummaryQwtPlot;
class RiuGroupedBarChartBuilder;

class RimAnalysisPlotDataEntry;
class RimCurveDefinitionAnalyser;
class RimPlotAxisPropertiesInterface;
class RimPlotAxisProperties;
class RimPlotDataFilterCollection;

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

    void updateCaseNameHasChanged();

    RimPlotDataFilterCollection* plotDataFilterCollection() const;

    std::set<RifEclipseSummaryAddress> unfilteredAddresses();

public: // Internal. Public needed for AppEnum setup
    enum BarOrientation
    {
        BARS_HORIZONTAL,
        BARS_VERTICAL
    };

    enum SortGroupType
    {
        NONE,
        SUMMARY_ITEM,
        QUANTITY,
        CASE,
        ENSEMBLE,
        VALUE,
        ABS_VALUE,
        OTHER_VALUE,
        ABS_OTHER_VALUE,
        TIME_STEP,
    };
    typedef caf::AppEnum<SortGroupType> SortGroupAppEnum;

private:
    // Overridden PDM methods

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

    caf::PdmFieldHandle*          userDescriptionField() override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    // RimViewWindow overrides

    QWidget* viewWidget() override;
    void     deleteViewWidget() override;
    void     onLoadDataAndUpdate() override;
    void     zoomAll() override {}
    QImage   snapshotWindowContent() override;
    bool     applyFontSize( RiaDefines::FontSettingType fontSettingType,
                            int                         oldFontSize,
                            int                         fontSize,
                            bool                        forceChange = false ) override;

    // RimPlotWindow overrides

    QString description() const override;
    void    doUpdateLayout() override {}

    // RimPlot Overrides

    RiuQwtPlotWidget* doCreatePlotViewWidget( QWidget* mainWindowParent = nullptr ) override;
    RiuQwtPlotWidget* viewer() override;

    void detachAllCurves() override;

    void reattachAllCurves() override {}
    void doRemoveFromCollection() override {}
    void updateAxes() override;
    void onAxisSelected( int axis, bool toggle ) override;
    void updateZoomInQwt() override {}
    void updateZoomFromQwt() override {}
    void setAutoScaleXEnabled( bool enabled ) override {}
    void setAutoScaleYEnabled( bool enabled ) override {}
    void updateLegend() override{};

    QString         asciiDataForPlotExport() const override { return ""; }
    caf::PdmObject* findPdmObjectFromQwtCurve( const QwtPlotCurve* curve ) const override { return nullptr; }

    // Private methods

    void cleanupBeforeClose();
    void addDataToChartBuilder( RiuGroupedBarChartBuilder& chartBuilder );
    void updatePlotTitle();

    RimCurveDefinitionAnalyser*               getOrCreateSelectedCurveDefAnalyser();
    std::vector<RiaSummaryCurveDefinition>    curveDefinitions() const;
    std::set<RimPlotAxisPropertiesInterface*> allPlotAxes() const;

    void buildTestPlot( RiuGroupedBarChartBuilder& chartBuilder );

private:
    std::unique_ptr<RimCurveDefinitionAnalyser> m_analyserOfSelectedCurveDefs;

    QPointer<RiuQwtPlotWidget> m_plotWidget;

    // Fields

    caf::PdmField<QString> m_selectedVarsUiField;
    caf::PdmField<bool>    m_selectVariablesButtonField;

    caf::PdmChildArrayField<RimAnalysisPlotDataEntry*> m_analysisPlotDataSelection;

    caf::PdmField<QDateTime>              m_addTimestepUiField;
    caf::PdmField<std::vector<QDateTime>> m_selectedTimeSteps;

    caf::PdmPtrField<RimSummaryCase*> m_referenceCase;

    caf::PdmField<bool>    m_showPlotTitle;
    caf::PdmField<bool>    m_useAutoPlotTitle;
    caf::PdmField<QString> m_description;

    caf::PdmField<caf::AppEnum<BarOrientation>> m_barOrientation;

    caf::PdmField<SortGroupAppEnum> m_majorGroupType;
    caf::PdmField<SortGroupAppEnum> m_mediumGroupType;
    caf::PdmField<SortGroupAppEnum> m_minorGroupType;
    caf::PdmField<SortGroupAppEnum> m_valueSortOperation;

    caf::PdmField<bool> m_useTopBarsFilter;
    caf::PdmField<int>  m_maxBarCount;

    caf::PdmField<SortGroupAppEnum> m_sortGroupForLegend;

    caf::PdmField<bool> m_useBarText;
    caf::PdmField<bool> m_useCaseInBarText;
    caf::PdmField<bool> m_useEnsembleInBarText;
    caf::PdmField<bool> m_useSummaryItemInBarText;
    caf::PdmField<bool> m_useTimeStepInBarText;
    caf::PdmField<bool> m_useQuantityInBarText;

    caf::PdmChildField<RimPlotAxisProperties*> m_valueAxisProperties;
    caf::PdmChildField<RimPlotDataFilterCollection*> m_plotDataFilterCollection;
};
