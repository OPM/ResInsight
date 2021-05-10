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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class cafTreeNode : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    cafTreeNode();

    void                      addChild( cafTreeNode* treeNode );
    std::vector<cafTreeNode*> childNodes() const;

    virtual caf::PdmObject* referencedObject() const;

protected:
    caf::PdmChildArrayField<cafTreeNode*> m_childNodes;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class cafNamedTreeNode : public cafTreeNode
{
    CAF_PDM_HEADER_INIT;

public:
    cafNamedTreeNode();

    void    setName( const QString& name );
    QString name() const;

    void setIcon( const QString& iconResourceName );

    void setCheckedState( bool enable );
    bool isChecked() const;

protected:
    caf::PdmFieldHandle* objectToggleField() override;
    caf::PdmFieldHandle* userDescriptionField() override;

private:
    bool m_showCheckedBox;

    caf::PdmField<QString> m_name;
    caf::PdmField<bool>    m_isChecked;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class cafObjectReferenceTreeNode : public cafTreeNode
{
    CAF_PDM_HEADER_INIT;

public:
    cafObjectReferenceTreeNode();

    void            setReferencedObject( caf::PdmObject* object );
    caf::PdmObject* referencedObject() const override;

protected:
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

private:
    caf::PdmPtrField<caf::PdmObject*> m_referencedObject;
};
