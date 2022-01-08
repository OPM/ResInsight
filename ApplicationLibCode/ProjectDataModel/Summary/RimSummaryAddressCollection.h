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

#include "RimNamedObject.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"

#include <QString>

class RimSummaryAddress;

class RimSummaryAddressCollection : public RimNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryAddressCollection();
    ~RimSummaryAddressCollection() override;

    void addAddress( RimSummaryAddress* address );
    void addToSubfolder( RimSummaryAddress* address, QString foldername );

protected:
    void initAfterRead() override;

    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    caf::PdmChildArrayField<RimSummaryAddress*>           m_adresses;
    caf::PdmChildArrayField<RimSummaryAddressCollection*> m_subfolders;
};
