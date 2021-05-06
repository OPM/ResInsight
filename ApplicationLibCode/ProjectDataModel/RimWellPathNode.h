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

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

namespace caf
{
class PdmUiOrdering;
class PdmUiTreeOrdering;
} // namespace caf

class QString;

//==================================================================================================
///
///
//==================================================================================================
class RimWellPathNode : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellPathNode();

    void setReferencedObject( caf::PdmObject* object );
    void addChild( RimWellPathNode* object );

    caf::PdmObject*               referencedObject();
    std::vector<RimWellPathNode*> childNodes() const;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

private:
    caf::PdmPtrField<caf::PdmObject*>         m_referencedObject;
    caf::PdmChildArrayField<RimWellPathNode*> m_childNodes;
};
