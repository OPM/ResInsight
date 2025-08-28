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

#include "RiaDateTimeDefines.h"
#include "RiaPlotDefines.h"
#include "Summary/RiaSummaryCurveAddress.h"
#include "Summary/RiaSummaryDefines.h"

#include "Appearance/RimCurveAppearanceDefines.h"
#include "RimEnsembleCrossPlotStatisticsCase.h"
#include "RimEnsembleCurveSetColorManager.h"
#include "RimEnsembleCurveSetInterface.h"
#include "RimTimeStepFilter.h"

#include "RigEnsembleParameter.h"

#include "RiuPlotAxis.h"
#include "RiuPlotCurveSymbol.h"
#include "RiuQwtPlotCurveDefines.h"

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

#include <QDate>
#include <QPointer>

class RimSummaryCase;
class RimSummaryEnsemble;
class RimSummaryCurve;
class RimSummaryAddress;
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
class RiuSummaryVectorSelectionDialog;
class RiuPlotWidget;
class RiuPlotCurve;
class RimPlotAxisPropertiesInterface;
class RimSummaryAddressSelector;
class RimSummaryPlot;
class RimTimeAxisAnnotation;

class QwtPlot;
class QwtPlotCurve;
class QKeyEvent;
class QFrame;

//==================================================================================================
///
//==================================================================================================
class RimEnsembleCurveSet : public caf::PdmObject, public RimEnsembleCurveSetInterface
{
    CAF_PDM_HEADER_INIT;

public:
    using ColorMode          = RimEnsembleCurveSetColorManager::ColorMode;
    using ColorModeEnum      = RimEnsembleCurveSetColorManager::ColorModeEnum;
    using TimeStepFilterEnum = caf::AppEnum<RimTimeStepFilter::TimeStepFilterTypeEnum>;
    using LineStyle          = caf::AppEnum<RiuQwtPlotCurveDefines::LineStyleEnum>;
    using PointSymbol        = caf::AppEnum<RiuPlotCurveSymbol::PointSymbolEnum>;

public:
    RimEnsembleCurveSet();
    ~RimEnsembleCurveSet() override;

    caf::Signal<> filterChanged;

    QString name() const;

    bool   isCurvesVisible() const;
    void   setColor( cvf::Color3f color );
    void   setStatisticsColor( const cvf::Color3f& color );
    void   enableStatisticsLables( bool enable );
    QColor mainEnsembleColor() const;

    void loadDataAndUpdate( bool updateParentPlot );
    void setParentPlotNoReplot( RiuPlotWidget* plot );
    void deletePlotCurves();
    void reattachPlotCurves();

    void addRealizationCurve( RimSummaryCurve* curve );
    void deleteRealizationCurve( RimSummaryCurve* curve );

    void                          setSummaryAddressY( RifEclipseSummaryAddress address );
    void                          setCurveAddress( RiaSummaryCurveAddress address );
    void                          setSummaryAddressYAndStatisticsFlag( RifEclipseSummaryAddress address );
    RifEclipseSummaryAddress      summaryAddressY() const;
    RiaSummaryCurveAddress        curveAddress() const;
    std::vector<RimSummaryCurve*> curves() const;

    int ensembleId() const;

    RimCustomObjectiveFunctionCollection* customObjectiveFunctionCollection();

    void deleteEnsembleCurves();
    void deleteStatisticsCurves();

    void onLegendDefinitionChanged();

    void                setSummaryEnsemble( RimSummaryEnsemble* sumEnsemble );
    RimSummaryEnsemble* summaryEnsemble() const;

    RimEnsembleCurveFilterCollection* filterCollection() const;

    ColorMode                  colorMode() const;
    void                       setColorMode( ColorMode mode );
    void                       setEnsembleParameter( const QString& parameterName );
    RigEnsembleParameter::Type currentEnsembleParameterType() const;

    RimRegularLegendConfig* legendConfig();

    void                      updateEnsembleLegendItem();
    RiuDraggableOverlayFrame* legendFrame() const;
    void                      setTimeSteps( const std::vector<size_t>& timeStepIndices );
    std::vector<time_t>       selectedTimeSteps() const;
    void                      setDefaultTimeRange();

    RimEnsembleCurveSet* clone() const;
    void                 showCurves( bool show );

    void                              updateAllTextInPlot();
    std::vector<RigEnsembleParameter> variationSortedEnsembleParameters() const;

    std::vector<std::pair<RigEnsembleParameter, double>> ensembleParameters( RimCurveAppearanceDefines::ParameterSorting sortingMode ) const;

    std::vector<RimSummaryCase*> filterEnsembleCases( const std::vector<RimSummaryCase*>& sumCases );
    void                         disableStatisticCurves();
    bool                         isFiltered() const;

    void updateEditors() override;
    void updateAllCurves() override;
    void updateStatisticsCurves() override;
    bool hasP10Data() const override;
    bool hasP50Data() const override;
    bool hasP90Data() const override;
    bool hasMeanData() const override;

    const RimEnsembleStatistics* statisticsOptions() const;

    void appendColorGroup( caf::PdmUiOrdering& uiOrdering );
    void appendOptionItemsForSummaryAddresses( QList<caf::PdmOptionItemInfo>* options, RimSummaryEnsemble* summaryCaseGroup );
    void appendTimeGroup( caf::PdmUiOrdering& uiOrdering );

    const RimEnsembleCurveFilterCollection* curveFilters() const;

    void updateFilterLegend();
    void updateObjectiveFunctionLegend();
    void onFilterChanged();

    ObjectiveFunctionTimeConfig objectiveFunctionTimeConfig() const;

    std::vector<cvf::Color3f> generateColorsForCases( const std::vector<RimSummaryCase*>& summaryCases ) const;

    RiuPlotAxis                    axisY() const;
    RiuPlotAxis                    axisX() const;
    void                           setLeftOrRightAxisY( RiuPlotAxis plotAxis );
    void                           setBottomOrTopAxisX( RiuPlotAxis plotAxis );
    bool                           isXAxisSummaryVector() const;
    RiaDefines::HorizontalAxisType xAxisType() const;
    void                           findOrAssignBottomAxisX( RiuPlotAxis plotAxis );

    std::vector<RimTimeAxisAnnotation*> createTimeAnnotations() const;

protected:
    void initAfterRead() override;

private:
    void updateEnsembleCurves( const std::vector<RimSummaryCase*>& sumCases );
    void updateStatisticsCurves( const std::vector<RimSummaryCase*>& sumCases );

    caf::PdmFieldHandle* userDescriptionField() override;
    caf::PdmFieldHandle* objectToggleField() override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    QList<caf::PdmOptionItemInfo>          calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    std::set<time_t>                       allAvailableTimeSteps() const;
    std::set<RimSummaryCase*>              timestepDefiningSourceCases();
    RiaSummaryCurveDefinitionAnalyser*     getOrCreateSelectedCurveDefAnalyser();
    std::vector<RiaSummaryCurveDefinition> curveDefinitions() const;
    void                                   defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    std::vector<RimSummaryCurve*> realizationCurves() const;

    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField ) override;

    void updatePlotAxis();

    QString createAutoName() const;

    void updateLegendMappingMode();
    void updateLegendTitle();

    void updateMaxMinAndDefaultValues();
    void updateCurveColors();
    void updateTimeAnnotations();
    void updateAddressesUiField();

    void onObjectiveFunctionChanged( const caf::SignalEmitter* emitter );
    void onCustomObjectiveFunctionChanged( const caf::SignalEmitter* emitter );

    void computeRealizationColor();
    void onColorTagClicked( const SignalEmitter* emitter, size_t index );

    void setSummaryAddressX( RifEclipseSummaryAddress address );

    std::pair<time_t, time_t> fullTimeStepRange() const;
    std::pair<time_t, time_t> selectedTimeStepRange() const;

    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;

    void createCurves( const std::vector<RimSummaryCase*>& sumCases, const RimSummaryAddress& addr );
    void recreatePlotCurveForLegend( RimSummaryPlot* plot );

private:
    caf::PdmField<bool>                       m_showCurves;
    caf::PdmChildArrayField<RimSummaryCurve*> m_realizationCurves;
    caf::PdmChildArrayField<RimSummaryCurve*> m_statisticsCurves;

    caf::PdmPointer<RimSummaryCurve> m_currentSummaryCurve;

    caf::PdmPtrField<RimSummaryEnsemble*>         m_yValuesSummaryEnsemble;
    caf::PdmChildField<RimSummaryAddress*>        m_yValuesSummaryAddress;
    caf::PdmField<RifEclipseSummaryAddress>       m_yValuesSummaryAddressUiField;
    caf::PdmField<bool>                           m_yPushButtonSelectSummaryAddress;
    caf::PdmField<RiaDefines::DateTimePeriodEnum> m_resampling;

    caf::PdmField<caf::AppEnum<RiaDefines::HorizontalAxisType>> m_xAxisType;
    caf::PdmChildField<RimSummaryAddressSelector*>              m_xAddressSelector;

    caf::PdmField<ColorModeEnum>                                             m_colorMode;
    caf::PdmField<cvf::Color3f>                                              m_mainEnsembleColor;
    cvf::Color3f                                                             m_colorForRealizations;
    caf::PdmField<double>                                                    m_colorOpacity;
    caf::PdmField<QString>                                                   m_ensembleParameter;
    caf::PdmField<caf::AppEnum<RimCurveAppearanceDefines::ParameterSorting>> m_ensembleParameterSorting;

    caf::PdmField<caf::AppEnum<RimCurveAppearanceDefines::AppearanceMode>> m_useCustomAppearance;
    caf::PdmField<LineStyle>                                               m_lineStyle;
    caf::PdmField<PointSymbol>                                             m_pointSymbol;
    caf::PdmField<int>                                                     m_symbolSize;

    caf::PdmField<caf::AppEnum<RimCurveAppearanceDefines::AppearanceMode>> m_statisticsUseCustomAppearance;
    caf::PdmField<LineStyle>                                               m_statisticsLineStyle;
    caf::PdmField<PointSymbol>                                             m_statisticsPointSymbol;
    caf::PdmField<int>                                                     m_statisticsSymbolSize;

    caf::PdmChildArrayField<RimSummaryAddress*>   m_objectiveValuesSummaryAddresses;
    caf::PdmField<QString>                        m_objectiveValuesSummaryAddressesUiField;
    caf::PdmField<bool>                           m_objectiveValuesSelectSummaryAddressPushButton;
    caf::PdmPtrField<RimCustomObjectiveFunction*> m_customObjectiveFunction;
    caf::PdmField<int>                            m_minTimeSliderPosition;
    caf::PdmField<int>                            m_maxTimeSliderPosition;
    caf::PdmField<QDate>                          m_minDateRange;
    caf::PdmField<QDate>                          m_maxDateRange;
    caf::PdmField<TimeStepFilterEnum>             m_timeStepFilter;
    caf::PdmField<std::vector<QDateTime>>         m_selectedTimeSteps;

    caf::PdmField<caf::AppEnum<RiaDefines::PlotAxis>> m_plotAxis_OBSOLETE;
    caf::PdmPtrField<RimPlotAxisPropertiesInterface*> m_yPlotAxisProperties;

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

    std::unique_ptr<RiuPlotCurve>      m_plotCurveForLegendText;
    QPointer<RiuDraggableOverlayFrame> m_legendOverlayFrame;
    QPointer<RiuDraggableOverlayFrame> m_filterOverlayFrame;
    QPointer<RiuDraggableOverlayFrame> m_objectiveFunctionOverlayFrame;

    std::unique_ptr<RimEnsembleStatisticsCase>          m_ensembleStatCaseY;
    std::unique_ptr<RimEnsembleCrossPlotStatisticsCase> m_ensembleStatCaseXY;

    std::unique_ptr<RiaSummaryCurveDefinitionAnalyser> m_analyserOfSelectedCurveDefs;

    bool m_disableStatisticCurves;
    bool m_isCurveSetFiltered;

    QList<caf::PdmOptionItemInfo> m_cachedAddressOptions;
    size_t                        m_hash;
    size_t                        m_realizationHash;

    caf::PdmField<cvf::Color3f> m_colorForRealizations_OBSOLETE;
};
