/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

#include "RimCurveAppearanceDefines.h"
#include "RimEnsembleCurveSetColorManager.h"

#include "RiuPlotCurveSymbol.h"
#include "RiuQwtPlotCurveDefines.h"

#include "cafPdmObject.h"

#include "cvfColor3.h"

class RimRegularLegendConfig;
class RimSummaryEnsemble;
class RimSummaryCase;

//==================================================================================================
///
///
//==================================================================================================
class RimCurveSetAppearance : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    using ColorMode     = RimEnsembleCurveSetColorManager::ColorMode;
    using ColorModeEnum = RimEnsembleCurveSetColorManager::ColorModeEnum;
    using LineStyle     = caf::AppEnum<RiuQwtPlotCurveDefines::LineStyleEnum>;
    using PointSymbol   = caf::AppEnum<RiuPlotCurveSymbol::PointSymbolEnum>;

    caf::Signal<> appearanceChanged;

public:
    RimCurveSetAppearance();

    void setMainEnsembleColor( const cvf::Color3f& color );
    void setEnsemble( RimSummaryEnsemble* ensemble );

    void setShowLineStyleAndSymbols( bool showCustomAppearance );

    cvf::Color3f curveColor( const RimSummaryEnsemble* ensemble, const RimSummaryCase* summaryCase ) const;
    cvf::Color3f statisticsCurveColor() const;

    void                    setColorMode( ColorModeEnum colorMode );
    void                    setEnsembleParameter( const QString& ensembleParameter );
    RimRegularLegendConfig* legendConfig() const;

private:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

    void updateRealizationColor();
    void updateEnsembleParameterLegend( RimRegularLegendConfig* legendConfig ) const;

private:
    caf::PdmField<ColorModeEnum>                                             m_colorMode;
    caf::PdmField<cvf::Color3f>                                              m_mainEnsembleColor;
    caf::PdmField<cvf::Color3f>                                              m_colorForRealizations;
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

    caf::PdmPtrField<RimSummaryEnsemble*>       m_ensemble;
    caf::PdmChildField<RimRegularLegendConfig*> m_ensembleLegendConfig;

    bool m_showLineStyleAndSymbols = false;
};
