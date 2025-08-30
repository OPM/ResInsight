/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025  Equinor ASA
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

#include <QString>

//==================================================================================================
//
//
//==================================================================================================

class RimSummaryEnsembleParameter : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryEnsembleParameter();
    ~RimSummaryEnsembleParameter() override;

    QString name() const;
    void    setName( const QString& name );

    void setEnsembleId( int ensembleId );
    int  ensembleId() const;

private:
    caf::PdmField<QString> m_name;
    caf::PdmField<int>     m_ensembleId;
};
