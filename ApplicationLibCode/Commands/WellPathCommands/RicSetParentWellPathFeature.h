/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "cafCmdFeature.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include <QList>

#include <vector>

namespace caf
{
class PdmOptionItemInfo;
}

class RimWellPath;

class RicSelectWellPathUi : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RicSelectWellPathUi();

    void setWellPaths( const std::vector<RimWellPath*>& wellPaths );
    void setSelectedWell( RimWellPath* selectedWell );

    RimWellPath* wellPath() const;

protected:
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

private:
    caf::PdmPtrField<RimWellPath*> m_selectedWellPath;
    std::vector<RimWellPath*>      m_wellPaths;
};

//==================================================================================================
///
//==================================================================================================
class RicSetParentWellPathFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

protected:
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;
};
