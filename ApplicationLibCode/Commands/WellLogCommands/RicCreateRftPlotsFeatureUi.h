/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023 Equinor ASA
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
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include <QString>

#include <vector>

class RimSummaryCaseCollection;

//==================================================================================================
///
//==================================================================================================
class RicCreateRftPlotsFeatureUi : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RicCreateRftPlotsFeatureUi();

    void                 setDefaultEnsemble( RimSummaryCaseCollection* ensemble );
    std::vector<QString> wellNames() const;

protected:
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

private:
    caf::PdmPtrField<RimSummaryCaseCollection*> m_caseCollection;
    caf::PdmField<std::vector<QString>>         m_selectedWellNames;
};
