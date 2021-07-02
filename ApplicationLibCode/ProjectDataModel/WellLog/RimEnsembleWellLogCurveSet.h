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
#include "RimEnsembleWellLogStatistics.h"

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
class RimWellLogFileCurve;
class RimWellLogFile;

class RiuDraggableOverlayFrame;

class QwtPlot;
class QwtPlotCurve;

//==================================================================================================
///
//==================================================================================================
class RimEnsembleWellLogCurveSet : public caf::PdmObject, public RimEnsembleCurveSetInterface
{
    CAF_PDM_HEADER_INIT;

public:
    enum class ColorMode
    {
        SINGLE_COLOR,
        COLOR_BY_ENSEMBLE_CURVE_SET
    };

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

    std::vector<RimWellLogCurve*> curves() const;

    void deleteEnsembleCurves();
    void deleteStatisticsCurves();

    ColorMode colorMode() const;
    void      setColorMode( ColorMode mode );

    void                      updateEnsembleLegendItem();
    RiuDraggableOverlayFrame* legendFrame() const;

    void showCurves( bool show );

    void updateAllTextInPlot();

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

    const RimEnsembleWellLogStatistics*             ensembleWellLogStatistics() const;
    RimEnsembleWellLogStatistics::DepthEqualization depthEqualization() const;

    void updateStatistics();

private:
    void updateEnsembleCurves( const std::vector<RimWellLogFile*>& curves );
    void updateStatisticsCurves( const std::vector<RimWellLogFile*>& curves );
    void updateStatistics( const std::vector<RimWellLogFile*>& sumCases );

    caf::PdmFieldHandle* userDescriptionField() override;
    caf::PdmFieldHandle* objectToggleField() override;
    void                 initAfterRead() override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    QString createAutoName() const;

    void updateCurveColors();

    bool            isSameRealization( RimSummaryCase* summaryCase, RimWellLogFile* wellLogFile ) const;
    RimSummaryCase* findMatchingSummaryCase( RimWellLogFileCurve* wellLogCurve ) const;

    void connectEnsembleCurveSetFilterSignals();
    void onFilterSourceChanged( const caf::SignalEmitter* emitter );

private:
    caf::PdmField<bool> m_showCurves;

    caf::PdmPtrArrayField<RimWellLogCurve*>                                      m_curves;
    caf::PdmPointer<RimWellLogCurve>                                             m_currentWellLogCurve;
    caf::PdmField<caf::AppEnum<RimEnsembleWellLogStatistics::DepthEqualization>> m_depthEqualization;

    caf::PdmField<caf::AppEnum<ColorMode>> m_colorMode;
    caf::PdmField<cvf::Color3f>            m_color;

    caf::PdmChildField<RimEnsembleStatistics*> m_statistics;

    caf::PdmField<bool>              m_isUsingAutoName;
    caf::PdmField<QString>           m_userDefinedName;
    caf::PdmProxyValueField<QString> m_autoGeneratedName;

    caf::PdmPtrField<RimEnsembleWellLogs*> m_ensembleWellLogs;
    caf::PdmField<QString>                 m_wellLogChannelName;
    caf::PdmPtrField<RimEnsembleCurveSet*> m_ensembleCurveSet;

    QwtPlotCurve*                      m_qwtPlotCurveForLegendText;
    QPointer<RiuDraggableOverlayFrame> m_legendOverlayFrame;
    QPointer<RiuDraggableOverlayFrame> m_filterOverlayFrame;

    std::unique_ptr<RimEnsembleWellLogStatistics> m_ensembleWellLogStatistics;

    bool m_disableStatisticCurves;
    bool m_isCurveSetFiltered;
};
