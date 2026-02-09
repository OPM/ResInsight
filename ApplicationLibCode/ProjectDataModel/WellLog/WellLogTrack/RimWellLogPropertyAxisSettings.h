/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RimDepthTrackPlot.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"

//==================================================================================================
///
/// Settings class for property axis configuration in well log tracks
///
//==================================================================================================
class RimWellLogPropertyAxisSettings : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellLogPropertyAxisSettings();
    ~RimWellLogPropertyAxisSettings() override;

    // Axis visibility
    bool isEnabled() const;
    void setEnabled( bool enable );

    // Value range
    double visibleRangeMin() const;
    double visibleRangeMax() const;
    void   setVisibleRange( double minValue, double maxValue );

    // Auto scale
    bool isAutoScaleEnabled() const;
    void setAutoScaleEnabled( bool enabled );

    // Logarithmic scale
    bool isLogarithmicScaleEnabled() const;
    void setLogarithmicScaleEnabled( bool enabled );

    // Axis inversion
    bool isAxisInverted() const;
    void setAxisInverted( bool inverted );

    // Grid visibility
    RimDepthTrackPlot::AxisGridVisibility gridVisibility() const;
    void                                  setGridVisibility( RimDepthTrackPlot::AxisGridVisibility visibility );

    // Tick settings
    bool isMinAndMaxTicksOnly() const;
    void setMinAndMaxTicksOnly( bool enable );

    bool isExplicitTickIntervals() const;
    void setExplicitTickIntervals( bool enable );

    double majorTickInterval() const;
    double minorTickInterval() const;
    void   setTickIntervals( double majorInterval, double minorInterval );

    void uiOrdering( const QString& uiConfigName, caf::PdmUiOrdering& uiOrdering );

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    caf::PdmField<bool>                            m_isEnabled;
    caf::PdmField<double>                          m_visibleRangeMin;
    caf::PdmField<double>                          m_visibleRangeMax;
    caf::PdmField<bool>                            m_isAutoScaleEnabled;
    caf::PdmField<bool>                            m_isLogarithmicScaleEnabled;
    caf::PdmField<bool>                            m_isAxisInverted;
    caf::PdmField<RimDepthTrackPlot::AxisGridEnum> m_gridVisibility;
    caf::PdmField<bool>                            m_minAndMaxTicksOnly;
    caf::PdmField<bool>                            m_explicitTickIntervals;
    caf::PdmField<double>                          m_majorTickInterval;
    caf::PdmField<double>                          m_minorTickInterval;
};
