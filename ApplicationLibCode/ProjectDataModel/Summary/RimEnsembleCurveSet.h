/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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
#include "RifSummaryReaderInterface.h"

#include "RiaDefines.h"
#include "RiaQDateTimeTools.h"

#include "RimEnsembleCurveSetColorManager.h"
#include "RimObjectiveFunction.h"
#include "RimRegularLegendConfig.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimTimeStepFilter.h"

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmFieldCvfColor.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrArrayField.h"
#include "cafPdmPtrField.h"

#include <QPointer>

class RimSummaryCase;
class RimSummaryCaseCollection;
class RimSummaryCurve;
class RimSummaryAddress;
class RimSummaryFilter_OBSOLETE;
class RimSummaryPlotSourceStepping;
class RimSummaryCurveAutoName;
class RimEnsembleCurveFilterCollection;
class RimEnsembleStatistics;
class RimEnsembleStatisticsCase;
class RimCustomObjectiveFunctionCollection;
class RimObjectiveFunction;
class RiuDraggableOverlayFrame;
class RiaSummaryCurveDefinitionAnalyser;
class RiaSummaryCurveDefinition;

class QwtPlot;
class QwtPlotCurve;
class QKeyEvent;
class QFrame;
class QDate;

//==================================================================================================
///
//==================================================================================================
class RimEnsembleCurveSet : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    using ColorMode          = RimEnsembleCurveSetColorManager::ColorMode;
    using ColorModeEnum      = RimEnsembleCurveSetColorManager::ColorModeEnum;
    using TimeStepFilterEnum = caf::AppEnum<RimTimeStepFilter::TimeStepFilterTypeEnum>;

public:
    RimEnsembleCurveSet();
    ~RimEnsembleCurveSet() override;

    caf::Signal<> filterChanged;

    QString name() const;

    bool isCurvesVisible();
    void setColor( cvf::Color3f color );

    void loadDataAndUpdate( bool updateParentPlot );
    void setParentQwtPlotNoReplot( QwtPlot* plot );
    void detachQwtCurves();
    void reattachQwtCurves();

    void addCurve( RimSummaryCurve* curve );
    void deleteCurve( RimSummaryCurve* curve );

    void                          setSummaryAddress( RifEclipseSummaryAddress address );
    RifEclipseSummaryAddress      summaryAddress() const;
    std::vector<RimSummaryCurve*> curves() const;

    RimCustomObjectiveFunctionCollection* customObjectiveFunctionCollection();

    void deleteEnsembleCurves();
    void deleteStatisticsCurves();

    void onLegendDefinitionChanged();

    void                      setSummaryCaseCollection( RimSummaryCaseCollection* sumCaseCollection );
    RimSummaryCaseCollection* summaryCaseCollection() const;

    RimEnsembleCurveFilterCollection* filterCollection() const;

    ColorMode               colorMode() const;
    void                    setColorMode( ColorMode mode );
    void                    setEnsembleParameter( const QString& parameterName );
    EnsembleParameter::Type currentEnsembleParameterType() const;

    RimRegularLegendConfig* legendConfig();

    void                      updateEnsembleLegendItem();
    RiuDraggableOverlayFrame* legendFrame() const;

    void                updateAllCurves();
    void                setTimeSteps( const std::vector<size_t>& timeStepIndices );
    std::vector<time_t> selectedTimeSteps() const;
    void                updateStatisticsCurves();

    RimEnsembleCurveSet* clone() const;
    void                 showCurves( bool show );

    void                           updateAllTextInPlot();
    std::vector<EnsembleParameter> variationSortedEnsembleParameters() const;

    std::vector<std::pair<EnsembleParameter, double>> correlationSortedEnsembleParameters() const;

    std::vector<RimSummaryCase*> filterEnsembleCases( const std::vector<RimSummaryCase*>& sumCases );
    void                         disableStatisticCurves();
    bool                         isFiltered() const;

    bool hasP10Data() const;
    bool hasP50Data() const;
    bool hasP90Data() const;
    bool hasMeanData() const;

    void appendColorGroup( caf::PdmUiOrdering& uiOrdering );

    static void appendOptionItemsForSummaryAddresses( QList<caf::PdmOptionItemInfo>* options,
                                                      RimSummaryCaseCollection*      summaryCaseGroup );

    void updateFilterLegend();
    void updateObjectiveFunctionLegend();

    ObjectiveFunctionTimeConfig objectiveFunctionTimeConfig() const;

private:
    void updateEnsembleCurves( const std::vector<RimSummaryCase*>& sumCases );
    void updateStatisticsCurves( const std::vector<RimSummaryCase*>& sumCases );

    caf::PdmFieldHandle* userDescriptionField() override;
    caf::PdmFieldHandle* objectToggleField() override;
    void                 defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                QString                    uiConfigName,
                                                caf::PdmUiEditorAttribute* attribute ) override;

    QList<caf::PdmOptionItemInfo>          calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                  bool*                      useOptionsOnly ) override;
    std::set<time_t>                       allAvailableTimeSteps();
    std::set<RimSummaryCase*>              timestepDefiningSourceCases();
    RiaSummaryCurveDefinitionAnalyser*     getOrCreateSelectedCurveDefAnalyser();
    std::vector<RiaSummaryCurveDefinition> curveDefinitions() const;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    void updateQwtPlotAxis();

    QString createAutoName() const;

    void updateLegendMappingMode();
    void updateMaxMinAndDefaultValues();
    void updateCurveColors();
    void updateTimeAnnotations();
    void updateAddressesUiField();

    void onObjectiveFunctionChanged( const caf::SignalEmitter* emitter );
    void onCustomObjectiveFunctionChanged( const caf::SignalEmitter* emitter );

private:
    caf::PdmField<bool>                       m_showCurves;
    caf::PdmChildArrayField<RimSummaryCurve*> m_curves;

    caf::PdmPointer<RimSummaryCurve> m_currentSummaryCurve;

    caf::PdmPtrField<RimSummaryCaseCollection*>          m_yValuesSummaryCaseCollection;
    caf::PdmChildField<RimSummaryAddress*>               m_yValuesSummaryAddress;
    caf::PdmField<RifEclipseSummaryAddress>              m_yValuesSummaryAddressUiField;
    caf::PdmField<bool>                                  m_yPushButtonSelectSummaryAddress;
    caf::PdmField<RiaQDateTimeTools::DateTimePeriodEnum> m_resampling;

    caf::PdmField<ColorModeEnum> m_colorMode;
    caf::PdmField<cvf::Color3f>  m_color;
    caf::PdmField<QString>       m_ensembleParameter;

    caf::PdmChildArrayField<RimSummaryAddress*>   m_objectiveValuesSummaryAddresses;
    caf::PdmField<QString>                        m_objectiveValuesSummaryAddressesUiField;
    caf::PdmField<bool>                           m_objectiveValuesSelectSummaryAddressPushButton;
    caf::PdmPtrField<RimCustomObjectiveFunction*> m_customObjectiveFunction;
    caf::PdmField<time_t>                         m_minTimeStep;
    caf::PdmField<time_t>                         m_maxTimeStep;
    caf::PdmField<QDate>                          m_minDateRange;
    caf::PdmField<QDate>                          m_maxDateRange;
    caf::PdmField<TimeStepFilterEnum>             m_timeStepFilter;
    caf::PdmField<std::vector<QDateTime>>         m_selectedTimeSteps;

    caf::PdmField<caf::AppEnum<RiaDefines::PlotAxis>> m_plotAxis;

    caf::PdmChildField<RimRegularLegendConfig*>           m_legendConfig;
    caf::PdmChildField<RimEnsembleCurveFilterCollection*> m_curveFilters;
    caf::PdmChildField<RimEnsembleStatistics*>            m_statistics;

    caf::PdmChildField<RimCustomObjectiveFunctionCollection*> m_customObjectiveFunctions;
    caf::PdmField<bool>                                       m_showObjectiveFunctionFormula;
    caf::PdmChildField<RimObjectiveFunction*>                 m_objectiveFunction;

    caf::PdmField<bool>                          m_isUsingAutoName;
    caf::PdmField<QString>                       m_userDefinedName;
    caf::PdmProxyValueField<QString>             m_autoGeneratedName;
    caf::PdmChildField<RimSummaryCurveAutoName*> m_summaryAddressNameTools;

    QwtPlotCurve*                      m_qwtPlotCurveForLegendText;
    QPointer<RiuDraggableOverlayFrame> m_legendOverlayFrame;
    QPointer<RiuDraggableOverlayFrame> m_filterOverlayFrame;
    QPointer<RiuDraggableOverlayFrame> m_objectiveFunctionOverlayFrame;

    std::unique_ptr<RimEnsembleStatisticsCase> m_ensembleStatCase;

    std::unique_ptr<RiaSummaryCurveDefinitionAnalyser> m_analyserOfSelectedCurveDefs;

    bool m_disableStatisticCurves;
    bool m_isCurveSetFiltered;

    // Obsolete fields
    caf::PdmChildField<RimSummaryFilter_OBSOLETE*> m_yValuesSummaryFilter_OBSOLETE;
};
