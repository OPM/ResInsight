/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

//==================================================================================================
/// Read-only description of the node selected in the network graph, shown in the property editor
//==================================================================================================
class RimGroupNetworkNode : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimGroupNetworkNode();

    void setNodeName( const QString& name );
    void setDescription( const QString& nodeType, const QString& parent, const QString& keyword );
    void setValues( const QString& properties, const QString& values );
    void clear();

    QString nodeName() const;

private:
    caf::PdmFieldHandle* userDescriptionField() override;

private:
    caf::PdmField<QString> m_treeText;
    caf::PdmField<QString> m_name;
    caf::PdmField<QString> m_nodeType;
    caf::PdmField<QString> m_parent;
    caf::PdmField<QString> m_keyword;
    caf::PdmField<QString> m_properties;
    caf::PdmField<QString> m_values;
};
