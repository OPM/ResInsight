/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RimSummaryEnsemble.h"

#include "cafPdmPtrField.h"

class RimEnsembleFileSet;

//==================================================================================================
//
//
//
//==================================================================================================
class RimSummaryFileSetEnsemble : public RimSummaryEnsemble
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryFileSetEnsemble();

    RimEnsembleFileSet* ensembleFileSet();
    void                setEnsembleFileSet( RimEnsembleFileSet* ensembleFileSet );
    void                updateName( const std::set<QString>& existingEnsembleNames ) override;

    void cleanupBeforeDelete() override;

    void reloadCases() override;

private:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          initAfterRead() override;
    void                          onLoadDataAndUpdate() override;

    void onFileSetChanged( const caf::SignalEmitter* emitter );
    void onFileSetNameChanged( const caf::SignalEmitter* emitter );

    void createSummaryCasesFromEnsembleFileSet( bool notifyChange );
    void connectSignals();

private:
    caf::PdmPtrField<RimEnsembleFileSet*> m_ensembleFileSet;
};
