/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RiaDateTimeDefines.h"

#include "RifEclipseSummaryAddress.h"
#include "RifEclipseSummaryAddressDefines.h"

#include "RimRegularLegendConfig.h"
#include "RimSummaryTableTools.h"

#include "cafPdmField.h"
#include "cafPdmPtrField.h"

#include <QDateTime>
#include <QPointer>
#include <QString>

#include <map>
#include <memory>
#include <string>
#include <vector>

class RifSummaryReaderInterface;
class RimSummaryCase;
class RimSummaryEnsemble;
class RimEnsembleStatisticsCase;
class RimRegularLegendConfig;
class RiuMatrixPlotWidget;
class RifEclipseSummaryAddress;

//==================================================================================================
///
//==================================================================================================
class RimSummaryTable : public RimPlotWindow
{
    CAF_PDM_HEADER_INIT;

public:
    enum class RangeType
    {
        AUTOMATIC,
        USER_DEFINED
    };

    enum class DataSource
    {
        SINGLE_CASE,
        ENSEMBLE_STATISTICS
    };

    enum class EnsembleStatistic
    {
        MIN,
        P90,
        P50,
        MEAN,
        P10,
        MAX
    };

public:
    RimSummaryTable();
    ~RimSummaryTable() override;

    void    setDefaultCaseAndCategoryAndVectorName();
    void    setFromCaseAndCategoryAndVectorName( RimSummaryCase*                                  summaryCase,
                                                 RifEclipseSummaryAddressDefines::SummaryCategory category,
                                                 const QString&                                   vectorName );
    void    setDescription( const QString& description );
    QString description() const override;

private:
    void cleanupBeforeClose();
    void initAfterRead() override;

    void onLoadDataAndUpdate() override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    // Inherited via RimPlotWindow
    void doRenderWindowContent( QPaintDevice* paintDevice ) override;

    // Inherited via RimViewWindow
    QWidget* viewWidget() override;

    void     zoomAll() override;
    QWidget* createViewWidget( QWidget* mainWindowParent ) override;
    void     deleteViewWidget() override;

    // PDM methods
    caf::PdmFieldHandle* userDescriptionField() override;

    int axisTitleFontSize() const;
    int axisLabelFontSize() const;
    int valueLabelFontSize() const;

    QString createTableName() const;

    std::pair<double, double> createLegendMinMaxValues( const double maxTableValue ) const;
    QString                   dateFormatString() const;

    std::set<RifEclipseSummaryAddress> getSummaryAddressesFromAddresses( const std::set<RifEclipseSummaryAddress>&        allAddresses,
                                                                         RifEclipseSummaryAddressDefines::SummaryCategory category,
                                                                         const QString&                                   vector ) const;
    std::set<QString>                  getCategoryVectorFromAddresses( const std::set<RifEclipseSummaryAddress>&        allAddresses,
                                                                       RifEclipseSummaryAddressDefines::SummaryCategory category ) const;
    QString                            getCategoryNameFromAddress( const RifEclipseSummaryAddress& address ) const;

    std::vector<RimSummaryCase*> getToplevelSummaryCases() const;
    void                         initializeDateRange();

    RifSummaryReaderInterface*         summaryReaderWithAddresses();
    std::set<RifEclipseSummaryAddress> availableAddresses();
    bool                               hasDataSource() const;
    bool                               isEnsembleStatistics() const;
    void                               resetVectorToFirstAvailable();
    QString                            dataSourceDescription() const;

private:
    // Matrix plot for visualizing table data
    QPointer<RiuMatrixPlotWidget> m_matrixPlotWidget;

    caf::PdmField<QString>                         m_tableName;
    caf::PdmField<bool>                            m_isAutomaticName;
    caf::PdmField<caf::AppEnum<DataSource>>        m_dataSource;
    caf::PdmPtrField<RimSummaryCase*>              m_case;
    caf::PdmPtrField<RimSummaryEnsemble*>          m_ensemble;
    caf::PdmField<caf::AppEnum<EnsembleStatistic>> m_ensembleStatistic;

    caf::PdmField<caf::AppEnum<RifEclipseSummaryAddressDefines::SummaryCategory>> m_category;
    caf::PdmField<QString>                                                        m_vector;
    caf::PdmField<caf::AppEnum<RiaDefines::DateTimePeriod>>                       m_resamplingSelection;
    caf::PdmField<std::pair<bool, double>>                                        m_upperThreshold;
    caf::PdmField<std::pair<bool, double>>                                        m_lowerThreshold;
    caf::PdmField<double>                                                         m_thresholdValue_OBSOLETE;

    caf::PdmField<std::vector<QString>> m_excludedRowsUiField;

    caf::PdmChildField<RimRegularLegendConfig*> m_legendConfig;

    RimFontSizeField    m_axisTitleFontSize;
    RimFontSizeField    m_axisLabelFontSize;
    RimFontSizeField    m_valueLabelFontSize;
    caf::PdmField<bool> m_showValueLabels;
    caf::PdmField<int>  m_maxTimeLabelCount;

    caf::PdmField<RimRegularLegendConfig::MappingEnum> m_mappingType;
    caf::PdmField<caf::AppEnum<RangeType>>             m_rangeType;

    caf::PdmField<bool>      m_filterTimeSteps;
    caf::PdmField<QDateTime> m_startDate;
    caf::PdmField<QDateTime> m_endDate;

private:
    using VectorData = RimSummaryTableTools::VectorData;
    using TableData  = RimSummaryTableTools::TableData;

    TableData m_tableData;

    std::unique_ptr<RimEnsembleStatisticsCase> m_ensembleStatisticsCase;

    struct EnsembleStatisticsData
    {
        std::vector<time_t>                              timeSteps;
        std::map<EnsembleStatistic, std::vector<double>> values;
        std::string                                      unitName;
    };

    std::map<RifEclipseSummaryAddress, EnsembleStatisticsData> m_ensembleStatisticsCache;
    const RimSummaryEnsemble*                                  m_ensembleStatisticsCacheEnsemble  = nullptr;
    size_t                                                     m_ensembleStatisticsCacheCaseCount = 0;

    const EnsembleStatisticsData& ensembleStatisticsData( const RifEclipseSummaryAddress& address );
    void                          clearEnsembleStatisticsCache();

    void createTableData();
    void setExcludedRowsUiSelectionsFromTableData();
};
