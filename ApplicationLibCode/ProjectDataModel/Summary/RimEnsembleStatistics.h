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

#include "cafPdmField.h"
#include "cafPdmFieldCvfColor.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"

class RimEnsembleCurveSetInterface;

//==================================================================================================
///
//==================================================================================================
class RimEnsembleStatistics : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimEnsembleStatistics( RimEnsembleCurveSetInterface* parentCurveSet = nullptr );

    bool isActive() const;
    void setShowStatisticsCurves( bool show );
    bool showStatisticsCurveLegends() const;
    bool hideEnsembleCurves() const;
    bool basedOnFilteredCases() const;
    bool showP10Curve() const;
    bool showP50Curve() const;
    bool showP90Curve() const;
    bool showMeanCurve() const;

    bool showCurveLabels() const;
    void enableCurveLabels( bool enable );

    void enableIncludeIncompleteCurves( bool enable );

    cvf::Color3f color() const { return m_color; }
    void         setColor( const cvf::Color3f& color );
    bool         customColor() const;

    bool includeIncompleteCurves() const;

    int crossPlotCurvesBinCount() const;
    int crossPlotRealizationCountThresholdPerBin() const;

    void disableP10Curve( bool disable );
    void disableP50Curve( bool disable );
    void disableP90Curve( bool disable );
    void disableMeanCurve( bool disable );

    void defaultUiOrdering( bool showCrossPlotGroup, caf::PdmUiOrdering& uiOrdering );

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    bool onShowEnsembleCurves() const;
    void onSetShowEnsembleCurves( const bool& enable );

private:
    RimEnsembleCurveSetInterface* m_parentCurveSet;

    caf::PdmField<bool>           m_active;
    caf::PdmField<bool>           m_showStatisticsCurveLegends;
    caf::PdmField<bool>           m_hideEnsembleCurves;
    caf::PdmProxyValueField<bool> m_showEnsembleCurves;
    caf::PdmField<bool>           m_basedOnFilteredCases;
    caf::PdmField<bool>           m_showP10Curve;
    caf::PdmField<bool>           m_showP50Curve;
    caf::PdmField<bool>           m_showP90Curve;
    caf::PdmField<bool>           m_showMeanCurve;
    caf::PdmField<bool>           m_showCurveLabels;
    caf::PdmField<bool>           m_includeIncompleteCurves;
    caf::PdmField<bool>           m_customColor;

    // Ensemble cross plot settings
    caf::PdmField<int> m_crossPlotCurvesBinCount;
    caf::PdmField<int> m_crossPlotCurvesStatisticsRealizationCountThresholdPerBin;

    caf::PdmField<QString> m_warningLabel;

    caf::PdmField<cvf::Color3f> m_color;
};
