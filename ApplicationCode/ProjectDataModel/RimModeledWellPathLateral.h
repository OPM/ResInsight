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

#include "RimWellPath.h"

#include "cafPdmChildField.h"
#include "cafPdmProxyValueField.h"

class RimWellPathTarget;
class RimWellPath;
class RimWellPathGroup;
class RimWellPathLateralGeometryDef;

class RimModeledWellPathLateral : public RimWellPath
{
    CAF_PDM_HEADER_INIT;

public:
    RimModeledWellPathLateral();
    ~RimModeledWellPathLateral() override;

    void                           createWellPathGeometry();
    void                           updateWellPathVisualization();
    void                           scheduleUpdateOfDependentVisualization();
    RimWellPathLateralGeometryDef* geometryDefinition() const;
    QString                        wellPlanText();

private:
    const RimWellPathGroup* parentGroup() const;

    void                 defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName ) override;
    void                 defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                 onGeometryDefinitionChanged( const caf::SignalEmitter* emitter, bool fullUpdate );
    caf::PdmFieldHandle* userDescriptionField() override;
    QString              createName() const;

    caf::PdmChildField<RimWellPathLateralGeometryDef*> m_geometryDefinition;
    caf::PdmProxyValueField<QString>                   m_lateralName;
};
