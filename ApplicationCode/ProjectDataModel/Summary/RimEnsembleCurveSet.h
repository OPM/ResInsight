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
#include "RifSummaryReaderInterface.h"

#include "RiaDefines.h"

#include "RimRegularLegendConfig.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmFieldCvfColor.h"
#include "cafPdmObject.h"
#include "cafPdmPtrArrayField.h"
#include "cafPdmPtrField.h"

#include "RifEclipseSummaryAddressQMetaType.h"
#include "RimEnsembleParameterColorHandlerInterface.h"
#include "cafPdmProxyValueField.h"

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
class RiuCvfOverlayItemWidget;

class QwtPlot;
class QwtPlotCurve;
class QKeyEvent;

//==================================================================================================
///
//==================================================================================================
class RimEnsembleCurveSet : public caf::PdmObject, public RimEnsembleParameterColorHandlerInterface
{
    CAF_PDM_HEADER_INIT;

public:
    RimEnsembleCurveSet();
    ~RimEnsembleCurveSet() override;

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

    void deleteEnsembleCurves();
    void deleteStatisticsCurves();

    void onLegendDefinitionChanged();

    void                      setSummaryCaseCollection( RimSummaryCaseCollection* sumCaseCollection );
    RimSummaryCaseCollection* summaryCaseCollection() const;

    RimEnsembleCurveFilterCollection* filterCollection() const;

    ColorMode               colorMode() const override;
    void                    setColorMode( ColorMode mode ) override;
    void                    setEnsembleParameter( const QString& parameterName ) override;
    void                    updateEnsembleLegendItem() override;
    RimRegularLegendConfig* legendConfig() override;
    QFrame*                 legendFrame() const override;

    EnsembleParameter::Type currentEnsembleParameterType() const;

    void updateAllCurves();
    void updateStatisticsCurves();

    RimEnsembleCurveSet* clone() const;
    void                 showCurves( bool show );

    void markCachedDataForPurge();

    void                           updateAllTextInPlot();
    std::vector<NameParameterPair> ensembleParameters() const;

    std::vector<RimSummaryCase*> filterEnsembleCases( const std::vector<RimSummaryCase*>& sumCases );
    void                         disableStatisticCurves();
    bool                         isFiltered() const;

    bool hasP10Data() const;
    bool hasP50Data() const;
    bool hasP90Data() const;
    bool hasMeanData() const;

    void appendColorGroup( caf::PdmUiOrdering& uiOrdering );

private:
    void updateEnsembleCurves( const std::vector<RimSummaryCase*>& sumCases );
    void updateStatisticsCurves( const std::vector<RimSummaryCase*>& sumCases );

    caf::PdmFieldHandle* userDescriptionField() override;
    caf::PdmFieldHandle* objectToggleField() override;
    void                 defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                QString                    uiConfigName,
                                                caf::PdmUiEditorAttribute* attribute ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                           const QVariant&            oldValue,
                           const QVariant&            newValue ) override;

    void appendOptionItemsForSummaryAddresses( QList<caf::PdmOptionItemInfo>* options,
                                               RimSummaryCaseCollection*      summaryCaseGroup );

    void updateCurveColors();
    void updateQwtPlotAxis();

    QString name() const;
    QString createAutoName() const;

    void updateLegendMappingMode();

private:
    caf::PdmField<bool>                       m_showCurves;
    caf::PdmChildArrayField<RimSummaryCurve*> m_curves;

    caf::PdmPointer<RimSummaryCurve> m_currentSummaryCurve;

    caf::PdmPtrField<RimSummaryCaseCollection*> m_yValuesSummaryCaseCollection;
    caf::PdmChildField<RimSummaryAddress*>      m_yValuesSummaryAddress;
    caf::PdmField<RifEclipseSummaryAddress>     m_yValuesSummaryAddressUiField;
    caf::PdmField<bool>                         m_yPushButtonSelectSummaryAddress;

    caf::PdmField<ColorModeEnum> m_colorMode;
    caf::PdmField<cvf::Color3f>  m_color;
    caf::PdmField<QString>       m_ensembleParameter;

    caf::PdmField<caf::AppEnum<RiaDefines::PlotAxis>> m_plotAxis;

    caf::PdmChildField<RimRegularLegendConfig*>           m_legendConfig;
    caf::PdmChildField<RimEnsembleCurveFilterCollection*> m_curveFilters;
    caf::PdmChildField<RimEnsembleStatistics*>            m_statistics;

    caf::PdmField<bool>                          m_isUsingAutoName;
    caf::PdmField<QString>                       m_userDefinedName;
    caf::PdmProxyValueField<QString>             m_autoGeneratedName;
    caf::PdmChildField<RimSummaryCurveAutoName*> m_summaryAddressNameTools;

    QwtPlotCurve*                     m_qwtPlotCurveForLegendText;
    QPointer<RiuCvfOverlayItemWidget> m_legendOverlayFrame;

    std::unique_ptr<RimEnsembleStatisticsCase> m_ensembleStatCase;

    bool m_disableStatisticCurves;
    bool m_isCurveSetFiltered;

    // Obsolete fields
    caf::PdmChildField<RimSummaryFilter_OBSOLETE*> m_yValuesSummaryFilter_OBSOLETE;
};
