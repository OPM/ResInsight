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

#include "RimSurfaceCollection.h"

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
class RimEnsembleSurface : public RimSurfaceCollection
{
    CAF_PDM_HEADER_INIT;

public:
    RimEnsembleSurface();

    void              addFileSurface( RimFileSurface* fileSurface );
    void              loadDataAndUpdate();
    const RigSurface* statisticsSurface() const;

    static QString ensembleSourceFileCollectionName();

    void loadData() override;

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

    RimSurfaceCollection*        sourceFileSurfaceCollection() const;
    std::vector<RimFileSurface*> sourceFileSurfaces() const;

private:
    caf::PdmPtrField<RimEnsembleCurveSet*> m_ensembleCurveSet;

    cvf::ref<RigSurface> m_statisticsSurface;
};
