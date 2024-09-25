/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include <QList>
#include <QString>
#include <vector>

class RimSummaryCase;
class RimSummaryEnsemble;

//==================================================================================================
///
//==================================================================================================
class RicSelectCaseOrEnsembleUi : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RicSelectCaseOrEnsembleUi();

    void setEnsembleSelectionMode( bool selectEnsemble );

    RimSummaryCase*     selectedSummaryCase() const;
    RimSummaryEnsemble* selectedEnsemble() const;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

private:
    caf::PdmPtrField<RimSummaryCase*>     m_selectedSummaryCase;
    caf::PdmPtrField<RimSummaryEnsemble*> m_selectedEnsemble;

    bool m_useEnsembleMode;
};
