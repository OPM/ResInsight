/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021- Equinor ASA
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

#include "RiaDefines.h"

#include "RimEnsembleCurveSetColorManager.h"
#include "RimEnsembleCurveSetInterface.h"
#include "RimRegularLegendConfig.h"

#include "RigEnsembleParameter.h"

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

class RimEnsembleWellLogs;
class RimEnsembleWellLogsCollection;
class RimEnsembleCurveFilterCollection;
class RimEnsembleStatistics;
class RimEnsembleStatisticsCase;
class RimWellLogCurve;
class RimWellLogFile;
class RimEnsembleWellLogStatistics;

class RiuDraggableOverlayFrame;

class QwtPlot;
class QwtPlotCurve;
class QKeyEvent;
class QFrame;
class QDate;

//==================================================================================================
///
//==================================================================================================
class RimEnsembleWellLogCurveSet : public caf::PdmObject, public RimEnsembleCurveSetInterface
{
    CAF_PDM_HEADER_INIT;

public:
    using ColorMode     = RimEnsembleCurveSetColorManager::ColorMode;
    using ColorModeEnum = RimEnsembleCurveSetColorManager::ColorModeEnum;

public:
    RimEnsembleWellLogCurveSet();
    ~RimEnsembleWellLogCurveSet() override;

    caf::Signal<> filterChanged;

    QString name() const;

    bool isCurvesVisible();
    void setColor( cvf::Color3f color );

    void loadDataAndUpdate( bool updateParentPlot );
    void setParentQwtPlotNoReplot( QwtPlot* plot );
    void detachQwtCurves();
    void reattachQwtCurves();

    void addCurve( RimWellLogCurve* curve );
    void deleteCurve( RimWellLogCurve* curve );

    std::vector<RimWellLogCurve*> curves() const;

    void deleteEnsembleCurves();
    void deleteStatisticsCurves();

    void onLegendDefinitionChanged();

    void                      setSummaryCaseCollection( RimSummaryCaseCollection* sumCaseCollection );
    RimSummaryCaseCollection* summaryCaseCollection() const;

    RimEnsembleCurveFilterCollection* filterCollection() const;

    ColorMode                  colorMode() const;
    void                       setColorMode( ColorMode mode );
    void                       setEnsembleParameter( const QString& parameterName );
    RigEnsembleParameter::Type currentEnsembleParameterType() const;

    RimRegularLegendConfig* legendConfig();

    void                      updateEnsembleLegendItem();
    RiuDraggableOverlayFrame* legendFrame() const;

    RimEnsembleWellLogCurveSet* clone() const;
    void                        showCurves( bool show );

    void                              updateAllTextInPlot();
    std::vector<RigEnsembleParameter> variationSortedEnsembleParameters() const;

    std::vector<std::pair<RigEnsembleParameter, double>> correlationSortedEnsembleParameters() const;

    std::vector<RimWellLogFile*> filterEnsembleCases( const std::vector<RimWellLogFile*>& sumCases );
    void                         disableStatisticCurves();
    bool                         isFiltered() const;

    void updateEditors() override;
    void updateAllCurves() override;
    void updateStatisticsCurves() override;
    bool hasP10Data() const override;
    bool hasP50Data() const override;
    bool hasP90Data() const override;
    bool hasMeanData() const override;

    void appendColorGroup( caf::PdmUiOrdering& uiOrdering );

    void updateFilterLegend();

    const RimEnsembleWellLogStatistics* ensembleWellLogStatistics() const;

private:
    void updateEnsembleCurves( const std::vector<RimWellLogFile*>& curves );
    void updateStatisticsCurves( const std::vector<RimWellLogFile*>& curves );

    caf::PdmFieldHandle* userDescriptionField() override;
    caf::PdmFieldHandle* objectToggleField() override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    QString createAutoName() const;

    void updateLegendMappingMode();
    void updateMaxMinAndDefaultValues();
    void updateCurveColors();

private:
    caf::PdmField<bool> m_showCurves;

    caf::PdmPtrArrayField<RimWellLogCurve*> m_curves;
    caf::PdmPointer<RimWellLogCurve>        m_currentWellLogCurve;

    caf::PdmField<ColorModeEnum> m_colorMode;
    caf::PdmField<cvf::Color3f>  m_color;
    caf::PdmField<QString>       m_ensembleParameter;

    caf::PdmChildField<RimRegularLegendConfig*>           m_legendConfig;
    caf::PdmChildField<RimEnsembleCurveFilterCollection*> m_curveFilters;
    caf::PdmChildField<RimEnsembleStatistics*>            m_statistics;

    caf::PdmField<bool>              m_isUsingAutoName;
    caf::PdmField<QString>           m_userDefinedName;
    caf::PdmProxyValueField<QString> m_autoGeneratedName;

    caf::PdmPtrField<RimEnsembleWellLogs*> m_ensembleWellLogs;
    caf::PdmField<QString>                 m_wellLogChannelName;

    QwtPlotCurve*                      m_qwtPlotCurveForLegendText;
    QPointer<RiuDraggableOverlayFrame> m_legendOverlayFrame;
    QPointer<RiuDraggableOverlayFrame> m_filterOverlayFrame;

    std::unique_ptr<RimEnsembleWellLogStatistics> m_ensembleWellLogStatistics;

    bool m_disableStatisticCurves;
    bool m_isCurveSetFiltered;
};
