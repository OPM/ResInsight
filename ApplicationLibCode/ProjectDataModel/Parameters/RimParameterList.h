/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021 -    Equinor ASA
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

#include <QString>
#include <vector>

class RimListParameter;
class RimGenericParameter;

//==================================================================================================
///
///
//==================================================================================================
class RimParameterList : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimParameterList();
    ~RimParameterList() override;

    void addParameter( QString paramName );
    bool containsParameter( QString paramName );

    RimListParameter* getAsListParameter( std::vector<RimGenericParameter*> parameters );

    void setName( QString name );
    void setLabel( QString labelText );

    QString name() const;
    QString label() const;

private:
    QString parameterValue( QString paramName, std::vector<RimGenericParameter*>& parameters );

private:
    caf::PdmField<QString>              m_name;
    caf::PdmField<QString>              m_label;
    caf::PdmField<std::vector<QString>> m_parameterNames;
};
