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

#include "RimWorkflowFieldBinding.h"

#include "cafPdmPtrArrayField.h"

class RimEclipseCase;
class RimEclipseView;
class RimWellPath;

//==================================================================================================
/// The `dict[str, Value]` a mapped task runs over. ResInsight objects are picked from the project
/// and keyed by name; other values are entered as a JSON object.
//==================================================================================================
class RimWorkflowMappingBinding : public RimWorkflowFieldBinding
{
    CAF_PDM_HEADER_INIT;

public:
    RimWorkflowMappingBinding();

    void       applySchema( const QJsonObject& fieldSchema ) override;
    QJsonValue toJsonValue() const override;
    bool       isObjectReference() const override;
    QString    displayValue() const override;

    caf::PdmFieldHandle* valueField() override;

private:
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    std::vector<std::pair<QString, QJsonObject>> references() const;

    caf::PdmField<QString>                 m_valueType;
    caf::PdmField<QString>                 m_text;
    caf::PdmPtrArrayField<RimEclipseCase*> m_cases;
    caf::PdmPtrArrayField<RimWellPath*>    m_wellPaths;
    caf::PdmPtrArrayField<RimEclipseView*> m_views;
};
