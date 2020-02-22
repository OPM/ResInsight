/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "cafPdmChildField.h"
#include "cafPdmObject.h"

class RimOilField;
class RimValveTemplateCollection;
class RimFractureTemplateCollection;

class RimCompletionTemplateCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimCompletionTemplateCollection();
    ~RimCompletionTemplateCollection() override;

    RimFractureTemplateCollection*       fractureTemplateCollection();
    const RimFractureTemplateCollection* fractureTemplateCollection() const;
    RimValveTemplateCollection*          valveTemplateCollection();
    const RimValveTemplateCollection*    valveTemplateCollection() const;
    void                                 setDefaultUnitSystemBasedOnLoadedCases();

private:
    friend class RimOilField;
    void setFractureTemplateCollection( RimFractureTemplateCollection* fractureTemplateCollection );

    caf::PdmChildField<RimFractureTemplateCollection*> m_fractureTemplates;
    caf::PdmChildField<RimValveTemplateCollection*>    m_valveTemplates;

protected:
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
};
