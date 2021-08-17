/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RimNamedObject.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmPtrField.h"

#include "cvfObject.h"

class RimFileSurface;
class RimSurface;
class RigSurface;
class RimEnsembleStatisticsSurface;
class RimEnsembleCurveSet;
class RimSummaryCase;

//==================================================================================================
///
//==================================================================================================
class RimEnsembleSurface : public RimNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimEnsembleSurface();
    void removeFileSurface( RimFileSurface* fileSurface );
    void addFileSurface( RimFileSurface* fileSurface );

    std::vector<RimFileSurface*> fileSurfaces() const;

    std::vector<RimSurface*> surfaces() const;

    void loadDataAndUpdate();

    const RigSurface* statisticsSurface() const;

protected:
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void initAfterRead() override;

    std::vector<RimFileSurface*> filterByEnsembleCurveSet( const std::vector<RimFileSurface*>& fileSurfaces ) const;

    bool isSameRealization( RimSummaryCase* summaryCase, RimFileSurface* fileSurface ) const;

private:
    void connectEnsembleCurveSetFilterSignals();
    void onFilterSourceChanged( const caf::SignalEmitter* emitter );

    caf::PdmChildArrayField<RimFileSurface*>               m_fileSurfaces;
    caf::PdmChildArrayField<RimEnsembleStatisticsSurface*> m_statisticsSurfaces;
    caf::PdmPtrField<RimEnsembleCurveSet*>                 m_ensembleCurveSet;

    cvf::ref<RigSurface> m_statisticsSurface;
};
