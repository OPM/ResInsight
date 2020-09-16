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

#include "RifEclipseSummaryAddress.h"
#include "RifEclipseSummaryAddressQMetaType.h"

#include "RimPlot.h"
#include "RimSummaryCaseCollection.h"
#include "RimTimeStepFilter.h"

#include "cafPdmPtrField.h"

#include <QString>

#include <ctime>

class RiaSummaryCurveDefinition;
class RiaSummaryCurveDefinitionAnalyser;
class RimAnalysisPlotDataEntry;
class RimSummaryAddress;

class RimAbstractCorrelationPlot : public RimPlot
{
    CAF_PDM_HEADER_INIT;

public:
    using TimeStepFilterEnum = caf::AppEnum<RimTimeStepFilter::TimeStepFilterTypeEnum>;

public:
    RimAbstractCorrelationPlot();
    ~RimAbstractCorrelationPlot() override;

public:
    std::vector<RiaSummaryCurveDefinition> curveDefinitions() const;
    void setCurveDefinitions( const std::vector<RiaSummaryCurveDefinition>& curveDefinitions );
    void setTimeStep( std::time_t timeStep );
    std::set<RimSummaryCaseCollection*> ensembles();
    RiuQwtPlotWidget*                   viewer() override;
    void                                detachAllCurves() override;
    QDateTime                           timeStep() const;
    QString                             timeStepString() const;

    int labelFontSize() const;
    int axisTitleFontSize() const;
    int axisValueFontSize() const;

    void setLabelFontSize( caf::FontTools::RelativeSize fontSize );
    void setAxisTitleFontSize( caf::FontTools::RelativeSize fontSize );
    void setAxisValueFontSize( caf::FontTools::RelativeSize fontSize );

protected:
    // Overridden PDM methods

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

    caf::PdmFieldHandle*          userDescriptionField() override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    std::set<time_t>                   allAvailableTimeSteps();
    RiaSummaryCurveDefinitionAnalyser* getOrCreateSelectedCurveDefAnalyser();

    std::set<RifEclipseSummaryAddress> addresses();
    std::set<EnsembleParameter>        ensembleParameters();
    std::set<EnsembleParameter>        variationSortedEnsembleParameters();
    EnsembleParameter                  ensembleParameter( const QString& ensembleParameterName );

    // RimViewWindow overrides
    QWidget* viewWidget() override;
    void     deleteViewWidget() override;
    void     zoomAll() override {}
    QImage   snapshotWindowContent() override;

    // RimPlotWindow overrides
    QString description() const override;
    void    doUpdateLayout() override {}

    // RimPlot Overrides
    RiuQwtPlotWidget* doCreatePlotViewWidget( QWidget* mainWindowParent = nullptr ) override;

    void reattachAllCurves() override {}
    void doRemoveFromCollection() override {}
    void updateZoomInQwt() override {}
    void updateZoomFromQwt() override {}
    void setAutoScaleXEnabled( bool enabled ) override {}
    void setAutoScaleYEnabled( bool enabled ) override {}
    void updateLegend() override;

    QString         asciiDataForPlotExport() const override { return ""; }
    caf::PdmObject* findPdmObjectFromQwtCurve( const QwtPlotCurve* curve ) const override { return nullptr; }

    void         cleanupBeforeClose();
    virtual void updatePlotTitle() = 0;

    static time_t timeDiff( time_t lhs, time_t rhs );

    QString selectedVarsText();

    void initAfterRead() final;

private:
    void onCaseRemoved( const SignalEmitter* emitter, RimSummaryCase* summaryCase );
    void connectAllCaseSignals();

protected:
    std::unique_ptr<RiaSummaryCurveDefinitionAnalyser> m_analyserOfSelectedCurveDefs;
    QPointer<RiuQwtPlotWidget>                         m_plotWidget;

    bool m_selectMultipleVectors;

    // Fields
    caf::PdmChildArrayField<RimAnalysisPlotDataEntry*> m_analysisPlotDataSelection;

    caf::PdmField<QString>            m_selectedVarsUiField;
    caf::PdmField<bool>               m_pushButtonSelectSummaryAddress;
    caf::PdmField<TimeStepFilterEnum> m_timeStepFilter;
    caf::PdmField<QDateTime>          m_timeStep;

    caf::PdmField<bool>    m_useAutoPlotTitle;
    caf::PdmField<QString> m_description;

    caf::PdmField<caf::FontTools::RelativeSizeEnum> m_labelFontSize;
    caf::PdmField<caf::FontTools::RelativeSizeEnum> m_axisTitleFontSize;
    caf::PdmField<caf::FontTools::RelativeSizeEnum> m_axisValueFontSize;
};
