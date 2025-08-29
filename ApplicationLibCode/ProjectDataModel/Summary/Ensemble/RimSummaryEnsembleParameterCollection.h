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

#include "RimNamedObject.h"

#include "cafPdmChildArrayField.h"

#include <QString>
#include <string>
#include <vector>

class RimSummaryEnsembleParameter;

class RimSummaryEnsembleParameterCollection : public RimNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryEnsembleParameterCollection();
    ~RimSummaryEnsembleParameterCollection() override;

    void addParameter( QString name, bool checkDuplicates = true );
    bool hasParameter( const QString name ) const;

    void setEnsembleId( int ensembleId );
    int  ensembleId() const;

    void deleteChildren();

    bool isEmpty() const;

    void updateUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering ) const;

private:
    caf::PdmChildArrayField<RimSummaryEnsembleParameter*> m_parameters;

    caf::PdmField<int> m_ensembleId;
};
