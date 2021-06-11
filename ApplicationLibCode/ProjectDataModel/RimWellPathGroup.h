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

class RimWellPathValve;

#include <QString>

//==================================================================================================
///
///
//==================================================================================================
class RimWellPathGroup : public RimWellPath
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellPathGroup();

    QString name() const override;

    void                      addChildWellPath( RimWellPath* wellPath );
    std::vector<RimWellPath*> childWellPaths() const;
    size_t                    childWellpathCount() const;
    bool                      hasChildWellPath( RimWellPath* wellPath );
    void                      removeChildWellPath( RimWellPath* wellPath );
    void                      removeAllChildWellPaths();

    void    createWellPathGeometry();
    void    makeMoreLevelsIfNecessary();
    QString createGroupName() const;

    const RimWellPathValve* outletValve() const;

protected:
    void                 defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName ) override;
    caf::PdmFieldHandle* userDescriptionField() override;
    void                 initAfterRead() override;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    std::vector<const RigWellPath*> wellPathGeometries() const;

    void onChildNameChanged( const caf::SignalEmitter* emitter );

private:
    caf::PdmChildArrayField<RimWellPath*> m_childWellPaths;
    caf::PdmField<bool>                   m_addValveAtConnection;
    caf::PdmChildField<RimWellPathValve*> m_valve;
    caf::PdmProxyValueField<QString>      m_groupName;
};
