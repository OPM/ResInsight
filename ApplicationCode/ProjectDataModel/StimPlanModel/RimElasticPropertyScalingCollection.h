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

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "RiaFractureModelDefines.h"

class RimElasticPropertyScaling;

//==================================================================================================
///
///
//==================================================================================================
class RimElasticPropertyScalingCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimElasticPropertyScalingCollection();
    ~RimElasticPropertyScalingCollection() override;

    caf::Signal<> changed;

    std::vector<RimElasticPropertyScaling*> elasticPropertyScalings() const;
    void                                    addElasticPropertyScaling( RimElasticPropertyScaling* templ );

    void onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                         std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

    double getScaling( const QString& formationName, const QString& faciesName, RiaDefines::CurveProperty property ) const;

private:
    void elasticPropertyScalingChanged( const caf::SignalEmitter* emitter );
    caf::PdmChildArrayField<RimElasticPropertyScaling*> m_elasticPropertyScalings;
};
