/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "Appearance/RimFontSizeField.h"
#include "RiaDefines.h"
#include "RimPlot.h"
#include "RimRftCrossPlotTools.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmPtrField.h"

#include <QColor>
#include <QDateTime>
#include <QPointer>

#include <map>
#include <optional>
#include <utility>
#include <vector>

class RimEclipseResultCase;
class RimSummaryEnsemble;
class RimSummaryCase;
class RimWellFormationsFile;
class RiuQwtPlotWidget;

//==================================================================================================
///
/// Cross plot of ensemble parameter (X) vs mean RFT pressure (Y) within a MD depth range.
///
//==================================================================================================
class RimParameterRftCrossPlot : public RimPlot
{
    Q_OBJECT;
    CAF_PDM_HEADER_INIT;

public:
    // Determines how the RFT pressure samples within the selected depth range are reduced to
    // (ensemble parameter, pressure) points shown in the cross plot.
    enum class SamplingMode
    {
        ALL_SAMPLES, // One point per individual RFT sample within the depth range.
        MEAN_PER_REALIZATION // One point per realization, using the mean pressure within the depth range.
    };
    using SamplingModeEnum = caf::AppEnum<SamplingMode>;

    struct CaseData
    {
        double          parameterValue;
        double          pressureValue; // pressure sample, or mean pressure in depth range
        RimSummaryCase* summaryCase;
    };

public:
    RimParameterRftCrossPlot();
    ~RimParameterRftCrossPlot() override;

    void setEnsemble( RimSummaryEnsemble* ensemble );
    void setWellName( const QString& wellName );
    void setTimeStep( const QDateTime& timeStep );
    void setDepthRange( double minMd, double maxMd );
    void setDepthType( RiaDefines::DepthType depthType );
    void setEnsembleParameter( const QString& paramName );
    void setWellFormations( RimWellFormationsFile* wellFormationsFile );
    void setFilterMode( RimRftCrossPlotTools::DepthFilterMode filterMode );
    void setSelectedZones( const std::vector<QString>& zones );
    void setZoneColors( const std::map<QString, QColor>& zoneColors );
    void appendDepthFilterUiOrdering( caf::PdmUiOrdering& uiOrdering );

    QString                               ensembleParameter() const;
    QString                               wellName() const;
    QDateTime                             selectedTimeStep() const;
    RimSummaryEnsemble*                   ensemble() const;
    RimEclipseResultCase*                 eclipseCase() const;
    double                                depthRangeMin() const;
    double                                depthRangeMax() const;
    RiaDefines::DepthType                 depthType() const;
    RimRftCrossPlotTools::DepthFilterMode filterMode() const;
    std::vector<QString>                  selectedZones() const;
    RimWellFormationsFile*                wellFormationsFile() const;

    RiuQwtPlotWidget* viewer();

    std::vector<CaseData> createCaseData() const;

    // RimPlot pure virtual overrides
    RiuPlotWidget* plotWidget() override;
    void           setAutoScaleXEnabled( bool ) override {}
    void           setAutoScaleYEnabled( bool ) override {}
    void           updateAxes() override;
    void           updateLegend() override {}
    QString        asciiDataForPlotExport() const override;
    void           reattachAllCurves() override {}
    void           detachAllCurves() override;
    QString        description() const override;

private:
    // RimViewWindow / RimPlotWindow overrides
    QWidget*       viewWidget() override;
    void           zoomAll() override {}
    void           deleteViewWidget() override;
    void           doUpdateLayout() override {}
    void           doRenderWindowContent( QPaintDevice* paintDevice ) override;
    RiuPlotWidget* doCreatePlotViewWidget( QWidget* parent = nullptr ) override;

    void onLoadDataAndUpdate() override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    void            createPoints();
    void            attachObservedPressure();
    void            updatePlotTitle();
    void            updateValueRanges();
    void            cleanupBeforeClose();
    RimSummaryCase* findClosestCase( const QPoint& canvasPos );

    std::vector<RimRftCrossPlotTools::DepthInterval> depthIntervals() const;

private:
    caf::PdmPtrField<RimSummaryEnsemble*>                    m_ensemble;
    caf::PdmField<QString>                                   m_wellName;
    caf::PdmField<QDateTime>                                 m_selectedTimeStep;
    caf::PdmPtrField<RimEclipseResultCase*>                  m_eclipseCase;
    caf::PdmField<RimRftCrossPlotTools::DepthFilterModeEnum> m_filterMode;
    caf::PdmField<double>                                    m_depthRangeMin;
    caf::PdmField<double>                                    m_depthRangeMax;
    caf::PdmPtrField<RimWellFormationsFile*>                 m_wellFormations;
    caf::PdmField<std::vector<QString>>                      m_selectedZones;
    std::map<QString, QColor>                                m_zoneColors;
    caf::PdmField<caf::AppEnum<RiaDefines::DepthType>>       m_depthType;
    caf::PdmField<SamplingModeEnum>                          m_samplingMode;
    caf::PdmField<QString>                                   m_ensembleParameter;

    caf::PdmField<bool>    m_useAutoPlotTitle;
    caf::PdmField<QString> m_description;

    RimFontSizeField m_axisTitleFontSize;
    RimFontSizeField m_axisValueFontSize;

    QPointer<RiuQwtPlotWidget> m_plotWidget;

    std::optional<std::pair<double, double>> m_xValueRange;
    std::optional<std::pair<double, double>> m_yValueRange;
};
