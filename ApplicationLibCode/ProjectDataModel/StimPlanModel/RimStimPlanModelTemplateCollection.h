/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

class RimStimPlanModelTemplate;

//==================================================================================================
///
///
//==================================================================================================
class RimStimPlanModelTemplateCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimStimPlanModelTemplateCollection();
    ~RimStimPlanModelTemplateCollection() override;

    RimStimPlanModelTemplate*              stimPlanModelTemplate( int id ) const;
    std::vector<RimStimPlanModelTemplate*> stimPlanModelTemplates() const;
    void                                   addStimPlanModelTemplate( RimStimPlanModelTemplate* templ );

    void loadAndUpdateData();

    void onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                         std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

protected:
    void initAfterRead() override;

private:
    int nextFractureTemplateId();

    caf::PdmChildArrayField<RimStimPlanModelTemplate*> m_stimPlanModelTemplates;
    caf::PdmField<int>                                 m_nextValidId; // Unique fracture template ID within a project
};
