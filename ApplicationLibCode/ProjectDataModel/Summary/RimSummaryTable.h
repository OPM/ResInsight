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

#include "cafPdmField.h"
#include "cafPdmPtrField.h"

#include <QDateTime>
#include <QPointer>
#include <QString>

class RifSummaryReaderInterface;
class RimSummaryCase;
class RimRegularLegendConfig;
class RiuMatrixPlotWidget;

//==================================================================================================
///
//==================================================================================================
class RimSummaryTable : public RimPlotWindow
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryTable();
    ~RimSummaryTable() override;

    void    setDefaultCaseAndCategoryAndVectorName();
    void    setFromCaseAndCategoryAndVectorName( RimSummaryCase*                              summaryCase,
                                                 RifEclipseSummaryAddress::SummaryVarCategory category,
                                                 const QString&                               vectorName );
    void    setDescription( const QString& description );
    QString description() const override;

private:
    void cleanupBeforeClose();

    void onLoadDataAndUpdate() override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    // Inherited via RimPlotWindow
    void doRenderWindowContent( QPaintDevice* paintDevice ) override;

    // Inherited via RimViewWindow
    QWidget* viewWidget() override;
    QImage   snapshotWindowContent() override;
    void     zoomAll() override;
    QWidget* createViewWidget( QWidget* mainWindowParent ) override;
    void     deleteViewWidget() override;

    // PDM methods
    caf::PdmFieldHandle* userDescriptionField() override;

    int axisTitleFontSize() const;
    int axisLabelFontSize() const;
    int valueLabelFontSize() const;

    QString createTableName() const;

private:
    std::pair<double, double> createLegendMinMaxValues( const double maxTableValue ) const;
    QString                   dateFormatString() const;

    std::set<RifEclipseSummaryAddress> getSummaryAddressesFromReader( const RifSummaryReaderInterface*             summaryReader,
                                                                      RifEclipseSummaryAddress::SummaryVarCategory category,
                                                                      const QString&                               vector ) const;
    std::set<QString>                  getCategoryVectorsFromSummaryReader( const RifSummaryReaderInterface*             summaryReader,
                                                                            RifEclipseSummaryAddress::SummaryVarCategory category ) const;
    QString                            getCategoryNameFromAddress( const RifEclipseSummaryAddress& address ) const;

    std::vector<RimSummaryCase*> getToplevelSummaryCases() const;

private:
    // Matrix plot for visualizing table data
    QPointer<RiuMatrixPlotWidget> m_matrixPlotWidget;

    caf::PdmField<QString>            m_tableName;
    caf::PdmField<bool>               m_isAutomaticName;
    caf::PdmPtrField<RimSummaryCase*> m_case;

    caf::PdmField<caf::AppEnum<RifEclipseSummaryAddress::SummaryVarCategory>> m_categories;
    caf::PdmField<QString>                                                    m_vector;
    caf::PdmField<caf::AppEnum<RiaDefines::DateTimePeriod>>                   m_resamplingSelection;
    caf::PdmField<double>                                                     m_thresholdValue;

    caf::PdmChildField<RimRegularLegendConfig*> m_legendConfig;

    caf::PdmField<caf::FontTools::RelativeSizeEnum> m_axisTitleFontSize;
    caf::PdmField<caf::FontTools::RelativeSizeEnum> m_axisLabelFontSize;
    caf::PdmField<caf::FontTools::RelativeSizeEnum> m_valueLabelFontSize;
    caf::PdmField<bool>                             m_showValueLabels;

    const int m_initialNumberOfTimeSteps = 10;
};
