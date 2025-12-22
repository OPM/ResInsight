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

#include "RiaStimPlanModelDefines.h"

#include "RimElasticPropertyScaling.h"

#include "cafPdmObjectCollection.h"

//==================================================================================================
///
///
//==================================================================================================
class RimElasticPropertyScalingCollection : public caf::PdmObjectCollection<RimElasticPropertyScaling>
{
    CAF_PDM_HEADER_INIT;

public:
    RimElasticPropertyScalingCollection();

    caf::Signal<> changed;

    // Domain-specific methods
    double getScaling( const QString& formationName, const QString& faciesName, RiaDefines::CurveProperty property ) const;

protected:
    void initAfterRead() override;
    void onItemsChanged() override;

private:
    void connectSignals();
    void elasticPropertyScalingChanged( const caf::SignalEmitter* emitter );
};
