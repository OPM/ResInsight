/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RimPlotCurve.h"

class RimStackablePlotCurve : public RimPlotCurve
{
public:
    caf::Signal<bool> stackingChanged;
    caf::Signal<bool> stackingColorsChanged;

public:
    RimStackablePlotCurve();
    ~RimStackablePlotCurve() override;

    virtual RiaDefines::PhaseType phaseType() const;
    void                          assignStackColor( size_t index, size_t count );
    bool                          isStacked() const;
    bool                          isStackedWithPhaseColors() const;
    void                          setIsStacked( bool stacked );
    void                          updateCurveAppearance() override;

    void defaultUiOrdering( caf::PdmUiOrdering& uiOrdering );
    void stackingUiOrdering( caf::PdmUiOrdering& uiOrdering );

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    void onFillColorChanged( const caf::SignalEmitter* emitter ) override;
    void updateStackingAppearance();

protected:
    caf::PdmField<bool> m_isStacked;

private:
    caf::PdmField<bool> m_isStackedWithPhaseColors;
};
